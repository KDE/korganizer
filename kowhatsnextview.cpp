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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>
#include <qfileinfo.h>
#include <qlabel.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkcal/calendar.h>

#include "koglobals.h"
#include "koprefs.h"
#include "koeventviewerdialog.h"

#include "kowhatsnextview.h"

using namespace KOrg;

void WhatsNextTextBrowser::setSource(const QString& n)
{
  kdDebug(5850) << "WhatsNextTextBrowser::setSource(): " << n << endl;

  if (n.startsWith("event:")) {
    emit showIncidence(n);
    return;
  } else if (n.startsWith("todo:")) {
    emit showIncidence(n);
    return;
  } else {
    QTextBrowser::setSource(n);
  }
}

KOWhatsNextView::KOWhatsNextView(Calendar *calendar, QWidget *parent,
                                 const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
//  QLabel *dateLabel =
//      new QLabel(KGlobal::locale()->formatDate(QDate::currentDate()),this);
//  dateLabel->setMargin(2);
//  dateLabel->setAlignment(AlignCenter);

  mView = new WhatsNextTextBrowser(this);
  connect(mView,SIGNAL(showIncidence(const QString &)),SLOT(showIncidence(const QString &)));

  QBoxLayout *topLayout = new QVBoxLayout(this);
//  topLayout->addWidget(dateLabel);
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
  KIconLoader kil("korganizer");
  QString *ipath = new QString();
  kil.loadIcon("korganizer",KIcon::NoGroup,32,KIcon::DefaultState,ipath);

  mText = "<table width=\"100%\">\n";
  mText += "<tr bgcolor=\"#3679AD\"><td><h1>";
  mText += "<img src=\"";
  mText += *ipath;
  mText += "\">";
  mText += "<font color=\"white\"> ";
  mText += i18n("What's next?") + "</font></h1>";
  mText += "</td></tr>\n<tr><td>";

  mText += "<h2>";
  if ( mStartDate.daysTo( mEndDate ) < 1 ) {
    mText += KGlobal::locale()->formatDate( mStartDate );
  } else {
    mText += i18n("Date from - to", "%1 - %2")
            .arg( KGlobal::locale()->formatDate( mStartDate ) )
            .arg( KGlobal::locale()->formatDate( mEndDate ) );
  }
  mText+="</h2>\n";

  Event::List unsortedevents = calendar()->events( mStartDate, mEndDate, false );
  Event::List events = Calendar::sortEvents( &unsortedevents,
                     EventSortStartDate, SortDirectionAscending );

  if (events.count() > 0) {
    mText += "<p></p>";
    kil.loadIcon("appointment",KIcon::NoGroup,22,KIcon::DefaultState,ipath);
    mText += "<h2><img src=\"";
    mText += *ipath;
    mText += "\">";
    mText += i18n("Events:") + "</h2>\n";
    mText += "<table>\n";
    Event::List::ConstIterator it;
    for( it = events.begin(); it != events.end(); ++it ) {
      Event *ev = *it;
      if ( !ev->doesRecur() ){
        appendEvent(ev);
      } else {
        // FIXME: This should actually be cleaned up. Libkcal should
        // provide a method to return a list of all recurrences in a
        // given time span.
        Recurrence *recur = ev->recurrence();
        int duration = ev->dtStart().secsTo( ev->dtEnd() );
        QDateTime start = recur->getPreviousDateTime(
                                QDateTime( mStartDate, QTime() ) );
        QDateTime end = start.addSecs( duration );
        if ( end.date() >= mStartDate ) {
          appendEvent( ev, start, end );
        }
        start = recur->getNextDateTime( start );
        while ( start.isValid() && start.date() <= mEndDate ) {
          appendEvent( ev, start );
          start = recur->getNextDateTime( start );
        }
      }
    }
    mText += "</table>\n";
  }

  mTodos.clear();
  Todo::List todos = calendar()->todos( TodoSortDueDate, SortDirectionAscending );
  if ( todos.count() > 0 ) {
    kil.loadIcon("todo",KIcon::NoGroup,22,KIcon::DefaultState,ipath);
    mText += "<h2><img src=\"";
    mText += *ipath;
    mText += "\">";
    mText += i18n("To-do:") + "</h2>\n";
    mText += "<ul>\n";

    // Start rendering Todos
    Todo::List::ConstIterator it;

    // Find highest priority Todo within the date range
    int priority, bestpriority = 9;
    for ( priority = 1; priority<=9; priority++ ) {
      for( it = todos.begin(); it != todos.end(); ++it ) {
        Todo *todo = *it;
        if ( !todo->isCompleted() && ( todo->priority() == priority ) ) {
	  // Record the highest (lowest number) priority
      	  if( priority < bestpriority )
	  {
            bestpriority = priority;
            kdDebug(5850) << "Setting bestpriority to " << bestpriority << endl;
          }
        }
      }
    }

    // First render those that are due before or within current range
    for( it = todos.begin(); it != todos.end(); ++it ) {
      Todo *todo = *it;
      if ( !todo->isCompleted() && todo->hasDueDate() &&
           todo->dtDue().date() <= mEndDate )
        appendTodo( findToplevelTodo( todo ), bestpriority );
    }
    // Render the others based on priority
    bool gotone = false;
    for ( priority = 1; !gotone && priority<=9; priority++ ) {
      for( it = todos.begin(); it != todos.end(); ++it ) {
        Todo *todo = *it;
        if ( !todo->isCompleted() && ( todo->priority() == priority ) ) {
          appendTodo( findToplevelTodo( todo ), bestpriority );
          gotone = true;
        }
      }
    }
    mText += "</ul>\n";
  }

  int replies = 0;
  events = calendar()->events( QDate::currentDate(), QDate(2975,12,6) );
  Event::List::ConstIterator it2;
  for( it2 = events.begin(); it2 != events.end(); ++it2 ) {
    Event *ev = *it2;
    Attendee *me = ev->attendeeByMails( KOPrefs::instance()->allEmails() );
    if (me!=0) {
      if (me->status()==Attendee::NeedsAction && me->RSVP()) {
        if (replies == 0) {
          mText += "<p></p>";
          kil.loadIcon("reply",KIcon::NoGroup,22,KIcon::DefaultState,ipath);
          mText += "<h2><img src=\"";
          mText += *ipath;
          mText += "\">";
          mText += i18n("Events and to-dos that need a reply:") + "</h2>\n";
          mText += "<table>\n";
        }
        replies++;
        appendEvent( ev );
      }
    }
  }
  todos = calendar()->todos();
  Todo::List::ConstIterator it3;
  for( it3 = todos.begin(); it3 != todos.end(); ++it3 ) {
    Todo *to = *it3;
    Attendee *me = to->attendeeByMails( KOPrefs::instance()->allEmails() );
    if (me!=0) {
      if (me->status()==Attendee::NeedsAction && me->RSVP()) {
        if (replies == 0) {
          mText += "<p></p>";
          kil.loadIcon("reply",KIcon::NoGroup,22,KIcon::DefaultState,ipath);
          mText += "<h2><img src=\"";
          mText += *ipath;
          mText += "\">";
          mText += i18n("Events and to-dos that need a reply:") + "</h2>\n";
          mText += "<table>\n";
        }
        replies++;
        appendEvent(to);
      }
    }
    kdDebug () << "check for todo-replies..." << endl;
  }
  if (replies > 0 ) mText += "</table>\n";


  mText += "</td></tr>\n</table>\n";

  kdDebug(5850) << "KOWhatsNextView::updateView: text: " << mText << endl;
  mView->setText(mText);
}

