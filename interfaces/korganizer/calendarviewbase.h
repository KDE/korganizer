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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KORG_CALENDARVIEWBASE_H
#define KORG_CALENDARVIEWBASE_H

#include <qwidget.h>

#include <libkcal/calendar.h>

#include <korganizer/baseview.h>

namespace KOrg {

/**
  @short interface for main calendar view widget
  @author Cornelius Schumacher
*/
class CalendarViewBase : public QWidget
{
  public:
    CalendarViewBase( QWidget *parent, const char *name )
      : QWidget( parent, name ) {}
    virtual ~CalendarViewBase() {}
  
    virtual KCal::Calendar *calendar() = 0;

    virtual QDate startDate() = 0;
    virtual QDate endDate() = 0;

    virtual Incidence *currentSelection() = 0;

    virtual void addView( KOrg::BaseView * ) = 0;

    /** changes the view to be the currently selected view */
    virtual void showView( KOrg::BaseView * ) = 0;

  public slots:
    virtual void updateView() = 0;
    virtual void updateCategories() = 0;
};

}

#endif
