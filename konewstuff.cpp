/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kapplication.h>
#include <kdebug.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/filestorage.h>

#include "koprefs.h"
#include "calendarview.h"

#include "konewstuff.h"

KONewStuff::KONewStuff( CalendarView *view ) :
  KNewStuff( "korganizer/calendar", view ),
  mView( view )
{
}

bool KONewStuff::install( const QString &fileName )
{
  kdDebug(5850) << "KONewStuff::install(): " << fileName << endl;

  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  FileStorage storage( &cal, fileName );
  if ( !storage.load() ) {
    KMessageBox::error( mView, i18n("Could not load calendar.") );
    return false;
  }

  Event::List events = cal.events();

  QStringList eventList;

  Event::List::ConstIterator it;
  for( it = events.begin(); it != events.end(); ++it ) {
    QString text = (*it)->summary();
    eventList.append( text );
  }

  int result = KMessageBox::warningContinueCancelList( mView,
    i18n("The downloaded events will be merged into your current calendar."),
    eventList );

  if ( result != KMessageBox::Continue ) return false;

  return mView->openCalendar( fileName, true );
}

bool KONewStuff::createUploadFile( const QString &fileName )
{
  return mView->saveCalendar( fileName );
}
