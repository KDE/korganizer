/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "summaryeventinfo.h"

#include <Akonadi/Item>

#include <KCalendarCore/Calendar>
#include <KCalendarCore/Event>
using namespace KCalendarCore;

#include <KCalUtils/IncidenceFormatter>
using namespace KCalUtils;

#include <KLocalizedString>

#include <QDate>
#include <QLocale>
#include <QStringList>

bool SummaryEventInfo::mShowBirthdays = true;
bool SummaryEventInfo::mShowAnniversaries = true;

using DateTimeByUidHash = QHash<QString, QDateTime>;
Q_GLOBAL_STATIC(DateTimeByUidHash, sDateTimeByUid)

static bool eventLessThan(const KCalendarCore::Event::Ptr &event1, const KCalendarCore::Event::Ptr &event2)
{
    QDateTime kdt1 = sDateTimeByUid()->value(event1->instanceIdentifier());
    QDateTime kdt2 = sDateTimeByUid()->value(event2->instanceIdentifier());
    if (kdt1 < kdt2) {
        return true;
    } else if (kdt1 > kdt2) {
        return false;
    } else {
        return event1->summary() < event2->summary();
    }
}

void SummaryEventInfo::setShowSpecialEvents(bool showBirthdays, bool showAnniversaries)
{
    mShowBirthdays = showBirthdays;
    mShowAnniversaries = showAnniversaries;
}

