/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008-2009 Allen Winter <winter@kde.org>
  Copyright (c) 2008 Thomas McGuire <mcguire@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "summaryeventinfo.h"

#include <KCalCore/Calendar>
#include <KCalCore/Event>
using namespace KCalCore;

#include <KCalUtils/IncidenceFormatter>
using namespace KCalUtils;

#include <KLocalizedString>
#include <KSystemTimeZones>
#include <KLocale>

#include <QDate>
#include <QStringList>

bool SummaryEventInfo::mShowBirthdays = true;
bool SummaryEventInfo::mShowAnniversaries = true;

static QHash<QString, KDateTime> sDateTimeByUid;

static bool eventLessThan(const KCalCore::Event::Ptr &event1, const KCalCore::Event::Ptr &event2)
{
    KDateTime kdt1 = sDateTimeByUid.value(event1->instanceIdentifier());
    KDateTime kdt2 = sDateTimeByUid.value(event2->instanceIdentifier());
    if (kdt1.date() < kdt2.date()) { // Compare dates first since comparing all day with non-all-day doesn't work
        return true;
    } else if (kdt1.date() > kdt2.date()) {
        return false;
    } else {
        if (kdt1.isDateOnly() && !kdt2.isDateOnly()) {
            return false;
        } else if (!kdt1.isDateOnly() && kdt2.isDateOnly())  {
            return true;
        } else {
            if (kdt1 > kdt2) {
                return true;
            } else if (kdt1 < kdt2) {
                return false;
            } else {
                return event1->summary() > event2->summary();
            }
        }
    }
}

void SummaryEventInfo::setShowSpecialEvents(bool showBirthdays,
        bool showAnniversaries)
{
    mShowBirthdays = showBirthdays;
    mShowAnniversaries = showAnniversaries;
}

