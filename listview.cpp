// 	$Id$	

#include <qpixmap.h>

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "listview.h"
#include "listview.moc"

ListView::ListView(CalObject *cal, QWidget *parent, const char *name) :
  BaseListView(cal, parent, name)
{
  QPixmap pixmap;
  rmbMenu = new QPopupMenu;
  rmbMenu->insertItem(i18n("Edit Event"), this,
		     SLOT (editEvent()));
  pixmap = BarIcon("delete");
  rmbMenu->insertItem(pixmap, i18n("Delete Event  "), this,
		     SLOT (deleteEvent()));
  rmbMenu->insertSeparator();
  rmbMenu->insertItem(i18n("Show Dates"), this,
		      SLOT(showDates()));
  rmbMenu->insertItem(i18n("Hide Dates"), this,
		      SLOT(hideDates()));

  connect(this, SIGNAL(popupMenu(int, int)), 
	  this, SLOT(doPopup(int, int)));

  hideDates();
}

ListView::~ListView()
{
  delete rmbMenu;
}

void ListView::updateView()
{
  // now, get a new list of entries and put them into the view.
  // BL: memory leak!!!
  
  currEvents = calendar->getEventsForDate(currDate);
  BaseListView::updateView();
}

void ListView::selectDates(const QDateList dateList)
{
  QDateList tmpList;

  tmpList = dateList;
  currDate = *tmpList.first();
  updateView();
  tmpList.clear();
  tmpList.append(new QDate(currDate));
  
  emit datesSelected(tmpList);
  // clean up temp date (we aren't supporting multi-column correctly. :) )
  tmpList.clear();
}

void ListView::doPopup(int row, int col)
{
  this->setCurrentItem(row, col);
  rmbMenu->popup(QCursor::pos());
}

QString ListView::makeDisplayStr(KOEvent *anEvent)
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
	
      if (anEvent->isMultiDay()) {
	if (anEvent->getDtStart().date() == currDate) {
	  str2.sprintf("%d:%02d %s --->\n%s",
		       h1, t1.minute(), ampm1.data(),
		       anEvent->getSummary().data());
	} else if (anEvent->getDtEnd().date() == currDate) {
	  str2.sprintf("<--- %d:%02d %s\n%s",
		       h2, t2.minute(), ampm2.data(),
		       anEvent->getSummary().data());
	} else {
	  str2.sprintf("<-----> \n%s",anEvent->getSummary().data());
	}
      } else {
	str2.sprintf("%d:%02d %s-%d:%02d %s\n%s",
		     h1, t1.minute(), ampm1.data(),
		     h2, t2.minute(), ampm2.data(),
		     anEvent->getSummary().data());
      }
    } else {
      if (anEvent->isMultiDay()) {
	if (anEvent->getDtStart().date() == currDate) {
	  str2.sprintf("%02d:%02d --->\n%s",
		       t1.hour(), t1.minute(),
		       anEvent->getSummary().data());
	} else if (anEvent->getDtEnd().date() == currDate) {
	  str2.sprintf("<--- %02d:%02d\n%s",
		       t2.hour(), t2.minute(),
		       anEvent->getSummary().data());
	} else {
	  str2.sprintf("<-----> \n%s",anEvent->getSummary().data());
	}
      } else {
	str2.sprintf("%02d:%02d-%02d:%02d\n%s",
		     t1.hour(), t1.minute(), 
		     t2.hour(), t2.minute(), 
		     anEvent->getSummary().data());
      }
    }
    str += str2;
  }
  return str;
}
