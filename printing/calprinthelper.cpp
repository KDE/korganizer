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

#include <libkcal/calendar.h>
#include <libkcal/todo.h>
#include <libkcal/event.h>

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

  // print previous month for month view, print current for to-do, day and week
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
  for (int col = 0; col < 7; ++col) {
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
    bool fullDate, bool printRecurDaily, bool printRecurWeekly )
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
  QRect dayBox( x, y, width, height );
  p.drawRect( dayBox );
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
  QString text;
  p.setFont( QFont( "helvetica", 8 ) );

  int textY=mSubHeaderHeight+3; // gives the relative y-coord of the next printed entry
  Event::List::ConstIterator it;

  for( it = eventList.begin(); it != eventList.end() && textY<height; ++it ) {
    Event *currEvent = *it;
    if ( ( !printRecurDaily  && currEvent->doesRecur() == Recurrence::rDaily  ) ||
         ( !printRecurWeekly && currEvent->doesRecur() == Recurrence::rWeekly ) ) {
      continue; }
    if ( currEvent->doesFloat() || currEvent->isMultiDay() )
      text = "";
    else
      text = local->formatTime( currEvent->dtStart().time() );

    drawIncidence( p, dayBox, text, currEvent->summary(), textY );
  }

  if ( textY<height ) {
    Todo::List todos = mCalendar->todos( qd );
    Todo::List::ConstIterator it2;
    for( it2 = todos.begin(); it2 != todos.end() && textY<height; ++it2 ) {
      Todo *todo = *it2;
      if ( ( !printRecurDaily  && todo->doesRecur() == Recurrence::rDaily  ) ||
           ( !printRecurWeekly && todo->doesRecur() == Recurrence::rWeekly ) )
        continue;
      if ( todo->hasDueDate() && !todo->doesFloat() )
        text += KGlobal::locale()->formatTime(todo->dtDue().time()) + " ";
      else
        text = "";
      drawIncidence( p, dayBox, text, i18n("To-do: %1").arg(todo->summary()), textY );
    }
  }

  p.setFont( oldFont );
}

