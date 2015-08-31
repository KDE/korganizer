/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004,2009 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "sdsummarywidget.h"
#include "korganizer_kontactplugins_specialdates_debug.h"
#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

#include <calendarsupport/utils.h>
#include <calendarsupport/calendarsingleton.h>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/SearchQuery>
#include <AkonadiCore/EntityDisplayAttribute>
#include <Akonadi/Contact/ContactSearchJob>
#include <Akonadi/Contact/ContactViewerDialog>

#include <KCalCore/Calendar>

#include <QMenu>
#include <KLocalizedString>
#include <KLocale>
#include <KUrlLabel>
#include <KIconLoader>
#include <KConfigGroup>
#include <QDesktopServices>
#include <KHolidays/HolidayRegion>

#include <QDate>
#include <QEvent>
#include <QLabel>
#include <QGridLayout>

using namespace KHolidays;

class BirthdaySearchJob : public Akonadi::ItemSearchJob
{
public:
    BirthdaySearchJob(QObject *parent, int daysInAdvance);
};

BirthdaySearchJob::BirthdaySearchJob(QObject *parent, int daysInAdvance)
    : ItemSearchJob(parent)
{
    fetchScope().fetchFullPayload();
    setMimeTypes(QStringList() << KContacts::Addressee::mimeType());

    Akonadi::SearchQuery query;
    query.addTerm(QStringLiteral("birthday"), QDate::currentDate().toJulianDay(),
                  Akonadi::SearchTerm::CondGreaterOrEqual);
    query.addTerm(QStringLiteral("birthday"), QDate::currentDate().addDays(daysInAdvance).toJulianDay(),
                  Akonadi::SearchTerm::CondLessOrEqual);

    ItemSearchJob::setQuery(query);
}

enum SDIncidenceType {
    IncidenceTypeContact,
    IncidenceTypeEvent
};

enum SDCategory {
    CategoryBirthday,
    CategoryAnniversary,
    CategoryHoliday,
    CategoryOther
};

class SDEntry
{
public:
    SDIncidenceType type;
    SDCategory category;
    int yearsOld;
    int daysTo;
    QDate date;
    QString summary;
    QString desc;
    int span; // #days in the special occassion.
    KContacts::Addressee addressee;
    Akonadi::Item item;

    bool operator<(const SDEntry &entry) const
    {
        return daysTo < entry.daysTo;
    }
};

SDSummaryWidget::SDSummaryWidget(KontactInterface::Plugin *plugin, QWidget *parent)
    : KontactInterface::Summary(parent), mPlugin(plugin), mHolidays(Q_NULLPTR)
{
    mCalendar = CalendarSupport::calendarSingleton();
    // Create the Summary Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(3);
    mainLayout->setMargin(3);

    //TODO: we want our own special dates icon
    QWidget *header = createHeader(
                          this, QStringLiteral("favorites"), i18n("Upcoming Special Dates"));
    mainLayout->addWidget(header);

    mLayout = new QGridLayout();
    mainLayout->addItem(mLayout);
    mLayout->setSpacing(3);
    mLayout->setRowStretch(6, 1);

    // Default settings
    mDaysAhead = 7;
    mShowBirthdaysFromKAB = true;
    mShowBirthdaysFromCal = true;
    mShowAnniversariesFromKAB = true;
    mShowAnniversariesFromCal = true;
    mShowHolidays = true;
    mJobRunning = false;
    mShowSpecialsFromCal = true;

    // Setup the Addressbook
    connect(mPlugin->core(), SIGNAL(dayChanged(QDate)),
            this, SLOT(updateView()));

    connect(mCalendar.data(), SIGNAL(calendarChanged()),
            this, SLOT(updateView()));
    connect(mPlugin->core(), SIGNAL(dayChanged(QDate)),
            this, SLOT(updateView()));

    // Update Configuration
    configUpdated();
}

SDSummaryWidget::~SDSummaryWidget()
{
    delete mHolidays;
}

