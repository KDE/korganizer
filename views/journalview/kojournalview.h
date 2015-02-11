/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KORG_VIEWS_KOJOURNALVIEW_H
#define KORG_VIEWS_KOJOURNALVIEW_H

#include "korganizer/baseview.h"
#include <KCalCore/Incidence> // for KCalCore::DateList typedef

namespace EventViews
{
class JournalView;
}

/**
 * This class provides a journal view.

 * @short View for Journal components.
 * @author Cornelius Schumacher <schumacher@kde.org>, Reinhold Kainhofer <reinhold@kainhofer.com>
 * @see KOBaseView
 */
class KOJournalView : public KOrg::BaseView
{
    Q_OBJECT
public:
    explicit KOJournalView(QWidget *parent = Q_NULLPTR);
    ~KOJournalView();

    int currentDateCount() const Q_DECL_OVERRIDE;
    Akonadi::Item::List selectedIncidences() Q_DECL_OVERRIDE;

    KCalCore::DateList selectedIncidenceDates() Q_DECL_OVERRIDE
    {
        return KCalCore::DateList();
    }

    void setCalendar(const Akonadi::ETMCalendar::Ptr &) Q_DECL_OVERRIDE;

    /** reimp */
    void getHighlightMode(bool &highlightEvents,
                          bool &highlightTodos,
                          bool &highlightJournals) Q_DECL_OVERRIDE;

    /** reimp */
    CalendarSupport::CalPrinterBase::PrintType printType() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void updateView() Q_DECL_OVERRIDE;
    void flushView() Q_DECL_OVERRIDE;

    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) Q_DECL_OVERRIDE;
    void showIncidences(const Akonadi::Item::List &incidences, const QDate &date) Q_DECL_OVERRIDE;

    void changeIncidenceDisplay(const Akonadi::Item &incidence,
                                Akonadi::IncidenceChanger::ChangeType) Q_DECL_OVERRIDE;
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) Q_DECL_OVERRIDE;
    void printJournal(const KCalCore::Journal::Ptr &journal, bool preview);

private:
    EventViews::JournalView *mJournalView;
};

#endif
