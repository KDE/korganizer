/*
  This file is part of KOrganizer.
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>
  SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "element.h"
#include "picoftheday.h"

#include "korganizer_picoftheday_plugin_debug.h"

#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrlQuery>

#include <chrono>

using namespace std::chrono_literals;

constexpr auto updateDelay = 1s;

void ElementData::updateFetchedThumbSize()
{
    int thumbWidth = mThumbSize.width();
    int thumbHeight = static_cast<int>(thumbWidth * mPictureHWRatio);
    if (mThumbSize.height() < thumbHeight) {
        /* if the requested height is less than the requested width * ratio
           we would download too much, as the downloaded picture would be
           taller than requested, so we adjust the width of the picture to
           be downloaded in consequence */
        thumbWidth /= (thumbHeight / static_cast<float>(mThumbSize.height()));
        thumbHeight = static_cast<int>(thumbWidth * mPictureHWRatio);
    }
    mFetchedThumbSize = QSize(thumbWidth, thumbHeight);
}

POTDElement::POTDElement(const QString &id, QDate date, ElementData *data)
    : Element(id)
    , mDate(date)
    , mData(data)
    , mThumbImageGetDelayTimer(new QTimer(this))
{
    mThumbImageGetDelayTimer->setSingleShot(true);
    mThumbImageGetDelayTimer->setInterval(updateDelay);
    connect(mThumbImageGetDelayTimer, &QTimer::timeout, this, &POTDElement::queryThumbImageInfoJson);

    // wait a bit to avoid data queries in case of quick paging through views
    QTimer::singleShot(updateDelay, this, &POTDElement::completeMissingData);
}

POTDElement::~POTDElement()
{
    // reset thumb update state
    if (mData->mState > DataLoaded) {
        mData->mState = DataLoaded;
    }
    Picoftheday::cacheData(mDate, mData);
}

void POTDElement::completeMissingData()
{
    if (mData->mState <= NeedingPageData) {
        queryImagesJson();
    } else if (mData->mState <= NeedingBasicImageInfo) {
        queryBasicImageInfoJson();
    } else if (mData->mState <= NeedingFirstThumbImage) {
        queryThumbImageInfoJson();
    }
}

KIO::SimpleJob *POTDElement::createJsonQueryJob(const QString &property, const QString &title, const QList<QueryItem> &otherQueryItems)
{
    QUrl url(QStringLiteral("https://en.wikipedia.org/w/api.php"));

    QUrlQuery urlQuery{
        {QStringLiteral("action"), QStringLiteral("query")},
        {QStringLiteral("format"), QStringLiteral("json")},
        {QStringLiteral("prop"), property},
        {QStringLiteral("titles"), title},
    };
    for (const auto &item : otherQueryItems) {
        urlQuery.addQueryItem(item.key, item.value);
    }
    url.setQuery(urlQuery);

    return KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
}

KIO::SimpleJob *POTDElement::createImagesJsonQueryJob(PageProtectionState state)
{
    const char *const templatePagePrefix = (state == ProtectedPage) ? "Template:POTD_protected/" : "Template:POTD/";
    const QString templatePageName = QLatin1StringView(templatePagePrefix) + mDate.toString(Qt::ISODate);
    const QList<QueryItem> otherQueryItems{
        // TODO: unsure if formatversion is needed, used by https://www.mediawiki.org/wiki/API:Picture_of_the_day_viewer in October 2021
        {QStringLiteral("formatversion"), QStringLiteral("2")},
    };

    return createJsonQueryJob(QStringLiteral("images"), templatePageName, otherQueryItems);
}

void POTDElement::queryImagesJson()
{
    auto queryImagesJob = createImagesJsonQueryJob(ProtectedPage);

    connect(queryImagesJob, &KIO::SimpleJob::result, this, &POTDElement::handleProtectedImagesJsonResponse);
}

void POTDElement::handleImagesJsonResponse(KJob *job, PageProtectionState pageProtectionState)
{
    if (job->error()) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": could not get POTD file name:" << job->errorString();
        setLoadingFailed();
        return;
    }

    auto const transferJob = static_cast<KIO::StoredTransferJob *>(job);

    const auto json = QJsonDocument::fromJson(transferJob->data());

    const auto pageObject = json.object().value(QLatin1StringView("query")).toObject().value(QLatin1StringView("pages")).toArray().at(0).toObject();

    auto missingIt = pageObject.find(QLatin1StringView("missing"));
    if ((missingIt != pageObject.end()) && missingIt.value().toBool(false)) {
        // fallback to unprotected variant in case there is no protected variant
        if (pageProtectionState == ProtectedPage) {
            qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": protected page reported as missing, trying unprocteded now.";
            auto queryImagesJob = createImagesJsonQueryJob(UnprotectedPage);

            connect(queryImagesJob, &KIO::SimpleJob::result, this, &POTDElement::handleUnprotectedImagesJsonResponse);
            return;
        }

        // no POTD set
        qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": also unprotected page reported as missing, Seems no POTD is declared.";
        setLoadingFailed();
        return;
    }

    const auto imageObject = pageObject.value(QLatin1StringView("images")).toArray().at(0).toObject();
    const QString imageFile = imageObject.value(QLatin1StringView("title")).toString();
    if (imageFile.isEmpty()) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": missing images data in reply:" << json;
        setLoadingFailed();
        return;
    }

    // store data
    mData->mPictureName = imageFile;
    mData->mState = NeedingBasicImageInfo;

    queryBasicImageInfoJson();
}