void SDSummaryWidget::configUpdated()
{
    KConfig config(QStringLiteral("kcmsdsummaryrc"));

    KConfigGroup group = config.group("Days");
    mDaysAhead = group.readEntry("DaysToShow", 7);

    group = config.group("Show");
    mShowBirthdaysFromKAB = group.readEntry("BirthdaysFromContacts", true);
    mShowBirthdaysFromCal = group.readEntry("BirthdaysFromCalendar", true);

    mShowAnniversariesFromKAB = group.readEntry("AnniversariesFromContacts", true);
    mShowAnniversariesFromCal = group.readEntry("AnniversariesFromCalendar", true);

    mShowHolidays = group.readEntry("HolidaysFromCalendar", true);

    mShowSpecialsFromCal = group.readEntry("SpecialsFromCalendar", true);

    group = config.group("Groupware");
    mShowMineOnly = group.readEntry("ShowMineOnly", false);

    updateView();
}

bool SDSummaryWidget::initHolidays()
{
    KConfig _hconfig(QStringLiteral("korganizerrc"));
    KConfigGroup hconfig(&_hconfig, "Time & Date");
    QString location = hconfig.readEntry("Holidays");
    if (!location.isEmpty()) {
        delete mHolidays;
        mHolidays = new HolidayRegion(location);
        return true;
    }
    return false;
}

// number of days remaining in an Event
int SDSummaryWidget::span(const KCalCore::Event::Ptr &event) const
{
    int span = 1;
    if (event->isMultiDay() && event->allDay()) {
        QDate d = event->dtStart().date();
        if (d < QDate::currentDate()) {
            d = QDate::currentDate();
        }
        while (d < event->dtEnd().date()) {
            span++;
            d = d.addDays(1);
        }
    }
    return span;
}

// day of a multiday Event
int SDSummaryWidget::dayof(const KCalCore::Event::Ptr &event, const QDate &date) const
{
    int dayof = 1;
    QDate d = event->dtStart().date();
    if (d < QDate::currentDate()) {
        d = QDate::currentDate();
    }
    while (d < event->dtEnd().date()) {
        if (d < date) {
            dayof++;
        }
        d = d.addDays(1);
    }
    return dayof;
}

void SDSummaryWidget::slotBirthdayJobFinished(KJob *job)
{
    // ;)
    BirthdaySearchJob *bJob = dynamic_cast< BirthdaySearchJob *>(job);
    if (bJob) {
        foreach (const Akonadi::Item &item, bJob->items()) {
            if (item.hasPayload<KContacts::Addressee>()) {
                const KContacts::Addressee addressee = item.payload<KContacts::Addressee>();
                const QDate birthday = addressee.birthday().date();
                if (birthday.isValid()) {
                    SDEntry entry;
                    entry.type = IncidenceTypeContact;
                    entry.category = CategoryBirthday;
                    dateDiff(birthday, entry.daysTo, entry.yearsOld);

                    entry.date = birthday;
                    entry.addressee = addressee;
                    entry.item = item;
                    entry.span = 1;
                    mDates.append(entry);
                }
            }
        }
        // Carry on.
        createLabels();
    }

    mJobRunning = false;
}

