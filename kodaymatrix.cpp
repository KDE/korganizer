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

#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif


/*TODO
Destructor to tidy up all dynamically allocated stuff ! (DONE?)
Manage Selection of serial days ! (DONE)

dheight and dwidth should be stored and recalculated on resize only !
Manage update of selection by pressing toolbar buttons

*/


DynamicTip::DynamicTip( QWidget * parent )
    : QToolTip( parent )
{
    matrix = (KODayMatrix*)parent;
}


void DynamicTip::maybeTip( const QPoint &pos )
{
  QRect sz = matrix->frameRect();
  int dheight = sz.height()*7 / 42;
  int dwidth = sz.width() / 7;
  int row = pos.y()/dheight;
  int col = pos.x()/dwidth;

  QRect rct(col*dwidth, row*dheight, dwidth, dheight);

  kdDebug() << "DynamicTip::maybeTip index " << (col+row*7) << endl;
  QString str = matrix->getHolidayLabel(col+row*7);
  if (str.isEmpty()) return;

  kdDebug() << "DynamicTip::maybeTip #" << *str << "#" << endl;

  tip(rct, str);
}

KODayMatrix::KODayMatrix(QWidget *parent, Calendar* calendar, QDate date, const char *name) :
  QFrame(parent, name)
{
  mCalendar = calendar;

  // initialize dynamic arrays
  daylbls = new QString[NUMDAYS];
  days = new QDate[NUMDAYS];
  events = new int[NUMDAYS];
  mToolTip = new DynamicTip(this);

  // set default values used for drawing the matrix
  mDefaultBackColor = palette().active().base();
  mDefaultTextColor = palette().active().foreground();
  mDefaultTextColorShaded = getShadedColor(mDefaultTextColor);
  mHolidayColorShaded = getShadedColor(KOPrefs::instance()->mHolidayColor);
  mTodayMarginWidth = 2;
  mTodayPen = 0;
  mSelStart = -1;

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
  delete events;
  delete mToolTip;

  // pen is created on the fly in the paintEvent method, so make sure there
  // is a pen before deleting it
  if (mTodayPen != 0) delete mTodayPen;
}

void KODayMatrix::setStartDate(QDate start)
{
  updateView(start);
}

void KODayMatrix::addSelectedDaysTo(DateList& selDays)
{
  for (int i = mSelStart; i <= mSelEnd; i++) {
    selDays.append(/*new QDate*/days[i]);
  }
}

void KODayMatrix::setSelectedDaysFrom(const QDate& start, const QDate& end)
{
  mSelStart = startdate.daysTo(start);
  mSelEnd = startdate.daysTo(end);
  kdDebug() << "startdate" << startdate.day() << endl;
  kdDebug() << "selection:" << mSelStart << " - " << mSelEnd << endl;
}


void KODayMatrix::updateView()
{
  updateView(startdate);
}

