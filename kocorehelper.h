/*
    This file is part of KOrganizer.
    Copyright (c) 1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOCOREHELPER_H
#define _KOCOREHELPER_H


#include "korganizer/corehelper.h"
#include "koprefs.h"
#include "koglobals.h"
#include "kocore.h"

class KCalendarSystem;

class KOCoreHelper : public KOrg::CoreHelper
{
  public:
    KOCoreHelper() {}
    virtual ~KOCoreHelper() {}

    virtual QColor defaultEventColor() { return KOPrefs::instance()->mEventColor; }
    virtual QColor textColor( const QColor &bgColor ) { return getTextColor( bgColor ); }
    virtual QColor categoryColor( const QStringList &cats );
    virtual QString holidayString( const QDate &dt );
    virtual QTime dayStart() { return KOPrefs::instance()->mDayBegins.time(); }
    virtual const KCalendarSystem *calendarSystem() { return KOGlobals::self()->calendarSystem(); }
    virtual KOrg::PrintPlugin::List loadPrintPlugins() { return KOCore::self()->loadPrintPlugins(); }
};

#endif
