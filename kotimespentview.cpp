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

#include <QDate>
#include <QMap>
#include <QBoxLayout>
#include <QPainter>
#include <QPainterPath>

#include "koglobals.h"
#include "koprefs.h"

#include <kcal/calendar.h>
#include <kcal/event.h>

#include "kotimespentview.h"

class TimeSpentWidget : public QWidget {

public:
  TimeSpentWidget( KOTimeSpentView *parent ) : 
    QWidget( parent ),
    mTimeSpentView( parent ) {};
  ~TimeSpentWidget() {};

  void paintEvent(QPaintEvent *e) 
  {   
    QPainter p( this );
    p.fillRect( e->rect(), Qt::white );
    int margin = 10;
    const int textlength = 10 * margin; //fixme bounding rect
    int y = 90;

    p.fillRect( QRect( 5, 5, width(), 35 ), QColor( 54, 121, 173 ) /* keep in sync with kowhatsnextview */ );
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
      dateText = i18nc("Date from - to", "%1 - %2",
		     KGlobal::locale()->formatDate( mTimeSpentView->mStartDate ) ,
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
    foreach( Event *e, mEventList ) {
      if ( e->categories().count() ) {
	foreach( QString s, e->categories() ) {
	  secondsSpent[ s ] += e->dtStart().secsTo( e->dtEnd() );
	}
      } else {
	secondsSpent[ i18n("No category") ] += e->dtStart().secsTo( e->dtEnd() );
      }
      
      total += e->dtStart().secsTo( e->dtEnd() );
    }
    
       
    QMapIterator<QString, int> i( secondsSpent );
    QFontMetrics fm = p.fontMetrics();
    int lineHeight = fm.boundingRect( "No category" ).height();
    int totalLineHeight = lineHeight + 2; // vertical margin included
    
    while ( i.hasNext() ) {
      i.next();

      // bar
      QColor color = KOPrefs::instance()->categoryColor( i.key() );
      int length =  ( (double) i.value() ) / total * ( width() - 3 * margin ); 
      QPainterPath path( QPoint( margin, y ) );
      path.lineTo( margin + length, y );
      if ( length < margin ) {
	path.lineTo( margin + length, y + lineHeight );
      }
      else {
	path.arcTo( QRect( margin + length, y, 2 * margin, lineHeight ), +90, -180 );
      }
      path.lineTo( margin, y + lineHeight );
      path.closeSubpath();
      p.setBrush( color );
      p.drawPath( path );

      // text
      p.drawText( QRect( margin + 2, y + 2, width() - 2 * margin, lineHeight ), i.key()
		  + ": "
		  + i18nc( "Example: 5 hours (80%)", "%1 hours (", QString::number( i.value() / (60 * 60) ) )
		  + QString::number( (int)( (double) i.value() * 100 / total ) )
		  + "% )" );
      y += totalLineHeight;    
    }
  }

  Event::List mEventList;
  KOTimeSpentView *mTimeSpentView;
};

KOTimeSpentView::KOTimeSpentView( Calendar *calendar, QWidget *parent )
  : KOrg::BaseView( calendar, parent)
{
  mView = new TimeSpentWidget(this);

  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addWidget(mView);
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

void KOTimeSpentView::changeIncidenceDisplay(Incidence *, int action)
{
  switch(action) {
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
  Event::List events;
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  for ( QDate date = mStartDate; date <= mEndDate; date = date.addDays( 1 ) )
    events += calendar()->events(date, timeSpec,
                                 EventSortStartDate, SortDirectionAscending);

  
  mView->mEventList = events;
  mView->repaint();
}

