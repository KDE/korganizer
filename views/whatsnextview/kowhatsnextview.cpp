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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kowhatsnextview.h"
#include "koglobals.h"
#include "koprefs.h"

#include <KCal/Calendar>
#include <KCal/IncidenceFormatter>
#include <KCal/Todo>

#include <QBoxLayout>

void WhatsNextTextBrowser::setSource( const QUrl &name )
{
  QString uri = name.toString();
  if ( uri.startsWith( QLatin1String( "event:" ) ) ) {
    emit showIncidence( uri );
  } else if ( uri.startsWith( QLatin1String( "todo:" ) ) ) {
    emit showIncidence( uri );
  } else {
    KTextBrowser::setSource( uri );
  }
}

KOWhatsNextView::KOWhatsNextView( Calendar *calendar, QWidget *parent )
  : KOrg::BaseView( calendar, parent )
{
  mView = new WhatsNextTextBrowser( this );
  connect( mView, SIGNAL(showIncidence(const QString &)),
           SLOT(showIncidence(const QString &)) );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addWidget(mView);
}

KOWhatsNextView::~KOWhatsNextView()
{
}

int KOWhatsNextView::currentDateCount()
{
  return mStartDate.daysTo( mEndDate );
}

void KOWhatsNextView::updateView()
{
  KIconLoader kil( "korganizer" );
  QString *ipath = new QString();
  kil.loadIcon( "office-calendar", KIconLoader::NoGroup, 32,
                KIconLoader::DefaultState, QStringList(), ipath );

  mText = "<table width=\"100%\">\n";
  mText += "<tr bgcolor=\"#3679AD\"><td><h1>";
  mText += "<img src=\"";
  mText += *ipath;
  mText += "\">";
  mText += "<font color=\"white\"> ";
  mText += i18n( "What's Next?" ) + "</font></h1>";
  mText += "</td></tr>\n<tr><td>";

  mText += "<h2>";
  if ( mStartDate.daysTo( mEndDate ) < 1 ) {
    mText += KGlobal::locale()->formatDate( mStartDate );
  } else {
    mText += i18nc(
      "date from - to", "%1 - %2",
      KGlobal::locale()->formatDate( mStartDate ),
      KGlobal::locale()->formatDate( mEndDate ) );
  }
  mText+="</h2>\n";

  Event::List events;
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();

  events = calendar()->events( mStartDate, mEndDate, timeSpec, false );
  calendar()->sortEvents( &events, EventSortStartDate, SortDirectionAscending );

  if ( events.count() > 0 ) {
    mText += "<p></p>";
    kil.loadIcon( "view-calendar-day", KIconLoader::NoGroup, 22,
                  KIconLoader::DefaultState, QStringList(), ipath );
    mText += "<h2><img src=\"";
    mText += *ipath;
    mText += "\">";
    mText += i18n( "Events:" ) + "</h2>\n";
    mText += "<table>\n";
    Event::List::ConstIterator it;
    for ( it = events.constBegin(); it != events.constEnd(); ++it ) {
      Event *ev = *it;
      if ( !ev->recurs() ) {
        appendEvent( ev );
      } else {
        Recurrence *recur = ev->recurrence();
        int duration = ev->dtStart().secsTo( ev->dtEnd() );
        KDateTime start = recur->getPreviousDateTime( KDateTime( mStartDate, QTime(), timeSpec ) );
        KDateTime end = start.addSecs( duration );
        KDateTime endDate( mEndDate, QTime( 23, 59, 59 ), timeSpec );
        if ( end.date() >= mStartDate ) {
          appendEvent( ev, start.dateTime(), end.dateTime() );
        }
        DateTimeList times = recur->timesInInterval( start, endDate );
        int count = times.count();
        if ( count > 0 ) {
          int i = 0;
          if ( times[0] == start ) {
            ++i;  // start has already been appended
          }
          if ( !times[count - 1].isValid() ) {
            --count;  // list overflow
          }
          for ( ;  i < count && times[i].date() <= mEndDate;  ++i ) {
            appendEvent( ev, times[i].dateTime() );
          }
        }
      }
    }
    mText += "</table>\n";
  }

  mTodos.clear();
  Todo::List todos = calendar()->todos( TodoSortDueDate, SortDirectionAscending );
  if ( todos.count() > 0 ) {
    kil.loadIcon( "view-calendar-tasks", KIconLoader::NoGroup, 22,
                  KIconLoader::DefaultState, QStringList(), ipath );
    mText += "<h2><img src=\"";
    mText += *ipath;
    mText += "\">";
    mText += i18n( "To-do:" ) + "</h2>\n";
    mText += "<ul>\n";
    Todo::List::ConstIterator it;
    for ( it = todos.constBegin(); it != todos.constEnd(); ++it ) {
      Todo *todo = *it;
      if ( !todo->isCompleted() && todo->hasDueDate() && todo->dtDue().date() <= mEndDate ) {
        appendTodo( todo );
      }
    }
    bool gotone = false;
    int priority = 1;
    while ( !gotone && priority <= 9 ) {
      for ( it = todos.constBegin(); it != todos.constEnd(); ++it ) {
        Todo *todo = *it;
        if ( !todo->isCompleted() && ( todo->priority() == priority ) ) {
          appendTodo( todo );
          gotone = true;
        }
      }
      priority++;
    }
    mText += "</ul>\n";
  }

  QStringList myEmails( KOPrefs::instance()->allEmails() );
  int replies = 0;
  events = calendar()->events( QDate::currentDate(), QDate( 2975, 12, 6 ), timeSpec );
  Event::List::ConstIterator it2;
  for ( it2 = events.constBegin(); it2 != events.constEnd(); ++it2 ) {
    Event *ev = *it2;
    Attendee *me = ev->attendeeByMails( myEmails );
    if ( me != 0 ) {
      if ( me->status() == Attendee::NeedsAction && me->RSVP() ) {
        if ( replies == 0 ) {
          mText += "<p></p>";
          kil.loadIcon( "mail-reply", KIconLoader::NoGroup, 22,
                        KIconLoader::DefaultState, QStringList(), ipath );
          mText += "<h2><img src=\"";
          mText += *ipath;
          mText += "\">";
          mText += i18n( "Events and to-dos that need a reply:" ) + "</h2>\n";
          mText += "<table>\n";
        }
        replies++;
        appendEvent( ev );
      }
    }
  }
  todos = calendar()->todos();
  Todo::List::ConstIterator it3;
  for ( it3 = todos.constBegin(); it3 != todos.constEnd(); ++it3 ) {
    Todo *to = *it3;
    Attendee *me = to->attendeeByMails( myEmails );
    if ( me != 0 ) {
      if ( me->status() == Attendee::NeedsAction && me->RSVP() ) {
        if ( replies == 0 ) {
          mText += "<p></p>";
          kil.loadIcon( "mail-reply", KIconLoader::NoGroup, 22,
                        KIconLoader::DefaultState, QStringList(), ipath );
          mText += "<h2><img src=\"";
          mText += *ipath;
          mText += "\">";
          mText += i18n( "Events and to-dos that need a reply:" ) + "</h2>\n";
          mText += "<table>\n";
        }
        replies++;
        appendEvent( to );
      }
    }
  }
  if ( replies > 0 ) {
    mText += "</table>\n";
  }

  mText += "</td></tr>\n</table>\n";

  delete ipath;

  mView->setText(mText);
}

