/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Eitzenberger Thomas <thomas.eitzenberger@siemens.at>
    Parts of the source code have been copied from kdpdatebutton.cpp

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qevent.h>
#include <qpainter.h>
#include <qptrlist.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/icaldrag.h>
#include <libkcal/dndfactory.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>

#include <kcalendarsystem.h>

#include "koprefs.h"
#include "koglobals.h"
#include "kodialogmanager.h"

#include "kodaymatrix.h"
#include "kodaymatrix.moc"

#ifndef NODND
#include <qcursor.h>
#include <kpopupmenu.h>
#include <X11/Xlib.h>
#undef FocusIn
#undef KeyPress
#undef None
#undef Status
#endif

// ============================================================================
//  D Y N A M I C   T I P
// ============================================================================

DynamicTip::DynamicTip( QWidget * parent )
    : QToolTip( parent )
{
  mMatrix = static_cast<KODayMatrix *>( parent );
}


void DynamicTip::maybeTip( const QPoint &pos )
{
  //calculate which cell of the matrix the mouse is in
  QRect sz = mMatrix->frameRect();
  int dheight = sz.height() * 7 / 42;
  int dwidth = sz.width() / 7;
  int row = pos.y() / dheight;
  int col = pos.x() / dwidth;

  QRect rct( col * dwidth, row * dheight, dwidth, dheight );

//  kdDebug(5850) << "DynamicTip::maybeTip matrix cell index [" <<
//                col << "][" << row << "] => " <<(col+row*7) << endl;

  //show holiday names only
  QString str = mMatrix->getHolidayLabel( col + row * 7 );
  if ( str.isEmpty() ) return;
  tip( rct, str );
}


// ============================================================================
//  K O D A Y M A T R I X
// ============================================================================

const int KODayMatrix::NOSELECTION = -1000;
const int KODayMatrix::NUMDAYS = 42;

KODayMatrix::KODayMatrix( QWidget *parent, const char *name )
  : QFrame( parent, name ), mCalendar( 0 ), mStartDate(), mPendingChanges( false )
{
  // initialize dynamic arrays
  mDays = new QDate[ NUMDAYS ];
  mDayLabels = new QString[ NUMDAYS ];
  mEvents = new int[ NUMDAYS ];
  mToolTip = new DynamicTip( this );

  mTodayMarginWidth = 2;
  mSelEnd = mSelStart = NOSELECTION;
  setBackgroundMode( NoBackground );
  recalculateToday();
}

void KODayMatrix::setCalendar( Calendar *cal )
{
  if ( mCalendar ) {
    mCalendar->unregisterObserver( this );
    mCalendar->disconnect( this );
  }

  mCalendar = cal;
  mCalendar->registerObserver( this );
  CalendarResources *calres = dynamic_cast<CalendarResources*>( cal );
  if ( calres ) {
    connect( calres, SIGNAL(signalResourceAdded(ResourceCalendar *)), SLOT(resourcesChanged()) );
    connect( calres, SIGNAL(signalResourceModified( ResourceCalendar *)), SLOT(resourcesChanged()) );
    connect( calres, SIGNAL(signalResourceDeleted(ResourceCalendar *)), SLOT(resourcesChanged()) );
  }

  setAcceptDrops( mCalendar );

  updateEvents();
}

QColor KODayMatrix::getShadedColor( const QColor &color )
{
  QColor shaded;
  int h = 0;
  int s = 0;
  int v = 0;
  color.hsv( &h, &s, &v );
  s = s / 4;
  v = 192 + v / 4;
  shaded.setHsv( h, s, v );

  return shaded;
}

KODayMatrix::~KODayMatrix()
{
  if ( mCalendar )
    mCalendar->unregisterObserver( this );
  delete [] mDays;
  delete [] mDayLabels;
  delete [] mEvents;
  delete mToolTip;
}

