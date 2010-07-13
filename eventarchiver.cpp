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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "eventarchiver.h"
#include <kcalprefs.h>
#include "koprefs.h"

#include <kio/netaccess.h>
#include <kcalcore/icalformat.h>
#include <kcalcore/filestorage.h>
#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/incidencechanger.h>

#include <kcalcore/calendarlocal.h>

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendaradaptor.h>
#include <akonadi/kcal/utils.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kmessagebox.h>

using namespace Akonadi;

EventArchiver::EventArchiver( QObject *parent )
 : QObject( parent )
{
}

EventArchiver::~EventArchiver()
{
}

void EventArchiver::runOnce( Akonadi::Calendar *calendar, Akonadi::IncidenceChanger* changer, const QDate &limitDate, QWidget *widget )
{
  run( calendar, changer, limitDate, widget, true, true );
}

void EventArchiver::runAuto( Akonadi::Calendar *calendar, Akonadi::IncidenceChanger* changer, QWidget *widget, bool withGUI )
{
  QDate limitDate( QDate::currentDate() );
  int expiryTime = KOPrefs::instance()->mExpiryTime;
  switch ( KOPrefs::instance()->mExpiryUnit ) {
  case KOPrefs::UnitDays: // Days
    limitDate = limitDate.addDays( -expiryTime );
    break;
  case KOPrefs::UnitWeeks: // Weeks
    limitDate = limitDate.addDays( -expiryTime * 7 );
    break;
  case KOPrefs::UnitMonths: // Months
    limitDate = limitDate.addMonths( -expiryTime );
    break;
  default:
    return;
  }
  run( calendar, changer, limitDate, widget, withGUI, false );
}

void EventArchiver::run( Akonadi::Calendar *calendar, Akonadi::IncidenceChanger* changer, const QDate &limitDate, QWidget *widget,
                         bool withGUI, bool errorIfNone )
{
  // We need to use rawEvents, otherwise events hidden by filters will not be archived.
  Item::List events;
  Item::List todos;
  Item::List journals;

  if ( KOPrefs::instance()->mArchiveEvents ) {
    events = calendar->rawEvents(
      QDate( 1769, 12, 1 ),
      // #29555, also advertised by the "limitDate not included" in the class docu
      limitDate.addDays( -1 ),
      KCalPrefs::instance()->timeSpec(),
      true );
  }
  if ( KOPrefs::instance()->mArchiveTodos ) {
    Item::List t = calendar->rawTodos();
    Item::List::ConstIterator it;
    for ( it = t.constBegin(); it != t.constEnd(); ++it ) {
      const Todo::Ptr todo = Akonadi::todo( *it );
      Q_ASSERT( todo );
      if ( isSubTreeComplete( todo.get(), limitDate ) ) {
        todos.append( *it );
      }
    }
  }

  const Item::List incidences = Akonadi::Calendar::mergeIncidenceList( events, todos, journals );

  kDebug() << "archiving incidences before" << limitDate
           << " ->" << incidences.count() <<" incidences found.";
  if ( incidences.isEmpty() ) {
    if ( withGUI && errorIfNone ) {
      KMessageBox::information( widget,
                                i18n( "There are no items before %1",
                                      KGlobal::locale()->formatDate( limitDate ) ),
                                "ArchiverNoIncidences" );
    }
    return;
  }

  switch ( KOPrefs::instance()->mArchiveAction ) {
  case KOPrefs::actionDelete:
    deleteIncidences( changer, limitDate, widget, incidences, withGUI );
    break;
  case KOPrefs::actionArchive:
    archiveIncidences( calendar, changer, limitDate, widget, incidences, withGUI );
    break;
  }
}

void EventArchiver::deleteIncidences( Akonadi::IncidenceChanger* changer, const QDate &limitDate, QWidget *widget,
                                      const Item::List &incidences, bool withGUI )
{
  QStringList incidenceStrs;
  Item::List::ConstIterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    incidenceStrs.append( Akonadi::incidence( *it )->summary() );
  }

  if ( withGUI ) {
    int result = KMessageBox::warningContinueCancelList(
      widget,
      i18n( "Delete all items before %1 without saving?\n"
            "The following items will be deleted:",
            KGlobal::locale()->formatDate( limitDate ) ),
      incidenceStrs,
      i18n( "Delete Old Items" ), KStandardGuiItem::del() );
    if ( result != KMessageBox::Continue ) {
      return;
    }
  }
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    if ( changer->isNotDeleted( ( *it ).id() ) ) {
      changer->deleteIncidence( *it, widget );
    }
  }
  emit eventsDeleted();
}

