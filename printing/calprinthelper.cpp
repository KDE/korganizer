/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <qpainter.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qptrlist.h>
#include <qintdict.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kcalendarsystem.h>
#include <kprinter.h>
#include <kwordwrap.h>

#include <libkcal/todo.h>
#include <libkcal/event.h>
#include <libkcal/calendar.h>

#include "korganizer/corehelper.h"
#include "cellitem.h"

#include "calprinthelper.h"

#ifndef KORG_NOPRINTER



class CalPrintHelper::TodoParentStart
{
  public:
    TodoParentStart( QRect pt = QRect(), bool page = true )
      : mRect( pt ), mSamePage( page ) {}

    QRect mRect;
    bool mSamePage;
};

class PrintCellItem : public KOrg::CellItem
{
  public:
    PrintCellItem( Event *event, const QDate &day )
      : mEvent( event ), mDay( day )
    {
    }

    Event *event() const { return mEvent; }

    QString label() const { return mEvent->summary(); }

    bool overlaps( KOrg::CellItem *o ) const
    {
      PrintCellItem *other = static_cast<PrintCellItem *>( o );

      QDateTime start = event()->dtStart();
      QDateTime end = event()->dtEnd();
      if ( event()->doesRecur() ) {
        start.setDate( mDay );
        end.setDate( mDay );
      }
      QDateTime otherStart = other->event()->dtStart();
      QDateTime otherEnd = other->event()->dtEnd();
      if ( other->event()->doesRecur() ) {
        otherStart.setDate( mDay );
        otherEnd.setDate( mDay );
      }

#if 0
      kdDebug(5850) << "PrintCellItem::overlaps() " << event()->summary()
                    << " <-> " << other->event()->summary() << endl;
      kdDebug(5850) << "  start     : " << start.toString() << endl;
      kdDebug(5850) << "  end       : " << end.toString() << endl;
      kdDebug(5850) << "  otherStart: " << otherStart.toString() << endl;
      kdDebug(5850) << "  otherEnd  : " << otherEnd.toString() << endl;
#endif

      return !( otherStart >= end || otherEnd <= start );
    }

  private:
    Event *mEvent;
    QDate mDay;
};

CalPrintHelper::CalPrintHelper( KPrinter *pr, Calendar *cal, KConfig *cfg,
                                KOrg::CoreHelper *corehelper )
  : mPrinter( pr ), mCalendar( cal ), mConfig( cfg ),
    mCoreHelper( corehelper ),
    mHeaderHeight( 72 ), mSubHeaderHeight( 20 ), mMargin( 36 )
{
  if ( corehelper )
    setCalendarSystem( mCoreHelper->calendarSystem() );
}

CalPrintHelper::~CalPrintHelper()
{
}



void CalPrintHelper::setCategoryColors( QPainter &p, Incidence *incidence )
{
  if ( mCoreHelper ) {
    QColor bgColor = mCoreHelper->categoryColor( incidence->categories() );
    QColor textColor = mCoreHelper->textColor( bgColor );
    p.setPen( textColor );
    p.setBrush( bgColor );
  }
}



///////////////////////////////////////////////////////////////////////////////

void CalPrintHelper::drawHeader( QPainter &p, QString title,
    const QDate &month1, const QDate &month2,
    int x, int y, int width, int height )
{
  p.drawRect( x, y, width, height );
  p.fillRect( x + 1, y + 1, width - 2, height - 2,
              QBrush( Qt::Dense7Pattern ) );

  int right=x+width;

  // print previous month for month view, print current for todo, day and week
  int smallMonthWidth=width/4-10;
  if (smallMonthWidth>100) smallMonthWidth=100;
  if (month2.isValid()) {
    right -= (10+smallMonthWidth);
    drawSmallMonth(p, QDate(month2.year(), month2.month(), 1),
                   right, y+2, smallMonthWidth, height-4);
    right-=10;
  }
  if (month1.isValid()) {
    right -= (10+smallMonthWidth);
    drawSmallMonth(p, QDate(month1.year(), month1.month(), 1),
                   right, y+2, smallMonthWidth, height-4);
    right-=10;
  }

  // Print the titles...
  QFont oldFont(p.font());
  p.setFont( QFont("helvetica", 18, QFont::Bold) );
  QRect textRect( x+5, y+5, right-10-x, height-10 );
  p.drawText( textRect, Qt::AlignLeft | Qt::AlignTop | Qt::WordBreak, title );
  p.setFont(oldFont);
}


