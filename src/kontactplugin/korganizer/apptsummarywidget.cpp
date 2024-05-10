/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "apptsummarywidget.h"
#include "korganizerinterface.h"
#include "korganizerplugin.h"
#include "summaryeventinfo.h"

#include <CalendarSupport/CalendarSingleton>
#include <CalendarSupport/Utils>

#include <Akonadi/Collection>
#include <Akonadi/IncidenceChanger>

#include <KCalendarCore/Calendar>
#include <KCalendarCore/Event>

#include <KontactInterface/Core>

#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KUrlLabel>
#include <QMenu>

#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

ApptSummaryWidget::ApptSummaryWidget(KOrganizerPlugin *plugin, QWidget *parent)
    : KontactInterface::Summary(parent)
    , mPlugin(plugin)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(3);
    mainLayout->setContentsMargins(3, 3, 3, 3);

    QWidget *header = createHeader(this, QStringLiteral("view-calendar-upcoming-events"), i18n("Upcoming Events"));
    mainLayout->addWidget(header);

    mLayout = new QGridLayout();
    mainLayout->addItem(mLayout);
    mLayout->setSpacing(3);
    mLayout->setRowStretch(6, 1);

    mCalendar = CalendarSupport::calendarSingleton();

    mChanger = new Akonadi::IncidenceChanger(parent);

    connect(mCalendar.data(), &Akonadi::ETMCalendar::calendarChanged, this, &ApptSummaryWidget::updateView);
    connect(mPlugin->core(), &KontactInterface::Core::dayChanged, this, &ApptSummaryWidget::updateView);

    // Update Configuration
    configUpdated();
}

ApptSummaryWidget::~ApptSummaryWidget() = default;

void ApptSummaryWidget::configUpdated()
{
    KConfig config(QStringLiteral("kcmapptsummaryrc"));

    KConfigGroup group = config.group(QStringLiteral("Days"));
    mDaysAhead = group.readEntry("DaysToShow", 7);

    group = config.group(QStringLiteral("Show"));
    mShowBirthdaysFromCal = group.readEntry("BirthdaysFromCalendar", true);
    mShowAnniversariesFromCal = group.readEntry("AnniversariesFromCalendar", true);

    group = config.group(QStringLiteral("Groupware"));
    mShowMineOnly = group.readEntry("ShowMineOnly", false);

    updateView();
}

