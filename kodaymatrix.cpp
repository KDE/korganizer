/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Eitzenberger Thomas <thomas.eitzenberger@siemens.at>
    Parts of the source code have been copied from kdpdatebutton.cpp

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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

#include <libkcal/vcaldrag.h>
#include <libkcal/dndfactory.h>

#include "koprefs.h"
#include "kodaymatrix.h"
#include "kodaymatrix.moc"

#include <calendarsystem/kcalendarsystem.h>

#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif


// ============================================================================
//  D Y N A M I C   T I P
// ============================================================================

DynamicTip::DynamicTip( QWidget * parent )
    : QToolTip( parent )
{
    matrix = (KODayMatrix*)parent;
}


void DynamicTip::maybeTip( const QPoint &pos )
{
  //calculate which cell of the matrix the mouse is in
  QRect sz = matrix->frameRect();
  int dheight = sz.height()*7 / 42;
  int dwidth = sz.width() / 7;
  int row = pos.y()/dheight;
  int col = pos.x()/dwidth;

  QRect rct(col*dwidth, row*dheight, dwidth, dheight);

//  kdDebug() << "DynamicTip::maybeTip matrix cell index [" <<
//                col << "][" << row << "] => " <<(col+row*7) << endl;

  //show holiday names only
  QString str = matrix->getHolidayLabel(col+row*7);
  if (str.isEmpty()) return;
  tip(rct, str);
}


// ============================================================================
//  K O D A Y M A T R I X
// ============================================================================

const int KODayMatrix::NOSELECTION = -1000;
const int KODayMatrix::NUMDAYS = 42;

KODayMatrix::KODayMatrix(QWidget *parent, Calendar* calendar, QDate date, const char *name, KCalendarSystem* calSys) :
  QFrame(parent, name)
{
  mCalendar = calendar;
  mCalendarSystem = calSys;

  // initialize dynamic arrays
  days = new QDate[NUMDAYS];
  daylbls = new QString[NUMDAYS];
  events = new int[NUMDAYS];
  mToolTip = new DynamicTip(this);

  // set default values used for drawing the matrix
  mDefaultBackColor = palette().active().base();
  mDefaultTextColor = palette().active().foreground();
  mDefaultTextColorShaded = getShadedColor(mDefaultTextColor);
  mHolidayColorShaded = getShadedColor(KOPrefs::instance()->mHolidayColor);
  mSelectedDaysColor = QColor("white");
  mTodayMarginWidth = 2;
  mSelEnd = mSelStart = NOSELECTION;

  setAcceptDrops(true);

  updateView(date);
}

QColor KODayMatrix::getShadedColor(QColor color)
{
  QColor shaded;
  int h=0;
  int s=0;
  int v=0;
  color.hsv(&h,&s,&v);
  s = s/4;
  v = 192+v/4;
  shaded.setHsv(h,s,v);

  return shaded;
}

KODayMatrix::~KODayMatrix()
{
  delete [] days;
  delete [] daylbls;
  delete [] events;
  delete mToolTip;
}

/*
void KODayMatrix::setStartDate(QDate start)
{
  updateView(start);
}
*/

void KODayMatrix::addSelectedDaysTo(DateList& selDays)
{
  kdDebug() << "KODayMatrix::addSelectedDaysTo() - " << "mSelStart:" << mSelStart << endl;

  if (mSelStart == NOSELECTION) {
    return;
  }

  //cope with selection being out of matrix limits at top (< 0)
  int i0 = mSelStart;
  if (i0 < 0) {
    for (int i = i0; i < 0; i++) {
      selDays.append(days[0].addDays(i));
    }
    i0 = 0;
  }

  //cope with selection being out of matrix limits at bottom (> NUMDAYS-1)
  if (mSelEnd > NUMDAYS-1) {
    for (int i = i0; i <= NUMDAYS-1; i++) {
      selDays.append(days[i]);
    }
    for (int i = NUMDAYS; i < mSelEnd; i++) {
      selDays.append(days[0].addDays(i));
    }

  // apply normal routine to selection being entirely within matrix limits
  } else {
    for (int i = i0; i <= mSelEnd; i++) {
      selDays.append(days[i]);
    }
  }
}

