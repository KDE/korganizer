/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOCORE_H
#define KOCORE_H
// $Id$

#include <ktrader.h>

#include <calendar/calendardecoration.h>
#include <korganizer/part.h>

class KOCore {
  public:
    static KOCore *self();

    KTrader::OfferList availablePlugins(const QString &type);
  
    KOrg::Plugin *loadPlugin(KService::Ptr service);
    KOrg::Plugin *loadPlugin(const QString &);
    
    KOrg::CalendarDecoration *loadCalendarDecoration(KService::Ptr service);
    KOrg::CalendarDecoration *loadCalendarDecoration(const QString &);

    KOrg::Part *loadPart(KService::Ptr,KOrg::MainWindow *parent);
    KOrg::Part *loadPart(const QString &,KOrg::MainWindow *parent);

    KOrg::CalendarDecoration::List calendarDecorations();
    KOrg::Part::List loadParts(KOrg::MainWindow *parent);

    void reloadPlugins();

    QString holiday( const QDate & );

  protected:
    KOCore();
    
  private:
    static KOCore *mSelf;
    
    KOrg::CalendarDecoration::List mCalendarDecorations;
    bool mCalendarDecorationsLoaded;
    
    KOrg::Part::List mParts;

    KOrg::CalendarDecoration *mHolidays;
    bool mHolidaysLoaded;    
};

#endif
