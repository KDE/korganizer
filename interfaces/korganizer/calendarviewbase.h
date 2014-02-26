/*
  This file is part of the KOrganizer interfaces.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KORG_INTERFACES_CALENDARVIEWBASE_H
#define KORG_INTERFACES_CALENDARVIEWBASE_H

#include "korganizer/baseview.h"
#include <Akonadi/Calendar/ETMCalendar>

namespace KOrg {

/**
  @short interface for main calendar view widget
  @author Cornelius Schumacher
*/
class CalendarViewBase : public QWidget
{
  public:
    explicit CalendarViewBase( QWidget *parent ) : QWidget( parent ) {}
    virtual ~CalendarViewBase() {}

    virtual Akonadi::ETMCalendar::Ptr calendar() const = 0;
    virtual Akonadi::IncidenceChanger *incidenceChanger() const = 0;

    virtual QDate startDate() = 0;
    virtual QDate endDate() = 0;

    virtual Akonadi::Item currentSelection() = 0;

    virtual void addView( KOrg::BaseView * ) = 0;

    /** changes the view to be the currently selected view */
    virtual void showView( KOrg::BaseView * ) = 0;

    virtual bool editIncidence( const Akonadi::Item &item, bool isCounter = false ) = 0;

  public Q_SLOTS:
    virtual void updateView() = 0;

  signals:
    virtual void newIncidenceChanger( Akonadi::IncidenceChanger * ) = 0;

};

}

#endif