bool SummaryEventInfo::skip(const KCalCore::Event::Ptr &event)
{
    //simply check categories because the birthdays resource always adds
    //the appropriate category to the event.
    QStringList c = event->categories();
    if (!mShowBirthdays &&
            c.contains(QLatin1String("BIRTHDAY"), Qt::CaseInsensitive)) {
        return true;
    }
    if (!mShowAnniversaries &&
            c.contains(QLatin1String("ANNIVERSARY"), Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

void SummaryEventInfo::dateDiff(const QDate &date, int &days)
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
        if (QDate::isLeapYear(QDate::currentDate().year())) {
            days++;
        }
    } else {
        days = offset;
    }
}

SummaryEventInfo::SummaryEventInfo()
    : makeBold(false)
{
}

/**static*/
SummaryEventInfo::List SummaryEventInfo::eventsForRange(const QDate &start, const QDate &end,
        KCalCore::Calendar *calendar)
{
    KCalCore::Event::List allEvents = calendar->events(); // calendar->rawEvents() isn't exactly what we want, doesn't handle recurrence right
    KCalCore::Event::List events;
    KDateTime::Spec spec = KSystemTimeZones::local();
    const KDateTime currentDateTime = KDateTime::currentDateTime(spec);
    const QDate currentDate = currentDateTime.date();

    sDateTimeByUid.clear();

    for (int i = 0; i < allEvents.count(); ++i) {
        KCalCore::Event::Ptr event = allEvents.at(i);
        if (skip(event)) {
            continue;
        }

        KDateTime eventStart = event->dtStart().toTimeSpec(spec);
        KDateTime eventEnd = event->dtEnd().toTimeSpec(spec);
        if (event->recurs()) {
            KCalCore::DateTimeList occurrences = event->recurrence()->timesInInterval(KDateTime(start, spec), KDateTime(end, spec));
            if (!occurrences.isEmpty()) {
                events << event;
                sDateTimeByUid.insert(event->instanceIdentifier(), occurrences.first());
            }
        } else {
            if ((end >= eventStart.date() && start <= eventEnd.date()) ||
                    (start >= eventStart.date() && end <= eventEnd.date())) {
                events << event;
                if (eventStart.date() < start) {
                    sDateTimeByUid.insert(event->instanceIdentifier(), KDateTime(start, spec));
                } else {
                    sDateTimeByUid.insert(event->instanceIdentifier(), eventStart);
                }
            }
        }
    }

    qSort(events.begin(), events.end(), eventLessThan);

    SummaryEventInfo::List eventInfoList;
    KCalCore::Event::Ptr ev;
    KCalCore::Event::List::ConstIterator itEnd = events.constEnd();
    for (KCalCore::Event::List::ConstIterator it = events.constBegin(); it != itEnd; ++it) {
        ev = *it;
        // Count number of days remaining in multiday event
        int span = 1;
        int dayof = 1;
        const KDateTime eventStart = ev->dtStart().toTimeSpec(spec);
        const KDateTime eventEnd = ev->dtEnd().toTimeSpec(spec);
        const QDate occurrenceStartDate = sDateTimeByUid.value(ev->instanceIdentifier()).date();

        QDate startOfMultiday = eventStart.date();
        if (startOfMultiday < currentDate) {
            startOfMultiday = currentDate;
        }
        bool firstDayOfMultiday = (start == startOfMultiday);

        SummaryEventInfo *summaryEvent = new SummaryEventInfo();
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
            str = KLocale::global()->formatDate(sD, KLocale::FancyLongDate);
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
            str = IncidenceFormatter::dateToString(ev->dtStart(), false, spec) +
                  QLatin1String(" -\n ") +
                  IncidenceFormatter::dateToString(ev->dtEnd(), false, spec);
        }
        summaryEvent->dateSpan = str;

        // Days to go label
        str.clear();
        const int daysTo = currentDate.daysTo(occurrenceStartDate);
        if (daysTo > 0) {
            str = i18np("in 1 day", "in %1 days", daysTo);
        } else {
            if (!ev->allDay()) {
                int secs;
                if (!ev->recurs()) {
                    secs = currentDateTime.secsTo(ev->dtStart());
                } else {
                    KDateTime kdt(start, QTime(0, 0, 0), spec);
                    kdt = kdt.addSecs(-1);
                    KDateTime next = ev->recurrence()->getNextDateTime(kdt);
                    secs = currentDateTime.secsTo(next);
                }
                if (secs > 0) {
                    str = i18nc("eg. in 1 hour 2 minutes", "in ");
                    int hours = secs / 3600;
                    if (hours > 0) {
                        str += i18ncp("use abbreviation for hour to keep the text short",
                                      "1 hr", "%1 hrs", hours);
                        str += QLatin1Char(' ');
                        secs -= (hours * 3600);
                    }
                    int mins = secs / 60;
                    if (mins > 0) {
                        str += i18ncp("use abbreviation for minute to keep the text short",
                                      "1 min", "%1 mins", mins);
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
            str.append(QString::fromLatin1(" (%1/%2)").arg(dayof).arg(span));
        }
        summaryEvent->summaryText = str;
        summaryEvent->summaryUrl = ev->uid();
        /*
         Commented out because a ETMCalendar doesn't have any name, it's a group of selected
         calendars, not an individual one.

         QString tipText( KCalUtils::IncidenceFormatter::toolTipStr(
                           KCalUtils::IncidenceFormatter::resourceString(
                             calendar, ev ), ev, start, true, spec ) );
        if ( !tipText.isEmpty() ) {
          summaryEvent->summaryTooltip = tipText;
        }*/

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
            str = i18nc("Time from - to", "%1 - %2",
                        KLocale::global()->formatTime(sST),
                        KLocale::global()->formatTime(sET));
            summaryEvent->timeRange = str;
        }

        // For recurring events, append the next occurrence to the time range label
        if (ev->recurs()) {
            KDateTime kdt(start, QTime(0, 0, 0), spec);
            kdt = kdt.addSecs(-1);
            KDateTime next = ev->recurrence()->getNextDateTime(kdt);
            QString tmp = IncidenceFormatter::dateTimeToString(
                              ev->recurrence()->getNextDateTime(next), ev->allDay(),
                              true, spec);
            if (!summaryEvent->timeRange.isEmpty()) {
                summaryEvent->timeRange += QLatin1String("<br>");
            }
            summaryEvent->timeRange += QLatin1String("<font size=\"small\"><i>") +
                                       i18nc("next occurrence", "Next: %1", tmp) +
                                       QLatin1String("</i></font>");
        }
    }

    return eventInfoList;
}

SummaryEventInfo::List SummaryEventInfo::eventsForDate(const QDate &date,
        KCalCore::Calendar *calendar)
{
    return eventsForRange(date, date, calendar);
}