void SDSummaryWidget::createLabels()
{
    KIconLoader loader(QStringLiteral("kdepim"));

    QLabel *label = Q_NULLPTR;

    // Remove all special date labels from the layout and delete them, as we
    // will re-create all labels below.
    setUpdatesEnabled(false);
    foreach (label, mLabels) {
        mLayout->removeWidget(label);
        delete(label);
        update();
    }
    mLabels.clear();

    QDate dt;
    for (dt = QDate::currentDate();
            dt <= QDate::currentDate().addDays(mDaysAhead - 1);
            dt = dt.addDays(1)) {
        KCalCore::Event::List events  = mCalendar->events(dt, mCalendar->timeSpec(),
                                        KCalCore::EventSortStartDate,
                                        KCalCore::SortDirectionAscending);
        foreach (const KCalCore::Event::Ptr &ev, events) {
            // Optionally, show only my Events
            /* if ( mShowMineOnly &&
                    !KCalCore::CalHelper::isMyCalendarIncidence( mCalendarAdaptor, ev. ) ) {
              // FIXME; does isMyCalendarIncidence work !? It's deprecated too.
              continue;
              }
              // TODO: CalHelper is deprecated, remove this?
              */

            if (ev->customProperty("KABC", "BIRTHDAY") == QLatin1String("YES")) {
                // Skipping, because these are got by the BirthdaySearchJob
                // See comments in updateView()
                continue;
            }

            if (!ev->categoriesStr().isEmpty()) {
                QStringList::ConstIterator it2;
                const QStringList c = ev->categories();
                QStringList::ConstIterator end(c.constEnd());
                for (it2 = c.constBegin(); it2 != end; ++it2) {

                    const QString itUpper((*it2).toUpper());
                    // Append Birthday Event?
                    if (mShowBirthdaysFromCal && (itUpper == QLatin1String("BIRTHDAY"))) {
                        SDEntry entry;
                        entry.type = IncidenceTypeEvent;
                        entry.category = CategoryBirthday;
                        entry.date = dt;
                        entry.summary = ev->summary();
                        entry.desc = ev->description();
                        dateDiff(ev->dtStart().date(), entry.daysTo, entry.yearsOld);
                        entry.span = 1;

                        /* The following check is to prevent duplicate entries,
                         * so in case of having a KCal incidence with category birthday
                         * with summary and date equal to some KABC Atendee we don't show it
                         * FIXME: port to akonadi, it's kresource based
                         * */
                        if (/*!check( bdayRes, dt, ev->summary() )*/true) {
                            mDates.append(entry);
                        }
                        break;
                    }

                    // Append Anniversary Event?
                    if (mShowAnniversariesFromCal && (itUpper == QStringLiteral("ANNIVERSARY"))) {
                        SDEntry entry;
                        entry.type = IncidenceTypeEvent;
                        entry.category = CategoryAnniversary;
                        entry.date = dt;
                        entry.summary = ev->summary();
                        entry.desc = ev->description();
                        dateDiff(ev->dtStart().date(), entry.daysTo, entry.yearsOld);
                        entry.span = 1;
                        if (/*!check( annvRes, dt, ev->summary() )*/true) {
                            mDates.append(entry);
                        }
                        break;
                    }

                    // Append Holiday Event?
                    if (mShowHolidays && (itUpper == QLatin1String("HOLIDAY"))) {
                        SDEntry entry;
                        entry.type = IncidenceTypeEvent;
                        entry.category = CategoryHoliday;
                        entry.date = dt;
                        entry.summary = ev->summary();
                        entry.desc = ev->description();
                        dateDiff(dt, entry.daysTo, entry.yearsOld);
                        entry.yearsOld = -1; //ignore age of holidays
                        entry.span = span(ev);
                        if (entry.span > 1 && dayof(ev, dt) > 1) {     // skip days 2,3,...
                            break;
                        }
                        mDates.append(entry);
                        break;
                    }

                    // Append Special Occasion Event?
                    if (mShowSpecialsFromCal && (itUpper == QLatin1String("SPECIAL OCCASION"))) {
                        SDEntry entry;
                        entry.type = IncidenceTypeEvent;
                        entry.category = CategoryOther;
                        entry.date = dt;
                        entry.summary = ev->summary();
                        entry.desc = ev->description();
                        dateDiff(dt, entry.daysTo, entry.yearsOld);
                        entry.yearsOld = -1; //ignore age of special occasions
                        entry.span = span(ev);
                        if (entry.span > 1 && dayof(ev, dt) > 1) {     // skip days 2,3,...
                            break;
                        }
                        mDates.append(entry);
                        break;
                    }
                }
            }
        }
    }

    // Seach for Holidays
    if (mShowHolidays) {
        if (initHolidays()) {
            for (dt = QDate::currentDate();
                    dt <= QDate::currentDate().addDays(mDaysAhead - 1);
                    dt = dt.addDays(1)) {
                QList<Holiday> holidays = mHolidays->holidays(dt);
                QList<Holiday>::ConstIterator it = holidays.constBegin();
                for (; it != holidays.constEnd(); ++it) {
                    SDEntry entry;
                    entry.type = IncidenceTypeEvent;
                    entry.category = ((*it).dayType() == Holiday::NonWorkday) ?
                                     CategoryHoliday : CategoryOther;
                    entry.date = dt;
                    entry.summary = (*it).name();
                    dateDiff(dt, entry.daysTo, entry.yearsOld);
                    entry.yearsOld = -1; //ignore age of holidays
                    entry.span = 1;

                    mDates.append(entry);
                }
            }
        }
    }

    // Sort, then Print the Special Dates
    qSort(mDates);

    if (!mDates.isEmpty()) {
        int counter = 0;
        QList<SDEntry>::Iterator addrIt;
        QList<SDEntry>::Iterator addrEnd(mDates.end());
        for (addrIt = mDates.begin(); addrIt != addrEnd; ++addrIt) {
            const bool makeBold = (*addrIt).daysTo == 0; // i.e., today

            // Pixmap
            QImage icon_img;
            QString icon_name;
            KContacts::Picture pic;
            switch ((*addrIt).category) {
            case CategoryBirthday:
                icon_name = QStringLiteral("view-calendar-birthday");
                pic = (*addrIt).addressee.photo();
                if (pic.isIntern() && !pic.data().isNull()) {
                    QImage img = pic.data();
                    if (img.width() > img.height()) {
                        icon_img = img.scaledToWidth(32);
                    } else {
                        icon_img = img.scaledToHeight(32);
                    }
                }
                break;
            case CategoryAnniversary:
                icon_name = QStringLiteral("view-calendar-wedding-anniversary");
                pic = (*addrIt).addressee.photo();
                if (pic.isIntern() && !pic.data().isNull()) {
                    QImage img = pic.data();
                    if (img.width() > img.height()) {
                        icon_img = img.scaledToWidth(32);
                    } else {
                        icon_img = img.scaledToHeight(32);
                    }
                }
                break;
            case CategoryHoliday:
                icon_name = QStringLiteral("view-calendar-holiday");
                break;
            case CategoryOther:
                icon_name = QStringLiteral("favorites");
                break;
            }
            label = new QLabel(this);
            if (icon_img.isNull()) {
                label->setPixmap(KIconLoader::global()->loadIcon(icon_name, KIconLoader::Small));
            } else {
                label->setPixmap(QPixmap::fromImage(icon_img));
            }
            label->setMaximumWidth(label->minimumSizeHint().width());
            label->setAlignment(Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 0);
            mLabels.append(label);

            // Event date
            QString datestr;

            //Muck with the year -- change to the year 'daysTo' days away
            int year = QDate::currentDate().addDays((*addrIt).daysTo).year();
            QDate sD = QDate(year, (*addrIt).date.month(), (*addrIt).date.day());

            if ((*addrIt).daysTo == 0) {
                datestr = i18nc("the special day is today", "Today");
            } else if ((*addrIt).daysTo == 1) {
                datestr = i18nc("the special day is tomorrow", "Tomorrow");
            } else {
                datestr = KLocale::global()->formatDate(sD, KLocale::FancyLongDate);
            }
            // Print the date span for multiday, floating events, for the
            // first day of the event only.
            if ((*addrIt).span > 1) {
                QString endstr =
                    KLocale::global()->formatDate(sD.addDays((*addrIt).span - 1));
                datestr += QLatin1String(" -\n ") + endstr;
            }

            label = new QLabel(datestr, this);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 1);
            mLabels.append(label);
            if (makeBold) {
                QFont font = label->font();
                font.setBold(true);
                label->setFont(font);
            }

            // Countdown
            label = new QLabel(this);
            if ((*addrIt).daysTo == 0) {
                label->setText(i18n("now"));
            } else {
                label->setText(i18np("in 1 day", "in %1 days", (*addrIt).daysTo));
            }

            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 2);
            mLabels.append(label);

            // What
            QString what;
            switch ((*addrIt).category) {
            case CategoryBirthday:
                what = i18n("Birthday");
                break;
            case CategoryAnniversary:
                what = i18n("Anniversary");
                break;
            case CategoryHoliday:
                what = i18n("Holiday");
                break;
            case CategoryOther:
                what = i18n("Special Occasion");
                break;
            }
            label = new QLabel(this);
            label->setText(what);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 3);
            mLabels.append(label);

            // Description
            if ((*addrIt).type == IncidenceTypeContact) {
                KUrlLabel *urlLabel = new KUrlLabel(this);
                urlLabel->installEventFilter(this);
                urlLabel->setUrl((*addrIt).item.url(Akonadi::Item::UrlWithMimeType).url());
                urlLabel->setText((*addrIt).addressee.realName());
                urlLabel->setTextFormat(Qt::RichText);
                urlLabel->setWordWrap(true);
                mLayout->addWidget(urlLabel, counter, 4);
                mLabels.append(urlLabel);

                connect(urlLabel, static_cast<void (KUrlLabel::*)(const QString &)>(&KUrlLabel::leftClickedUrl), this, &SDSummaryWidget::mailContact);
                connect(urlLabel, static_cast<void (KUrlLabel::*)(const QString &)>(&KUrlLabel::rightClickedUrl), this, &SDSummaryWidget::popupMenu);
            } else {
                label = new QLabel(this);
                label->setText((*addrIt).summary);
                label->setTextFormat(Qt::RichText);
                mLayout->addWidget(label, counter, 4);
                mLabels.append(label);
                if (!(*addrIt).desc.isEmpty()) {
                    label->setToolTip((*addrIt).desc);
                }
            }

            // Age
            if ((*addrIt).category == CategoryBirthday ||
                    (*addrIt).category == CategoryAnniversary) {
                label = new QLabel(this);
                if ((*addrIt).yearsOld <= 0) {
                    label->setText(QString());
                } else {
                    label->setText(i18np("one year", "%1 years", (*addrIt).yearsOld));
                }
                label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                mLayout->addWidget(label, counter, 5);
                mLabels.append(label);
            }

            counter++;
        }
    } else {
        label = new QLabel(
            i18np("No special dates within the next 1 day",
                  "No special dates pending within the next %1 days",
                  mDaysAhead), this);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        mLayout->addWidget(label, 0, 0);
        mLabels.append(label);
    }

    QList<QLabel *>::ConstIterator lit;
    QList<QLabel *>::ConstIterator endLit(mLabels.constEnd());
    for (lit = mLabels.constBegin(); lit != endLit; ++lit) {
        (*lit)->show();
    }
    setUpdatesEnabled(true);
}