void KODayMatrix::setSelectedDaysFrom(const QDate& start, const QDate& end)
{
  //kdDebug() << "startdate = " << startdate.day() << endl;
  mSelStart = startdate.daysTo(start);
  mSelEnd = startdate.daysTo(end);
}


void KODayMatrix::updateView()
{
  updateView(startdate);
}

void KODayMatrix::updateView(QDate actdate)
{
  //flag to indicate if the starting day of the matrix has changed by this call
  bool daychanged = false;

  // if a new startdate is to be set then apply Cornelius's calculation
  // of the first day to be shown
  if (actdate != startdate) {
    // calculation is from Cornelius
    int fstDayOfWk = mCalendarSystem->dayOfTheWeek( actdate );
    
    int nextLine = ((fstDayOfWk == 1) && (KGlobal::locale()->weekStartsMonday() == 1)) ? 7 : 0;

    int offset = (KGlobal::locale()->weekStartsMonday() ? 1 : 0) - fstDayOfWk - nextLine;

    // reset index of selection according to shift of starting date from startdate to actdate
    if (mSelStart != NOSELECTION) {
      int tmp = actdate.daysTo(startdate);
      //kdDebug() << "Shift of Selection1: " << mSelStart << " - " << mSelEnd << " -> " << tmp << "(" << offset << ")" << endl;
      // shift selection if new one would be visible at least partly !
      if (mSelStart+tmp < NUMDAYS && mSelEnd+tmp >= 0) {
        mSelStart = mSelStart + tmp;
        mSelEnd = mSelEnd + tmp;
      }
    }

    startdate = actdate;
    daychanged = true;
  }

  if (daychanged) {
    today = -1;
  }

  for(int i = 0; i < NUMDAYS; i++) {
    if (daychanged) {    
      days[i] = startdate.addDays(i);
      daylbls[i] = QString::number( mCalendarSystem->getDay( days[i] ));

      // if today is in the currently displayed month, hilight today
      if (days[i].year() == QDate::currentDate().year() &&
          days[i].month() == QDate::currentDate().month() &&
          days[i].day() == QDate::currentDate().day()) {
        today = i;
      }
    }

    // if events are set for the day then remember to draw it bold
    QPtrList<Event> eventlist = mCalendar->getEventsForDate(days[i]);
    Event *event;
    int numEvents = eventlist.count();

    for(event=eventlist.first();event != 0;event=eventlist.next()) {
      ushort recurType = event->recurrence()->doesRecur();

      if ((recurType == Recurrence::rDaily && !KOPrefs::instance()->mDailyRecur) ||
          (recurType == Recurrence::rWeekly && !KOPrefs::instance()->mWeeklyRecur)) {
        numEvents--;
      }
    }
    events[i] = numEvents;

    //if it is a holy day then draw it red. Sundays are consider holidays, too
#ifndef KORG_NOPLUGINS
    QString holiStr = KOCore::self()->holiday(days[i]);
#else
    QString holiStr = QString::null;
#endif
    if (!KGlobal::locale()->weekStartsMonday() && (float(i)/7 == float(i/7)) ||
        KGlobal::locale()->weekStartsMonday() && (float(i-6)/7 == float((i-6)/7)) ||
        !holiStr.isEmpty()) {
      if (holiStr.isNull()) holiStr = "";
      mHolidays[i] = holiStr;

    } else {
      mHolidays[i] = QString::null;
    }
  }
}

const QDate& KODayMatrix::getDate(int offset)
{
  if (offset < 0 || offset > NUMDAYS-1) {
    kdDebug() << "Wrong offset (" << offset << ") in KODayMatrix::getDate(int)" << endl;
    return days[0];
  }
  return days[offset];
}

QString KODayMatrix::getHolidayLabel(int offset)
{
  if (offset < 0 || offset > NUMDAYS-1) {
    kdDebug() << "Wrong offset (" << offset << ") in KODayMatrix::getHolidayLabel(int)" << endl;
    return 0;
  }
  return mHolidays[offset];
}