void CalPrintHelper::drawSmallMonth(QPainter &p, const QDate &qd,
    int x, int y, int width, int height)
{
  bool firstCol = true;
  QDate monthDate(QDate(qd.year(), qd.month(), 1));
  QDate monthDate2;
  int month = monthDate.month();

  // draw the title
  QFont oldFont( p.font() );
  p.setFont(QFont("helvetica", 8, QFont::Bold));
  //  int lineSpacing = p.fontMetrics().lineSpacing();
  if ( mCalSys )
    p.drawText(x, y, width, height/4, Qt::AlignCenter,
      mCalSys->monthName( qd ) );

  int cellWidth = width/7;
  int cellHeight = height/8;
  QString tmpStr;

  // correct begin of week
  int weekdayCol = weekdayColumn( qd.dayOfWeek() );
  monthDate2 = monthDate.addDays( -weekdayCol );

  // draw days of week
  for (int col = 0; col < 7; col++) {
    // tmpStr.sprintf("%c",(const char*)monthDate2.dayName(monthDate2.dayOfWeek()));
    tmpStr=mCalSys->weekDayName( monthDate2 )[0].upper();
    p.drawText( x + col*cellWidth, y + height/4, cellWidth, cellHeight,
               Qt::AlignCenter, tmpStr );
    monthDate2 = monthDate2.addDays( 1 );
  }

  // draw separator line
  p.drawLine( x, y + height/4 + cellHeight, x + width,
      y + height/4 + cellHeight );

  for ( int row = 0; row < 5; row++ ) {
    for ( int col = 0; col < 7; col++ ) {
      if ( monthDate.month() != month )
        break;
      if (firstCol) {
        firstCol = true;
        col = weekdayColumn( monthDate.dayOfWeek() );
      }
      p.drawText( x+col*cellWidth,
                  y+height/4+cellHeight+(row*cellHeight),
                  cellWidth, cellHeight, Qt::AlignCenter,
                  tmpStr.setNum( monthDate.day() ) );
      monthDate = monthDate.addDays(1);
    }
  }
  p.setFont( oldFont );
}


///////////////////////////////////////////////////////////////////////////////

/*
 * This routine draws a header box over the main part of the calendar
 * containing the days of the week.
 */
void CalPrintHelper::drawDaysOfWeek(QPainter &p,
    const QDate &fromDate, const QDate &toDate,
    int x, int y, int width, int height)
{
  int cellWidth = width/(fromDate.daysTo( toDate )+1);
  int currx=x;
  QDate cellDate(fromDate);

  while (cellDate<=toDate) {
    drawDaysOfWeekBox(p, cellDate, currx, y, cellWidth, height);
    currx+=cellWidth;
    cellDate = cellDate.addDays(1);
  }
}


void CalPrintHelper::drawDaysOfWeekBox(QPainter &p, const QDate &qd,
    int x, int y, int width, int height)
{
  QFont oldFont( p.font() );
  p.setFont( QFont( "helvetica", 10, QFont::Bold ) );
  p.drawRect( x, y, width, height );
  p.fillRect( x+1, y+1,
              width-2, height-2,
              QBrush( Qt::Dense7Pattern ) );
  if ( mCalSys )
    p.drawText( x+5, y, width-10, height, Qt::AlignCenter | Qt::AlignVCenter,
             mCalSys->weekDayName( qd ) );
  p.setFont( oldFont );
}