void KODayMatrix::addSelectedDaysTo( DateList &selDays )
{
  kdDebug(5850) << "KODayMatrix::addSelectedDaysTo() - " << "mSelStart:" << mSelStart << endl;

  if ( mSelStart == NOSELECTION ) {
    return;
  }

  // cope with selection being out of matrix limits at top (< 0)
  int i0 = mSelStart;
  if ( i0 < 0 ) {
    for ( int i = i0; i < 0; i++ ) {
      selDays.append( mDays[ 0 ].addDays( i ) );
    }
    i0 = 0;
  }

  // cope with selection being out of matrix limits at bottom (> NUMDAYS-1)
  if ( mSelEnd > NUMDAYS-1 ) {
    for ( int i = i0; i <= NUMDAYS - 1; i++ ) {
      selDays.append( mDays[ i ] );
    }
    for ( int i = NUMDAYS; i < mSelEnd; i++ ) {
      selDays.append( mDays[ 0 ].addDays( i ) );
    }
  } else {
    // apply normal routine to selection being entirely within matrix limits
    for ( int i = i0; i <= mSelEnd; i++ ) {
      selDays.append( mDays[ i ] );
    }
  }
}

void KODayMatrix::setSelectedDaysFrom( const QDate &start, const QDate &end )
{
  if ( mStartDate.isValid() ) {
    mSelStart = mStartDate.daysTo( start );
    mSelEnd = mStartDate.daysTo( end );
  }
}

void KODayMatrix::clearSelection()
{
  mSelEnd = mSelStart = NOSELECTION;
}

void KODayMatrix::recalculateToday()
{
  if ( !mStartDate.isValid() ) return;
  mToday = -1;
  for ( int i = 0; i < NUMDAYS; i++ ) {
    mDays[ i ] = mStartDate.addDays( i );
    mDayLabels[ i ] = QString::number( KOGlobals::self()->calendarSystem()->day( mDays[i] ));

    // if today is in the currently displayed month, hilight today
    if ( mDays[ i ].year() == QDate::currentDate().year() &&
         mDays[ i ].month() == QDate::currentDate().month() &&
         mDays[ i ].day() == QDate::currentDate().day() ) {
      mToday = i;
    }
  }
  // kdDegug(5850) << "Today is visible at "<< today << "." << endl;
}

/* slot */ void KODayMatrix::updateView()
{
  updateView( mStartDate );
}

void KODayMatrix::updateView( const QDate &actdate )
{
 kdDebug(5850) << "KODayMatrix::updateView() " << actdate << ", day start="<<mStartDate<< endl;
 if ( !actdate.isValid() ) return;

  //flag to indicate if the starting day of the matrix has changed by this call
  bool daychanged = false;

  // if a new startdate is to be set then apply Cornelius's calculation
  // of the first day to be shown
  if ( actdate != mStartDate ) {
    // reset index of selection according to shift of starting date from startdate to actdate
    if ( mSelStart != NOSELECTION ) {
      int tmp = actdate.daysTo( mStartDate );
      //kdDebug(5850) << "Shift of Selection1: " << mSelStart << " - " << mSelEnd << " -> " << tmp << "(" << offset << ")" << endl;
      // shift selection if new one would be visible at least partly !

      if ( mSelStart + tmp < NUMDAYS && mSelEnd + tmp >= 0 ) {
        // nested if is required for next X display pushed from a different month - correction required
        // otherwise, for month forward and backward, it must be avoided
        if( mSelStart > NUMDAYS || mSelStart < 0 )
          mSelStart = mSelStart + tmp;
        if( mSelEnd > NUMDAYS || mSelEnd < 0 )
          mSelEnd = mSelEnd + tmp;
      }
    }

    mStartDate = actdate;
    daychanged = true;
  }

  if ( daychanged ) {
    recalculateToday();
  }

  // the calendar hasn't changed in the meantime and the selected range is still the same
  // so we can safe the expensive updateEvents() call
  if ( !daychanged && !mPendingChanges )
    return;

  // TODO_Recurrence: If we just change the selection, but not the data, there's
  // no need to update the whole list of events... This is just a waste of
  // computational power (and it takes forever!)
  updateEvents();
  for( int i = 0; i < NUMDAYS; i++ ) {
    //if it is a holy day then draw it red. Sundays are consider holidays, too
    QStringList holidays = KOGlobals::self()->holiday( mDays[ i ] );
    QString holiStr = QString::null;

    if ( ( KOGlobals::self()->calendarSystem()->dayOfWeek( mDays[ i ] ) ==
           KOGlobals::self()->calendarSystem()->weekDayOfPray() ) ||
         !holidays.isEmpty() ) {
      if ( !holidays.isEmpty() ) holiStr = holidays.join( i18n("delimiter for joining holiday names", ", " ) );
      if ( holiStr.isNull() ) holiStr = "";
    }
    mHolidays[ i ] = holiStr;
  }
}