void POTDElement::handleUnprotectedImagesJsonResponse(KJob *job)
{
    handleImagesJsonResponse(job, UnprotectedPage);
}

void POTDElement::handleProtectedImagesJsonResponse(KJob *job)
{
    handleImagesJsonResponse(job, ProtectedPage);
}

void POTDElement::queryBasicImageInfoJson()
{
    const QList<QueryItem> otherQueryItems{
        {QStringLiteral("iiprop"), QStringLiteral("url|size|canonicaltitle")},
    };
    auto queryBasicImageInfoJob = createJsonQueryJob(QStringLiteral("imageinfo"), mData->mPictureName, otherQueryItems);

    connect(queryBasicImageInfoJob, &KIO::SimpleJob::result, this, &POTDElement::handleBasicImageInfoJsonResponse);
}

void POTDElement::handleBasicImageInfoJsonResponse(KJob *job)
{
    if (job->error()) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": could not get POTD file name:" << job->errorString();
        setLoadingFailed();
        return;
    }

    auto const transferJob = static_cast<KIO::StoredTransferJob *>(job);

    const auto json = QJsonDocument::fromJson(transferJob->data());

    const auto pagesObject = json.object().value(QLatin1StringView("query")).toObject().value(QLatin1StringView("pages")).toObject();
    const auto pageObject = pagesObject.isEmpty() ? QJsonObject() : pagesObject.begin()->toObject();
    const auto imageInfo = pageObject.value(QLatin1StringView("imageinfo")).toArray().at(0).toObject();

    const QString url = imageInfo.value(QLatin1StringView("url")).toString();
    if (url.isEmpty()) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": missing imageinfo data in reply:" << json;
        setLoadingFailed();
        return;
    }

    const QString descriptionUrl = imageInfo.value(QLatin1StringView("descriptionurl")).toString();
    mData->mAboutPageUrl = QUrl(descriptionUrl);

    const QString description = imageInfo.value(QLatin1StringView("canonicaltitle")).toString();
    mData->mTitle = i18n("Wikipedia POTD: %1", description);

    const int width = imageInfo.value(QLatin1StringView("width")).toInt();
    const int height = imageInfo.value(QLatin1StringView("height")).toInt();
    mData->mPictureHWRatio = ((width != 0) && (height != 0)) ? height / static_cast<float>(width) : 1.0;
    qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": thumb width" << width << " thumb height" << height << "ratio" << mData->mPictureHWRatio;
    mData->updateFetchedThumbSize();
    mData->mState = NeedingFirstThumbImageInfo;

    queryThumbImageInfoJson();
}

void POTDElement::queryThumbImageInfoJson()
{
    qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": thumb size" << mData->mThumbSize << " adapted size" << mData->mFetchedThumbSize;

    const QList<QueryItem> otherQueryItems{
        {QStringLiteral("iiprop"), QStringLiteral("url")},
        {QStringLiteral("iiurlwidth"), QString::number(mData->mFetchedThumbSize.width())},
        {QStringLiteral("iiurlheight"), QString::number(mData->mFetchedThumbSize.height())},
    };
    mQueryThumbImageInfoJob = createJsonQueryJob(QStringLiteral("imageinfo"), mData->mPictureName, otherQueryItems);

    connect(mQueryThumbImageInfoJob, &KIO::SimpleJob::result, this, &POTDElement::handleThumbImageInfoJsonResponse);
}

void POTDElement::handleThumbImageInfoJsonResponse(KJob *job)
{
    mQueryThumbImageInfoJob = nullptr;

    if (job->error()) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": could not get thumb info:" << job->errorString();
        if (mData->mState == NeedingFirstThumbImageInfo) {
            setLoadingFailed();
        }
        return;
    }

    auto const transferJob = static_cast<KIO::StoredTransferJob *>(job);

    const auto json = QJsonDocument::fromJson(transferJob->data());
    auto pagesObject = json.object().value(QLatin1StringView("query")).toObject().value(QLatin1StringView("pages")).toObject();
    auto pageObject = pagesObject.isEmpty() ? QJsonObject() : pagesObject.begin()->toObject();
    auto imageInfo = pageObject.value(QLatin1StringView("imageinfo")).toArray().at(0).toObject();

    const QString thumbUrl = imageInfo.value(QStringLiteral("thumburl")).toString();
    if (thumbUrl.isEmpty()) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": missing imageinfo data in reply:" << json;
        return;
    }

    mData->mState = (mData->mState == NeedingFirstThumbImageInfo) ? NeedingFirstThumbImage : NeedingNextThumbImage;

    getThumbImage(QUrl(thumbUrl));
}