void CalPrintHelper::drawTimeLine(QPainter &p,
    const QTime &fromTime, const QTime &toTime,
    int x, int y, int width, int height)
{
  p.drawRect(x, y, width, height);

  int totalsecs=fromTime.secsTo(toTime);
  float minlen=(float)height*60./(float)totalsecs;
  float cellHeight=(60.*(float)minlen);
  float currY=y;

  QTime curTime( fromTime );
  QTime endTime( toTime );
  if ( fromTime.minute() > 30 )
    curTime = QTime( fromTime.hour()+1, 0, 0 );
  else if ( fromTime.minute() > 0 ) {
    curTime = QTime( fromTime.hour(), 30, 0 );
    float yy = currY + minlen*(float)fromTime.secsTo( curTime )/60.;
    p.drawLine( x+width/2, (int)yy, x+width, (int)yy );
    curTime = QTime( fromTime.hour()+1, 0, 0 );
  }
  currY += ( fromTime.secsTo(curTime)*minlen/60 );

  while ( curTime < endTime ) {
    p.drawLine( x, (int)currY, x+width, (int)currY );
    int newY=(int)(currY+cellHeight/2.);
    QString numStr;
    if (newY < y+height) {
      QFont oldFont( p.font() );
      p.drawLine(x+width/2, (int)newY, x+width, (int)newY);
      // draw the time:
      if ( !KGlobal::locale()->use12Clock() ) {
        numStr.setNum(curTime.hour());
        if (cellHeight > 30) {
          p.setFont(QFont("helvetica", 16, QFont::Bold));
        } else {
          p.setFont(QFont("helvetica", 12, QFont::Bold));
        }
        p.drawText(x+2, (int)currY+2, width/2-2, (int)cellHeight,
                  Qt::AlignTop | Qt::AlignRight, numStr);
        p.setFont(QFont("helvetica", 10, QFont::Normal));
        p.drawText(x+width/2, (int)currY+2, width/2+2, (int)(cellHeight/2)-3,
                  Qt::AlignTop | Qt::AlignLeft, "00");
      } else {
        QTime time( curTime.hour(), 0 );
        numStr = KGlobal::locale()->formatTime( time );
        p.setFont(QFont("helvetica", 14, QFont::Bold));
        p.drawText(x+2, (int)currY+2, width-4, (int)cellHeight/2-3,
                  Qt::AlignTop|Qt::AlignLeft, numStr);
      }
      currY+=cellHeight;
    p.setFont( oldFont );
    } // enough space for half-hour line and time
    if (curTime.secsTo(endTime)>3600)
      curTime=curTime.addSecs(3600);
    else curTime=endTime;
  } // currTime<endTime
}

Event *CalPrintHelper::holiday( const QDate &dt )
{
#ifndef KORG_NOPLUGINS
  if ( !mCoreHelper ) return 0;
  QString hstring( mCoreHelper->holidayString( dt ) );
  if ( !hstring.isEmpty() ) {
    Event*holiday=new Event();
    holiday->setDtStart( dt );
    holiday->setDtEnd( dt );
    holiday->setFloats( true );
    holiday->setCategories( i18n("Holiday") );
    return holiday;
  }
  return 0;
#endif

}

///////////////////////////////////////////////////////////////////////////////

/** prints the all-day box for the agenda print view. if expandable is set,
    height is the cell height of a single cell, and the returned height will
    be the total height used for the all-day events. If !expandable, only one
    cell will be used, and multiple events are concatenated using ", ".
*/
void CalPrintHelper::drawAllDayBox(QPainter &p, Event::List &eventList,
    const QDate &qd, bool expandable,
    int x, int y, int width, int &height)
{
  Event::List::Iterator it, itold;

  int offset=y;

  p.setBrush( QBrush( Qt::Dense7Pattern ) );
  QPen oldPen( p.pen() );
  QColor oldBgColor( p.backgroundColor() );
  QBrush oldBrush( p.brush() );
  QString multiDayStr;

  Event*hd = holiday( qd );
  if ( hd ) eventList.prepend( hd );

  it = eventList.begin();
  Event *currEvent = 0;
  // First, print all floating events
  while( it!=eventList.end() ) {
    currEvent=*it;
    itold=it;
    ++it;
    if ( currEvent->doesFloat() ) {
      // set the colors according to the categories
      if ( expandable ) {
        if ( mUseColors )
          setCategoryColors( p, currEvent );

        p.drawRect( x, offset, width, height );
        p.drawText( x + 5, offset + 5, width - 10, height - 10,
                    Qt::AlignCenter | Qt::AlignVCenter | Qt::AlignJustify |
                    Qt::WordBreak,
                    currEvent->summary() );
        // reset the colors
        p.setBrush( oldBrush );
        p.setPen( oldPen );
        p.setBackgroundColor( oldBgColor );

        offset += height;
      } else {
        if ( !multiDayStr.isEmpty() ) multiDayStr += ", ";
        multiDayStr += currEvent->summary() + "\n";
      }
      eventList.remove( itold );
    }
  }

  if (!expandable) {
    p.drawRect(x, offset, width, height);
    if (!multiDayStr.isEmpty()) {
      p.fillRect( x + 1, offset + 1, width - 2, height - 2,
                  QBrush( Qt::Dense5Pattern ) );
      p.drawText( x + 5, offset + 5, width - 10, height - 10,
                  Qt::AlignCenter | Qt::AlignVCenter | Qt::AlignJustify |
                  Qt::WordBreak,
                  multiDayStr);
    }
  } else {
    height=offset-y;
  }
}


