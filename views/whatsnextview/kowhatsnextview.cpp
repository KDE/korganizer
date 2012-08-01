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

#include <calendarsupport/calendar.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalUtils/IncidenceFormatter>

#include <KIconLoader>

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

KOWhatsNextView::KOWhatsNextView( QWidget *parent )
  : KOrg::BaseView( parent )
{
  mView = new WhatsNextTextBrowser( this );
  connect( mView, SIGNAL(showIncidence(QString)),
           SLOT(showIncidence(QString)) );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addWidget(mView);
}

KOWhatsNextView::~KOWhatsNextView()
{
}

int KOWhatsNextView::currentDateCount() const
{
  return mStartDate.daysTo( mEndDate );
}

void KOWhatsNextView::updateView()
{
  KIconLoader kil( "korganizer" );
  QString ipath;
  kil.loadIcon( "office-calendar", KIconLoader::NoGroup, 32,
                KIconLoader::DefaultState, QStringList(), &ipath );

  mText = "<table width=\"100%\">\n";
  mText += "<tr bgcolor=\"#3679AD\"><td><h1>";
  mText += "<img src=\"";
  mText += ipath;
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

  Akonadi::Item::List events;
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();

  events = calendar()->events( mStartDate, mEndDate, timeSpec, false );
  events = calendar()->sortEvents( events, CalendarSupport::EventSortStartDate,
                                   CalendarSupport::SortDirectionAscending );

  if ( events.count() > 0 ) {
    mText += "<p></p>";
    kil.loadIcon( "view-calendar-day", KIconLoader::NoGroup, 22,
                  KIconLoader::DefaultState, QStringList(), &ipath );
    mText += "<h2><img src=\"";
    mText += ipath;
    mText += "\">";
    mText += i18n( "Events:" ) + "</h2>\n";
    mText += "<table>\n";
    Q_FOREACH ( const Akonadi::Item &evItem, events ) {
      KCalCore::Event::Ptr ev = CalendarSupport::event( evItem );
      if ( !ev->recurs() ) {
        appendEvent( evItem );
      } else {
        KCalCore::Recurrence *recur = ev->recurrence();
        int duration = ev->dtStart().secsTo( ev->dtEnd() );
        KDateTime start = recur->getPreviousDateTime( KDateTime( mStartDate, QTime(), timeSpec ) );
        KDateTime end = start.addSecs( duration );
        KDateTime endDate( mEndDate, QTime( 23, 59, 59 ), timeSpec );
        if ( end.date() >= mStartDate ) {
          appendEvent( evItem, start.dateTime(), end.dateTime() );
        }
        KCalCore::DateTimeList times = recur->timesInInterval( start, endDate );
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
            appendEvent( evItem, times[i].dateTime() );
          }
        }
      }
    }
    mText += "</table>\n";
  }

  mTodos.clear();
  Akonadi::Item::List todos = calendar()->todos( CalendarSupport::TodoSortDueDate,
                                        CalendarSupport::SortDirectionAscending );
  if ( todos.count() > 0 ) {
    kil.loadIcon( "view-calendar-tasks", KIconLoader::NoGroup, 22,
                  KIconLoader::DefaultState, QStringList(), &ipath );
    mText += "<h2><img src=\"";
    mText += ipath;
    mText += "\">";
    mText += i18n( "To-do:" ) + "</h2>\n";
    mText += "<ul>\n";
    Q_FOREACH ( const Akonadi::Item & todoItem, todos ) {
      KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
      if ( !todo->isCompleted() && todo->hasDueDate() && todo->dtDue().date() <= mEndDate ) {
        appendTodo( todoItem );
      }
    }
    bool gotone = false;
    int priority = 1;
    while ( !gotone && priority <= 9 ) {
      Q_FOREACH ( const Akonadi::Item & todoItem, todos ) {
        KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
        if ( !todo->isCompleted() && ( todo->priority() == priority ) ) {
          appendTodo( todoItem );
          gotone = true;
        }
      }
      priority++;
    }
    mText += "</ul>\n";
  }

  QStringList myEmails( CalendarSupport::KCalPrefs::instance()->allEmails() );
  int replies = 0;
  events = calendar()->events( QDate::currentDate(), QDate( 2975, 12, 6 ), timeSpec );
  Q_FOREACH ( const Akonadi::Item &evItem, events ) {
    KCalCore::Event::Ptr ev = CalendarSupport::event( evItem );
    KCalCore::Attendee::Ptr me = ev->attendeeByMails( myEmails );
    if ( me != 0 ) {
      if ( me->status() == KCalCore::Attendee::NeedsAction && me->RSVP() ) {
        if ( replies == 0 ) {
          mText += "<p></p>";
          kil.loadIcon( "mail-reply-sender", KIconLoader::NoGroup, 22,
                        KIconLoader::DefaultState, QStringList(), &ipath );
          mText += "<h2><img src=\"";
          mText += ipath;
          mText += "\">";
          mText += i18n( "Events and to-dos that need a reply:" ) + "</h2>\n";
          mText += "<table>\n";
        }
        replies++;
        appendEvent( evItem );
      }
    }
  }
  todos = calendar()->todos();
  Q_FOREACH ( const Akonadi::Item & todoItem, todos ) {
    KCalCore::Todo::Ptr to = CalendarSupport::todo( todoItem );
    KCalCore::Attendee::Ptr me = to->attendeeByMails( myEmails );
    if ( me != 0 ) {
      if ( me->status() == KCalCore::Attendee::NeedsAction && me->RSVP() ) {
        if ( replies == 0 ) {
          mText += "<p></p>";
          kil.loadIcon( "mail-reply-sender", KIconLoader::NoGroup, 22,
                        KIconLoader::DefaultState, QStringList(), &ipath );
          mText += "<h2><img src=\"";
          mText += ipath;
          mText += "\">";
          mText += i18n( "Events and to-dos that need a reply:" ) + "</h2>\n";
          mText += "<table>\n";
        }
        replies++;
        appendEvent( todoItem );
      }
    }
  }
  if ( replies > 0 ) {
    mText += "</table>\n";
  }

  mText += "</td></tr>\n</table>\n";

  mView->setText(mText);
}

