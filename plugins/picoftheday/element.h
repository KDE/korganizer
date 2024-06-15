/*
  This file is part of KOrganizer.
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>
  SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <EventViews/CalendarDecoration>
using namespace EventViews::CalendarDecoration;

#include <KIO/SimpleJob>

#include <QUrl>

enum DataState {
    LoadingFailed = -1,
    NeedingPageData = 0,
    NeedingBasicImageInfo,
    NeedingFirstThumbImageInfo,
    NeedingFirstThumbImage,
    DataLoaded,
    NeedingNextThumbImageInfo,
    NeedingNextThumbImage,
};

struct ElementData {
    float mPictureHWRatio = 1;
    QString mPictureName;
    QUrl mAboutPageUrl;
    QSize mThumbSize;
    QSize mFetchedThumbSize;
    QPixmap mThumbnail;
    QString mTitle;

    DataState mState = NeedingPageData;

    void updateFetchedThumbSize();
};

class POTDElement : public Element
{
    Q_OBJECT

public:
    POTDElement(const QString &id, QDate date, ElementData *data);
    ~POTDElement() override;

public: // Element API
    [[nodiscard]] QString shortText() const override;
    [[nodiscard]] QString longText() const override;
    [[nodiscard]] QUrl url() const override;
    [[nodiscard]] QPixmap newPixmap(const QSize &size) override;

private:
    void queryImagesJson();
    void queryBasicImageInfoJson();
    void queryThumbImageInfoJson();
    void getThumbImage(const QUrl &thumbUrl);

    // POTD pages once decided about should get an edit-protected variant, but not all have that
    enum PageProtectionState { ProtectedPage, UnprotectedPage };
    KIO::SimpleJob *createImagesJsonQueryJob(PageProtectionState pageProtectionState);

    struct QueryItem {
        QString key;
        QString value;
    };
    KIO::SimpleJob *createJsonQueryJob(const QString &property, const QString &title, const QList<QueryItem> &otherQueryItems = {});

    void handleImagesJsonResponse(KJob *job, PageProtectionState pageProtectionState);

    void setLoadingFailed();

private Q_SLOTS:
    void handleProtectedImagesJsonResponse(KJob *job);
    void handleUnprotectedImagesJsonResponse(KJob *job);
    void handleBasicImageInfoJsonResponse(KJob *job);
    void handleThumbImageInfoJsonResponse(KJob *job);
    void handleGetThumbImageResponse(KJob *job);
    void completeMissingData();

private:
    const QDate mDate;
    QSize mRequestedThumbSize;

    ElementData *const mData;

    QTimer *const mThumbImageGetDelayTimer;
    KIO::SimpleJob *mQueryThumbImageInfoJob = nullptr;
    KIO::SimpleJob *mGetThumbImageJob = nullptr;
};
