/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>

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

#include "kotimespentview.h"
#include "koglobals.h"
#include "koprefs.h"

#include <kcal/calendar.h>
#include <kcal/event.h>

#include <QDate>
#include <QMap>
#include <QBoxLayout>
#include <QPainter>
#include <QPainterPath>

class TimeSpentWidget : public QWidget
{
  public:
    TimeSpentWidget( KOTimeSpentView *parent )
      : QWidget( parent ), mTimeSpentView( parent ) {}
    ~TimeSpentWidget() {}

    void paintEvent( QPaintEvent *e )
    {
      QPainter p( this );
      p.fillRect( e->rect(), Qt::white );
      int margin = 10;
      int y = 90;

      p.fillRect( QRect( 5, 5, width(), 35 ), QColor( 54, 121, 173 ) ); // sync with kowhatsnextview
      QPen oldPen = p.pen();
      QFont oldFont = p.font();
      QFont font = p.font();
      font.setPixelSize( 25 );
      font.setBold( true );
      p.setFont( font );
      p.setPen( QColor( Qt::white ) );
      p.drawText( QPoint( 25, 35 ), i18n( "Time Tracker" ) );
      p.setPen( oldPen );
      p.setFont( oldFont );

      QString dateText;
      if ( mTimeSpentView->mStartDate.daysTo( mTimeSpentView->mEndDate ) < 1 ) {
        dateText = KGlobal::locale()->formatDate( mTimeSpentView->mStartDate );
      } else {
        dateText = i18nc( "Date from - to", "%1 - %2",
                          KGlobal::locale()->formatDate( mTimeSpentView->mStartDate ),
                          KGlobal::locale()->formatDate( mTimeSpentView->mEndDate ) );
      }
      font.setPixelSize( 20 );
      font.setBold( true );
      p.setFont( font );
      p.drawText( QPoint( margin, 60 ), dateText );
      p.setPen( oldPen );
      p.setFont( oldFont );

      QMap<QString, int> secondsSpent;

      int total = 0;

      foreach ( Event *e, mEventList ) {

        KDateTime selectedStart( mTimeSpentView->mStartDate,
                                 QTime( 0, 0 ),
                                 e->dtStart().timeSpec() );

        KDateTime selectedEnd( mTimeSpentView->mEndDate.addDays( 1 ),
                               QTime( 0, 0 ),
                               e->dtEnd().timeSpec() );

        KDateTime start;
        KDateTime end;

        // duration of all occurrences added
        int totalDuration = 0;

        if ( e->recurs() ) {
          int eventDuration = e->dtStart().secsTo( e->dtEnd() );

          // timesInInterval only return events that have their start inside the interval
          // so we resize the interval by -eventDuration
          DateTimeList times = e->recurrence()->timesInInterval(
            selectedStart.addSecs( -eventDuration ), selectedEnd );

          foreach ( const KDateTime &kdt, times ) {
            // either the event's start or the event's end must be in the view's interval
            if ( kdt >= selectedStart ||
                 kdt.addSecs( eventDuration ) >= selectedStart ) {

              start = kdt > selectedStart ? kdt : selectedStart;
              end   = kdt.addSecs( eventDuration ) < selectedEnd ?
                      kdt.addSecs( eventDuration ) : selectedEnd;
              totalDuration += start.secsTo( end );
            }
          }

        } else {
          // The event's start can be before the view's start date or end after the view's end
          start  = e->dtStart() > selectedStart ? e->dtStart() : selectedStart;
          end    = e->dtEnd()   < selectedEnd   ? e->dtEnd()   : selectedEnd;

          totalDuration += start.secsTo( end );
        }

        if ( totalDuration == 0 ) {
          continue;
        }

        if ( e->categories().count() ) {
          foreach ( const QString &s, e->categories() ) {
            secondsSpent[ s ] += totalDuration;
          }
        } else {
          secondsSpent[ i18n( "No category" ) ] += totalDuration;
        }

        total += totalDuration;
      }

      QMapIterator<QString, int> i( secondsSpent );
      QFontMetrics fm = p.fontMetrics();
      int lineHeight = fm.boundingRect( "No category" ).height();
      int totalLineHeight = lineHeight + 2; // vertical margin included

      while ( i.hasNext() ) {
        i.next();

        // bar
        QColor color = KOPrefs::instance()->categoryColor( i.key() );
        int length =  static_cast<int>( ( (double) i.value() ) / total * ( width() - 3 * margin ) );
        QPainterPath path( QPoint( margin, y ) );
        path.lineTo( margin + length, y );
        if ( length < margin ) {
          path.lineTo( margin + length, y + lineHeight );
        } else {
          path.arcTo( QRect( margin + length, y, 2 * margin, lineHeight ), +90, -180 );
        }
        path.lineTo( margin, y + lineHeight );
        path.closeSubpath();
        p.setBrush( color );
        p.drawPath( path );

        // text
        int totHr, perHr;
        if ( total > 0 ) {
          totHr = i.value() / ( 60 * 60 );
          perHr = static_cast<int>( ( (double) i.value() * 100 / total ) );
        } else {
          totHr = 0;
          perHr = 0;
        }
        p.drawText( QRect( margin + 2, y + 2, width() - 2 * margin, lineHeight ),
                    i.key() + ": " +
                    i18ncp( "number of hours spent", "%1 hour", "%1 hours", totHr ) +
                    i18nc( "percent of hours spent", " (%1%)", perHr ) );
        y += totalLineHeight;
      }
    }

    Event::List mEventList;
    KOTimeSpentView *mTimeSpentView;
};

KOTimeSpentView::KOTimeSpentView( Calendar *calendar, QWidget *parent )
  : KOrg::BaseView( calendar, parent )
{
  mView = new TimeSpentWidget( this );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addWidget( mView );
}

KOTimeSpentView::~KOTimeSpentView()
{
}

int KOTimeSpentView::currentDateCount()
{
  return mStartDate.daysTo( mEndDate );
}

void KOTimeSpentView::showDates( const QDate &start, const QDate &end )
{
  mStartDate = start;
  mEndDate = end;
  updateView();
}

void KOTimeSpentView::showIncidences( const Incidence::List & )
{
}

void KOTimeSpentView::changeIncidenceDisplay( Incidence *, int action )
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

void KOTimeSpentView::updateView()
{
  /*
 QString text;

  text = "<table width=\"100%\">\n";
  text += "<tr bgcolor=\"#3679AD\"><td><h1>";
  text += "<img src=\"";
  // text +=  todo: put something there.
  text += "\">";
  text += "<font color=\"white\"> ";
  text += i18n("Time tracker") + "</font></h1>";
  text += "</td></tr>\n<tr><td>";

  text += "<h2>";
  if ( mStartDate.daysTo( mEndDate ) < 1 ) {
    text += KGlobal::locale()->formatDate( mStartDate );
  } else {
    text += i18nc("Date from - to", "%1 - %2",
              KGlobal::locale()->formatDate( mStartDate ) ,
              KGlobal::locale()->formatDate( mEndDate ) );
  }
  text+="</h2>\n";
  */

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  mView->mEventList = calendar()->events( mStartDate, mEndDate, timeSpec );
  mView->repaint();
}

