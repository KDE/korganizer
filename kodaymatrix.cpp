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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kodaymatrix.h"
#include "koprefs.h"
#include "koglobals.h"
#include "kodialogmanager.h"

#include <kcal/vcaldrag.h>
#include <kcal/icaldrag.h>
#include <kcal/dndfactory.h>
#include <kcal/calendarresources.h>
#include <kcal/resourcecalendar.h>

#include <kaction.h>
#include <kstandardaction.h>
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcalendarsystem.h>
#ifndef KORG_NODND
#include <kmenu.h>
#endif

#include <QEvent>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QDragMoveEvent>
#include <QFrame>
#include <QDropEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QToolTip>
#ifndef KORG_NODND
#include <QCursor>
#endif

#include "kodaymatrix.moc"

// ============================================================================
//  K O D A Y M A T R I X
// ============================================================================

const int KODayMatrix::NOSELECTION = -1000;
const int KODayMatrix::NUMDAYS = 42;

KODayMatrix::KODayMatrix( QWidget *parent )
  : QFrame( parent ), mCalendar( 0 ), mStartDate(), mPendingChanges( false )
{
  // initialize dynamic arrays
  mDays = new QDate[NUMDAYS];
  mDayLabels = new QString[NUMDAYS];

  mTodayMarginWidth = 2;
  mSelEnd = mSelStart = NOSELECTION;

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
    connect( calres, SIGNAL(signalResourceAdded(ResourceCalendar *)),
             SLOT(resourcesChanged()) );
    connect( calres, SIGNAL(signalResourceModified(ResourceCalendar *)),
             SLOT(resourcesChanged()) );
    connect( calres, SIGNAL(signalResourceDeleted(ResourceCalendar *)),
             SLOT(resourcesChanged()) );
  }

  setAcceptDrops( mCalendar != 0 );
  updateEvents();
}

QColor KODayMatrix::getShadedColor( const QColor &color ) const
{
  QColor shaded;
  int h = 0;
  int s = 0;
  int v = 0;
  color.getHsv( &h, &s, &v );
  s = s / 4;
  v = 192 + v / 4;
  shaded.setHsv( h, s, v );

  return shaded;
}

KODayMatrix::~KODayMatrix()
{
  if ( mCalendar ) {
    mCalendar->unregisterObserver( this );
  }

  delete [] mDays;
  delete [] mDayLabels;
}

