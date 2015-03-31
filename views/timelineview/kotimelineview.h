/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Till Adam <adam@kde.org>

  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Copyright (c) 2010 Sérgio Martins <sergio.martins@kdab.com>

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

#ifndef KORG_VIEWS_KOTIMELINEVIEW_H
#define KORG_VIEWS_KOTIMELINEVIEW_H

#include "koeventview.h"

namespace Akonadi
{
class IncidenceChanger;
}

/**
  This class provides a view ....
*/
class KOTimelineView : public KOEventView
{
    Q_OBJECT
public:
    explicit KOTimelineView(QWidget *parent = Q_NULLPTR);
    ~KOTimelineView();

    Akonadi::Item::List selectedIncidences() Q_DECL_OVERRIDE;
    KCalCore::DateList selectedIncidenceDates() Q_DECL_OVERRIDE;
    int currentDateCount() const Q_DECL_OVERRIDE;
    void showDates(const QDate &, const QDate &, const QDate &preferredMonth = QDate()) Q_DECL_OVERRIDE;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) Q_DECL_OVERRIDE;
    void updateView() Q_DECL_OVERRIDE;
    void changeIncidenceDisplay(const Akonadi::Item &incidence,
                                        Akonadi::IncidenceChanger::ChangeType) Q_DECL_OVERRIDE;
    int maxDatesHint() const Q_DECL_OVERRIDE
    {
        return 0;
    }
    bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) Q_DECL_OVERRIDE;
    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) Q_DECL_OVERRIDE;
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) Q_DECL_OVERRIDE;

    CalendarSupport::CalPrinterBase::PrintType printType() const Q_DECL_OVERRIDE;

private:
    class Private;
    Private *const d;
};

#endif
