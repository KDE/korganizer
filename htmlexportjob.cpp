/*
  This file is part of the kcal library.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "htmlexportjob.h"
#include "htmlexportsettings.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/utils.h>

#include <kcalcore/calendar.h>
#include <kcalcore/event.h>
#include <kcalcore/todo.h>
#include <kcalutils/incidenceformatter.h>

#include <akonadi/contact/contactsearchjob.h>

#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QRegExp>
#include <QtCore/QMap>
#include <QtGui/QApplication>

using namespace KCalCore;
using namespace KCalUtils;
using namespace KOrg;

static QString cleanChars( const QString &txt );

//@cond PRIVATE
class KOrg::HtmlExportJob::Private
{
  public:
    Private( CalendarSupport::Calendar *calendar, KOrg::HTMLExportSettings *settings, QWidget *parent )
      : mCalendar( calendar ),
        mSettings( settings ),
        mParentWidget( parent ),
        mSubJobCount( 0 )
    {}

    CalendarSupport::Calendar *mCalendar;
    KOrg::HTMLExportSettings *mSettings;
    QWidget *mParentWidget;
    QMap<QDate,QString> mHolidayMap;
    qulonglong mSubJobCount;
    QMap<QString, KABC::Addressee> mOrganizersMap;
};
//@endcond

HtmlExportJob::HtmlExportJob( CalendarSupport::Calendar *calendar, KOrg::HTMLExportSettings *settings, QWidget *parent )
  : KJob( parent ), d( new Private( calendar, settings, parent ) )
{
}

HtmlExportJob::~HtmlExportJob()
{
  //kDebug()<<"HtmlExportJob::~HtmlExportJob()";
  delete d;
}

void HtmlExportJob::start()
{
  bool canExportItem = false;
  // first collect the email addresses of all organisators
  const Akonadi::Item::List events = d->mCalendar->events();
  foreach ( const Akonadi::Item &event, events ) {
    Q_ASSERT( event.hasPayload<Event::Ptr>() );
    const Event::Ptr eventPtr = event.payload<Event::Ptr>();
    const Attendee::List attendees = eventPtr->attendees();
    if ( !attendees.isEmpty() ) {
      Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob( this );
      job->setQuery( Akonadi::ContactSearchJob::Email, eventPtr->organizer()->email() );
      job->setProperty( "incidenceUid", eventPtr->uid() );
      connect( job, SIGNAL( result( KJob* ) ), SLOT( receivedOrganizerInfo( KJob* ) ) );
      job->start();

      d->mSubJobCount++;
      canExportItem = true;
    }
  }

  const Akonadi::Item::List todos = d->mCalendar->todos();
  foreach ( const Akonadi::Item &todo, todos ) {
    Q_ASSERT( todo.hasPayload<Todo::Ptr>() );
    const Todo::Ptr todoPtr = todo.payload<Todo::Ptr>();
    const Attendee::List attendees = todoPtr->attendees();
    if ( !attendees.isEmpty() ) {
      Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob( this );
      job->setQuery( Akonadi::ContactSearchJob::Email, todoPtr->organizer()->email() );
      job->setProperty( "incidenceUid", todoPtr->uid() );
      connect( job, SIGNAL( result( KJob* ) ), SLOT( receivedOrganizerInfo( KJob* ) ) );
      job->start();

      d->mSubJobCount++;
      canExportItem = true;
    }
  }
  if( !canExportItem ) {
    finishExport();
  }
}

void HtmlExportJob::receivedOrganizerInfo( KJob *job )
{
  d->mSubJobCount--;

  if ( !job->error() ) {
    Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
    const KABC::Addressee::List contacts = searchJob->contacts();
    if ( !contacts.isEmpty() )
      d->mOrganizersMap.insert( searchJob->property( "incidenceUid" ).toString(), contacts.first() );
  }
  if ( d->mSubJobCount == 0 )
    finishExport();
}

void HtmlExportJob::finishExport()
{
  QApplication::setOverrideCursor( QCursor ( Qt::WaitCursor ) );

  bool saveStatus;
  QString errorMessage;
  KUrl dest( d->mSettings->outputFile() );
  if ( dest.isLocalFile() ) {
    saveStatus = save( dest.toLocalFile() );
    errorMessage = i18n( "Unable to write the output file." );
  } else {
    KTemporaryFile tf;
    tf.open();
    QString tfile = tf.fileName();
    saveStatus = save( tfile );
    errorMessage = i18n( "Unable to write the temporary file for uploading." );
    if ( !KIO::NetAccess::upload( tfile, dest, d->mParentWidget ) ) {
      saveStatus = false;
      errorMessage = i18n( "Unable to upload the export file." );
    }
  }

  QApplication::restoreOverrideCursor();

  QString saveMessage;
  if ( saveStatus ) {
    saveMessage = i18n( "Web page successfully written to \"%1\"", dest.url() );
  } else {
    saveMessage = i18n( "Export failed. %1", errorMessage );
  }

  KMessageBox::information( d->mParentWidget, saveMessage,
               i18nc( "@title:window", "Export Status" ) );
  deleteLater();
}

bool HtmlExportJob::save( const QString &fileName )
{
  QString fn( fileName );
  if ( fn.isEmpty() && d->mSettings ) {
    fn = d->mSettings->outputFile();
  }
  if ( !d->mSettings || fn.isEmpty() ) {
    return false;
  }
  QFile f( fileName );
  if ( !f.open( QIODevice::WriteOnly ) ) {
    return false;
  }
  QTextStream ts( &f );
  bool success = save( &ts );
  f.close();
  return success;
}

bool HtmlExportJob::save( QTextStream *ts )
{
  if ( !d->mSettings ) {
    return false;
  }
  ts->setCodec( "UTF-8" );
  // Write HTML header
  *ts << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << endl;

  *ts << "<html><head>" << endl;
  *ts << "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=";
  *ts << "UTF-8\" />" << endl;
  if ( !d->mSettings->pageTitle().isEmpty() ) {
    *ts << "  <title>" << d->mSettings->pageTitle() << "</title>" << endl;
  }
  *ts << "  <style type=\"text/css\">" << endl;
  *ts << styleSheet();
  *ts << "  </style>" << endl;
  *ts << "</head><body>" << endl;

  // FIXME: Write header
  // (Heading, Calendar-Owner, Calendar-Date, ...)

  if ( d->mSettings->eventView() || d->mSettings->monthView() || d->mSettings->weekView() ) {
    if ( !d->mSettings->eventTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->eventTitle() << "</h1>" << endl;
    }

    // Write Week View
    if ( d->mSettings->weekView() ) {
      createWeekView( ts );
    }
    // Write Month View
    if ( d->mSettings->monthView() ) {
      createMonthView( ts );
    }
    // Write Event List
    if ( d->mSettings->eventView() ) {
      createEventList( ts );
    }
  }

  // Write Todo List
  if ( d->mSettings->todoView() ) {
    if ( !d->mSettings->todoListTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->todoListTitle() << "</h1>" << endl;
    }
    createTodoList( ts );
  }

  // Write Journals
  if ( d->mSettings->journalView() ) {
    if ( !d->mSettings->journalTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->journalTitle() << "</h1>" << endl;
    }
    createJournalView( ts );
  }

  // Write Free/Busy
  if ( d->mSettings->freeBusyView() ) {
    if ( !d->mSettings->freeBusyTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->freeBusyTitle() << "</h1>" << endl;
    }
    createFreeBusyView( ts );
  }

  createFooter( ts );

  // Write HTML trailer
  *ts << "</body></html>" << endl;

  return true;
}

void HtmlExportJob::createMonthView( QTextStream *ts )
{
  QDate start = fromDate();
  start.setYMD( start.year(), start.month(), 1 );  // go back to first day in month

  QDate end( start.year(), start.month(), start.daysInMonth() );

  int startmonth = start.month();
  int startyear = start.year();

  while ( start < toDate() ) {
    // Write header
    QDate hDate( start.year(), start.month(), 1 );
    QString hMon = hDate.toString( "MMMM" );
    QString hYear = hDate.toString( "yyyy" );
    *ts << "<h2>"
        << i18nc( "@title month and year", "%1 %2", hMon, hYear )
        << "</h2>" << endl;
    if ( KGlobal::locale()->weekStartDay() == 1 ) {
      start = start.addDays( 1 - start.dayOfWeek() );
    } else {
      if ( start.dayOfWeek() != 7 ) {
        start = start.addDays( -start.dayOfWeek() );
      }
    }
    *ts << "<table border=\"1\">" << endl;

    // Write table header
    *ts << "  <tr>";
    for ( int i=0; i < 7; ++i ) {
      *ts << "<th>" << KGlobal::locale()->calendar()->weekDayName( start.addDays(i) ) << "</th>";
    }
    *ts << "</tr>" << endl;

    // Write days
    while ( start <= end ) {
      *ts << "  <tr>" << endl;
      for ( int i=0; i < 7; ++i ) {
        *ts << "    <td valign=\"top\"><table border=\"0\">";

        *ts << "<tr><td ";
        if ( d->mHolidayMap.contains( start ) || start.dayOfWeek() == 7 ) {
          *ts << "class=\"dateholiday\"";
        } else {
          *ts << "class=\"date\"";
        }
        *ts << ">" << QString::number( start.day() );

        if ( d->mHolidayMap.contains( start ) ) {
          *ts << " <em>" << d->mHolidayMap[start] << "</em>";
        }

        *ts << "</td></tr><tr><td valign=\"top\">";

        // Only print events within the from-to range
        if ( start >= fromDate() && start <= toDate() ) {
          Akonadi::Item::List events = d->mCalendar->events( start, d->mCalendar->timeSpec(),
                                                     CalendarSupport::EventSortStartDate,
                                                     CalendarSupport::SortDirectionAscending );
          if ( events.count() ) {
            *ts << "<table>";
            foreach(const Akonadi::Item &event, events) {
              Q_ASSERT( event.hasPayload<Event::Ptr>() );
              Event::Ptr e = event.payload<Event::Ptr>();
              if ( checkSecrecy( e ) ) {
                createEvent( ts, e, start, false );
              }
            }
            *ts << "</table>";
          } else {
            *ts << "&nbsp;";
          }
        }

        *ts << "</td></tr></table></td>" << endl;
        start = start.addDays( 1 );
      }
      *ts << "  </tr>" << endl;
    }
    *ts << "</table>" << endl;
    startmonth += 1;
    if ( startmonth > 12 ) {
      startyear += 1;
      startmonth = 1;
    }
    start.setYMD( startyear, startmonth, 1 );
    end.setYMD( start.year(), start.month(), start.daysInMonth() );
  }
}

void HtmlExportJob::createEventList( QTextStream *ts )
{
  int columns = 3;
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">" << endl;
  *ts << "  <tr>" << endl;
  *ts << "    <th class=\"sum\">" << i18nc( "@title:column event start time",
                                            "Start Time" ) << "</th>" << endl;
  *ts << "    <th>" << i18nc( "@title:column event end time",
                              "End Time" ) << "</th>" << endl;
  *ts << "    <th>" << i18nc( "@title:column event description",
                              "Event" ) << "</th>" << endl;
  if ( d->mSettings->eventLocation() ) {
    *ts << "    <th>" << i18nc( "@title:column event locatin",
                                "Location" ) << "</th>" << endl;
    ++columns;
  }
  if ( d->mSettings->eventCategories() ) {
    *ts << "    <th>" << i18nc( "@title:column event categories",
                                "Categories" ) << "</th>" << endl;
    ++columns;
  }
  if ( d->mSettings->eventAttendees() ) {
    *ts << "    <th>" << i18nc( "@title:column event attendees",
                                "Attendees" ) << "</th>" << endl;
    ++columns;
  }

  *ts << "  </tr>" << endl;

  for ( QDate dt = fromDate(); dt <= toDate(); dt = dt.addDays(1) ) {
    kDebug() << "Getting events for" << dt.toString();
    Akonadi::Item::List events = d->mCalendar->events( dt, d->mCalendar->timeSpec(),
                                               CalendarSupport::EventSortStartDate,
                                               CalendarSupport::SortDirectionAscending );
    if ( events.count() ) {
      *ts << "  <tr><td colspan=\"" << QString::number( columns )
          << "\" class=\"datehead\"><i>"
          << KGlobal::locale()->formatDate( dt )
          << "</i></td></tr>" << endl;

      foreach(const Akonadi::Item &event, events) {
        Q_ASSERT( event.hasPayload<Event::Ptr>() );
        Event::Ptr e = event.payload<Event::Ptr>();
        if ( checkSecrecy( e ) ) {
          createEvent( ts, e, dt );
        }
      }
    }
  }

  *ts << "</table>" << endl;
}

void HtmlExportJob::createEvent ( QTextStream *ts, const Event::Ptr &event,
                                  QDate date, bool withDescription )
{
  kDebug() << event->summary();
  *ts << "  <tr>" << endl;

  if ( !event->allDay() ) {
    if ( event->isMultiDay( d->mCalendar->timeSpec() ) && ( event->dtStart().date() != date ) ) {
      *ts << "    <td>&nbsp;</td>" << endl;
    } else {
      *ts << "    <td valign=\"top\">"
          << IncidenceFormatter::timeToString( event->dtStart(), true, d->mCalendar->timeSpec() )
          << "</td>" << endl;
    }
    if ( event->isMultiDay( d->mCalendar->timeSpec() ) && ( event->dtEnd().date() != date ) ) {
      *ts << "    <td>&nbsp;</td>" << endl;
    } else {
      *ts << "    <td valign=\"top\">"
          << IncidenceFormatter::timeToString( event->dtEnd(), true, d->mCalendar->timeSpec() )
          << "</td>" << endl;
    }
  } else {
    *ts << "    <td>&nbsp;</td><td>&nbsp;</td>" << endl;
  }

  *ts << "    <td class=\"sum\">" << endl;
  *ts << "      <b>" << cleanChars( event->summary() ) << "</b>" << endl;
  if ( withDescription && !event->description().isEmpty() ) {
    *ts << "      <p>" << breakString( cleanChars( event->description() ) ) << "</p>" << endl;
  }
  *ts << "    </td>" << endl;

  if ( d->mSettings->eventLocation() ) {
    *ts << "  <td>" << endl;
    formatLocation( ts, event );
    *ts << "  </td>" << endl;
  }

  if ( d->mSettings->eventCategories() ) {
    *ts << "  <td>" << endl;
    formatCategories( ts, event );
    *ts << "  </td>" << endl;
  }

  if ( d->mSettings->eventAttendees() ) {
    *ts << "  <td>" << endl;
    formatAttendees( ts, event );
    *ts << "  </td>" << endl;
  }

  *ts << "  </tr>" << endl;
}

void HtmlExportJob::createTodoList ( QTextStream *ts )
{
  Akonadi::Item::List rawTodoList = d->mCalendar->todos();

  int index = 0;
  while ( index < rawTodoList.count() ) {
    const Akonadi::Item rawTodo = rawTodoList.value( index );
    Q_ASSERT( rawTodo.hasPayload<Todo::Ptr>() );
    Todo::Ptr ev = CalendarSupport::todo( rawTodo );
    const Akonadi::Item parentItem = d->mCalendar->findParent( rawTodo );

    ++index;
  }

  // FIXME: Sort list by priorities. This is brute force and should be
  // replaced by a real sorting algorithm.
  Todo::List todoList;
  Todo::List::ConstIterator it;
  for ( int i = 1; i <= 9; ++i ) {
    foreach(const Akonadi::Item &rawTodo, rawTodoList) {
      Todo::Ptr t = rawTodo.payload<Todo::Ptr>();
      if ( t->priority() == i && checkSecrecy( t ) ) {
        todoList.append( t );
      }
    }
  }
  foreach(const Akonadi::Item &rawTodo, rawTodoList) {
    Todo::Ptr t = rawTodo.payload<Todo::Ptr>();
    if ( t->priority() == 0 && checkSecrecy( t ) ) {
      todoList.append( t );
    }
  }

  int columns = 3;
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">" << endl;
  *ts << "  <tr>" << endl;
  *ts << "    <th class=\"sum\">" << i18nc( "@title:column", "To-do" ) << "</th>" << endl;
  *ts << "    <th>" << i18nc( "@title:column to-do priority", "Priority" ) << "</th>" << endl;
  *ts << "    <th>" << i18nc( "@title:column to-do percent completed",
                              "Completed" ) << "</th>" << endl;
  if ( d->mSettings->taskDueDate() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do due date", "Due Date" ) << "</th>" << endl;
    ++columns;
  }
  if ( d->mSettings->taskLocation() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do location", "Location" ) << "</th>" << endl;
    ++columns;
  }
  if ( d->mSettings->taskCategories() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do categories", "Categories" ) << "</th>" << endl;
    ++columns;
  }
  if ( d->mSettings->taskAttendees() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do attendees", "Attendees" ) << "</th>" << endl;
    ++columns;
  }
  *ts << "  </tr>" << endl;

  // Create top-level list.
  for ( it = todoList.constBegin(); it != todoList.constEnd(); ++it ) {
    const QString parentUid = (*it)->relatedTo();
    if ( parentUid.isEmpty() ) {
      createTodo( ts, *it );
    }
  }

  //REVIEW(AKONADI_PORT) relations/relatedTo usage: ok right now, as relations should yield the same result as mCalendar->findChildren and items are not needed here
  // Create sub-level lists
  for ( it = todoList.constBegin(); it != todoList.constEnd(); ++it ) {

    Akonadi::Item item = d->mCalendar->itemForIncidenceUid( ( *it )->uid() );
    const Akonadi::Item::List relations = d->mCalendar->findChildren( item );

    if ( !relations.isEmpty() ) {
      // Generate sub-to-do list
      *ts << "  <tr>" << endl;
      *ts << "    <td class=\"subhead\" colspan=";
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><a name=\"sub" << (*it)->uid() << "\"></a>"
          << i18nc( "@title:column sub-to-dos of the parent to-do",
                    "Sub-To-dos of: " ) << "<a href=\"#"
          << (*it)->uid() << "\"><b>" << cleanChars( (*it)->summary() )
          << "</b></a></td>" << endl;
      *ts << "  </tr>" << endl;

      Todo::List sortedList;
      // FIXME: Sort list by priorities. This is brute force and should be
      // replaced by a real sorting algorithm.
      for ( int i = 1; i <= 9; ++i ) {
        foreach( const Akonadi::Item &item, relations ) {
          Todo::Ptr ev3 = CalendarSupport::todo( item );
          if ( ev3 && ev3->priority() == i ) {
            sortedList.append( ev3 );
          }
        }
      }

      foreach( const Akonadi::Item &item, relations ) {
        Todo::Ptr ev3 = CalendarSupport::todo( item );
        if ( ev3 && ev3->priority() == 0 ) {
          sortedList.append( ev3 );
        }
      }

      Todo::List::ConstIterator it3;
      for ( it3 = sortedList.constBegin(); it3 != sortedList.constEnd(); ++it3 ) {
        createTodo( ts, *it3 );
      }
    }
  }

  *ts << "</table>" << endl;
}

void HtmlExportJob::createTodo( QTextStream *ts, const Todo::Ptr &todo )
{
  kDebug();

  const bool completed = todo->isCompleted();

  Akonadi::Item it = d->mCalendar->itemForIncidenceUid( todo->uid() );
  Akonadi::Item::List relations = d->mCalendar->findChildren( it );

  *ts << "<tr>" << endl;

  *ts << "  <td class=\"sum";
  if (completed) *ts << "done";
  *ts << "\">" << endl;
  *ts << "    <a name=\"" << todo->uid() << "\"></a>" << endl;
  *ts << "    <b>" << cleanChars( todo->summary() ) << "</b>" << endl;
  if ( !todo->description().isEmpty() ) {
    *ts << "    <p>" << breakString( cleanChars( todo->description() ) ) << "</p>" << endl;
  }
  if ( relations.count() ) {
    *ts << "    <div align=\"right\"><a href=\"#sub" << todo->uid()
        << "\">" << i18nc( "@title:column sub-to-dos of the parent to-do",
                           "Sub-To-dos" ) << "</a></div>" << endl;
  }
  *ts << "  </td>" << endl;

  *ts << "  <td";
  if ( completed ) {
    *ts << " class=\"done\"";
  }
  *ts << ">" << endl;
  *ts << "    " << todo->priority() << endl;
  *ts << "  </td>" << endl;

  *ts << "  <td";
  if ( completed ) {
    *ts << " class=\"done\"";
  }
  *ts << ">" << endl;
  *ts << "    " << i18nc( "@info/plain to-do percent complete",
                          "%1 %", todo->percentComplete() ) << endl;
  *ts << "  </td>" << endl;

  if ( d->mSettings->taskDueDate() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">" << endl;
    if ( todo->hasDueDate() ) {
      *ts << "    " << IncidenceFormatter::dateToString( todo->dtDue( true ) ) << endl;
    } else {
      *ts << "    &nbsp;" << endl;
    }
    *ts << "  </td>" << endl;
  }

  if ( d->mSettings->taskLocation() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">" << endl;
    formatLocation( ts, todo );
    *ts << "  </td>" << endl;
  }

  if ( d->mSettings->taskCategories() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">" << endl;
    formatCategories( ts, todo );
    *ts << "  </td>" << endl;
  }

  if ( d->mSettings->taskAttendees() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">" << endl;
    formatAttendees( ts, todo );
    *ts << "  </td>" << endl;
  }

  *ts << "</tr>" << endl;
}

void HtmlExportJob::createWeekView( QTextStream *ts )
{
  Q_UNUSED( ts );
  // FIXME: Implement this!
}

void HtmlExportJob::createJournalView( QTextStream *ts )
{
  Q_UNUSED( ts );
//   Journal::List rawJournalList = d->mCalendar->journals();
  // FIXME: Implement this!
}

void HtmlExportJob::createFreeBusyView( QTextStream *ts )
{
  Q_UNUSED( ts );
  // FIXME: Implement this!
}

bool HtmlExportJob::checkSecrecy( const Incidence::Ptr &incidence )
{
  int secrecy = incidence->secrecy();
  if ( secrecy == Incidence::SecrecyPublic ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyPrivate && !d->mSettings->excludePrivate() ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyConfidential &&
       !d->mSettings->excludeConfidential() ) {
    return true;
  }
  return false;
}

void HtmlExportJob::formatLocation( QTextStream *ts, const Incidence::Ptr &incidence )
{
  if ( !incidence->location().isEmpty() ) {
    *ts << "    " << cleanChars( incidence->location() ) << endl;
  } else {
    *ts << "    &nbsp;" << endl;
  }
}

void HtmlExportJob::formatCategories( QTextStream *ts, const Incidence::Ptr &incidence )
{
  if ( !incidence->categoriesStr().isEmpty() ) {
    *ts << "    " << cleanChars( incidence->categoriesStr() ) << endl;
  } else {
    *ts << "    &nbsp;" << endl;
  }
}

void HtmlExportJob::formatAttendees( QTextStream *ts, const Incidence::Ptr &incidence )
{
  Attendee::List attendees = incidence->attendees();
  if ( attendees.count() ) {
    *ts << "<em>";
    const KABC::Addressee organizer = d->mOrganizersMap.value( incidence->uid() );
    if ( !organizer.isEmpty() ) {
      *ts << "<a href=\"mailto:" << incidence->organizer()->email() << "\">";
      *ts << cleanChars( organizer.formattedName() ) << "</a>" << endl;
    } else {
      *ts << incidence->organizer()->fullName();
    }
    *ts << "</em><br />";
    Attendee::List::ConstIterator it;
    for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
      Attendee::Ptr a = *it;
      if ( !a->email().isEmpty() ) {
        *ts << "<a href=\"mailto:" << a->email();
        *ts << "\">" << cleanChars( a->name() ) << "</a>";
      } else {
        *ts << "    " << cleanChars( a->name() );
      }
      *ts << "<br />" << endl;
    }
  } else {
    *ts << "    &nbsp;" << endl;
  }
}

QString HtmlExportJob::breakString( const QString &text )
{
  int number = text.count( '\n' );
  if ( number <= 0 ) {
    return text;
  } else {
    QString out;
    QString tmpText = text;
    int pos = 0;
    QString tmp;
    for ( int i = 0; i <= number; ++i ) {
      pos = tmpText.indexOf( '\n' );
      tmp = tmpText.left( pos );
      tmpText = tmpText.right( tmpText.length() - pos - 1 );
      out += tmp + "<br />";
    }
    return out;
  }
}

void HtmlExportJob::createFooter( QTextStream *ts )
{
  // FIXME: Implement this in a translatable way!
  QString trailer = i18nc( "@info/plain", "This page was created " );

/*  bool hasPerson = false;
  bool hasCredit = false;
  bool hasCreditURL = false;
  QString mail, name, credit, creditURL;*/
  if ( !d->mSettings->eMail().isEmpty() ) {
    if ( !d->mSettings->name().isEmpty() ) {
      trailer += i18nc( "@info/plain page creator email link with name",
                        "by <link url='mailto:%1'>%2</link> ",
                        d->mSettings->eMail(), d->mSettings->name() );
    } else {
      trailer += i18nc( "@info/plain page creator email link",
                        "by <link url='mailto:%1'>%2</link> ",
                        d->mSettings->eMail(), d->mSettings->eMail() );
    }
  } else {
    if ( !d->mSettings->name().isEmpty() ) {
      trailer += i18nc( "@info/plain page creator name only",
                        "by %1 ", d->mSettings->name() );
    }
  }
  if ( !d->mSettings->creditName().isEmpty() ) {
    if ( !d->mSettings->creditURL().isEmpty() ) {
      trailer += i18nc( "@info/plain page credit with name and link",
                        "with <link url='%1'>%2</link>",
                        d->mSettings->creditURL(), d->mSettings->creditName() );
    } else {
      trailer += i18nc( "@info/plain page credit name only",
                        "with %1", d->mSettings->creditName() );
    }
  }
  *ts << "<p>" << trailer << "</p>" << endl;
}