void KODayMatrix::updateView(QDate actdate)
{
  bool daychanged = false;

  // if a new startdate is to be set then apply Cornelius's calculation
  // of the first day to be shown
  if (actdate != startdate) {
    // calculation is from Cornelius
    int fstDayOfWk = actdate.dayOfWeek();
    int nextLine = ((fstDayOfWk == 1) && (KGlobal::locale()->weekStartsMonday() == 1)) ? 7 : 0;
    int offset = (KGlobal::locale()->weekStartsMonday() ? 1 : 0) - fstDayOfWk - nextLine;
    startdate = actdate.addDays(offset);
    daychanged = true;
  }


  if (daychanged) {
    today = -1;
  }

  for(int i = 0; i < NUMDAYS; i++) {

    if (daychanged) {
      days[i] = startdate.addDays(i);
      daylbls[i] = QString::number(days[i].day());

      // if today is in the currently displayed month, hilight today
      if (days[i].year() == QDate::currentDate().year() &&
          days[i].month() == QDate::currentDate().month() &&
          days[i].day() == QDate::currentDate().day()) {
//        kdDebug() << "KODayMatrix::updateView() today:" << days[i].day() << "i" << i << endl;
        today = i;
      }
    }


    // if events are set for the day then remember to draw it bold
    int numEvents = 0;
    QPtrList<Event> eventlist = mCalendar->getEventsForDate(days[i]);
    Event *event;
    for(event=eventlist.first();event != 0;event=eventlist.next()) {
      ushort recurType = event->recurrence()->doesRecur();
//      kdDebug() << "KODayMatrix::updateView recurType " << recurType << endl;
      if ((recurType == Recurrence::rNone) ||
          (recurType == Recurrence::rDaily && KOPrefs::instance()->mDailyRecur) ||
          (recurType == Recurrence::rWeekly && KOPrefs::instance()->mWeeklyRecur)) {
        numEvents++;
        break;
      }
    }
    events[i] = numEvents;
//    kdDebug() << "KODayMatrix::updateView events " << i << " = " << events[i] << endl;

    //if it is a holy day then draw it red
#ifndef KORG_NOPLUGINS
    QString holiStr = KOCore::self()->holiday(days[i]);
#else
    QString holiStr = QString::null;
#endif
    // Calculate holidays. Sunday is also treated as holiday.
    if (!KGlobal::locale()->weekStartsMonday() && (float(i)/7 == float(i/7)) ||
        KGlobal::locale()->weekStartsMonday() && (float(i-6)/7 == float((i-6)/7)) ||
        !holiStr.isEmpty()) {
      if (holiStr.isNull()) holiStr = "";
      mHolidays[i] = holiStr;

//      kdDebug() << "KODayMatrix::updateView holidays " << i << " = #" << holiStr << "#" << endl;
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
  QRect sz = frameRect();
  int dheight = sz.height()*7 / NUMDAYS;
  int dwidth = sz.width() / 7;
  return 7*(y/dheight) + x/dwidth;
}

// ----------------------------------------------------------------------------
//  M O U S E   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::mousePressEvent (QMouseEvent* e)
{
  //QRect sz = frameRect();
  //int dheight = sz.height()*7 / NUMDAYS;
  //int dwidth = sz.width() / 7;
  mSelStart = getDayIndexFrom(e->x(), e->y()); //7*(e->y()/dheight) + e->x()/dwidth;
  if (mSelStart > 41) mSelStart=41;
  mSelInit = mSelStart;
}

void KODayMatrix::mouseReleaseEvent (QMouseEvent* e)
{
  //QRect sz = frameRect();
  //int dheight = sz.height()*7 / NUMDAYS;
  //int dwidth = sz.width() / 7;
  int tmp = getDayIndexFrom(e->x(), e->y());//7*(e->y()/dheight) + e->x()/dwidth;
  if (tmp > 41) tmp=41;

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
  //QRect sz = frameRect();
  //int dheight = sz.height()*7 / NUMDAYS;
  //int dwidth = sz.width() / 7;
  int tmp = getDayIndexFrom(e->x(), e->y());//7*(e->y()/dheight) + e->x()/dwidth;
  if (tmp > 41) tmp=41;

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
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  // some visual feedback
//  oldPalette = palette();
//  setPalette(my_HilitePalette);
//  update();
}

void KODayMatrix::dragMoveEvent(QDragMoveEvent *e)
{
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  e->accept();
}

void KODayMatrix::dragLeaveEvent(QDragLeaveEvent */*dl*/)
{
//  setPalette(oldPalette);
//  update();
}

void KODayMatrix::dropEvent(QDropEvent *e)
{
  kdDebug() << "KODayMatrix::dropEvent(e) begin" << endl;

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  DndFactory factory( mCalendar );
  Event *event = factory.createDrop(e);

  if (event) {
    e->acceptAction();

    Event *existingEvent = mCalendar->getEvent(event->VUID());

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
    kdDebug() << "KODayMatrix::dropEvent(): Event from drop not decodable" << endl;
    e->ignore();
  }

}

// ----------------------------------------------------------------------------
//  P A I N T   E V E N T   H A N D L I N G
// ----------------------------------------------------------------------------

void KODayMatrix::paintEvent(QPaintEvent *)
{
  QPainter p(this);

  QRect sz = frameRect();
  int dheight = sz.height()*7 / NUMDAYS;
  int dwidth = sz.width() / 7;
  int row,col;
  int selw, selh;

  // draw background and topleft frame
  p.fillRect(0, 0, sz.width(), sz.height(), mDefaultBackColor);
  p.setPen(mDefaultTextColor);
  p.drawRect(0, 0, sz.width()+1, sz.height()+1);

  // draw selected days with highlighted background color
  if (mSelStart != -1) {

//    kdDebug() << "Sel:" << mSelStart << " - " << mSelEnd << endl;
    row = mSelStart/7;
    col = mSelStart -row*7;
    QColor selcol = KOPrefs::instance()->mHighlightColor;

    if (row == mSelEnd/7) {
      // Single row selection
      p.fillRect(col*dwidth, row*dheight, (mSelEnd-mSelStart+1)*dwidth, dheight, selcol);
    } else {
      // draw first row to the right
      p.fillRect(col*dwidth, row*dheight, (7-col)*dwidth, dheight, selcol);
      // draw full block till last line
      selh = mSelEnd/7-row;
      if (selh > 1) {
        p.fillRect(0, (row+1)*dheight, 7*dwidth, (selh-1)*dheight,selcol);
      }
      // draw last block from left to mSelEnd
      selw = mSelEnd-7*(mSelEnd/7)+1;
      p.fillRect(0, (row+selh)*dheight, selw*dwidth, dheight, selcol);
    }
  }

  QColor actcol = mDefaultTextColorShaded;
  p.setPen(actcol);
  QPen tmppen;
  for(int i = 0; i < NUMDAYS; i++) {
    row = i/7;
    col = i-row*7;

    //TODO ET: if i > mSelStart and <= mSelEnd then set textcolor to white(?)

    // if it is the first day of a month switch color from normal to shaded and vice versa
    if (days[i].day() == 1) {
      if (actcol == mDefaultTextColorShaded) {
        actcol = mDefaultTextColor;
      } else {
        actcol = mDefaultTextColorShaded;
      }
      p.setPen(actcol);
    }

    // if today then draw rectangle around day
    if (today == i) {
      tmppen = p.pen();
      mTodayPen = new QPen(tmppen);
      mTodayPen->setWidth(mTodayMarginWidth);
      p.setPen(*mTodayPen);
      p.drawRect(col*dwidth, row*dheight, dwidth, dheight);
      p.setPen(tmppen);
    }

    // if any events are ont hat day then draw it using a bold font
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
