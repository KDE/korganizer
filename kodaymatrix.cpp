/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Eitzenberger Thomas <thomas.eitzenberger@siemens.at>
    Parts of the source code have been copied from kdpdatebutton.cpp

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif
#include "koprefs.h"
#include "koglobals.h"
#include "kodialogmanager.h"

#include "kodaymatrix.h"
#include "kodaymatrix.moc"

#ifndef NODND
#include <qcursor.h>
#include <kpopupmenu.h>
#include <X11/Xlib.h>
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
  : QFrame( parent, name ), mCalendar( 0 )
{
  // initialize dynamic arrays
  mDays = new QDate[ NUMDAYS ];
  mDayLabels = new QString[ NUMDAYS ];
  mEvents = new int[ NUMDAYS ];
  mToolTip = new DynamicTip( this );

  // set default values used for drawing the matrix
  mDefaultBackColor = palette().active().base();
  mDefaultTextColor = palette().active().foreground();
  mDefaultTextColorShaded = getShadedColor( mDefaultTextColor );
  mHolidayColorShaded = getShadedColor( KOPrefs::instance()->mHolidayColor );
  mSelectedDaysColor = QColor( "white" );
  mTodayMarginWidth = 2;
  mSelEnd = mSelStart = NOSELECTION;
}

void KODayMatrix::setCalendar( Calendar *cal )
{
  mCalendar = cal;

  setAcceptDrops( mCalendar );

  updateEvents();
}

QColor KODayMatrix::getShadedColor( QColor color )
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
  mSelStart = mStartDate.daysTo( start );
  mSelEnd = mStartDate.daysTo( end );
}

void KODayMatrix::clearSelection()
{
  mSelEnd = mSelStart = NOSELECTION;
}