void KODayMatrix::updateEvents()
{
  kdDebug( 5850 ) << k_funcinfo << endl;
  if ( !mCalendar ) return;

  for( int i = 0; i < NUMDAYS; i++ ) {
    // if events are set for the day then remember to draw it bold
    Event::List eventlist = mCalendar->events( mDays[ i ] );
    int numEvents = eventlist.count();
    Event::List::ConstIterator it;
    for( it = eventlist.begin(); it != eventlist.end(); ++it ) {
      Event *event = *it;
      ushort recurType = event->recurrenceType();
      if ( ( recurType == Recurrence::rDaily &&
             !KOPrefs::instance()->mDailyRecur ) ||
           ( recurType == Recurrence::rWeekly &&
             !KOPrefs::instance()->mWeeklyRecur ) ) {
        numEvents--;
      }
    }
    mEvents[ i ] = numEvents;
  }

  mPendingChanges = false;
}

const QDate& KODayMatrix::getDate( int offset )
{
  if ( offset < 0 || offset > NUMDAYS - 1 ) {
    kdDebug(5850) << "Wrong offset (" << offset << ") in KODayMatrix::getDate(int)" << endl;
    return mDays[ 0 ];
  }
  return mDays[ offset ];
}

QString KODayMatrix::getHolidayLabel( int offset )
{
  if ( offset < 0 || offset > NUMDAYS - 1 ) {
    kdDebug(5850) << "Wrong offset (" << offset << ") in KODayMatrix::getHolidayLabel(int)" << endl;
    return 0;
  }
  return mHolidays[ offset ];
}

int KODayMatrix::getDayIndexFrom( int x, int y )
{
  return 7 * ( y / mDaySize.height() ) +
         ( KOGlobals::self()->reverseLayout() ?
           6 - x / mDaySize.width() : x / mDaySize.width() );
}

void KODayMatrix::calendarIncidenceAdded(Incidence * incidence)
{
  mPendingChanges = true;
}

void KODayMatrix::calendarIncidenceChanged(Incidence * incidence)
{
  mPendingChanges = true;
}

void KODayMatrix::calendarIncidenceRemoved(Incidence * incidence)
{
  mPendingChanges = true;
}

void KODayMatrix::resourcesChanged()
{
  mPendingChanges = true;
}


// ----------------------------------------------------------------------------
//  M O U S E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::mousePressEvent( QMouseEvent *e )
{
  mSelStart = getDayIndexFrom(e->x(), e->y());
  if (mSelStart > NUMDAYS-1) mSelStart=NUMDAYS-1;
  mSelInit = mSelStart;
}

void KODayMatrix::mouseReleaseEvent( QMouseEvent *e )
{
  int tmp = getDayIndexFrom(e->x(), e->y());
  if (tmp > NUMDAYS-1) tmp=NUMDAYS-1;

  if (mSelInit > tmp) {
    mSelEnd = mSelInit;
    if (tmp != mSelStart) {
      mSelStart = tmp;
      repaint();
    }
  } else {
    mSelStart = mSelInit;

    //repaint only if selection has changed
    if (tmp != mSelEnd) {
      mSelEnd = tmp;
      repaint();
    }
  }

  DateList daylist;
  if ( mSelStart < 0 ) mSelStart = 0;
  for ( int i = mSelStart; i <= mSelEnd; ++i ) {
    daylist.append( mDays[i] );
  }
  emit selected((const DateList)daylist);
}

void KODayMatrix::mouseMoveEvent( QMouseEvent *e )
{
  int tmp = getDayIndexFrom(e->x(), e->y());
  if (tmp > NUMDAYS-1) tmp=NUMDAYS-1;

  if (mSelInit > tmp) {
    mSelEnd = mSelInit;
    if (tmp != mSelStart) {
      mSelStart = tmp;
      repaint();
    }
  } else {
    mSelStart = mSelInit;

    //repaint only if selection has changed
    if (tmp != mSelEnd) {
      mSelEnd = tmp;
      repaint();
    }
  }
}

// ----------------------------------------------------------------------------
//  D R A G ' N   D R O P   H A N D L I N G
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Drag and Drop handling -- based on the Troll Tech dirview example

enum {
  DRAG_COPY = 0,
  DRAG_MOVE = 1,
  DRAG_CANCEL = 2
};