void POTDElement::getThumbImage(const QUrl &thumbUrl)
{
    if (mGetThumbImageJob) {
        mGetThumbImageJob->kill();
    }

    qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": fetching POTD thumbnail:" << thumbUrl;

    mGetThumbImageJob = KIO::storedGet(thumbUrl, KIO::NoReload, KIO::HideProgressInfo);

    connect(mGetThumbImageJob, &KIO::SimpleJob::result, this, &POTDElement::handleGetThumbImageResponse);
}

void POTDElement::handleGetThumbImageResponse(KJob *job)
{
    mGetThumbImageJob = nullptr;

    const bool isAboutFirstThumbImage = (mData->mState == NeedingFirstThumbImage);

    if (job->error()) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": could not get POTD thumb:" << job->errorString();
        if (isAboutFirstThumbImage) {
            setLoadingFailed();
        }
        return;
    }

    // Last step completed: we get the pixmap from the transfer job's data
    auto const transferJob = static_cast<KIO::StoredTransferJob *>(job);
    if (!mData->mThumbnail.loadFromData(transferJob->data())) {
        qCWarning(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": could not load POTD thumb data.";
        if (isAboutFirstThumbImage) {
            setLoadingFailed();
        }
        return;
    }

    mData->mState = DataLoaded;

    if (isAboutFirstThumbImage) {
        // update other properties
        Q_EMIT gotNewShortText(shortText());
        Q_EMIT gotNewLongText(mData->mTitle);
        Q_EMIT gotNewUrl(mData->mAboutPageUrl);
    }

    if (!mRequestedThumbSize.isNull()) {
        Q_EMIT gotNewPixmap(mData->mThumbnail.scaled(mRequestedThumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void POTDElement::setLoadingFailed()
{
    mData->mState = LoadingFailed;

    Q_EMIT gotNewShortText(QString());
    Q_EMIT gotNewLongText(QString());
}

QString POTDElement::shortText() const
{
    return (mData->mState >= DataLoaded) ? i18n("Picture Page") : (mData->mState >= NeedingPageData) ? i18n("Loading...") : QString();
}

QString POTDElement::longText() const
{
    return (mData->mState >= DataLoaded)     ? mData->mTitle
        : (mData->mState >= NeedingPageData) ? i18n("<qt>Loading <i>Picture of the Day</i>...</qt>")
                                             : QString();
}

QUrl POTDElement::url() const
{
    return (mData->mState >= DataLoaded) ? mData->mAboutPageUrl : QUrl();
}

QPixmap POTDElement::newPixmap(const QSize &size)
{
    mRequestedThumbSize = size;

    if ((mData->mThumbSize.width() < size.width()) || (mData->mThumbSize.height() < size.height())) {
        qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << mDate << ": called for a new pixmap size (" << size << "instead of" << mData->mThumbSize
                                                 << ", stored pixmap:" << mData->mThumbnail.size() << ")";
        mData->mThumbSize = size;

        if (mData->mState >= NeedingFirstThumbImageInfo) {
            mData->updateFetchedThumbSize();

            if ((mData->mFetchedThumbSize.width() < size.width()) || (mData->mFetchedThumbSize.height() < size.height())) {
                // only if there is already an initial pixmap to show at least something,
                // kill current update and trigger new delayed update
                if (mData->mState >= DataLoaded) {
                    if (mQueryThumbImageInfoJob) {
                        mQueryThumbImageInfoJob->kill();
                        mQueryThumbImageInfoJob = nullptr;
                    }
                    if (mGetThumbImageJob) {
                        mGetThumbImageJob->kill();
                        mGetThumbImageJob = nullptr;
                    }
                    mData->mState = NeedingNextThumbImageInfo;
                }

                // We start a new thumbnail download a little later; the following code
                // is to avoid too frequent transfers e.g. when resizing
                mThumbImageGetDelayTimer->start();
            }
        }
    }

    /* else, either we already got a sufficiently big pixmap (stored in mData->mThumbnail),
       or we will get one anytime soon (we are downloading it already) and we will
       actualize what we return here later via gotNewPixmap */
    if (mData->mThumbnail.isNull()) {
        return {};
    }
    return mData->mThumbnail.scaled(mRequestedThumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

#include "moc_element.cpp"
