/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 David Faure <faure@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
  // We need to use rawEvents, otherwise events hidden by filters will not be archived.
  Incidence::List incidences;
  Event::List events;
  Todo::List todos;
  Journal::List journals;
  
  if ( KOPrefs::instance()->mArchiveEvents ) {
    events = calendar->rawEvents(
      QDate( 1769, 12, 1 ),
      // #29555, also advertised by the "limitDate not included" in the class docu
      limitDate.addDays( -1 ),
      true );
  }
  if ( KOPrefs::instance()->mArchiveTodos ) {
    Todo::List t = calendar->rawTodos();
    Todo::List::ConstIterator it;
    for( it = t.begin(); it != t.end(); ++it ) {
      if ( (*it) && ( (*it)->isCompleted() ) &&  ( (*it)->completed().date() < limitDate ) ) {
        todos.append( *it );
      }
    }
  }
  
  incidences = Calendar::mergeIncidenceList( events, todos, journals );
  

  kdDebug(5850) << "EventArchiver: archiving incidences before " << limitDate << " -> " << incidences.count() << " incidences found." << endl;
  if ( incidences.isEmpty() ) {
    if ( withGUI && errorIfNone )
      KMessageBox::information( widget, i18n("There are no incidences before %1")
                          .arg(KGlobal::locale()->formatDate(limitDate)),
                          "ArchiverNoIncidences" );
    return;
  }


  switch ( KOPrefs::instance()->mArchiveAction ) {
  case KOPrefs::actionDelete:
    deleteIncidences( calendar, limitDate, widget, incidences, withGUI );
    break;
  case KOPrefs::actionArchive:
    archiveIncidences( calendar, limitDate, widget, incidences, withGUI );
    break;
  }
}

void EventArchiver::deleteIncidences( Calendar* calendar, const QDate& limitDate, QWidget* widget, const Incidence::List& incidences, bool withGUI )
{
  QStringList incidenceStrs;
  Incidence::List::ConstIterator it;
  for( it = incidences.begin(); it != incidences.end(); ++it ) {
    incidenceStrs.append( (*it)->summary() );
  }

  if ( withGUI ) {
    int result = KMessageBox::warningContinueCancelList(
      widget, i18n("Delete all incidences before %1 without saving?\n"
                 "The following incidences will be deleted:")
      .arg(KGlobal::locale()->formatDate(limitDate)), incidenceStrs,
		 i18n("Delete Old Incidences"),KStdGuiItem::del());
    if (result != KMessageBox::Continue)
      return;
  }
  for( it = incidences.begin(); it != incidences.end(); ++it ) {
    calendar->deleteIncidence( *it );
  }
  emit eventsDeleted();
}

void EventArchiver::archiveIncidences( Calendar* calendar, const QDate& /*limitDate*/, QWidget* widget, const Incidence::List& incidences, bool /*withGUI*/)
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
  // remain. This is not really efficient, but there is no other easy way.
  QStringList uids;
  Incidence::List allIncidences = archiveCalendar.rawIncidences();
  Incidence::List::ConstIterator it;
  for( it = incidences.begin(); it != incidences.end(); ++it ) {
    uids << (*it)->uid();
  }
  for( it = allIncidences.begin(); it != allIncidences.end(); ++it ) {
    if ( !uids.contains( (*it)->uid() ) ) {
      archiveCalendar.deleteIncidence( *it );
    }
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
  for( it = incidences.begin(); it != incidences.end(); ++it ) {
    calendar->deleteIncidence( *it );
  }
  emit eventsDeleted();
}

#include "eventarchiver.moc"