void CalPrintHelper::drawAgendaDayBox( QPainter &p, Event::List &events,
                                     const QDate &qd, bool expandable,
                                     QTime &fromTime, QTime &toTime,
                                     int x, int y, int width, int height )
{
  p.drawRect( x, y, width, height );

  Event *event;

  if ( expandable ) {
    // Adapt start/end times to include complete events
    Event::List::ConstIterator it;
    for ( it = events.begin(); it != events.end(); ++it ) {
      event = *it;
      if ( event->dtStart().time() < fromTime )
        fromTime = event->dtStart().time();
      if ( event->dtEnd().time() > toTime )
        toTime = event->dtEnd().time();
    }
  }

  // Show at least one hour
  if ( fromTime.secsTo( toTime ) < 3600 ) {
    fromTime = QTime( fromTime.hour(), 0, 0 );
    toTime = fromTime.addSecs( 3600 );
  }

  // calculate the height of a cell and of a minute
  int totalsecs = fromTime.secsTo( toTime );
  float minlen = height * 60. / totalsecs;
  float cellHeight = 60. * minlen;
  float currY = y;

  // print grid:
  QTime curTime( QTime( fromTime.hour(), 0, 0 ) );
  currY += fromTime.secsTo( curTime ) * minlen / 60;

  while ( curTime < toTime && curTime.isValid() ) {
    if ( currY > y ) p.drawLine( x, int( currY ), x + width, int( currY ) );
    currY += cellHeight / 2;
    if ( ( currY > y ) && ( currY < y + height ) ) {
      QPen oldPen( p.pen() );
      p.setPen( QColor( 192, 192, 192 ) );
      p.drawLine( x, int( currY ), x + width, int( currY ) );
      p.setPen( oldPen );
    } // enough space for half-hour line
    if ( curTime.secsTo( toTime ) > 3600 )
      curTime = curTime.addSecs( 3600 );
    else curTime = toTime;
    currY += cellHeight / 2;
  }

  QDateTime startPrintDate = QDateTime( qd, fromTime );
  QDateTime endPrintDate = QDateTime( qd, toTime );

  // Calculate horizontal positions and widths of events taking into account
  // overlapping events

  QPtrList<KOrg::CellItem> cells;
  cells.setAutoDelete( true );

  Event::List::ConstIterator itEvents;
  for( itEvents = events.begin(); itEvents != events.end(); ++itEvents ) {
    cells.append( new PrintCellItem( *itEvents, qd ) );
  }

  QPtrListIterator<KOrg::CellItem> it1( cells );
  for( it1.toFirst(); it1.current(); ++it1 ) {
    KOrg::CellItem *placeItem = it1.current();

    KOrg::CellItem::placeItem( cells, placeItem );
  }

  QPen oldPen( p.pen() );
  QColor oldBgColor( p.backgroundColor() );
  QBrush oldBrush( p.brush() );
  QFont oldFont( p.font() );

  p.setFont( QFont( "helvetica", 10 ) );
  p.setBrush( QBrush( Qt::Dense7Pattern ) );

  for( it1.toFirst(); it1.current(); ++it1 ) {
    PrintCellItem *placeItem = static_cast<PrintCellItem *>( it1.current() );

    drawAgendaItem( placeItem, p, qd, startPrintDate, endPrintDate, minlen, x,
                    y, width );

    p.setBrush( oldBrush );
    p.setPen( oldPen );
    p.setBackgroundColor( oldBgColor );
  }
  p.setFont( oldFont );
//  p.setBrush( QBrush( NoBrush ) );
}


void CalPrintHelper::drawAgendaItem( PrintCellItem *item, QPainter &p,
                                   const QDate &qd,
                                   const QDateTime &startPrintDate,
                                   const QDateTime &endPrintDate,
                                   float minlen, int x, int y, int width )
{
  Event *event = item->event();

  // set the colors according to the categories
  if ( mUseColors ) setCategoryColors( p, event );

  // start/end of print area for event
  QDateTime startTime = event->dtStart();
  QDateTime endTime = event->dtEnd();
  if ( event->doesRecur() ) {
    startTime.setDate( qd );
    endTime.setDate( qd );
  }
  if ( ( startTime < endPrintDate && endTime > startPrintDate ) ||
       ( endTime > startPrintDate && startTime < endPrintDate ) ) {
    if ( startTime < startPrintDate ) startTime = startPrintDate;
    if ( endTime > endPrintDate ) endTime = endPrintDate;
    int eventLength = int( startTime.secsTo( endTime ) / 60. * minlen );
    int currentyPos = int( y + startPrintDate.secsTo( startTime ) *
                           minlen / 60. );
    int currentWidth = width / item->subCells();
    int currentX = x + item->subCell() * currentWidth;

    p.drawRect( currentX, currentyPos, currentWidth, eventLength );
    int offset = 4;
    // print the text vertically centered. If it doesn't fit inside the
    // box, align it at the top so the beginning is visible
    int flags = Qt::AlignLeft | Qt::WordBreak;
    QRect bound = p.boundingRect ( currentX + offset, currentyPos,
                                   currentWidth - 2 * offset, eventLength,
                                   flags, event->summary() );
    if ( bound.height() >= eventLength - 4 ) flags |= Qt::AlignTop;
    else flags |= Qt::AlignVCenter;
    p.drawText( currentX+offset, currentyPos+offset, currentWidth-2*offset,
                eventLength-2*offset, flags, event->summary() );
  }
}