void SDSummaryWidget::updateView()
{
    mDates.clear();

    /* KABC Birthdays are got through a ItemSearchJob/SPARQL Query
     * I then added an ETM/CalendarModel because we need to search
     * for calendar entries that have birthday/anniversary categories too.
     *
     * Also, we can't get KABC Anniversaries through nepomuk because the
     * current S.D.O doesn't support it, so i also them through the ETM.
     *
     * So basically we have:
     * Calendar anniversaries - ETM
     * Calendar birthdays - ETM
     * KABC birthdays - BirthdaySearchJob
     * KABC anniversaries - ETM ( needs Birthday Agent running )
     *
     * We could remove thomas' BirthdaySearchJob and use the ETM for that
     * but it has the advantage that we don't need a Birthday agent running.
     *
     **/

    // Search for Birthdays
    if (mShowBirthdaysFromKAB && !mJobRunning) {
        BirthdaySearchJob *job = new BirthdaySearchJob(this, mDaysAhead);

        connect(job, &BirthdaySearchJob::result, this, &SDSummaryWidget::slotBirthdayJobFinished);
        job->start();
        mJobRunning = true;

        // The result slot will trigger the rest of the update
    }
}

void SDSummaryWidget::mailContact(const QString &url)
{
    const Akonadi::Item item = Akonadi::Item::fromUrl(url);
    if (!item.isValid()) {
        qCDebug(KORGANIZER_KONTACTPLUGINS_SPECIALDATES_LOG) << QStringLiteral("Invalid item found");
        return;
    }

    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item, this);
    job->fetchScope().fetchFullPayload();
    connect(job, &Akonadi::ItemFetchJob::result, this, &SDSummaryWidget::slotItemFetchJobDone);
}

