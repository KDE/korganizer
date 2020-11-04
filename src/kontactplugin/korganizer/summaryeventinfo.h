/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef SUMMARYEVENTINFO_H
#define SUMMARYEVENTINFO_H

#include <Akonadi/Calendar/ETMCalendar>

class QDate;

class SummaryEventInfo
{
public:

    using List = QList<SummaryEventInfo *>;

    SummaryEventInfo();

    static List eventsForDate(const QDate &date, const Akonadi::ETMCalendar::Ptr &calendar);
    static List eventsForRange(const QDate &start, const QDate &end, // range is inclusive
                               const Akonadi::ETMCalendar::Ptr &calendar);
    static void setShowSpecialEvents(bool skipBirthdays, bool skipAnniversaries);

    KCalendarCore::Event::Ptr ev;
    QString startDate;
    QString dateSpan;
    QString daysToGo;
    QString timeRange;
    QString summaryText;
    QString summaryUrl;
    QString summaryTooltip;
    bool makeBold;
    bool makeUrgent;

private:
    static bool skip(const KCalendarCore::Event::Ptr &event);
    static bool mShowBirthdays, mShowAnniversaries;
};

#endif