bool SummaryEventInfo::skip(const KCalendarCore::Event::Ptr &event)
{
    // simply check categories because the birthdays resource always adds
    // the appropriate category to the event.
    QStringList c = event->categories();
    if (!mShowBirthdays && c.contains(QLatin1StringView("BIRTHDAY"), Qt::CaseInsensitive)) {
        return true;
    }
    if (!mShowAnniversaries && c.contains(QLatin1StringView("ANNIVERSARY"), Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

SummaryEventInfo::SummaryEventInfo() = default;

/**static*/
SummaryEventInfo::List SummaryEventInfo::eventsForRange(QDate start, QDate end, const Akonadi::ETMCalendar::Ptr &calendar)
{
    KCalendarCore::Event::List allEvents = calendar->events();
    KCalendarCore::Event::List events;
    const auto currentDateTime = QDateTime::currentDateTime();
    const QDate currentDate = currentDateTime.date();

    sDateTimeByUid()->clear();
    for (int i = 0; i < allEvents.count(); ++i) {
        KCalendarCore::Event::Ptr event = allEvents.at(i);
        if (skip(event)) {
            continue;
        }

        const auto eventStart = event->dtStart().toLocalTime();
        const auto eventEnd = event->dtEnd().toLocalTime();
        if (event->recurs()) {
            const auto occurrences = event->recurrence()->timesInInterval(QDateTime(start, {}), QDateTime(end, {}));
            if (!occurrences.isEmpty()) {
                events << event;
                sDateTimeByUid()->insert(event->instanceIdentifier(), occurrences.first());
            }
        } else {
            if ((end >= eventStart.date() && start <= eventEnd.date()) || (start >= eventStart.date() && end <= eventEnd.date())) {
                events << event;
                if (eventStart.date() < start) {
                    sDateTimeByUid()->insert(event->instanceIdentifier(), QDateTime(start.startOfDay()));
                } else {
                    sDateTimeByUid()->insert(event->instanceIdentifier(), eventStart);
                }
            }
        }
    }

    std::sort(events.begin(), events.end(), eventLessThan);

    SummaryEventInfo::List eventInfoList;
    KCalendarCore::Event::Ptr ev;
    eventInfoList.reserve(events.count());
    auto itEnd = events.constEnd();
    for (auto it = events.constBegin(); it != itEnd; ++it) {
        ev = *it;
        // Count number of days remaining in multiday event
        int span = 1;
        int dayof = 1;
        const auto eventStart = ev->dtStart().toLocalTime();
        const auto eventEnd = ev->dtEnd().toLocalTime();
        const QDate occurrenceStartDate = sDateTimeByUid()->value(ev->instanceIdentifier()).date();

        QDate startOfMultiday = eventStart.date();
        if (startOfMultiday < currentDate) {
            startOfMultiday = currentDate;
        }
        bool firstDayOfMultiday = (start == startOfMultiday);

        auto summaryEvent = new SummaryEventInfo();
        eventInfoList.append(summaryEvent);

        // Event
        summaryEvent->ev = ev;

        // Start date label
        QString str;
        QDate sD = occurrenceStartDate;
        if (currentDate >= sD) {
            str = i18nc("the appointment is today", "Today");
            summaryEvent->makeBold = true;
        } else if (sD == currentDate.addDays(1)) {
            str = i18nc("the appointment is tomorrow", "Tomorrow");
        } else {
            const auto locale = QLocale::system();
            for (int i = 3; i < 8; ++i) {
                if (sD < currentDate.addDays(i)) {
                    str = locale.dayName(sD.dayOfWeek(), QLocale::LongFormat);
                    break;
                }
            }
            if (str.isEmpty()) {
                str = locale.toString(sD, QLocale::LongFormat);
            }
        }
        summaryEvent->startDate = str;

        if (ev->isMultiDay()) {
            dayof = eventStart.date().daysTo(start) + 1;
            dayof = dayof <= 0 ? 1 : dayof;
            span = eventStart.date().daysTo(eventEnd.date()) + 1;
        }
        // Print the date span for multiday, floating events, for the
        // first day of the event only.
        if (ev->isMultiDay() && ev->allDay() && firstDayOfMultiday && span > 1) {
            str = IncidenceFormatter::dateToString(ev->dtStart().toLocalTime().date(), false) + QLatin1StringView(" -\n ")
                + IncidenceFormatter::dateToString(ev->dtEnd().toLocalTime().date(), false);
        }
        summaryEvent->dateSpan = str;

        // Days to go label
        str.clear();
        const qint64 daysTo = currentDate.daysTo(occurrenceStartDate);
        if (daysTo > 0) {
            str = i18np("in 1 day", "in %1 days", daysTo);
        } else {
            if (!ev->allDay()) {
                int secs;
                if (!ev->recurs()) {
                    secs = currentDateTime.secsTo(ev->dtStart());
                } else {
                    QDateTime kdt(start, QTime(0, 0, 0), QTimeZone::LocalTime);
                    kdt = kdt.addSecs(-1);
                    const auto next = ev->recurrence()->getNextDateTime(kdt);
                    secs = currentDateTime.secsTo(next);
                }
                if (secs > 0) {
                    str = i18nc("eg. in 1 hour 2 minutes", "in ");
                    int hours = secs / 3600;
                    if (hours > 0) {
                        str += i18ncp("use abbreviation for hour to keep the text short", "1 hr", "%1 hrs", hours);
                        str += QLatin1Char(' ');
                        secs -= (hours * 3600);
                    }
                    int mins = secs / 60;
                    if (mins > 0) {
                        str += i18ncp("use abbreviation for minute to keep the text short", "1 min", "%1 mins", mins);
                        if (hours < 1) {
                            // happens in less than 1 hour
                            summaryEvent->makeUrgent = true;
                        }
                    }
                } else {
                    str = i18n("now");
                }
            } else {
                str = i18n("all day");
            }
        }
        summaryEvent->daysToGo = str;

        // Summary label
        str = ev->richSummary();
        if (ev->isMultiDay() && !ev->allDay()) {
            str.append(QStringLiteral(" (%1/%2)").arg(dayof).arg(span));
        }
        summaryEvent->summaryText = str;
        if (!ev->location().isEmpty()) {
            summaryEvent->summaryText.append(QStringLiteral(" (%1)").arg(ev->location()));
        }
        summaryEvent->summaryUrl = ev->uid();

        QString displayName;
        Akonadi::Item item = calendar->item(ev);
        if (item.isValid()) {
            const Akonadi::Collection col = item.parentCollection();
            if (col.isValid()) {
                displayName = col.displayName();
            }
        }
        summaryEvent->summaryTooltip = KCalUtils::IncidenceFormatter::toolTipStr(displayName, ev, start, true);

        // Time range label (only for non-floating events)
        str.clear();
        if (!ev->allDay()) {
            QTime sST = eventStart.time();
            QTime sET = eventEnd.time();
            if (ev->isMultiDay()) {
                if (eventStart.date() < start) {
                    sST = QTime(0, 0);
                }
                if (eventEnd.date() > end) {
                    sET = QTime(23, 59);
                }
            }
            str = i18nc("Time from - to",
                        "%1 - %2",
                        QLocale::system().toString(sST, QLocale::ShortFormat),
                        QLocale::system().toString(sET, QLocale::ShortFormat));
            summaryEvent->timeRange = str;
        }

        // For recurring events, append the next occurrence to the time range label
        if (ev->recurs()) {
            QDateTime kdt(start, QTime(0, 0, 0));
            kdt = kdt.addSecs(-1);
            QDateTime next = ev->recurrence()->getNextDateTime(kdt);
            QString tmp = IncidenceFormatter::dateTimeToString(ev->recurrence()->getNextDateTime(next), ev->allDay(), true);
            if (!summaryEvent->timeRange.isEmpty()) {
                summaryEvent->timeRange += QLatin1StringView("<br>");
            }
            summaryEvent->timeRange +=
                QLatin1StringView("<font size=\"small\"><i>") + i18nc("next occurrence", "Next: %1", tmp) + QLatin1StringView("</i></font>");
        }
    }

    return eventInfoList;
}

SummaryEventInfo::List SummaryEventInfo::eventsForDate(QDate date, const Akonadi::ETMCalendar::Ptr &calendar)
{
    return eventsForRange(date, date, calendar);
}