void SDSummaryWidget::slotItemFetchJobDone(KJob *job)
{
    if (job->error()) {
        qCWarning(KORGANIZER_KONTACTPLUGINS_SPECIALDATES_LOG) << job->errorString();
        return;
    }
    const Akonadi::Item::List lst = qobject_cast<Akonadi::ItemFetchJob *>(job)->items();
    if (lst.isEmpty()) {
        return;
    }
    const KContacts::Addressee contact = lst.first().payload<KContacts::Addressee>();

    QDesktopServices::openUrl(QUrl(contact.fullEmail()));
}

void SDSummaryWidget::viewContact(const QString &url)
{
    const Akonadi::Item item = Akonadi::Item::fromUrl(url);
    if (!item.isValid()) {
        qCDebug(KORGANIZER_KONTACTPLUGINS_SPECIALDATES_LOG) << "Invalid item found";
        return;
    }

    Akonadi::ContactViewerDialog dlg(this);
    dlg.setContact(item);
    dlg.exec();
}

void SDSummaryWidget::popupMenu(const QString &url)
{
    QMenu popup(this);
    const QAction *sendMailAction = popup.addAction(
                                        KIconLoader::global()->loadIcon(QStringLiteral("mail-message-new"), KIconLoader::Small),
                                        i18n("Send &Mail"));
    const QAction *viewContactAction = popup.addAction(
                                           KIconLoader::global()->loadIcon(QStringLiteral("view-pim-contacts"), KIconLoader::Small),
                                           i18n("View &Contact"));

    const QAction *ret = popup.exec(QCursor::pos());
    if (ret == sendMailAction) {
        mailContact(url);
    } else if (ret == viewContactAction) {
        viewContact(url);
    }
}