void KOWhatsNextView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  mStartDate = start;
  mEndDate = end;
  updateView();
}

void KOWhatsNextView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

void KOWhatsNextView::changeIncidenceDisplay( const Akonadi::Item &,
                                             Akonadi::IncidenceChanger::ChangeType )
{
  updateView();
}

void KOWhatsNextView::appendEvent( const Akonadi::Item &aitem, const QDateTime &start,
                                   const QDateTime &end )
{
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  mText += "<tr><td><b>";
  if ( const KCalCore::Event::Ptr event = CalendarSupport::event( aitem ) ) {
    KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
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
          starttime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ) ),
        KGlobal::locale()->formatDateTime(
          endtime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ) ) );
    } else {
      mText += i18nc(
        "date, from - to", "%1, %2 - %3",
        KGlobal::locale()->formatDate(
          starttime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date(),
          KLocale::ShortDate ),
        KGlobal::locale()->formatTime(
          starttime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).time() ),
        KGlobal::locale()->formatTime(
          endtime.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).time() ) );
    }
  }
  mText += "</b></td><td><a ";
  if ( incidence->type() == KCalCore::Incidence::TypeEvent ) {
    mText += "href=\"event:";
  }
  if ( incidence->type() == KCalCore::Incidence::TypeTodo ) {
    mText += "href=\"todo:";
  }
  mText += incidence->uid() + "\">";
  mText += incidence->summary();
  mText += "</a></td></tr>\n";
}

void KOWhatsNextView::appendTodo( const Akonadi::Item &aitem )
{
  if ( mTodos.contains( aitem ) ) {
    return;
  }

  mTodos.append( aitem );

  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  mText += "<li><a href=\"todo:" + incidence->uid() + "\">";
  mText += incidence->summary();
  mText += "</a>";

  if ( const KCalCore::Todo::Ptr todo = CalendarSupport::todo( aitem ) ) {
    if ( todo->hasDueDate() ) {
      mText += i18nc( "to-do due date", "  (Due: %1)",
                      KCalUtils::IncidenceFormatter::dateTimeToString(
                        todo->dtDue(), todo->allDay() ) );
    }
  }
  mText += "</li>\n";
}

void KOWhatsNextView::showIncidence( const QString &uid )
{
  Akonadi::Item incidence;

  CalendarSupport::Calendar* cal = dynamic_cast<CalendarSupport::Calendar*>( calendar() );
  if ( !cal ) {
    return;
  }

  if ( uid.startsWith( QLatin1String( "event:" ) ) ) {
    incidence = cal->incidence( cal->itemIdForIncidenceUid( uid.mid( 6 ) ) );
  } else if ( uid.startsWith( QLatin1String( "todo:" ) ) ) {
    incidence = cal->incidence( cal->itemIdForIncidenceUid( uid.mid( 5 ) ) );
  }

  if ( incidence.isValid() ) {
    emit showIncidenceSignal( incidence );
  }
}

KOrg::CalPrinterBase::PrintType KOWhatsNextView::printType() const
{
  // If up to three days are selected, use day style, otherwise week
  if ( currentDateCount() <= 3 ) {
    return KOrg::CalPrinterBase::Day;
  } else {
    return KOrg::CalPrinterBase::Week;
  }
}

#include "kowhatsnextview.moc"