void KOWhatsNextView::showDates( const QDate &start, const QDate &end )
{
  mStartDate = start;
  mEndDate = end;
  updateView();
}

void KOWhatsNextView::showIncidences( const Incidence::List & )
{
}

void KOWhatsNextView::changeIncidenceDisplay( Incidence *, int action )
{
  switch( action ) {
  case KOGlobals::INCIDENCEADDED:
  case KOGlobals::INCIDENCEEDITED:
  case KOGlobals::INCIDENCEDELETED:
    updateView();
    break;
  default:
    break;
  }
}

void KOWhatsNextView::appendEvent( Incidence *ev, const QDateTime &start,
                                   const QDateTime &end )
{

  mText += "<tr><td><b>";
  if ( ev->type() == "Event" ) {
    Event *event = static_cast<Event *>(ev);
    KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
    KDateTime starttime( start, timeSpec );
    if ( !starttime.isValid() ) {
      starttime = event->dtStart();
    }
    KDateTime endtime( end, timeSpec );
    if ( !endtime.isValid() ) {
      endtime = starttime.addSecs( event->dtStart().secsTo( event->dtEnd() ) );
    }

    if ( starttime.date().daysTo( endtime.date() ) >= 1 ) {
      mText += i18nc(
        "date from - to", "%1 - %2",
        KGlobal::locale()->formatDateTime(
          starttime.toTimeSpec( KOPrefs::instance()->timeSpec() ) ),
        KGlobal::locale()->formatDateTime(
          endtime.toTimeSpec( KOPrefs::instance()->timeSpec() ) ) );
    } else {
      mText += i18nc(
        "date, from - to", "%1, %2 - %3",
        KGlobal::locale()->formatDate(
          starttime.toTimeSpec( KOPrefs::instance()->timeSpec() ).date(), KLocale::ShortDate ),
        KGlobal::locale()->formatTime(
          starttime.toTimeSpec( KOPrefs::instance()->timeSpec() ).time() ),
        KGlobal::locale()->formatTime(
          endtime.toTimeSpec( KOPrefs::instance()->timeSpec() ).time() ) );
    }
  }
  mText += "</b></td><td><a ";
  if ( ev->type() == "Event" ) {
    mText += "href=\"event:";
  }
  if ( ev->type() == "Todo" ) {
    mText += "href=\"todo:";
  }
  mText += ev->uid() + "\">";
  mText += ev->summary();
  mText += "</a></td></tr>\n";
}

void KOWhatsNextView::appendTodo( Incidence *incidence )
{
  if ( mTodos.contains( incidence ) ) {
    return;
  }

  mTodos.append( incidence );

  mText += "<li><a href=\"todo:" + incidence->uid() + "\">";
  mText += incidence->summary();
  mText += "</a>";

  if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>( incidence );
    if ( todo->hasDueDate() ) {
      mText += i18nc( "to-do due date", "  (Due: %1)",
                      IncidenceFormatter::dateTimeToString( todo->dtDue(), todo->allDay() ) );
    }
  }
  mText += "</li>\n";
}

void KOWhatsNextView::showIncidence( const QString &uid )
{
  Incidence *incidence = 0;

  if ( uid.startsWith( QLatin1String( "event:" ) ) ) {
    incidence = calendar()->incidence( uid.mid( 6 ) );
  } else if ( uid.startsWith( QLatin1String( "todo:" ) ) ) {
    incidence = calendar()->incidence( uid.mid( 5 ) );
  }
  if ( incidence ) {
    emit showIncidenceSignal( incidence );
  }
}

#include "kowhatsnextview.moc"