bool SDSummaryWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj->inherits("KUrlLabel")) {
        KUrlLabel *label = static_cast<KUrlLabel *>(obj);
        if (e->type() == QEvent::Enter) {
            Q_EMIT message(i18n("Mail to:\"%1\"", label->text()));
        }
        if (e->type() == QEvent::Leave) {
            Q_EMIT message(QString());
        }
    }

    return KontactInterface::Summary::eventFilter(obj, e);
}

void SDSummaryWidget::dateDiff(const QDate &date, int &days, int &years) const
{
    QDate currentDate;
    QDate eventDate;

    if (QDate::isLeapYear(date.year()) && date.month() == 2 && date.day() == 29) {
        currentDate = QDate(date.year(), QDate::currentDate().month(), QDate::currentDate().day());
        if (!QDate::isLeapYear(QDate::currentDate().year())) {
            eventDate = QDate(date.year(), date.month(), 28);   // celebrate one day earlier ;)
        } else {
            eventDate = QDate(date.year(), date.month(), date.day());
        }
    } else {
        currentDate = QDate(QDate::currentDate().year(),
                            QDate::currentDate().month(),
                            QDate::currentDate().day());
        eventDate = QDate(QDate::currentDate().year(), date.month(), date.day());
    }

    int offset = currentDate.daysTo(eventDate);
    if (offset < 0) {
        days = 365 + offset;
        years = QDate::currentDate().year() + 1 - date.year();
    } else {
        days = offset;
        years = QDate::currentDate().year() - date.year();
    }
}

QStringList SDSummaryWidget::configModules() const
{
    return QStringList() << QStringLiteral("kcmsdsummary.desktop");
}