void KODayMatrix::dragEnterEvent( QDragEnterEvent *e )
{
#ifndef KORG_NODND
  if ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }

  // some visual feedback
//  oldPalette = palette();
//  setPalette(my_HilitePalette);
//  update();
#endif
}

void KODayMatrix::dragMoveEvent( QDragMoveEvent *e )
{
#ifndef KORG_NODND
  if ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }

  e->accept();
#endif
}

void KODayMatrix::dragLeaveEvent( QDragLeaveEvent * /*dl*/ )
{
#ifndef KORG_NODND
//  setPalette(oldPalette);
//  update();
#endif
}

void KODayMatrix::dropEvent( QDropEvent *e )
{
#ifndef KORG_NODND
  kdDebug(5850) << "KODayMatrix::dropEvent(e) begin" << endl;

  if ( !mCalendar ||
       ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) ) ) {
    e->ignore();
    return;
  }

  DndFactory factory( mCalendar );
  Event *event = factory.createDrop( e );
  Todo *todo = factory.createDropTodo( e );
  if ( !event && !todo ) {
    e->ignore();
    return;
  }

  Todo *existingTodo = 0;
  Event *existingEvent = 0;

  // Find the incidence in the calendar, then we don't need the drag object any more
  if ( event ) existingEvent = mCalendar->event( event->uid() );
  if ( todo ) existingTodo = mCalendar->todo( todo->uid() );

  int action = DRAG_CANCEL;

  int root_x, root_y, win_x, win_y;
  uint keybstate;
  Window rootw, childw;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &rootw, &childw,
                 &root_x, &root_y, &win_x, &win_y, &keybstate );

  if ( keybstate & ControlMask ) {
    action = DRAG_COPY;
  } else if ( keybstate & ShiftMask ) {
    action = DRAG_MOVE;
  } else {
    KPopupMenu *menu = new KPopupMenu( this );
    if ( existingEvent || existingTodo ) {
      menu->insertItem( i18n("Move"), DRAG_MOVE, 0 );
      if (existingEvent)
        menu->insertItem( KOGlobals::self()->smallIcon("editcopy"), i18n("Copy"), DRAG_COPY, 1 );
    } else {
      menu->insertItem( i18n("Add"), DRAG_MOVE, 0 );
    }
    menu->insertSeparator();
    menu->insertItem( KOGlobals::self()->smallIcon("cancel"), i18n("Cancel"), DRAG_CANCEL, 3 );
    action = menu->exec( QCursor::pos(), 0 );
  }

  if ( action == DRAG_COPY  || action == DRAG_MOVE ) {
    e->accept();
    int idx = getDayIndexFrom( e->pos().x(), e->pos().y() );

    if ( action == DRAG_COPY ) {
      if ( event ) emit incidenceDropped( event, mDays[idx] );
      if ( todo )  emit incidenceDropped( todo, mDays[idx] );
    } else if ( action == DRAG_MOVE ) {
      if ( event ) emit incidenceDroppedMove( event, mDays[idx] );
      if ( todo )  emit incidenceDroppedMove( todo, mDays[idx] );
    }
  }
  delete event;
  delete todo;
#endif
}