void ApptSummaryWidget::updateView()
{
    qDeleteAll(mLabels);
    mLabels.clear();

    // The event print consists of the following fields:
    //  icon:start date:days-to-go:summary:time range
    // where,
    //   the icon is the typical event icon
    //   the start date is the event start date
    //   the days-to-go is the #days until the event starts
    //   the summary is the event summary
    //   the time range is the start-end time (only for non-floating events)

    int counter = 0;

    QPixmap pm = QIcon::fromTheme(QStringLiteral("view-calendar-day")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    QPixmap pmb = QIcon::fromTheme(QStringLiteral("view-calendar-birthday")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    QPixmap pma = QIcon::fromTheme(QStringLiteral("view-calendar-wedding-anniversary")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));

    QStringList uidList;
    SummaryEventInfo::setShowSpecialEvents(mShowBirthdaysFromCal, mShowAnniversariesFromCal);
    QDate currentDate = QDate::currentDate();

    const SummaryEventInfo::List events = SummaryEventInfo::eventsForRange(currentDate, currentDate.addDays(mDaysAhead - 1), mCalendar);

    QPalette todayPalette = palette();
    KColorScheme::adjustBackground(todayPalette, KColorScheme::ActiveBackground, QPalette::Window);
    QPalette urgentPalette = palette();
    KColorScheme::adjustBackground(urgentPalette, KColorScheme::NegativeBackground, QPalette::Window);

    for (SummaryEventInfo *event : events) {
        // Optionally, show only my Events
        /*      if ( mShowMineOnly &&
                  !KCalendarCore::CalHelper::isMyCalendarIncidence( mCalendarAdaptor, event->ev ) ) {
              continue;
            }
            TODO: CalHelper is deprecated, remove this?
        */

        KCalendarCore::Event::Ptr ev = event->ev;
        // print the first of the recurring event series only
        if (ev->recurs()) {
            if (uidList.contains(ev->instanceIdentifier())) {
                continue;
            }
            uidList.append(ev->instanceIdentifier());
        }

        // Icon label
        auto label = new QLabel(this);
        if (ev->categories().contains(QLatin1StringView("BIRTHDAY"), Qt::CaseInsensitive)) {
            label->setPixmap(pmb);
        } else if (ev->categories().contains(QLatin1StringView("ANNIVERSARY"), Qt::CaseInsensitive)) {
            label->setPixmap(pma);
        } else {
            label->setPixmap(pm);
        }
        label->setMaximumWidth(label->minimumSizeHint().width());
        mLayout->addWidget(label, counter, 0);
        mLabels.append(label);

        // Start date or date span label
        QString dateToDisplay = event->startDate;
        if (!event->dateSpan.isEmpty()) {
            dateToDisplay = event->dateSpan;
        }
        label = new QLabel(dateToDisplay, this);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        mLayout->addWidget(label, counter, 1);
        mLabels.append(label);
        if (event->makeBold) {
            QFont font = label->font();
            font.setBold(true);
            label->setFont(font);
            if (!event->makeUrgent) {
                label->setPalette(todayPalette);
            } else {
                label->setPalette(urgentPalette);
            }
            label->setAutoFillBackground(true);
        }

        // Days to go label
        label = new QLabel(event->daysToGo, this);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        mLayout->addWidget(label, counter, 2);
        mLabels.append(label);

        // Summary label
        auto urlLabel = new KUrlLabel(this);
        urlLabel->setText(event->summaryText);
        urlLabel->setUrl(event->summaryUrl);
        urlLabel->installEventFilter(this);
        urlLabel->setTextFormat(Qt::RichText);
        urlLabel->setWordWrap(true);
        mLayout->addWidget(urlLabel, counter, 3);
        mLabels.append(urlLabel);
        connect(urlLabel, &KUrlLabel::leftClickedUrl, this, [this, urlLabel] {
            viewEvent(urlLabel->url());
        });
        connect(urlLabel, &KUrlLabel::rightClickedUrl, this, [this, urlLabel] {
            popupMenu(urlLabel->url());
        });
        if (!event->summaryTooltip.isEmpty()) {
            urlLabel->setToolTip(event->summaryTooltip);
        }

        // Time range label (only for non-floating events)
        if (!event->timeRange.isEmpty()) {
            label = new QLabel(event->timeRange, this);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 4);
            mLabels.append(label);
        }

        counter++;
    }

    qDeleteAll(events);

    if (!counter) {
        auto noEvents =
            new QLabel(i18np("No upcoming events starting within the next day", "No upcoming events starting within the next %1 days", mDaysAhead), this);
        noEvents->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        mLayout->addWidget(noEvents, 0, 0);
        mLabels.append(noEvents);
    }

    for (QLabel *label : std::as_const(mLabels)) {
        label->show();
    }
}

void ApptSummaryWidget::viewEvent(const QString &uid)
{
    Akonadi::Item::Id id = mCalendar->item(uid).id();

    if (id != -1) {
        mPlugin->core()->selectPlugin(QStringLiteral("kontact_korganizerplugin")); // ensure loaded
        OrgKdeKorganizerKorganizerInterface korganizer(QStringLiteral("org.kde.korganizer"), QStringLiteral("/Korganizer"), QDBusConnection::sessionBus());
        korganizer.editIncidence(QString::number(id));
    }
}

void ApptSummaryWidget::removeEvent(const Akonadi::Item &item)
{
    (void)mChanger->deleteIncidence(item);
}

void ApptSummaryWidget::popupMenu(const QString &uid)
{
    QMenu popup(this);

    // FIXME: Should say "Show Appointment" if we don't have rights to edit
    // Doesn't make sense to edit events from birthday resource for example
    QAction *editIt = popup.addAction(i18n("&Edit Appointment…"));
    editIt->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    QAction *delIt = popup.addAction(i18n("&Delete Appointment"));
    delIt->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));

    Akonadi::Item item = mCalendar->item(uid);
    delIt->setEnabled(mCalendar->hasRight(item, Akonadi::Collection::CanDeleteItem));

    const QAction *selectedAction = popup.exec(QCursor::pos());
    if (selectedAction == editIt) {
        viewEvent(uid);
    } else if (selectedAction == delIt) {
        removeEvent(item);
    }
}

bool ApptSummaryWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj->inherits("KUrlLabel")) {
        auto label = static_cast<KUrlLabel *>(obj);
        if (e->type() == QEvent::Enter) {
            Q_EMIT message(i18n("Edit Event: \"%1\"", label->text()));
        }
        if (e->type() == QEvent::Leave) {
            Q_EMIT message(QString());
        }
    }

    return KontactInterface::Summary::eventFilter(obj, e);
}

#include "moc_apptsummarywidget.cpp"