void KODayMatrix::recalculateToday()
{
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

void KODayMatrix::updateView( QDate actdate )
{
//  kdDebug(5850) << "KODayMatrix::updateView() " << actdate.toString() << endl;

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

  updateEvents();
  for( int i = 0; i < NUMDAYS; i++ ) {
    //if it is a holy day then draw it red. Sundays are consider holidays, too
#ifndef KORG_NOPLUGINS
    QString holiStr = KOCore::self()->holiday( mDays[ i ] );
#else
    QString holiStr = QString::null;
#endif
    if ( ( KOGlobals::self()->calendarSystem()->dayOfWeek( mDays[ i ] ) ==
           KOGlobals::self()->calendarSystem()->weekDayOfPray() ) ||
         !holiStr.isEmpty() ) {
      if ( holiStr.isNull() ) holiStr = "";
      mHolidays[ i ] = holiStr;
    } else {
      mHolidays[ i ] = QString::null;
    }
  }
}

void KODayMatrix::updateEvents()
{
  if ( !mCalendar ) return;

  for( int i = 0; i < NUMDAYS; i++ ) {
    // if events are set for the day then remember to draw it bold
    Event::List eventlist = mCalendar->events( mDays[ i ] );
    int numEvents = eventlist.count();
    Event::List::ConstIterator it;
    for( it = eventlist.begin(); it != eventlist.end(); ++it ) {
      Event *event = *it;
      ushort recurType = event->doesRecur();

      if ( ( recurType == Recurrence::rDaily &&
             !KOPrefs::instance()->mDailyRecur ) ||
           ( recurType == Recurrence::rWeekly &&
             !KOPrefs::instance()->mWeeklyRecur ) ) {
        numEvents--;
      }
    }
    mEvents[ i ] = numEvents;
  }
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
  for (int i = mSelStart; i <= mSelEnd; i++) {
    daylist.append(mDays[i]);
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

  Todo *existingTodo = 0, *oldTodo = 0;
  Event *existingEvent = 0, *oldEvent = 0;

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
  
    // When copying, clear the UID:
    if ( action == DRAG_COPY ) {
      if ( todo ) todo->recreate();
      if ( event ) event->recreate();
    } else {
      if ( existingEvent ) oldEvent = existingEvent->clone();
      if ( event ) delete event;
      event = existingEvent;
      if ( existingTodo ) oldTodo = existingTodo->clone();
      if ( todo ) delete todo;
      todo = existingTodo;
    }

    e->accept();
    if ( event ) {
      // Adjust date
      QDateTime start = event->dtStart();
      QDateTime end = event->dtEnd();
      int duration = start.daysTo( end );
      int idx = getDayIndexFrom( e->pos().x(), e->pos().y() );

      start.setDate( mDays[idx] );
      end.setDate( mDays[idx].addDays( duration ) );

      event->setDtStart( start );
      event->setDtEnd( end );
      // When moving, we don't need to insert  the item!
      if ( action != DRAG_MOVE ) {
        if ( !mCalendar->addIncidence( event ) ) {
          KODialogManager::errorSaveIncidence( this, event );
          return;
        }
      }

      if ( oldEvent ) {
        emit incidenceDroppedMove( oldEvent, event );
      } else {
        emit incidenceDropped( event );
      }
    }
    if ( todo ) {
      // Adjust date
      QDateTime due = todo->dtDue();
      int idx = getDayIndexFrom( e->pos().x(), e->pos().y() );
      due.setDate( mDays[idx] );

      todo->setDtDue( due );
      todo->setHasDueDate( true );

      // When moving, we don't need to insert  the item!
      if ( action != DRAG_MOVE ) {
        if ( !mCalendar->addIncidence( todo ) ) {
          KODialogManager::errorSaveIncidence( this, todo );
        }
      }

      if ( oldTodo ) {
        emit incidenceDroppedMove( oldTodo, todo );
      } else {
        emit incidenceDropped( todo );
      }
    }
  } else {
    if ( todo ) delete todo;
    if ( event ) delete event;
    e->ignore();
  }
#endif
}

// ----------------------------------------------------------------------------
//  P A I N T   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::paintEvent( QPaintEvent *pevent )
{
//kdDebug(5850) << "KODayMatrix::paintEvent() BEGIN" << endl;

  QPainter p(this);

  QRect sz = frameRect();
  int dheight = mDaySize.height();
  int dwidth = mDaySize.width();
  int row,col;
  int selw, selh;
  bool isRTL = KOGlobals::self()->reverseLayout();

  // draw background and topleft frame
  p.fillRect(pevent->rect(), mDefaultBackColor);
  p.setPen(mDefaultTextColor);
  p.drawRect(0, 0, sz.width()+1, sz.height()+1);
  // don't paint over borders
  p.translate(1,1);

  // draw selected days with highlighted background color
  if (mSelStart != NOSELECTION) {

    row = mSelStart/7;
    col = mSelStart -row*7;
    QColor selcol = KOPrefs::instance()->mHighlightColor;

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
      if (selh > 1) {
        p.fillRect(0, (row+1)*dheight, 7*dwidth, (selh-1)*dheight,selcol);
      }
      // draw last block from left to mSelEnd
      selw = mSelEnd-7*(mSelEnd/7)+1;
      p.fillRect(isRTL ? (7-selw)*dwidth : 0, (row+selh)*dheight,
                 selw*dwidth, dheight, selcol);
    }
  }

  // iterate over all days in the matrix and draw the day label in appropriate colors
  QColor actcol = mDefaultTextColorShaded;
  p.setPen(actcol);
  QPen tmppen;
  for(int i = 0; i < NUMDAYS; i++) {
    row = i/7;
    col = isRTL ? 6-(i-row*7) : i-row*7;

    // if it is the first day of a month switch color from normal to shaded and vice versa
    if ( KOGlobals::self()->calendarSystem()->day( mDays[i] ) == 1) {
      if (actcol == mDefaultTextColorShaded) {
        actcol = mDefaultTextColor;
      } else {
        actcol = mDefaultTextColorShaded;
      }
      p.setPen(actcol);
    }

    //Reset pen color after selected days block
    if (i == mSelEnd+1) {
      p.setPen(actcol);
    }

    // if today then draw rectangle around day
    if (mToday == i) {
      tmppen = p.pen();
      QPen mTodayPen(p.pen());

      mTodayPen.setWidth(mTodayMarginWidth);
      //draw red rectangle for holidays
      if (!mHolidays[i].isNull()) {
        if (actcol == mDefaultTextColor) {
          mTodayPen.setColor(KOPrefs::instance()->mHolidayColor);
        } else {
          mTodayPen.setColor(mHolidayColorShaded);
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
    if (!mHolidays[i].isNull()) {
      if (actcol == mDefaultTextColor) {
        p.setPen(KOPrefs::instance()->mHolidayColor);
      } else {
        p.setPen(mHolidayColorShaded);
      }
    }

    // draw selected days with special color
    // DO NOT specially highlight holidays in selection !
    if (i >= mSelStart && i <= mSelEnd) {
      p.setPen(mSelectedDaysColor);
    }

    p.drawText(col*dwidth, row*dheight, dwidth, dheight,
              Qt::AlignHCenter | Qt::AlignVCenter,  mDayLabels[i]);

    // reset color to actual color
    if (!mHolidays[i].isNull()) {
      p.setPen(actcol);
    }
    // reset bold font to plain font
    if (mEvents[i] > 0) {
      QFont myFont = font();
      myFont.setBold(false);
      p.setFont(myFont);
    }
  }
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
