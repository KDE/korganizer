// Base class for all event list views
// (c) 1998 by Preston Brown
// Part of the KOrganizer project

#include <qpixmap.h>
#include <qtooltip.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "baselistview.h"
#include "baselistview.moc"

BaseListView::BaseListView(CalObject *cal, QWidget *parent, const char *name)
  : KTabListBox(parent, name, 5)
{
  timeAmPm = TRUE;
  toolTipList.setAutoDelete(TRUE);
  
  calendar = cal;
  clearTableFlags(Tbl_hScrollBar);
  clearTableFlags(Tbl_autoHScrollBar);
  setTableFlags(Tbl_autoVScrollBar);

  setColumn(0, "A", 20, KTabListBox::PixmapColumn);
  setColumn(1, "R", 20, KTabListBox::PixmapColumn);
  setColumn(2, i18n("Date"), 100);
  setColumn(3, i18n("Time"), 120);
  setColumn(4, i18n("Summary"), 300);

  dict().insert("B", new QPixmap(BarIcon("bell")));
  dict().insert("R", new QPixmap(BarIcon("recur")));

  connect(this, SIGNAL(selected(int, int)),
	  this, SLOT(editEvent()));

  updateConfig();
}

BaseListView::~BaseListView()
{
}

void BaseListView::updateView()
{
  QRect *rectPtr;
  // clear old tooltips
  for (rectPtr = toolTipList.first(); rectPtr;
       rectPtr = toolTipList.next()) {
        QToolTip::remove(this, *rectPtr);
  }

  // delete old list of tooltip locations/free dynamic memory
  toolTipList.clear();

  entries = makeEntries();
  clear();
  appendStrList(&entries);

  // update so the changes become visible.
  update();

  // clear out the (dynamically allocated) summary list
  entries.setAutoDelete(TRUE);
  entries.clear();

  // do tooltip stuff
  for (uint i = 0; i < count(); i++) {
    KOEvent *anEvent = calendar->getEvent(*(currIds.at(i)));
    QString str;
    int x, y;

    if (colXPos(1, &x) && rowYPos(i, &y)) {
      if (anEvent->doesRecur()) {
	// do tooltip stuff for recurrence
	str = i18n("started ");
	str += anEvent->getDtStart().date().toString() + ", ";
	if (anEvent->getRecursDuration() == 0) {
	  str += i18n("ends ");
	  str += anEvent->getRecursEndDate().toString();
	} else {
	  if (anEvent->getRecursDuration() > 0) {
	    QString str3;
	    str3.sprintf(i18n("repeats %d times"),
			 anEvent->getRecursDuration());
	    str += str3;
	  } else {
	    str += i18n("repeats forever");
	  }
	}
	toolTipList.append(new QRect(x, y+labelHeight, 
				     cellWidth(1), cellHeight(0)));
	QToolTip::add(this, QRect(x, y+labelHeight, 
				  cellWidth(1), cellHeight(0)),
		      str.data());
      } // tooltip if
    } // if x and y were OK
  } // for loop
}

void BaseListView::updateConfig()
{
  fmt = KOPrefs::instance()->mTimeFormat;
  timeAmPm = (fmt == 1);

  // Is this used?
//  config->setGroup("Fonts");
//  setTableFont(config->readFontEntry("List Font"));

  updateView();
}

void BaseListView::selectEvents(QList<KOEvent> eventList)
{
  selectedEvents = eventList;
}

void BaseListView::changeEventDisplay(KOEvent *, int)
{
  // we're going to go the inefficient yet effective route here for
  // right now, because I don't feel like coding anything better.
  updateView();
    
}

KOEvent *BaseListView::getSelected()
{
  KOEvent *anEvent;
  int ci;

  ci = currentItem();
  if (ci == -1) {
    return 0L; // nothing selected.
  } else {
    anEvent = calendar->getEvent(*(currIds.at(ci)));
    return anEvent;
  }
}

int BaseListView::getSelectedId()
{
  int ci;

  ci = currentItem();
  if (ci == -1) {
    return 0; // nothing selected.
  } else {
    return *currIds.at(ci);
  }
  
}

void BaseListView::editEvent()
{
  emit editEventSignal(getSelected());
}

bool BaseListView::prepareForDrag(int /*col*/, int row,
			      char **data, int *size, int *type)
{
#if 0
  // get data
  *data = (text(row, 3)).data();
  *size = strlen(*data);
  *type = DndText;
  return TRUE;
#else
#warning WABA: Drag & drop broken!
  return FALSE;
#endif
}


void BaseListView::deleteEvent()
{
  emit deleteEventSignal(getSelected());
}


// the implementation of currIds to match Unique Identifiers
// to the entries is messy and ugly at best.  I haven't proved to
// myself that it is even bug-free, but it seems to be working.
// memory leaks?  I love 'em.
QStrList BaseListView::makeEntries()
{

  KOEvent *anEvent;
  QStrList tmpList;
  int index;
  int *i;

  // clear out list of IDs associated with entries
  currIds.clear();
  for (anEvent = currEvents.first(); anEvent;
       anEvent = currEvents.next()) {
    QString str = makeDisplayStr(anEvent);
    tmpList.inSort(str.data());
    index = tmpList.find(str.data());
    i = new int(anEvent->getEventId());
    currIds.insert(index, i);
  }
  return tmpList;
}

QString BaseListView::makeDisplayStr(KOEvent *anEvent)
{
  QTime t1, t2;
  int h1, h2;
  QString str, str2, ampm1, ampm2;

  str = "";
  if (anEvent->getAlarmRepeatCount())
    str += "B\n";
  else
    str += " \n";
  if (anEvent->doesRecur())
    str += "R\n";
  else
    str += " \n";
    
    // date goes in column 2
  str += anEvent->getDtStart().date().toString();
  str += "\n";
    
    // time goes in column 3
  if (anEvent->doesFloat()) {
    str += " \n" + anEvent->getSummary();
  } else {
    t1 = anEvent->getDtStart().time();
    t2 = anEvent->getDtEnd().time();
      
    if (timeAmPm) {
      h1 = t1.hour();
      if(h1 == 0) {
	h1 = 12;
	ampm1 = "am";
      } else if(h1 > 11) {
	ampm1 = "pm";
	if(h1 != 12) {
	  h1 -= 12;
	} 
      } else {
	ampm1 = "am";
      }
	
      h2 = t2.hour();
      if(h2 == 0) {
	h2 = 12;
	ampm2 = "am";
      } else if(h2 > 11) {
	ampm2 = "pm";
	if(h2 != 12) {
	  h2 -= 12;
	} 
      } else {
	ampm2 = "am";
      }
	
      str2.sprintf("%d:%02d %s-%d:%02d %s\n%s",
		   h1, t1.minute(), ampm1.data(),
		   h2, t2.minute(), ampm2.data(),
		   anEvent->getSummary().data());
    } else {
      str2.sprintf("%02d:%02d-%02d:%02d\n%s",
		   t1.hour(), t1.minute(), 
		   t2.hour(), t2.minute(), 
		   anEvent->getSummary().data());
    }
    str += str2;
  }
  return str.data();
}

void BaseListView::showDates(bool show)
{
  static int oldColWidth = 0;

  if (!show) {
    oldColWidth = columnWidth(2);
    setColumnWidth(2, 0);
  } else {
    setColumnWidth(2, oldColWidth);
  } 
  repaint();
}

inline void BaseListView::showDates()
{
  showDates(TRUE);
}

inline void BaseListView::hideDates()
{
  showDates(FALSE);
}