void KODayMatrix::addSelectedDaysTo( DateList &selDays )
{
  if ( mSelStart == NOSELECTION ) {
    return;
  }

  // cope with selection being out of matrix limits at top (< 0)
  int i0 = mSelStart;
  if ( i0 < 0 ) {
    for ( int i = i0; i < 0; i++ ) {
      selDays.append( mDays[0].addDays( i ) );
    }
    i0 = 0;
  }

  // cope with selection being out of matrix limits at bottom (> NUMDAYS-1)
  if ( mSelEnd > NUMDAYS-1 ) {
    for ( int i = i0; i <= NUMDAYS - 1; i++ ) {
      selDays.append( mDays[i] );
    }
    for ( int i = NUMDAYS; i < mSelEnd; i++ ) {
      selDays.append( mDays[0].addDays( i ) );
    }
  } else {
    // apply normal routine to selection being entirely within matrix limits
    for ( int i = i0; i <= mSelEnd; i++ ) {
      selDays.append( mDays[i] );
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
  if ( !mStartDate.isValid() ) {
    return;
  }

  mToday = -1;
  for ( int i = 0; i < NUMDAYS; i++ ) {
    mDays[i] = mStartDate.addDays( i );
    mDayLabels[i] = QString::number( KOGlobals::self()->calendarSystem()->day( mDays[i] ) );

    // if today is in the currently displayed month, hilight today
    if ( mDays[i].year() == QDate::currentDate().year() &&
         mDays[i].month() == QDate::currentDate().month() &&
         mDays[i].day() == QDate::currentDate().day() ) {
      mToday = i;
    }
  }
}

void KODayMatrix::updateView()
{
  updateView( mStartDate );
}

void KODayMatrix::updateView( const QDate &actdate )
{
  if ( !actdate.isValid() ) {
    return;
  }

  //flag to indicate if the starting day of the matrix has changed by this call
  bool daychanged = false;

  // if a new startdate is to be set then apply Cornelius's calculation
  // of the first day to be shown
  if ( actdate != mStartDate ) {
    // reset index of selection according to shift of starting date from
    // startdate to actdate.
    if ( mSelStart != NOSELECTION ) {
      int tmp = actdate.daysTo( mStartDate );
      // shift selection if new one would be visible at least partly !
      if ( mSelStart + tmp < NUMDAYS && mSelEnd + tmp >= 0 ) {
        // nested if required for next X display pushed from a different month
        // correction required. otherwise, for month forward and backward,
        // it must be avoided.
        if ( mSelStart > NUMDAYS || mSelStart < 0 ) {
          mSelStart = mSelStart + tmp;
        }
        if ( mSelEnd > NUMDAYS || mSelEnd < 0 ) {
          mSelEnd = mSelEnd + tmp;
        }
      }
    }

    mStartDate = actdate;
    daychanged = true;
  }

  if ( daychanged ) {
    recalculateToday();
  }

  // The calendar has not changed in the meantime and the selected range
  // is still the same so we can save the expensive updateEvents() call
  if ( !daychanged && !mPendingChanges ) {
    return;
  }

  // TODO_Recurrence: If we just change the selection, but not the data,
  // there's no need to update the whole list of events... This is just a
  // waste of computational power (and it takes forever!)
  updateEvents();
  for ( int i = 0; i < NUMDAYS; i++ ) {
    //if it is a holy day then draw it red. Sundays are consider holidays, too
    QStringList holidays = KOGlobals::self()->holiday( mDays[i] );
    QString holiStr;

    if ( ( KOGlobals::self()->calendarSystem()->dayOfWeek( mDays[i] ) ==
           KOGlobals::self()->calendarSystem()->weekDayOfPray() ) ||
         !holidays.isEmpty() ) {
      if ( !holidays.isEmpty() ) {
        holiStr = holidays.join( i18nc( "delimiter for joining holiday names", "," ) );
      }
      if ( holiStr.isEmpty() ) {
        holiStr = "";
      }
    }
    mHolidays[i] = holiStr;
  }
}

void KODayMatrix::updateEvents()
{
  if ( !mCalendar ) {
    return;
  }

  Event::List eventlist = mCalendar->events( mDays[0], mDays[NUMDAYS-1],
                                             mCalendar->timeSpec() );
  mEvents.clear();
  Event::List::ConstIterator it;

  for ( it=eventlist.constBegin(); it != eventlist.constEnd(); ++it ) {
    Event *event = *it;
    ushort recurType = event->recurrenceType();

    if ( !( recurType == Recurrence::rDaily  && !KOPrefs::instance()->mDailyRecur ) &&
         !( recurType == Recurrence::rWeekly && !KOPrefs::instance()->mWeeklyRecur ) ) {

      DateTimeList timeDateList;
      bool isRecurrent = event->recurs();
      int eventDuration = event->dtStart().daysTo( event->dtEnd() );

      if ( isRecurrent ) {
        //Its a recurring event, find out in which days it occurs
        timeDateList = event->recurrence()->timesInInterval(
          KDateTime( mDays[0], mCalendar->timeSpec() ),
          KDateTime( mDays[NUMDAYS-1], mCalendar->timeSpec() ) );
      } else {
        if ( event->dtStart().date() >= mDays[0] ) {
          timeDateList.append( event->dtStart() );
        } else {
          // The event starts in another month (not visible))
          timeDateList.append( KDateTime( mDays[0], mCalendar->timeSpec() ) );
        }
      }

      DateTimeList::iterator t;
      for ( t=timeDateList.begin(); t != timeDateList.end(); ++t ) {
        //This could be a multiday event, so iterate from dtStart() to dtEnd()
        QDate d = t->date();
        int j   = 0;

        QDate occurrenceEnd;
        if ( isRecurrent ) {
          occurrenceEnd = d.addDays( eventDuration );
        } else {
          occurrenceEnd = event->dtEnd().date();
        }

        do {

          mEvents.append( d );

          ++j;
          d = d.addDays( 1 );

        } while ( d <= occurrenceEnd && j < NUMDAYS );
      }
    }
  }
  mPendingChanges = false;
}

const QDate &KODayMatrix::getDate( int offset ) const
{
  if ( offset < 0 || offset > NUMDAYS - 1 ) {
    return mDays[0];
  }
  return mDays[offset];
}

QString KODayMatrix::getHolidayLabel( int offset ) const
{
  if ( offset < 0 || offset > NUMDAYS - 1 ) {
    return 0;
  }
  return mHolidays[offset];
}

int KODayMatrix::getDayIndexFrom( int x, int y ) const
{
  return 7 * ( y / mDaySize.height() ) +
         ( KOGlobals::self()->reverseLayout() ?
           6 - x / mDaySize.width() : x / mDaySize.width() );
}

void KODayMatrix::calendarIncidenceAdded( Incidence *incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KODayMatrix::calendarIncidenceChanged( Incidence *incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KODayMatrix::calendarIncidenceDeleted( Incidence *incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KODayMatrix::resourcesChanged()
{
  mPendingChanges = true;
}

// ----------------------------------------------------------------------------
//  M O U S E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

bool KODayMatrix::event( QEvent *event )
{
  if ( KOPrefs::instance()->mEnableToolTips && event->type() == QEvent::ToolTip ) {
    QHelpEvent *helpEvent = static_cast<QHelpEvent*>( event );

    // calculate which cell of the matrix the mouse is in
    QRect sz = frameRect();
    int dheight = sz.height() * 7 / 42;
    int dwidth = sz.width() / 7;
    int row = helpEvent->pos().y() / dheight;
    int col = helpEvent->pos().x() / dwidth;

    // show holiday names only
    QString tipText = getHolidayLabel( col + row * 7 );
    if ( !tipText.isEmpty() ) {
      QToolTip::showText( helpEvent->globalPos(), tipText );
    } else {
      QToolTip::hideText();
    }
  }
  return QWidget::event( event );
}

void KODayMatrix::mousePressEvent( QMouseEvent *e )
{
  mSelStart = getDayIndexFrom( e->x(), e->y() );
  if ( mSelStart > NUMDAYS - 1 ) {
    mSelStart = NUMDAYS - 1;
  }
  mSelInit = mSelStart;
}

void KODayMatrix::mouseReleaseEvent( QMouseEvent *e )
{
  int tmp = getDayIndexFrom( e->x(), e->y() );
  if ( tmp > NUMDAYS - 1 ) {
    tmp = NUMDAYS - 1;
  }

  if ( mSelInit > tmp ) {
    mSelEnd = mSelInit;
    if ( tmp != mSelStart ) {
      mSelStart = tmp;
      repaint();
    }
  } else {
    mSelStart = mSelInit;

    //repaint only if selection has changed
    if ( tmp != mSelEnd ) {
      mSelEnd = tmp;
      repaint();
    }
  }

  DateList daylist;
  if ( mSelStart < 0 ) {
    mSelStart = 0;
  }
  for ( int i = mSelStart; i <= mSelEnd; ++i ) {
    daylist.append( mDays[i] );
  }
  emit selected( ( const DateList )daylist );
}

void KODayMatrix::mouseMoveEvent( QMouseEvent *e )
{
  int tmp = getDayIndexFrom( e->x(), e->y() );
  if ( tmp > NUMDAYS - 1 ) {
    tmp = NUMDAYS - 1;
  }

  if ( mSelInit > tmp ) {
    mSelEnd = mSelInit;
    if ( tmp != mSelStart ) {
      mSelStart = tmp;
      repaint();
    }
  } else {
    mSelStart = mSelInit;

    //repaint only if selection has changed
    if ( tmp != mSelEnd ) {
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
  e->acceptProposedAction();
#ifndef KORG_NODND
  const QMimeData *md = e->mimeData();
  if ( !ICalDrag::canDecode( md ) && !VCalDrag::canDecode( md ) ) {
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
  const QMimeData *md = e->mimeData();
  if ( !ICalDrag::canDecode( md ) && !VCalDrag::canDecode( md ) ) {
    e->ignore();
    return;
  }

  e->accept();
#endif
}

void KODayMatrix::dragLeaveEvent( QDragLeaveEvent *dl )
{
  Q_UNUSED( dl );
#ifndef KORG_NODND
//  setPalette(oldPalette);
//  update();
#endif
}

void KODayMatrix::dropEvent( QDropEvent *e )
{
#ifndef KORG_NODND
  if ( !mCalendar ) {
    e->ignore();
    return;
  }

  DndFactory factory( mCalendar );
  Event *event = factory.createDropEvent( e );
  Todo *todo = factory.createDropTodo( e );
  if ( !event && !todo ) {
    e->ignore();
    return;
  }

  Todo *existingTodo = 0;
  Event *existingEvent = 0;

  // Find the incidence in the calendar, then we don't need the drag object any more
  if ( event ) {
    existingEvent = mCalendar->event( event->uid() );
  }
  if ( todo ) {
    existingTodo = mCalendar->todo( todo->uid() );
  }

  int action = DRAG_CANCEL;

  Qt::KeyboardModifiers keyboardModifiers = e->keyboardModifiers();

  if ( keyboardModifiers & Qt::ControlModifier ) {
    action = DRAG_COPY;
  } else if ( keyboardModifiers & Qt::ShiftModifier ) {
    action = DRAG_MOVE;
  } else {
    QAction *copy = 0, *move = 0, *cancel = 0;
    KMenu *menu = new KMenu( this );
    if ( existingEvent || existingTodo ) {
      move = menu->addAction( KOGlobals::self()->smallIcon( "edit-paste" ), i18n( "&Move" ) );
      if ( existingEvent ) {
        copy = menu->addAction( KOGlobals::self()->smallIcon( "edit-copy" ), i18n( "&Copy" ) );
      }
    } else {
      move = menu->addAction( KOGlobals::self()->smallIcon( "list-add" ), i18n( "&Add" ) );
    }
    menu->addSeparator();
    cancel = menu->addAction( KOGlobals::self()->smallIcon( "process-stop" ), i18n( "&Cancel" ) );
    QAction *a = menu->exec( QCursor::pos() );
    if ( a == copy ) {
      action = DRAG_COPY;
    } else if ( a == move ) {
      action = DRAG_MOVE;
    }
  }

  if ( action == DRAG_COPY  || action == DRAG_MOVE ) {
    e->accept();
    int idx = getDayIndexFrom( e->pos().x(), e->pos().y() );

    if ( action == DRAG_COPY ) {
      if ( event ) {
        emit incidenceDropped( event, mDays[idx] );
      }
      if ( todo ) {
        emit incidenceDropped( todo, mDays[idx] );
      }
    } else if ( action == DRAG_MOVE ) {
      if ( event ) {
        emit incidenceDroppedMove( event, mDays[idx] );
      }
      if ( todo ) {
        emit incidenceDroppedMove( todo, mDays[idx] );
      }
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
  QPainter p;
  QRect sz = frameRect();
  int dheight = mDaySize.height();
  int dwidth = mDaySize.width();
  int row, col;
  int selw, selh;
  bool isRTL = KOGlobals::self()->reverseLayout();

  QPalette pal = palette();

  p.begin( this );

  // draw background
  p.fillRect( 0, 0, sz.width(), sz.height(), QBrush( pal.color( QPalette::Base ) ) );

  // draw topleft frame
  p.setPen( pal.color( QPalette::Mid ) );
  p.drawRect( 0, 0, sz.width() - 1, sz.height() - 1 );
  // don't paint over borders
  p.translate( 1, 1 );

  // draw selected days with highlighted background color
  if ( mSelStart != NOSELECTION ) {

    row = mSelStart / 7;
    // fix larger selections starting in the previous month
    if ( row < 0 && mSelEnd > 0 ) {
      row = 0;
    }
    col = mSelStart - row * 7;
    QColor selcol = KOPrefs::instance()->agendaGridHighlightColor();

    if ( row < 6 && row >= 0 ) {
      if ( row == mSelEnd / 7 ) {
        // Single row selection
        p.fillRect( isRTL ? ( 7 - ( mSelEnd - mSelStart + 1 ) - col ) * dwidth : col * dwidth,
                    row * dheight,
                    ( mSelEnd - mSelStart + 1 ) * dwidth, dheight, selcol );
      } else {
        // draw first row to the right
        p.fillRect( isRTL ? 0 : col * dwidth, row * dheight,
                    ( 7 - col ) * dwidth, dheight, selcol );
        // draw full block till last line
        selh = mSelEnd / 7 - row;
        if ( selh + row >= 6 ) {
          selh = 6 - row;
        }
        if ( selh > 1 ) {
          p.fillRect( 0, ( row + 1 ) * dheight, 7 * dwidth,
                      ( selh - 1 ) * dheight, selcol );
        }
        // draw last block from left to mSelEnd
        if ( mSelEnd / 7 < 6 ) {
          selw = mSelEnd - 7 * ( mSelEnd / 7 ) + 1;
          p.fillRect( isRTL ? ( 7 - selw ) * dwidth : 0, ( row + selh ) * dheight,
                      selw * dwidth, dheight, selcol );
        }
      }
    }
  }

  // iterate over all days in the matrix and draw the day label in appropriate colors
  QColor textColor = pal.color( QPalette::Text );
  QColor textColorShaded = getShadedColor( textColor );
  QColor actcol = textColorShaded;
  p.setPen( actcol );
  QPen tmppen;
  for ( int i=0; i<NUMDAYS; ++i ) {
    row = i / 7;
    col = isRTL ? 6 - ( i - row * 7 ) : i - row * 7;

    // if it is the first day of a month switch color from normal to shaded and vice versa
    if ( KOGlobals::self()->calendarSystem()->day( mDays[i] ) == 1 ) {
      if ( actcol == textColorShaded ) {
        actcol = textColor;
      } else {
        actcol = textColorShaded;
      }
      p.setPen( actcol );
    }

    //Reset pen color after selected days block
    if ( i == mSelEnd + 1 ) {
      p.setPen( actcol );
    }

    bool holiday = ! KOGlobals::self()->isWorkDay( mDays[i] );
    QColor holidayColorShaded =
      getShadedColor( KOPrefs::instance()->agendaHolidaysBackgroundColor() );

    // if today then draw rectangle around day
    if ( mToday == i ) {
      tmppen = p.pen();
      QPen mTodayPen( p.pen() );

      mTodayPen.setWidth( mTodayMarginWidth );
      //draw red rectangle for holidays
      if ( holiday ) {
        if ( actcol == textColor ) {
          mTodayPen.setColor( KOPrefs::instance()->agendaHolidaysBackgroundColor() );
        } else {
          mTodayPen.setColor( holidayColorShaded );
        }
      }
      //draw gray rectangle for today if in selection
      if ( i >= mSelStart && i <= mSelEnd ) {
        QColor grey( "grey" );
        mTodayPen.setColor( grey );
      }
      p.setPen( mTodayPen );
      p.drawRect( col * dwidth, row * dheight, dwidth, dheight );
      p.setPen( tmppen );
    }

    // if any events are on that day then draw it using a bold font
    if ( mEvents.contains( mDays[i] ) ) {
      QFont myFont = font();
      myFont.setBold( true );
      p.setFont( myFont );
    }

    // if it is a holiday then use the default holiday color
    if ( holiday ) {
      if ( actcol == textColor ) {
        p.setPen( KOPrefs::instance()->agendaHolidaysBackgroundColor() );
      } else {
        p.setPen( holidayColorShaded );
      }
    }

    // draw selected days with special color
    // DO NOT specially highlight holidays in selection !
    if ( i >= mSelStart && i <= mSelEnd ) {
      p.setPen( QColor( "white" ) );
    }

    p.drawText( col * dwidth, row * dheight, dwidth, dheight,
                Qt::AlignHCenter | Qt::AlignVCenter, mDayLabels[i]);

    // reset color to actual color
    if ( holiday ) {
      p.setPen( actcol );
    }
    // reset bold font to plain font
    if ( mEvents.contains( mDays[i] ) > 0 ) {
      QFont myFont = font();
      myFont.setBold( false );
      p.setFont( myFont );
    }
  }
  p.end();
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