QString cleanChars( const QString &text )
{
  QString txt = text;
  txt = txt.replace( '&', "&amp;" );
  txt = txt.replace( '<', "&lt;" );
  txt = txt.replace( '>', "&gt;" );
  txt = txt.replace( '\"', "&quot;" );
  txt = txt.replace( QString::fromUtf8( "ä" ), "&auml;" );
  txt = txt.replace( QString::fromUtf8( "Ä" ), "&Auml;" );
  txt = txt.replace( QString::fromUtf8( "ö" ), "&ouml;" );
  txt = txt.replace( QString::fromUtf8( "Ö" ), "&Ouml;" );
  txt = txt.replace( QString::fromUtf8( "ü" ), "&uuml;" );
  txt = txt.replace( QString::fromUtf8( "Ü" ), "&Uuml;" );
  txt = txt.replace( QString::fromUtf8( "ß" ), "&szlig;" );
  txt = txt.replace( QString::fromUtf8( "€" ), "&euro;" );
  txt = txt.replace( QString::fromUtf8( "é" ), "&eacute;" );

  return txt;
}

QString HtmlExportJob::styleSheet() const
{
  if ( !d->mSettings->styleSheet().isEmpty() ) {
    return d->mSettings->styleSheet();
  }

  QString css;

  if ( QApplication::isRightToLeft() ) {
    css += "    body { background-color:white; color:black; direction: rtl }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sumdone { background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.dateholiday { color:red }\n";
  } else {
    css += "    body { background-color:white; color:black }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sum { text-align:left }\n";
    css += "    td.sumdone { text-align:left; background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.date { text-align:left }\n";
    css += "    td.dateholiday { text-align:left; color:red }\n";
  }

  return css;
}

void HtmlExportJob::addHoliday( const QDate &date, const QString &name )
{
  if ( d->mHolidayMap[date].isEmpty() ) {
    d->mHolidayMap[date] = name;
  } else {
    d->mHolidayMap[date] = i18nc( "@info/plain holiday by date and name",
                                  "%1, %2", d->mHolidayMap[date], name );
  }
}

QDate HtmlExportJob::fromDate() const
{
  return d->mSettings->dateStart().date();
}

QDate HtmlExportJob::toDate() const
{
  return d->mSettings->dateEnd().date();
}

#include "htmlexportjob.moc"
