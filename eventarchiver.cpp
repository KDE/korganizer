/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 David Faure <faure@kde.org>

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

#include "eventarchiver.h"
#include <kglobal.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kglobal.h>
#include <libkcal/filestorage.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include "koprefs.h"

EventArchiver::EventArchiver( QObject* parent, const char* name )
 : QObject( parent, name )
{
}

EventArchiver::~EventArchiver()
{
}

void EventArchiver::runOnce( Calendar* calendar, const QDate& limitDate, QWidget* widget )
{
  run( calendar, limitDate, widget, true, true );
}

void EventArchiver::runAuto( Calendar* calendar, QWidget* widget, bool withGUI )
{
  QDate limitDate( QDate::currentDate() );
  int expiryTime = KOPrefs::instance()->mExpiryTime;
  switch (KOPrefs::instance()->mExpiryUnit) {
  case KOPrefs::UnitDays: // Days
    limitDate = limitDate.addDays( -expiryTime );
    break;
  case KOPrefs::UnitWeeks: // Weeks
    limitDate = limitDate.addDays( -expiryTime*7 );
    break;
  case KOPrefs::UnitMonths: // Months
    limitDate = limitDate.addMonths( -expiryTime );
    break;
  default:
    return;
  }
  run( calendar, limitDate, widget, withGUI, false );
}

void EventArchiver::run( Calendar* calendar, const QDate& limitDate, QWidget* widget, bool withGUI, bool errorIfNone )
{
  Event::List events = calendar->events(
    QDate( 1769, 12, 1 ),
    // #29555, also advertised by the "limitDate not included" in the class docu
    limitDate.addDays( -1 ),
    true );

  kdDebug(5850) << "EventArchiver: archiving events before " << limitDate << " -> " << events.count() << " events found." << endl;
  if ( events.isEmpty() ) {
    if ( withGUI && errorIfNone )
      KMessageBox::sorry(widget, i18n("There are no events before %1")
                         .arg(KGlobal::locale()->formatDate(limitDate)));
    return;
  }


  switch ( KOPrefs::instance()->mArchiveAction ) {
  case KOPrefs::actionDelete:
    deleteEvents( calendar, limitDate, widget, events, withGUI );
    break;
  case KOPrefs::actionArchive:
    archiveEvents( calendar, limitDate, widget, events, withGUI );
    break;
  }
}

void EventArchiver::deleteEvents( Calendar* calendar, const QDate& limitDate, QWidget* widget, const Event::List& events, bool withGUI )
{
  QStringList eventStrs;
  Event::List::ConstIterator it;
  for( it = events.begin(); it != events.end(); ++it ) {
    eventStrs.append( (*it)->summary() );
  }

  if ( withGUI ) {
    int result = KMessageBox::warningContinueCancelList(
      widget, i18n("Delete all events before %1 without saving?\n"
                 "The following events will be deleted:")
      .arg(KGlobal::locale()->formatDate(limitDate)),eventStrs,
      i18n("Delete Old Events"),i18n("&Delete"));
    if (result != KMessageBox::Continue)
      return;
  }
  for( it = events.begin(); it != events.end(); ++it ) {
    calendar->deleteEvent( *it );
  }
  emit eventsDeleted();
}

void EventArchiver::archiveEvents( Calendar* calendar, const QDate& limitDate, QWidget* widget, const Event::List& events, bool /*withGUI*/)
{
  FileStorage storage( calendar );

  // Save current calendar to disk
  KTempFile tmpFile;
  tmpFile.setAutoDelete(true);
  storage.setFileName( tmpFile.name() );
  if ( !storage.save() ) {
    kdDebug(5850) << "EventArchiver::archiveEvents(): Can't save calendar to temp file" << endl;
    return;
  }

  // Duplicate current calendar by loading in new calendar object
  CalendarLocal archiveCalendar( KOPrefs::instance()->mTimeZoneId );

  FileStorage archiveStore( &archiveCalendar );
  archiveStore.setFileName( tmpFile.name() );
  if (!archiveStore.load()) {
    kdDebug(5850) << "EventArchiver::archiveEvents(): Can't load calendar from temp file" << endl;
    return;
  }

  // Strip active events from calendar so that only events to be archived
  // remain.
  Event::List activeEvents = archiveCalendar.events( limitDate,
                                                     QDate( 3000, 1, 1 ),
                                                     false );
  Event::List::ConstIterator it;
  for( it = activeEvents.begin(); it != activeEvents.end(); ++it ) {
    archiveCalendar.deleteEvent( *it );
  }

  // Get or create the archive file
  KURL archiveURL( KOPrefs::instance()->mArchiveFile );
  QString archiveFile;

  if ( KIO::NetAccess::exists( archiveURL, true, widget ) ) {
    if( !KIO::NetAccess::download( archiveURL, archiveFile, widget ) ) {
      kdDebug(5850) << "EventArchiver::archiveEvents(): Can't download archive file" << endl;
      return;
    }
    // Merge with events to be archived.
    archiveStore.setFileName( archiveFile );
    if ( !archiveStore.load() ) {
      kdDebug(5850) << "EventArchiver::archiveEvents(): Can't merge with archive file" << endl;
      return;
    }
/*
    QPtrList<Event> es = archiveCalendar.events(QDate(1800,1,1),
                                                QDate(3000,1,1),
                                                false);
    kdDebug(5850) << "--Following events in archive calendar:" << endl;
    Event *e;
    for(e=es.first();e;e=es.next()) {
      kdDebug(5850) << "-----Event: " << e->getSummary() << endl;
    }
*/
  } else {
    archiveFile = tmpFile.name();
  }

  // Save archive calendar
  if ( !archiveStore.save() ) {
    KMessageBox::error(widget,i18n("Cannot write archive file %1.").arg( archiveStore.fileName() ));
    return;
  }

  // Upload if necessary
  KURL srcUrl;
  srcUrl.setPath(archiveFile);
  if (srcUrl != archiveURL) {
    if ( !KIO::NetAccess::upload( archiveFile, archiveURL, widget ) ) {
      KMessageBox::error(widget,i18n("Cannot write archive to final destination."));
      return;
    }
  }

  KIO::NetAccess::removeTempFile(archiveFile);

  // Delete archived events from calendar
  for( it = events.begin(); it != events.end(); ++it ) {
    calendar->deleteEvent( *it );
  }
  emit eventsDeleted();
}

#include "eventarchiver.moc"