void CalPrintHelper::drawDayBox( QPainter &p, const QDate &qd,
    int x, int y, int width, int height,
    bool fullDate )
{
  QString dayNumStr;
  QString ampm;
  const KLocale*local = KGlobal::locale();


  // This has to be localized
  if ( fullDate && mCalSys ) {

    dayNumStr = i18n("weekday month date", "%1 %2 %3")
        .arg( mCalSys->weekDayName( qd ) )
        .arg( mCalSys->monthName( qd ) )
        .arg( qd.day() );
//    dayNumStr = local->formatDate(qd);
  } else {
    dayNumStr = QString::number( qd.day() );
  }

  p.eraseRect( x, y, width, height );
  p.drawRect( x, y, width, height );
  p.drawRect( x, y, width, mSubHeaderHeight );
  p.fillRect( x + 1, y + 1, width - 2, mSubHeaderHeight - 2,
              QBrush( Qt::Dense7Pattern ) );
  QString hstring;
#ifndef KORG_NOPLUGINS
  if ( mCoreHelper ) hstring = mCoreHelper->holidayString(qd);
#endif
  QFont oldFont( p.font() );

  if (!hstring.isEmpty()) {
    p.setFont( QFont( "helvetica", 8, QFont::Bold, true ) );

    p.drawText( x+5, y, width-25, mSubHeaderHeight,
                Qt::AlignLeft | Qt::AlignVCenter, hstring );
  }
  p.setFont(QFont("helvetica", 10, QFont::Bold));
  p.drawText( x + 5, y, width - 10, mSubHeaderHeight,
              Qt::AlignRight | Qt::AlignVCenter, dayNumStr);

  Event::List eventList = mCalendar->events( qd, true );
  QString outStr;
  p.setFont( QFont( "helvetica", 8 ) );
  int lineSpacing = p.fontMetrics().lineSpacing();

  int textY=mSubHeaderHeight+3; // gives the relative y-coord of the next printed entry
  int flags = Qt::AlignLeft;
  Event::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end() && textY<height; ++it ) {
    Event *currEvent = *it;
    if (currEvent->doesFloat() || currEvent->isMultiDay())
      outStr = "";
    else
      outStr = local->formatTime( currEvent->dtStart().time() );

    QRect timeBound = p.boundingRect( x + 5, y + textY, width - 10, lineSpacing, flags, outStr );
    p.drawText( timeBound, flags, outStr );

    QFontMetrics fm = p.fontMetrics();
    int summaryWidth = outStr.isEmpty() ? 0 : timeBound.width() + 4;
    QRect summaryBound( x + 5 + summaryWidth, y + textY, width - summaryWidth - 5 , height );
    KWordWrap *ww = KWordWrap::formatText( fm, summaryBound, flags, currEvent->summary() );
    ww->drawText( &p, x + 5 + summaryWidth, y + textY, flags );

    textY+=ww->boundingRect().height();

    delete ww;
  }

  if ( textY<height ) {
    Todo::List todos = mCalendar->todos( qd );
    Todo::List::ConstIterator it2;
    for( it2 = todos.begin(); it2 != todos.end() && textY<height; ++it2 ) {
      Todo *todo = *it2;
      QString text;
      if (todo->hasDueDate()) {
        if (!todo->doesFloat()) {
          text += KGlobal::locale()->formatTime(todo->dtDue().time());
          text += " ";
        }
      }
      text += i18n("To-do: %1").arg(todo->summary());

      p.drawText( x + 5, y + textY, width - 10, lineSpacing,
                  Qt::AlignLeft | Qt::AlignBottom, text );
      textY += lineSpacing;
    }
  }
  p.setFont( oldFont );
}


///////////////////////////////////////////////////////////////////////////////