int KODayMatrix::getDayIndexFrom(int x, int y)
{
  return 7*(y/daysize.height()) + (QApplication::reverseLayout() ? 
            6 - x/daysize.width() : x/daysize.width());
}

// ----------------------------------------------------------------------------
//  M O U S E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::mousePressEvent (QMouseEvent* e)
{
  mSelStart = getDayIndexFrom(e->x(), e->y());
  if (mSelStart > NUMDAYS-1) mSelStart=NUMDAYS-1;
  mSelInit = mSelStart;
}

void KODayMatrix::mouseReleaseEvent (QMouseEvent* e)
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
  for (int i = mSelStart; i <= mSelEnd; i++) {
    daylist.append(days[i]);
  }
  emit selected((const DateList)daylist);
}

void KODayMatrix::mouseMoveEvent (QMouseEvent* e)
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

void KODayMatrix::dragEnterEvent(QDragEnterEvent *e)
{
#ifndef KORG_NODND
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  // some visual feedback
//  oldPalette = palette();
//  setPalette(my_HilitePalette);
//  update();
#endif
}

void KODayMatrix::dragMoveEvent(QDragMoveEvent *e)
{
#ifndef KORG_NODND
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  e->accept();
#endif
}

void KODayMatrix::dragLeaveEvent(QDragLeaveEvent */*dl*/)
{
#ifndef KORG_NODND
//  setPalette(oldPalette);
//  update();
#endif
}

void KODayMatrix::dropEvent(QDropEvent *e)
{
#ifndef KORG_NODND
//  kdDebug() << "KODayMatrix::dropEvent(e) begin" << endl;

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  DndFactory factory( mCalendar );
  Event *event = factory.createDrop(e);

  if (event) {
    e->acceptAction();

    Event *existingEvent = mCalendar->getEvent(event->uid());

    if(existingEvent) {
      // uniquify event
      event->recreate();
/*
      KMessageBox::sorry(this,
              i18n("Event already exists in this calendar."),
              i18n("Drop Event"));
      delete event;
      return;
*/
    }
//      kdDebug() << "Drop new Event" << endl;
    // Adjust date
    QDateTime start = event->dtStart();
    QDateTime end = event->dtEnd();
    int duration = start.daysTo(end);
    int idx = getDayIndexFrom(e->pos().x(), e->pos().y());

    start.setDate(days[idx]);
    end.setDate(days[idx].addDays(duration));

    event->setDtStart(start);
    event->setDtEnd(end);
    mCalendar->addEvent(event);

    emit eventDropped(event);
  } else {
//    kdDebug() << "KODayMatrix::dropEvent(): Event from drop not decodable" << endl;
    e->ignore();
  }
#endif
}

// ----------------------------------------------------------------------------
//  P A I N T   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::paintEvent(QPaintEvent * pevent)
{
//kdDebug() << "KODayMatrix::paintEvent() BEGIN" << endl;

  QPainter p(this);

  QRect sz = frameRect();
  int dheight = daysize.height();
  int dwidth = daysize.width();
  int row,col;
  int selw, selh;
  bool isRTL = QApplication::reverseLayout();

  // draw background and topleft frame
  p.fillRect(pevent->rect(), mDefaultBackColor);
  p.setPen(mDefaultTextColor);
  p.drawRect(0, 0, sz.width()+1, sz.height()+1);

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
    if ( mCalendarSystem->getDay( days[i] ) == 1) {
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
    if (today == i) {
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
    if (events[i] > 0) {
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
              Qt::AlignHCenter | Qt::AlignVCenter,  daylbls[i]);

    // reset color to actual color
    if (!mHolidays[i].isNull()) {
      p.setPen(actcol);
    }
    // reset bold font to plain font
    if (events[i] > 0) {
      QFont myFont = font();
      myFont.setBold(false);
      p.setFont(myFont);
    }
  }
}

// ----------------------------------------------------------------------------
//  R E SI Z E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::resizeEvent(QResizeEvent *)
{
  QRect sz = frameRect();
  daysize.setHeight(sz.height()*7 / NUMDAYS);
  daysize.setWidth(sz.width() / 7);
}