void EventArchiver::archiveIncidences( Akonadi::Calendar *calendar, Akonadi::IncidenceChanger* changer, const QDate &limitDate, QWidget *widget,
                                       const Item::List &incidences, bool withGUI )
{
  Q_UNUSED( limitDate );
  Q_UNUSED( withGUI );

  Akonadi::CalendarAdaptor cal( calendar, widget );
  FileStorage storage( &cal );

  // Save current calendar to disk
  KTemporaryFile tmpFile;
  tmpFile.open();
  storage.setFileName( tmpFile.fileName() );
  if ( !storage.save() ) {
    kDebug() << "Can't save calendar to temp file";
    return;
  }

  // Duplicate current calendar by loading in new calendar object
  CalendarLocal archiveCalendar( KCalPrefs::instance()->timeSpec() );

  FileStorage archiveStore( &archiveCalendar );
  archiveStore.setFileName( tmpFile.fileName() );
  ICalFormat *format = new ICalFormat();
  archiveStore.setSaveFormat( format );
  if ( !archiveStore.load() ) {
    kDebug() << "Can't load calendar from temp file";
    return;
  }

  // Strip active events from calendar so that only events to be archived
  // remain. This is not really efficient, but there is no other easy way.
  QStringList uids;
  Incidence::List allIncidences = archiveCalendar.rawIncidences();
  foreach(const Akonadi::Item &item, incidences) {
    uids.append( Akonadi::incidence(item)->uid() );
  }
  foreach(Incidence *inc, allIncidences) {
    if ( !uids.contains( inc->uid() ) ) {
      archiveCalendar.deleteIncidence( inc );
    }
  }

  // Get or create the archive file
  KUrl archiveURL( KOPrefs::instance()->mArchiveFile );
  QString archiveFile;

  if ( KIO::NetAccess::exists( archiveURL, KIO::NetAccess::SourceSide, widget ) ) {
    if( !KIO::NetAccess::download( archiveURL, archiveFile, widget ) ) {
      kDebug() << "Can't download archive file";
      return;
    }
    // Merge with events to be archived.
    archiveStore.setFileName( archiveFile );
    if ( !archiveStore.load() ) {
      kDebug() << "Can't merge with archive file";
      return;
    }
  } else {
    archiveFile = tmpFile.fileName();
  }

  // Save archive calendar
  if ( !archiveStore.save() ) {
    QString errmess;
    if ( format->exception() ) {
      errmess = format->exception()->message();
    } else {
      errmess = i18nc( "save failure cause unknown", "Reason unknown" );
    }
    KMessageBox::error( widget, i18n( "Cannot write archive file %1. %2",
                                      archiveStore.fileName(), errmess ) );
    return;
  }

  // Upload if necessary
  KUrl srcUrl;
  srcUrl.setPath( archiveFile );
  if ( srcUrl != archiveURL ) {
    if ( !KIO::NetAccess::upload( archiveFile, archiveURL, widget ) ) {
      KMessageBox::error( widget, i18n( "Cannot write archive. %1",
                                        KIO::NetAccess::lastErrorString() ) );
      return;
    }
  }

  KIO::NetAccess::removeTempFile( archiveFile );

  // Delete archived events from calendar
  foreach(const Akonadi::Item &item, incidences) {
    changer->deleteIncidence( item, widget );
  }
  emit eventsDeleted();
}

bool EventArchiver::isSubTreeComplete( const Todo *todo, const QDate &limitDate, QStringList checkedUids ) const
{
  if ( !todo->isCompleted() || todo->completed().date() >= limitDate ) {
    return false;
  }

  // This QList is only to prevent infinit recursion
  if ( checkedUids.contains( todo->uid() ) ) {
    // Probably will never happen, calendar.cpp checks for this
    kWarning() << "To-do hierarchy loop detected!";
    return false;
  }

  checkedUids.append( todo->uid() );

  foreach( const Incidence *i, todo->relations() ) {

    if ( i->type() == Incidence::TypeTodo ) {
      const Todo *t = static_cast<const Todo*>( i );
      if ( !isSubTreeComplete( t, limitDate, checkedUids ) ) {
        return false;
      }      
    }
  }

  return true;
}

#include "eventarchiver.moc"