void CalPrintHelper::drawWeek(QPainter &p, const QDate &qd,
    int x, int y, int width, int height)
{
  QDate weekDate = qd;
  bool portrait = ( height > width );
  int cellWidth, cellHeight;
  int vcells;
  if (portrait) {
    cellWidth = width/2;
    vcells=3;
  } else {
    cellWidth = width/6;
    vcells=1;
  }
  cellHeight = height/vcells;

  // correct begin of week
  int weekdayCol = weekdayColumn( qd.dayOfWeek() );
  weekDate = qd.addDays( -weekdayCol );

  for (int i = 0; i < 7; i++, weekDate = weekDate.addDays(1)) {
    if (i<5) {
      drawDayBox(p, weekDate, x+cellWidth*(int)(i/vcells), y+cellHeight*(i%vcells),
        cellWidth, cellHeight, true);
    } else if (i==5) {
      drawDayBox(p, weekDate, x+cellWidth*(int)(i/vcells), y+cellHeight*(i%vcells),
        cellWidth, cellHeight/2, true);
    } else if (i==6) {
      drawDayBox(p, weekDate, x+cellWidth*(int)((i-1)/vcells),
        y+cellHeight*((i-1)%vcells)+cellHeight/2, cellWidth, cellHeight/2, true);
    }
  } // for i through all weekdays
}


void CalPrintHelper::drawTimeTable(QPainter &p,
    const QDate &fromDate, const QDate &toDate,
    QTime &fromTime, QTime &toTime,
    int x, int y, int width, int height)
{
  // timeline is 1.5 hours:
  int alldayHeight = (int)( 3600.*height/(fromTime.secsTo(toTime)+3600.) );
  int timelineWidth = 50;
  int cellWidth = (int)( (width-timelineWidth)/(fromDate.daysTo(toDate)+1) );
  int currY=y;
  int currX=x;

  drawDaysOfWeek( p, fromDate, toDate, x+timelineWidth, currY, width-timelineWidth, mSubHeaderHeight);
  currY+=mSubHeaderHeight;
  drawTimeLine( p, fromTime, toTime, x, currY+alldayHeight,
    timelineWidth, height-mSubHeaderHeight-alldayHeight );

  currX=x+timelineWidth;
  // draw each day
  QDate curDate(fromDate);
  while (curDate<=toDate) {
    Event::List eventList = mCalendar->events(curDate, true);
    drawAllDayBox( p, eventList, curDate, false, currX, currY, cellWidth, alldayHeight);
    drawAgendaDayBox( p, eventList, curDate, false, fromTime, toTime, currX,
      currY+alldayHeight, cellWidth, height-mSubHeaderHeight-alldayHeight );
    currX+=cellWidth;
    curDate=curDate.addDays(1);
  }

}


///////////////////////////////////////////////////////////////////////////////

