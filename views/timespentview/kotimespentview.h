/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>

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

#ifndef KORG_VIEWS_KOTIMESPENTVIEW_H
#define KORG_VIEWS_KOTIMESPENTVIEW_H

#include "korganizer/baseview.h"
#include <Akonadi/Calendar/ETMCalendar>

namespace EventViews {
  class TimeSpentView;
}

/**
  This view show the time spent on each category.
*/
class KOTimeSpentView : public KOrg::BaseView
{
  Q_OBJECT
  public:
    explicit KOTimeSpentView( QWidget *parent = 0 );
    ~KOTimeSpentView();

    virtual int currentDateCount() const;

    virtual Akonadi::Item::List selectedIncidences()
    {
      return Akonadi::Item::List();
    }

    KCalCore::DateList selectedIncidenceDates()
    {
      return KCalCore::DateList();
    }

    void setCalendar( const Akonadi::ETMCalendar::Ptr &cal );

  public Q_SLOTS:
    virtual void updateView();
    virtual void showDates( const QDate &start, const QDate &end,
                            const QDate &preferredMonth = QDate() );
    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );

    void changeIncidenceDisplay( const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType );
    virtual KOrg::CalPrinterBase::PrintType printType() const;

  private:
    EventViews::TimeSpentView *mView;
};

#endif