// ----------------------------------------------------------------------------
//  P A I N T   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::paintEvent( QPaintEvent * )
{
// kdDebug(5850) << "KODayMatrix::paintEvent() BEGIN" << endl;

  QPainter p;
  QRect sz = frameRect();
  QPixmap pm( sz.size() );
  int dheight = mDaySize.height();
  int dwidth = mDaySize.width();
  int row,col;
  int selw, selh;
  bool isRTL = KOGlobals::self()->reverseLayout();

  QColorGroup cg = palette().active();

  p.begin(  &pm, this );
  pm.fill( cg.base() );

  // draw topleft frame
  p.setPen( cg.mid() );
  p.drawRect(0, 0, sz.width()-1, sz.height()-1);
  // don't paint over borders
  p.translate(1,1);

  // draw selected days with highlighted background color
  if (mSelStart != NOSELECTION) {

    row = mSelStart/7;
    // fix larger selections starting in the previous month
    if ( row < 0 && mSelEnd > 0 ) row = 0;
    col = mSelStart -row*7;
    QColor selcol = KOPrefs::instance()->mHighlightColor;

    if ( row < 6 && row >= 0 ) {
      if (row == mSelEnd/7) {
        // Single row selection
        p.fillRect(isRTL ? (7 - (mSelEnd-mSelStart+1) - col)*dwidth : col*dwidth,
                    row*dheight, (mSelEnd-mSelStart+1)*dwidth, dheight, selcol);
      } else {
        // draw first row to the right
        p.fillRect(isRTL ? 0 : col*dwidth, row*dheight, (7-col)*dwidth,
                   dheight, selcol);
        // draw full block till last line
        selh = mSelEnd/7-row;
        if ( selh + row >= 6 ) selh = 6-row;
        if ( selh > 1 ) {
          p.fillRect(0, (row+1)*dheight, 7*dwidth, (selh-1)*dheight,selcol);
        }
        // draw last block from left to mSelEnd
        if ( mSelEnd/7 < 6 ) {
          selw = mSelEnd-7*(mSelEnd/7)+1;
          p.fillRect(isRTL ? (7-selw)*dwidth : 0, (row+selh)*dheight,
                     selw*dwidth, dheight, selcol);
        }
      }
    }
  }

  // iterate over all days in the matrix and draw the day label in appropriate colors
  QColor textColor = cg.text();
  QColor textColorShaded = getShadedColor( textColor );
  QColor actcol = textColorShaded;
  p.setPen(actcol);
  QPen tmppen;
  for ( int i = 0; i < NUMDAYS; ++i ) {
    row = i/7;
    col = isRTL ? 6-(i-row*7) : i-row*7;

    // if it is the first day of a month switch color from normal to shaded and vice versa
    if ( KOGlobals::self()->calendarSystem()->day( mDays[i] ) == 1) {
      if (actcol == textColorShaded) {
        actcol = textColor;
      } else {
        actcol = textColorShaded;
      }
      p.setPen(actcol);
    }

    //Reset pen color after selected days block
    if (i == mSelEnd+1) {
      p.setPen(actcol);
    }

    bool holiday = ! KOGlobals::self()->isWorkDay( mDays[ i ] );

    QColor holidayColorShaded = getShadedColor( KOPrefs::instance()->mHolidayColor );
    // if today then draw rectangle around day
    if (mToday == i) {
      tmppen = p.pen();
      QPen mTodayPen(p.pen());

      mTodayPen.setWidth(mTodayMarginWidth);
      //draw red rectangle for holidays
      if (holiday) {
        if (actcol == textColor) {
          mTodayPen.setColor(KOPrefs::instance()->mHolidayColor);
        } else {
          mTodayPen.setColor(holidayColorShaded);
        }
      }
      //draw gray rectangle for today if in selection
      if (i >= mSelStart && i <= mSelEnd) {
        QColor grey("grey");
        mTodayPen.setColor(grey);
      }
      p.setPen(mTodayPen);
      p.drawRect(col*dwidth, row*dheight, dwidth, dheight);
      p.setPen(tmppen);
    }

    // if any events are on that day then draw it using a bold font
    if (mEvents[i] > 0) {
      QFont myFont = font();
      myFont.setBold(true);
      p.setFont(myFont);
    }

    // if it is a holiday then use the default holiday color
    if (holiday) {
      if (actcol == textColor) {
        p.setPen(KOPrefs::instance()->mHolidayColor);
      } else {
        p.setPen(holidayColorShaded);
      }
    }

    // draw selected days with special color
    // DO NOT specially highlight holidays in selection !
    if (i >= mSelStart && i <= mSelEnd) {
      p.setPen( QColor( "white" ) );
    }

    p.drawText(col*dwidth, row*dheight, dwidth, dheight,
              Qt::AlignHCenter | Qt::AlignVCenter,  mDayLabels[i]);

    // reset color to actual color
    if (holiday) {
      p.setPen(actcol);
    }
    // reset bold font to plain font
    if (mEvents[i] > 0) {
      QFont myFont = font();
      myFont.setBold(false);
      p.setFont(myFont);
    }
  }
  p.end();
  bitBlt(  this, 0, 0, &pm );
}

// ----------------------------------------------------------------------------
//  R E SI Z E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::resizeEvent( QResizeEvent * )
{
  QRect sz = frameRect();
  mDaySize.setHeight( sz.height() * 7 / NUMDAYS );
  mDaySize.setWidth( sz.width() / 7 );
}