void CalPrintHelper::drawMonth(QPainter &p, const QDate &qd, bool weeknumbers,
    int x, int y, int width, int height)
{
  int yoffset = mSubHeaderHeight;
  int xoffset = 0;
  QDate monthDate(QDate(qd.year(), qd.month(), 1));
  QDate monthFirst(monthDate);
  QDate monthLast(monthDate.addMonths(1).addDays(-1));


  int weekdayCol = weekdayColumn( monthDate.dayOfWeek() );
  monthDate = monthDate.addDays(-weekdayCol);

  int rows=(weekdayCol + qd.daysInMonth() - 1)/7 +1;
  int cellHeight = (height-yoffset) / rows;

  if (weeknumbers) {
    QFont oldFont(p.font());
    QFont newFont(p.font());
    newFont.setPointSize(6);
    p.setFont(newFont);
    xoffset += 14;
    QDate weekDate(monthDate);
    for (int row = 0; row<rows; row++) {
      int calWeek = weekDate.weekNumber();
      QRect rc( x, y + yoffset + cellHeight*row, xoffset - 1, cellHeight );
      p.drawText( rc, Qt::AlignRight | Qt::AlignVCenter,
                  QString::number( calWeek ) );
      weekDate = weekDate.addDays( 7 );
    }
    p.setFont( oldFont );
  }

  drawDaysOfWeek( p, monthDate, monthDate.addDays( 6 ), x + xoffset,
                  y, width - xoffset, mSubHeaderHeight );
  int cellWidth = ( width - xoffset ) / 7;

  QColor back = p.backgroundColor();
  bool darkbg = false;
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < 7; col++) {
      // show days from previous/next month with a grayed background
      if ( (monthDate < monthFirst) || (monthDate > monthLast) ) {
        p.setBackgroundColor( back.dark( 120 ) );
        darkbg = true;
      }
      drawDayBox(p, monthDate, x+xoffset+col*cellWidth, y+yoffset+row*cellHeight,
                 cellWidth, cellHeight);
      if ( darkbg ) {
        p.setBackgroundColor( back );
        darkbg = false;
      }
      monthDate = monthDate.addDays(1);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////

void CalPrintHelper::drawTodo( int &count, Todo * item, QPainter &p, bool connectSubTodos,
    bool desc, int pospriority, int possummary, int posDueDt, int level,
    int x, int &y, int width, int pageHeight, const Todo::List &todoList,
    TodoParentStart *r )
{
  QString outStr;
//  int fontHeight = 10;
  const KLocale *local = KGlobal::locale();
  int priority=item->priority();
  int posdue=posDueDt;
  if (posdue<0) posdue=x+width;
  QRect rect;
  TodoParentStart startpt;
  // This list keeps all starting points of the parent todos so the connection
  // lines of the tree can easily be drawn (needed if a new page is started)
  static QPtrList<TodoParentStart> startPoints;
  if (level<1) {
    startPoints.clear();
  }

  // size of item
  outStr=item->summary();
  int left = possummary+(level*10);
  rect = p.boundingRect(left, y, (posdue-left-5),-1, Qt::WordBreak, outStr);
  if ( !item->description().isEmpty() && desc ) {
    outStr = item->description();
    rect = p.boundingRect( left+20, rect.bottom()+5, width-(left+10-x), -1,
                           Qt::WordBreak, outStr );
  }
  // if too big make new page
  if ( rect.bottom() > pageHeight) {
    // first draw the connection lines from parent todos:
    if (level > 0 && connectSubTodos) {
      TodoParentStart *rct;
      for ( rct = startPoints.first(); rct; rct = startPoints.next() ) {
        int start;
        int center = rct->mRect.left() + (rct->mRect.width()/2);
        int to = p.viewport().bottom();

        // draw either from start point of parent or from top of the page
        if (rct->mSamePage)
          start = rct->mRect.bottom() + 1;
        else
          start = p.viewport().top();
        p.moveTo( center, start );
        p.lineTo( center, to );
        rct->mSamePage=false;
      }
    }
    y=0;
    mPrinter->newPage();
  }

  // If this is a sub-item, r will not be 0, and we want the LH side of the priority line up
  //to the RH side of the parent item's priority
  bool showPriority = pospriority>=0;
  if (r) {
    pospriority = r->mRect.right() + 1;
  }

  outStr.setNum(priority);
  rect = p.boundingRect(pospriority, y + 10, 5, -1, Qt::AlignCenter, outStr);
  // Make it a more reasonable size
  rect.setWidth(18);
  rect.setHeight(18);

  // Priority
  if ( priority > 0 && showPriority ) {
    p.drawText(rect, Qt::AlignCenter, outStr);
    p.drawRect(rect);
    // cross out the rectangle for completed items
    if ( item->isCompleted() ) {
      p.drawLine( rect.topLeft(), rect.bottomRight() );
      p.drawLine( rect.topRight(), rect.bottomLeft() );
    }
  }
  startpt.mRect = rect; //save for later

  // Connect the dots
  if (level > 0 && connectSubTodos) {
    int bottom;
    int center( r->mRect.left() + (r->mRect.width()/2) );
    if (r->mSamePage )
      bottom = r->mRect.bottom() + 1;
    else
      bottom = 0;
    int to( rect.top() + (rect.height()/2) );
    int endx( rect.left() );
    p.moveTo(center, bottom);
    p.lineTo(center, to);
    p.lineTo(endx, to);
  }

  // if completed, use strike out font
  QFont ft( p.font() );
  ft.setStrikeOut( item->isCompleted() );
  p.setFont( ft );
  // summary
  outStr=item->summary();
  rect = p.boundingRect( left, rect.top(), (posdue-(left + rect.width() + 5)),
    -1, Qt::WordBreak, outStr);
  QRect newrect;
  p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  ft.setStrikeOut(false);
  p.setFont(ft);

  // due
  if ( item->hasDueDate() && posDueDt>=0 ) {
    outStr = local->formatDate(item->dtDue().date(),true);
    rect = p.boundingRect( posdue, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  if ( !item->description().isEmpty() && desc ) {
    y=newrect.bottom() + 5;
    outStr = item->description();
    rect = p.boundingRect( left+20, y, x+width-(left+10), -1,
                           Qt::WordBreak, outStr );
    p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  }

  // Set the new line position
  y=newrect.bottom() + 10; //set the line position

  // If the item has subitems, we need to call ourselves recursively
  Incidence::List l = item->relations();
  Incidence::List::ConstIterator it;
  startPoints.append( &startpt );
  for( it = l.begin(); it != l.end(); ++it ) {
    count++;
    // In the future, todos might also be related to events
    // Manually check if the subtodo is in the list of todos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if (subtodo && todoList.contains( subtodo ) ) {
      drawTodo( count, subtodo, p, connectSubTodos,
          desc, pospriority, possummary, posDueDt, level+1,
          x, y, width, pageHeight, todoList, &startpt);
    }
  }
  startPoints.remove(&startpt);
}

int CalPrintHelper::weekdayColumn( int weekday )
{
  return ( weekday + 7 - KGlobal::locale()->weekStartDay() ) % 7;
}

void CalPrintHelper::drawJournalField( QPainter &p, QString field, QString text,
                                       int x, int &y, int width, int pageHeight )
{
  if ( text.isEmpty() ) return;

  QString entry( field.arg( text ) );

  QRect rect( p.boundingRect( x, y, width, -1, Qt::WordBreak, entry) );
  if ( rect.bottom() > pageHeight) {
    // Start new page...
    // FIXME: If it's a multi-line text, draw a few lines on this page, and the
    // remaining lines on the next page.
    y=0;
    mPrinter->newPage();
    rect = p.boundingRect( x, y, width, -1, Qt::WordBreak, entry);
  }
  QRect newrect;
  p.drawText( rect, Qt::WordBreak, entry, -1, &newrect );
  y = newrect.bottom() + 7;
}

void CalPrintHelper::drawJournal( Journal * journal, QPainter &p, int x, int &y,
                                  int width, int pageHeight )
{
  QFont oldFont( p.font() );
  p.setFont( QFont( "helvetica", 15 ) );
  QString headerText;
  QString dateText( KGlobal::locale()->
        formatDate( journal->dtStart().date(), false ) );

  if ( journal->summary().isEmpty() ) {
    headerText = dateText;
  } else {
    headerText = i18n("Description - date", "%1 - %2")
                     .arg( journal->summary() )
                     .arg( dateText );
  }

  QRect rect( p.boundingRect( x, y, width, -1, Qt::WordBreak, headerText) );
  if ( rect.bottom() > pageHeight) {
    // Start new page...
    y=0;
    mPrinter->newPage();
    rect = p.boundingRect( x, y, width, -1, Qt::WordBreak, headerText );
  }
  QRect newrect;
  p.drawText( rect, Qt::WordBreak, headerText, -1, &newrect );
  p.setFont( oldFont );

  y = newrect.bottom() + 4;

  p.drawLine( x + 3, y, x + width - 6, y );
  y += 5;

  drawJournalField( p, i18n("Person: %1"), journal->organizer().fullName(), x, y, width, pageHeight );
  drawJournalField( p, i18n("%1"), journal->description(), x, y, width, pageHeight );
  y += 10;
}


void CalPrintHelper::drawSplitHeaderRight( QPainter &p, const QDate &fd,
                                           const QDate &td,
                                           const QDate &,
                                           int width, int )
{
  QFont oldFont( p.font() );

  QPen oldPen( p.pen() );
  QPen pen( Qt::black, 4 );

  QString title;
  if ( mCalSys ) {
    if ( fd.month() == td.month() ) {
      title = i18n("Date range: Month dayStart - dayEnd", "%1 %2 - %3")
        .arg( mCalSys->monthName( fd.month(), false ) )
        .arg( mCalSys->dayString( fd, false ) )
        .arg( mCalSys->dayString( td, false ) );
    } else {
      title = i18n("Date range: monthStart dayStart - monthEnd dayEnd", "%1 %2 - %3 %4")
        .arg( mCalSys->monthName( fd.month(), false ) )
        .arg( mCalSys->dayString( fd, false ) )
        .arg( mCalSys->monthName( td.month(), false ) )
        .arg( mCalSys->dayString( td, false ) );
    }
  }

  QFont serifFont("Times", 30);
  p.setFont(serifFont);

  int lineSpacing = p.fontMetrics().lineSpacing();
  p.drawText( 0, lineSpacing * 0, width, lineSpacing,
              Qt::AlignRight | Qt::AlignTop, title );

  title.truncate(0);

  p.setPen( pen );
  p.drawLine(300, lineSpacing * 1, width, lineSpacing * 1);
  p.setPen( oldPen );

  p.setFont(QFont("Times", 20, QFont::Bold, TRUE));
  int newlineSpacing = p.fontMetrics().lineSpacing();
  title += QString::number(fd.year());
  p.drawText( 0, lineSpacing * 1 + 4, width, newlineSpacing,
              Qt::AlignRight | Qt::AlignTop, title );

  p.setFont( oldFont );
}

#endif
