// $Id$	

#include <qstring.h>

#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kconfig.h>

#include "vcaldrag.h"
#include "calobject.h"
#include "koprefs.h"

#include "kdpdatebutton.h"
#include "kdpdatebutton.moc"


KDateButton::KDateButton(QDate date, int index, CalObject *_calendar,
			 QWidget *parent, const char *name)
  : QLabel(parent, name)
{
  mSelected = false;
  mEvent = false;
  mToday = false;
  mHoliday = false;

  mDefaultBackColor = palette().active().base();
  mDefaultTextColor = palette().active().foreground();

  calendar = _calendar;

  setFrameStyle(QFrame::Box|QFrame::Plain);
  setLineWidth(0);
  setAlignment(AlignCenter);
  my_index = index;
  bt_Date = date;
  QString tstr;
  tstr.setNum(date.day());
  setText(tstr);
  adjustSize();

  updateConfig();
}

KDateButton::~KDateButton()
{
}

void KDateButton::updateConfig()
{
  setColors();
}

inline QDate KDateButton::date()
{
  return bt_Date;
}

void KDateButton::setItalic(bool italic)
{
  if (mItalic == italic) return;
  mItalic = italic;
  
  QFont myFont = font();
  myFont.setItalic(mEvent);
  setFont(myFont);
  update();
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
    setLineWidth(2);
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
    setTextColor(KOPrefs::instance()->mHolidayColor);
  } else {
    if (mSelected) {
      if (mToday) setTextColor("grey");
      else setTextColor("white");
    } else {
      setTextColor(mDefaultTextColor);
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
  bt_Date = date;
  QString tstr;
  tstr.setNum(date.day());
  setText(tstr);
}

void KDateButton::mousePressEvent(QMouseEvent *e)
{
  bool c;

  c = (e->state() & ControlButton);
  // do the actual work.... :)
  emit selected(bt_Date, my_index, c);
}

void KDateButton::dragEnterEvent(QDragEnterEvent *de)
{
  if (VCalDrag::canDecode(de)) {
    de->accept();
  }
  
  // some visual feedback
//  oldPalette = palette();
//  setPalette(my_HilitePalette);
//  update();
}

void KDateButton::dragLeaveEvent(QDragLeaveEvent */*dl*/)
{
//  setPalette(oldPalette);
//  update();
}

// some of this really doesn't belong here, but rather probably in calobject.
// KOrganizer is starting to get messy.  Needs some major reorganization and
// re-architecting after 1.0.
void KDateButton::dropEvent(QDropEvent *de)
{
  VObject *vcal;

  if (VCalDrag::decode(de, &vcal)) {
    // note that vcal is destroyed in pasteEvent(), so we don't have to
    // free it here.  No leak.
    KOEvent *newEvent, *oldEvent;

    VObjectIterator i;
    VObject *curvo;
    initPropIterator(&i, vcal);
    
    // we only take the first object.
    do  {
      curvo = nextVObject(&i);
    } while (strcmp(vObjectName(curvo), VCEventProp) &&
	     strcmp(vObjectName(curvo), VCTodoProp));

    if ((curvo = isAPropertyOf(curvo, VCUniqueStringProp))) {
      char *s;
      s = fakeCString(vObjectUStringZValue(curvo));
      oldEvent = calendar->getEvent(s);
      deleteStr(s);
      if (oldEvent) {
	if (oldEvent->doesRecur()) { // only add an exception if it recurs
	  oldEvent->addExDate(bt_Date);
	} else {
	  calendar->deleteEvent(oldEvent); // do the full delete
	}
      }
    }
    newEvent = calendar->pasteEvent(&bt_Date, (QTime *) 0L, vcal);
    updateMe(my_index);
  }
}