Incidence* KOWhatsNextView::findToplevelTodo( Incidence *todo)
{
  if( !todo ) return NULL;
  Incidence* parentTodo;
  while( ( parentTodo = todo->relatedTo() ) )
    todo = parentTodo;
  return todo;
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

void KOWhatsNextView::changeIncidenceDisplay(Incidence *, int action)
{
  switch(action) {
    case KOGlobals::INCIDENCEADDED:
    case KOGlobals::INCIDENCEEDITED:
    case KOGlobals::INCIDENCEDELETED:
      updateView();
      break;
    default:
      kdDebug(5850) << "KOWhatsNextView::changeIncidenceDisplay(): Illegal action " << action << endl;
  }
}

void KOWhatsNextView::appendEvent( Incidence *ev, const QDateTime &start,
                                   const QDateTime &end )
{
  kdDebug(5850) << "KOWhatsNextView::appendEvent(): " << ev->uid() << endl;

  mText += "<tr><td><b>";
//  if (!ev->doesFloat()) {
    if (ev->type()=="Event") {
      Event *event = static_cast<Event *>(ev);
      QDateTime starttime( start );
      if ( !starttime.isValid() )
        starttime = event->dtStart();
      QDateTime endtime( end );
      if ( !endtime.isValid() )
        endtime = starttime.addSecs(
                  event->dtStart().secsTo( event->dtEnd() ) );

      if ( starttime.date().daysTo( endtime.date() ) >= 1 ) {
        mText += i18n("date from - to", "%1 - %2")
              .arg( KGlobal::locale()->formatDateTime( starttime ) )
              .arg( KGlobal::locale()->formatDateTime( endtime ) );
      } else {
        /*if (reply) */
        mText += i18n("on date: from - to", "on %1: %2 - %3")
            .arg( KGlobal::locale()->formatDate( starttime.date(), true ) )
            .arg( KGlobal::locale()->formatTime( starttime.time() ) )
            .arg( KGlobal::locale()->formatTime( endtime.time() ) );
      }
    }
//  }
  mText += "</b></td><td><a ";
  if (ev->type()=="Event") mText += "href=\"event:";
  if (ev->type()=="Todo") mText += "href=\"todo:";
  mText += ev->uid() + "\">";
  mText += ev->summary();
  mText += "</a></td></tr>\n";
}

void KOWhatsNextView::appendTodo( Incidence *ev, int bestpriority )
{
  if ( mTodos.find( ev ) != mTodos.end()) return;

  // Don't show completed TODOs or their sub-todos
  if ( ev->type()=="Todo" && static_cast<Todo*>(ev)->isCompleted() ) return;

  mTodos.append( ev );

  kdDebug(5850) << "adding todo \"" << ev->summary() << "\" with priority '" << ev->priority() << "'" << endl;

  // Grey-out ToDos below the highest priority Todo and configure other colours
  // TODO: Make these configurable (perhaps with a stylesheet & KHTML)
  QString linkstyle = QString(" <font color=\"#000055\"> ");

  if( ev->priority() > bestpriority )
    linkstyle = " <font color=\"#999999\"> ";
  if ( ev->type()=="Todo" ) {
    Todo *todo = static_cast<Todo*>(ev);
    if ( todo->hasDueDate() && todo->dtDue().date() < mStartDate)
      linkstyle = " <font color=\"#FF0000\"> ";
    else if ( todo->hasDueDate() && todo->dtDue().date() <= mEndDate)
      linkstyle = " <font color=\"#0000FF\"> ";
  }

  mText += "<li>" + linkstyle + "<a href=\"todo:" + ev->uid() + "\">";
  mText += ev->summary();
  mText += "</a>";

  if ( ev->type()=="Todo" ) {
    Todo *todo = static_cast<Todo*>(ev);
    if ( todo->hasDueDate() ) {
      mText += i18n("  (Due: %1)")
         .arg( (todo->doesFloat())?(todo->dtDueDateStr()):(todo->dtDueStr()) );
    }
  }
  // Find its child TODOs
  mText += "</font>\n<ul>\n";
 Incidence::List children = ev->relations();
 Incidence::List::ConstIterator it;
 for( it = children.begin(); it != children.end(); ++it ) {
     appendTodo( *it, bestpriority );
 }
  mText += "</ul>\n";

  mText += "</li>\n";
}

void KOWhatsNextView::showIncidence( const QString &uid )
{
  kdDebug(5850) << "KOWhatsNextView::showIncidence(): " << uid << endl;
  Incidence *incidence = 0;

  if ( uid.startsWith( "event://" ) ) {
    incidence = calendar()->incidence( uid.mid( 8 ) );
  } else if ( uid.startsWith( "todo://" ) ) {
    incidence = calendar()->incidence( uid.mid( 7 ) );
  }
  if ( incidence ) emit showIncidenceSignal( incidence );
}

#include "kowhatsnextview.moc"
