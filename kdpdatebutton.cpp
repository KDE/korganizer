// $Id$	

#include <qstring.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include "vcaldrag.h"
#include "calendar.h"
#include "koprefs.h"

#include "kdpdatebutton.h"
#include "kdpdatebutton.moc"


KDateButton::KDateButton(QDate date, int index, Calendar *calendar,
			 QWidget *parent, const char *name)
  : QLabel(parent, name)
{
  mSelected = false;
  mEvent = false;
  mItalic = false;
  mToday = false;
  mHoliday = false;
  mShaded = false;

  mTodayMarginWidth = 2;

//  setBackgroundMode(PaletteBase);

  mDefaultBackColor = palette().active().base();
  mDefaultTextColor = palette().active().foreground();
  mDefaultTextColorShaded = getShadedColor(mDefaultTextColor);

  mCalendar = calendar;

  setFrameStyle(QFrame::Box|QFrame::Plain);
  setLineWidth(0);
  setAlignment(AlignCenter);
  my_index = index;
  mDate = date;
  QString tstr;
  tstr.setNum(date.day());
  setText(tstr);
  adjustSize();

  setAcceptDrops(true);

  updateConfig();
}

KDateButton::~KDateButton()
{
}

void KDateButton::updateConfig()
{
  mHolidayColorShaded = getShadedColor(KOPrefs::instance()->mHolidayColor);
  
  setColors();
}

QDate KDateButton::date()
{
  return mDate;
}

void KDateButton::setItalic(bool italic)
{
  if (mItalic == italic) return;
  mItalic = italic;

  QFont myFont = font();
  myFont.setItalic(mItalic);
  setFont(myFont);
}

void KDateButton::setShaded(bool shaded)
{
  if (mShaded == shaded) return;
  mShaded = shaded;

  setColors();

// This disables mouse press events, which is bad.
//  setEnabled(!mShaded);
}

void KDateButton::setEvent(bool event)
{
  if (mEvent == event) return;  
  mEvent = event;
  
  QFont myFont = font();
  myFont.setBold(mEvent);
  setFont(myFont);
}

void KDateButton::setSelected(bool selected)
{
  if (mSelected == selected) return;
  mSelected = selected;  

  setColors();
}

void KDateButton::setToday(bool today)
{
  if (mToday == today) return;
  mToday = today;
  
  if (mToday) {
    setLineWidth(mTodayMarginWidth);
  } else {
    setLineWidth(0);
  }
  setColors();
}

void KDateButton::setHoliday(bool holiday)
{
  if (mHoliday == holiday) return;
  mHoliday = holiday;

  setColors();
}

void KDateButton::setColors()
{
  if (mHoliday) {
    if (mShaded) setTextColor(mHolidayColorShaded);
    else setTextColor(KOPrefs::instance()->mHolidayColor);
  } else {
    if (mSelected) {
      if (mToday) setTextColor("grey");
      else setTextColor("white");
    } else {
      if (mShaded) setTextColor(mDefaultTextColorShaded);
      else setTextColor(mDefaultTextColor);
    }
  }
  
  if(mSelected) {
    setBackColor(KOPrefs::instance()->mHighlightColor);
  } else {
    setBackColor(mDefaultBackColor);
  }
}

void KDateButton::setBackColor(const QColor & color)
{
  QPalette pal = palette();
  QColorGroup cg = palette().active();
  cg.setColor(QColorGroup::Background,color);
  pal.setActive(cg);
  pal.setInactive(cg);
  setPalette(pal);
}

void KDateButton::setTextColor(const QColor & color)
{
  QPalette pal = palette();
  QColorGroup cg = palette().active();
  cg.setColor(QColorGroup::Foreground,color);
  pal.setActive(cg);
  pal.setInactive(cg);
  setPalette(pal);
}

void KDateButton::setDate(QDate date)
{
  mDate = date;
  QString tstr;
  tstr.setNum(date.day());
  setText(tstr);
}

void KDateButton::mousePressEvent(QMouseEvent *e)
{
  bool c;

  c = (e->state() & ControlButton);
  // do the actual work.... :)
  emit selected(mDate, my_index, c);
}

void KDateButton::dragEnterEvent(QDragEnterEvent *e)
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

void KDateButton::dragMoveEvent(QDragMoveEvent *e)
{
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  e->accept();
}

void KDateButton::dragLeaveEvent(QDragLeaveEvent */*dl*/)
{
//  setPalette(oldPalette);
//  update();
}


void KDateButton::dropEvent(QDropEvent *e)
{
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  Event *event = mCalendar->createDrop(e);

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
    start.setDate(mDate);
    end.setDate(mDate.addDays(duration));
    event->setDtStart(start);
    event->setDtEnd(end);
    mCalendar->addEvent(event);

    emit eventDropped(event);
  } else {
    kdDebug() << "KDateButton::dropEvent(): Event from drop not decodable" << endl;
    e->ignore();
  }
}

QSize KDateButton::sizeHint () const
{
  // Prefered size is independent of today margin or bold font as event
  // indicator. Return a standard value based on font size and today margin.
  // May not be perfect in all cases.
  
  QFontMetrics fm = fontMetrics();
  QSize size = fm.size(SingleLine,"30");
  int add = 2*mTodayMarginWidth + 1;
  size += QSize(add,add);
  return size;
}

QColor KDateButton::getShadedColor(QColor color)
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