void CalPrintHelper::drawIncidence( QPainter &p, QRect &dayBox, const QString &time, const QString &summary, int &textY )
{
  kdDebug(5850) << "summary = " << summary << endl;

  int flags = Qt::AlignLeft;
  QFontMetrics fm = p.fontMetrics();
  QRect timeBound = p.boundingRect( dayBox.x() + 5, dayBox.y() + textY,
                                    dayBox.width() - 10, fm.lineSpacing(),
                                    flags, time );
  p.drawText( timeBound, flags, time );

  int summaryWidth = time.isEmpty() ? 0 : timeBound.width() + 4;
  QRect summaryBound = QRect( dayBox.x() + 5 + summaryWidth, dayBox.y() + textY,
                              dayBox.width() - summaryWidth -5, dayBox.height() );

  KWordWrap *ww = KWordWrap::formatText( fm, summaryBound, flags, summary );
  ww->drawText( &p, dayBox.x() + 5 + summaryWidth, dayBox.y() + textY, flags );

  textY += ww->boundingRect().height();

  delete ww;
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
                               bool recurDaily, bool recurWeekly, int x, int y,
                               int width, int height)
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
    for (int row = 0; row<rows; ++row ) {
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
  for ( int row = 0; row < rows; ++row ) {
    for ( int col = 0; col < 7; ++col ) {
      // show days from previous/next month with a grayed background
      if ( (monthDate < monthFirst) || (monthDate > monthLast) ) {
        p.setBackgroundColor( back.dark( 120 ) );
        darkbg = true;
      }
      drawDayBox(p, monthDate, x+xoffset+col*cellWidth, y+yoffset+row*cellHeight,
                 cellWidth, cellHeight, false,  recurDaily, recurWeekly );
      if ( darkbg ) {
        p.setBackgroundColor( back );
        darkbg = false;
      }
      monthDate = monthDate.addDays(1);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////

void CalPrintHelper::drawTodo( int &count, Todo *todo, QPainter &p,
                               TodoSortField sortField, SortDirection sortDir,
                               bool connectSubTodos, bool strikeoutCompleted,
                               bool desc, int posPriority, int posSummary,
                               int posDueDt, int posPercentComplete,
                               int level, int x, int &y, int width,
                               int pageHeight, const Todo::List &todoList,
                               TodoParentStart *r )
{
  QString outStr;
  const KLocale *local = KGlobal::locale();
  QRect rect;
  TodoParentStart startpt;

  // This list keeps all starting points of the parent to-dos so the connection
  // lines of the tree can easily be drawn (needed if a new page is started)
  static QPtrList<TodoParentStart> startPoints;
  if ( level < 1 ) {
    startPoints.clear();
  }

  // Compute the right hand side of the to-do box
  int rhs = posPercentComplete;
  if ( rhs < 0 ) rhs = posDueDt; //not printing percent completed
  if ( rhs < 0 ) rhs = x+width;  //not printing due dates either

  // size of to-do
  outStr=todo->summary();
  int left = posSummary + ( level*10 );
  rect = p.boundingRect( left, y, ( rhs-left-5 ), -1, Qt::WordBreak, outStr );
  if ( !todo->description().isEmpty() && desc ) {
    outStr = todo->description();
    rect = p.boundingRect( left+20, rect.bottom()+5, width-(left+10-x), -1,
                           Qt::WordBreak, outStr );
  }
  // if too big make new page
  if ( rect.bottom() > pageHeight ) {
    // first draw the connection lines from parent to-dos:
    if ( level > 0 && connectSubTodos ) {
      TodoParentStart *rct;
      for ( rct = startPoints.first(); rct; rct = startPoints.next() ) {
        int start;
        int center = rct->mRect.left() + (rct->mRect.width()/2);
        int to = p.viewport().bottom();

        // draw either from start point of parent or from top of the page
        if ( rct->mSamePage )
          start = rct->mRect.bottom() + 1;
        else
          start = p.viewport().top();
        p.moveTo( center, start );
        p.lineTo( center, to );
        rct->mSamePage = false;
      }
    }
    y=0;
    mPrinter->newPage();
  }

  // If this is a sub-to-do, r will not be 0, and we want the LH side
  // of the priority line up to the RH side of the parent to-do's priority
  bool showPriority = posPriority>=0;
  int lhs = posPriority;
  if ( r ) {
    lhs = r->mRect.right() + 1;
  }

  outStr.setNum( todo->priority() );
  rect = p.boundingRect( lhs, y + 10, 5, -1, Qt::AlignCenter, outStr );
  // Make it a more reasonable size
  rect.setWidth(18);
  rect.setHeight(18);

  // Draw a checkbox
  p.setBrush( QBrush( Qt::NoBrush ) );
  p.drawRect( rect );
  if ( todo->isCompleted() ) {
    // cross out the rectangle for completed to-dos
    p.drawLine( rect.topLeft(), rect.bottomRight() );
    p.drawLine( rect.topRight(), rect.bottomLeft() );
  }
  lhs = rect.right() + 3;

  // Priority
  if ( todo->priority() > 0 && showPriority ) {
    p.drawText( rect, Qt::AlignCenter, outStr );
  }
  startpt.mRect = rect; //save for later

  // Connect the dots
  if ( level > 0 && connectSubTodos ) {
    int bottom;
    int center( r->mRect.left() + (r->mRect.width()/2) );
    if ( r->mSamePage )
      bottom = r->mRect.bottom() + 1;
    else
      bottom = 0;
    int to( rect.top() + (rect.height()/2) );
    int endx( rect.left() );
    p.moveTo( center, bottom );
    p.lineTo( center, to );
    p.lineTo( endx, to );
  }

  // summary
  outStr=todo->summary();
  rect = p.boundingRect( lhs, rect.top(), (rhs-(left + rect.width() + 5)),
                         -1, Qt::WordBreak, outStr );

  QRect newrect;
  //FIXME: the following code prints underline rather than strikeout text
#if 0
  QFont f( p.font() );
  if ( todo->isCompleted() && strikeoutCompleted ) {
    f.setStrikeOut( true );
    p.setFont( f );
  }
  p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  f.setStrikeOut( false );
  p.setFont( f );
#endif
  //TODO: Remove this section when the code above is fixed
  p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  if ( todo->isCompleted() && strikeoutCompleted ) {
    // strike out the summary text if to-do is complete
    // Note: we tried to use a strike-out font and for unknown reasons the
    // result was underline instead of strike-out, so draw the lines ourselves.
    int delta = p.fontMetrics().lineSpacing();
    int lines = ( rect.height() / delta ) + 1;
    for ( int i=0; i<lines; i++ ) {
      p.moveTo( rect.left(),  rect.top() + ( delta/2 ) + ( i*delta ) );
      p.lineTo( rect.right(), rect.top() + ( delta/2 ) + ( i*delta ) );
    }
  }

  // due date
  if ( todo->hasDueDate() && posDueDt>=0 ) {
    outStr = local->formatDate( todo->dtDue().date(), true );
    rect = p.boundingRect( posDueDt, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  // percentage completed
  bool showPercentComplete = posPercentComplete>=0;
  if ( showPercentComplete ) {
    int lwidth = 24;
    int lheight = 12;
    //first, draw the progress bar
    int progress = (int)(( lwidth*todo->percentComplete())/100.0 + 0.5);

    p.setBrush( QBrush( Qt::NoBrush ) );
    p.drawRect( posPercentComplete, y+3, lwidth, lheight );
    if ( progress > 0 ) {
      p.setBrush( QBrush( Qt::Dense5Pattern ) );
      p.drawRect( posPercentComplete, y+3, progress, lheight );
    }

    //now, write the percentage
    outStr = i18n( "%1%" ).arg( todo->percentComplete() );
    rect = p.boundingRect( posPercentComplete+lwidth+3, y, x + width, -1,
                           Qt::AlignTop | Qt::AlignLeft, outStr );
    p.drawText( rect, Qt::AlignTop | Qt::AlignLeft, outStr );
  }

  // description
  if ( !todo->description().isEmpty() && desc ) {
    y = newrect.bottom() + 5;
    outStr = todo->description();
    rect = p.boundingRect( left+20, y, x+width-(left+10), -1,
                           Qt::WordBreak, outStr );
    p.drawText( rect, Qt::WordBreak, outStr, -1, &newrect );
  }

  // Set the new line position
  y = newrect.bottom() + 10; //set the line position

  // If the to-do has sub-to-dos, we need to call ourselves recursively
#if 0
  Incidence::List l = todo->relations();
  Incidence::List::ConstIterator it;
  startPoints.append( &startpt );
  for( it = l.begin(); it != l.end(); ++it ) {
    count++;
    // In the future, to-dos might also be related to events
    // Manually check if the sub-to-do is in the list of to-dos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if (subtodo && todoList.contains( subtodo ) ) {
      drawTodo( count, subtodo, p, connectSubTodos, strikeoutCompleted,
                desc, posPriority, posSummary, posDueDt, posPercentComplete,
                level+1, x, y, width, pageHeight, todoList, &startpt );
    }
  }
#endif
  // Make a list of all the sub-to-dos related to this to-do.
  Todo::List t;
  Incidence::List l = todo->relations();
  Incidence::List::ConstIterator it;
  for( it=l.begin(); it!=l.end(); ++it ) {
    // In the future, to-dos might also be related to events
    // Manually check if the sub-to-do is in the list of to-dos to print
    // The problem is that relations() does not apply filters, so
    // we need to compare manually with the complete filtered list!
    Todo* subtodo = dynamic_cast<Todo *>( *it );
    if ( subtodo && todoList.contains( subtodo ) ) {
      t.append( subtodo );
    }
  }

  // Sort the sub-to-dos and then print them
  Todo::List sl = mCalendar->sortTodos( &t, sortField, sortDir );
  Todo::List::ConstIterator isl;
  startPoints.append( &startpt );
  for( isl = sl.begin(); isl != sl.end(); ++isl ) {
    count++;
    drawTodo( count, ( *isl ), p, sortField,  sortDir,
              connectSubTodos, strikeoutCompleted,
              desc, posPriority, posSummary, posDueDt, posPercentComplete,
              level+1, x, y, width, pageHeight, todoList, &startpt );
  }
  startPoints.remove( &startpt );
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
