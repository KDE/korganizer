/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown

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

#include <math.h>

#include <qpainter.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qrect.h>

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kdebug.h>
//#include <kseparator.h>

#include <libkcal/todo.h>

#include "koprefsdialog.h"
#include "koprefs.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif
#include <libkdepim/kdateedit.h>

#include "calprinter.h"
#include "calprinter.moc"

CalPrinter::CalPrinter(QWidget *parent, Calendar *calendar)
  : QObject(0L, "CalPrinter")
{
  mCalendar = calendar;
  mParent = parent;
  mPrinter = new KPrinter;
  mPrintDialog = new CalPrintDialog(mPrinter,parent);
  mPrinter->setOrientation(KPrinter::Landscape);

  updateConfig();
}

CalPrinter::~CalPrinter()
{
  delete mPrinter;
}

void CalPrinter::setupPrinter()
{
  KOPrefsDialog *optionsDlg = new KOPrefsDialog(mParent);
  optionsDlg->readConfig();
  optionsDlg->showPrinterTab();
  connect(optionsDlg, SIGNAL(configChanged()),
	  mParent, SLOT(updateConfig()));
//  connect(optionsDlg, SIGNAL(closed(QWidget *)), 
//	  parent, SLOT(cleanWindow(QWidget *)));
  optionsDlg->show(); 
}

void CalPrinter::preview(PrintType pt, const QDate &fd, const QDate &td)
{
  mPrintDialog->setPreview(true);
  mPrintDialog->setRange(fd,td);

  switch(pt) {
    case Day: 
      mPrintDialog->setPrintDay();
      break;
    case Week:
      mPrintDialog->setPrintWeek();
      break;
    case Month: 
      mPrintDialog->setPrintMonth();
      break;
    case Todolist:
      mPrintDialog->setPrintTodo();
      break;
    case TimeTable:
      mPrintDialog->setPrintTimeTable();
      break;
  }

  if (mPrintDialog->exec() == QDialog::Accepted) {
    doPreview(mPrintDialog->printType(),mPrintDialog->fromDate(),
              mPrintDialog->toDate());
  }
}

void CalPrinter::doPreview(int pt, QDate fd, QDate td)
{
  mPrinter->setPreviewOnly(true);

  switch(pt) {
    case Day:
      printDay(fd, td);
      break;
    case Week:
      printWeek(fd, td);
      break;
    case Month:
      printMonth(fd, td);
      break;
    case Todolist:
      printTodo(fd, td);
      break;
    case TimeTable:
      printTimeTable(fd, td);
      break;
  }

  // restore previous settings that were used before the preview.
  mPrinter->setPreviewOnly(false);
}

void CalPrinter::print(PrintType pt, const QDate &fd, const QDate &td)
{
  mPrintDialog->setPreview(false);
  mPrintDialog->setRange(fd,td);

  switch(pt) {
    case Day: 
      mPrintDialog->setPrintDay();
      break;
    case Week: 
      mPrintDialog->setPrintWeek();
      break;
    case Month: 
      mPrintDialog->setPrintMonth();
      break;
    case Todolist: 
      mPrintDialog->setPrintTodo();
      break;
    case TimeTable:
      mPrintDialog->setPrintTimeTable();
      break;
  }

  if (mPrintDialog->exec() == QDialog::Accepted) {
    doPrint(mPrintDialog->printType(),mPrintDialog->fromDate(),
            mPrintDialog->toDate());
  }
}

void CalPrinter::doPrint(int pt, QDate fd, QDate td)
{
  if (!mPrinter->setup(mParent)) return;

  switch(pt) {
    case Day: 
      printDay(fd, td);
      break;
    case Week: 
      printWeek(fd, td);
      break;
    case Month: 
      printMonth(fd, td);
      break;
    case Todolist: 
      printTodo(fd, td);
      break;
    case TimeTable:
      printTimeTable(fd, td);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void CalPrinter::updateConfig()
{
  mStartHour = KOPrefs::instance()->mDayBegins;
}

void CalPrinter::printDay(const QDate &fd, const QDate &td)
{
  QPainter p;
  QDate curDay, fromDay, toDay;

  mPrinter->setOrientation(KPrinter::Portrait);

  fromDay = fd;
  curDay = fd;
  toDay = td;

  p.begin(mPrinter);
  // the painter initially begins at 72 dpi per the Qt docs. 
  // we want half-inch margins.
  int margin = 36;
  p.setViewport(margin, margin, 
		p.viewport().width()-margin, 
		p.viewport().height()-margin);
  int pageWidth = p.viewport().width();
  int pageHeight = p.viewport().height();
  mHeaderHeight = 72;
  mSubHeaderHeight = 20;

  do {
    drawHeader(p, curDay,toDay,curDay,
	       pageWidth, mHeaderHeight, Day);
    drawDay(p, curDay, pageWidth, pageHeight);
    curDay = curDay.addDays(1);
    if (curDay <= toDay)
      mPrinter->newPage();
  } while (curDay <= toDay);
  
  p.end();
}

void CalPrinter::printWeek(const QDate &fd, const QDate &td)
{
  QPainter p;
  QDate curWeek, fromWeek, toWeek;

  mPrinter->setOrientation(KPrinter::Portrait);

  if (KGlobal::locale()->weekStartsMonday()) {
    // correct to monday
    fromWeek = fd.addDays(-(fd.dayOfWeek()-1));
    // correct to sunday
    toWeek = td.addDays(7-fd.dayOfWeek());
  } else {
    // correct to sunday
    fromWeek = fd.addDays(-(fd.dayOfWeek()%7));
    // correct to saturday
    toWeek = td.addDays(6-td.dayOfWeek());
  }

  p.begin(mPrinter);
  // the painter initially begins at 72 dpi per the Qt docs. 
  // we want half-inch margins.
  int margin = 36;
  p.setViewport(margin, margin, 
  p.viewport().width()-margin, 
  p.viewport().height()-margin);
  int pageWidth = p.viewport().width();
  int pageHeight = p.viewport().height();
  mHeaderHeight = 72;
  mSubHeaderHeight = 20;

  curWeek = fromWeek.addDays(6);
  do {
     drawHeader(p, curWeek.addDays(-6), curWeek,
	       curWeek,
	       pageWidth, mHeaderHeight, Week);
    drawWeek(p, curWeek, pageWidth, pageHeight);
    curWeek = curWeek.addDays(7);
    if (curWeek <= toWeek)
      mPrinter->newPage();
  } while (curWeek <= toWeek);
  
  p.end();
}

void CalPrinter::printMonth(const QDate &fd, const QDate &td)
{
  QPainter p;
  QDate curMonth, fromMonth, toMonth;

  mPrinter->setOrientation(KPrinter::Landscape);

  fromMonth = fd.addDays(-(fd.day()-1));
  toMonth = td.addDays(td.daysInMonth()-td.day());

  p.begin(mPrinter);
  // the painter initially begins at 72 dpi per the Qt docs.
  // we want half-inch margins.
  int margin = 36;
  p.setViewport(margin, margin,
		p.viewport().width()-margin,
		p.viewport().height()-margin);
  int pageWidth = p.viewport().width();
  int pageHeight = p.viewport().height();
  mHeaderHeight = 72;
  mSubHeaderHeight = 20;

  curMonth = fromMonth;
  do {
    drawHeader(p, fromMonth,
	       toMonth, curMonth,
	       pageWidth, mHeaderHeight, Month);
    drawDaysOfWeek(p, curMonth, pageWidth, pageHeight);
    drawMonth(p, curMonth, pageWidth, pageHeight);
    curMonth = curMonth.addDays(fromMonth.daysInMonth()+1);
    if (curMonth <= toMonth)
      mPrinter->newPage();
  } while (curMonth <= toMonth);

  p.end();
}

void CalPrinter::printTimeTable(const QDate &fd, const QDate &td)
{
  QPainter p;
  QDate curWeek, fromWeek, toWeek;

  mPrinter->setOrientation(KPrinter::Landscape);

  if (KGlobal::locale()->weekStartsMonday()) {
    // correct to monday
    fromWeek = fd.addDays(-(fd.dayOfWeek()-1));
    // correct to sunday
    toWeek = td.addDays(7-fd.dayOfWeek());
  } else {
    // correct to sunday
    fromWeek = fd.addDays(-(fd.dayOfWeek()%7));
    // correct to saturday
    toWeek = td.addDays(6-td.dayOfWeek());
  }

  p.begin(mPrinter);
  // the painter initially begins at 72 dpi per the Qt docs.
  // we want half-inch margins.
  int margin = 36;
  p.setViewport(margin, margin,
		p.viewport().width()-margin,
		p.viewport().height()-margin);
  int pageWidth = p.viewport().width();
  int pageHeight = p.viewport().height();
  mHeaderHeight = 36;
  mSubHeaderHeight = 20;

  curWeek = fromWeek.addDays(6);
  do {
     //drawHeader(p, curWeek.addDays(-6), curWeek,
//	       curWeek,
	//       pageWidth, mHeaderHeight, TimeTable);
    drawTimeTable(p, curWeek, pageWidth, pageHeight);
    curWeek = curWeek.addDays(7);
    if (curWeek <= toWeek)
      mPrinter->newPage();
  } while (curWeek <= toWeek);

  p.end();
}


void CalPrinter::printTodo(const QDate &fd, const QDate &td)
{
  QPainter p;

  mPrinter->setOrientation(KPrinter::Portrait);

  p.begin(mPrinter);
  int pageWidth = p.viewport().width();
  int pageHeight = p.viewport().height();
  mHeaderHeight = pageHeight/7 - 20;

  int pospriority = 10;
  int possummary = 60;
  int posdue = pageWidth - 85;
  int lineSpacing = 15;
  int fontHeight = 10;

  drawHeader(p, fd, td, fd, pageWidth, mHeaderHeight, Todolist);

  mCurrentLinePos = mHeaderHeight + 5;
  kdDebug(5850) << "Header Height: " << mCurrentLinePos << endl;

  QPtrList<Todo> todoList = mCalendar->todos();
  todoList.first();
  int count = 1;
  QString outStr;
 
  p.setFont(QFont("helvetica", 10));
  lineSpacing = p.fontMetrics().lineSpacing();
  // draw the headers
  p.setFont(QFont("helvetica", 10, QFont::Bold));
  outStr += i18n("Priority");
  
  p.drawText(pospriority, mHeaderHeight - 2,
	     outStr);
  outStr.truncate(0);
  outStr += i18n("Summary");
  
  p.drawText(possummary, mHeaderHeight - 2,
		 outStr);
  outStr.truncate(0);
  outStr += i18n("Due");
 
  p.drawText(posdue,  mHeaderHeight - 2,
		 outStr);  
  p.setFont(QFont("helvetica", 10));

  fontHeight =  p.fontMetrics().height();
  for(int cprior = 1; cprior <= 6; cprior++) {
    Todo *currEvent(todoList.first());
    while (currEvent != NULL) {
                
      //Filter out the subitems.
      if (currEvent->relatedTo()){
            currEvent = todoList.next();
            continue;
        }

      QDate start = currEvent->dtStart().date();
      // if it is not to start yet, skip.
      if ( (!start.isValid()) && (start >= td) ) {
	currEvent = todoList.next();
	continue;
      }      
      // priority
      int priority = currEvent->priority();
      // 6 is the lowest priority (the unspecified one)
      if ((priority != cprior) || ((cprior==6) && (priority==0))) {
	currEvent = todoList.next();
	continue;
      }
      bool connect = true;
      drawTodo(count,currEvent,p,connect);
      currEvent = todoList.next();
      ++count;
    }
  }
  p.end();
}

void CalPrinter::drawTodo(int count, Todo * item, QPainter &p, bool &connect,
			  int level, QRect *r)
{
  QString outStr;
  KLocale *local = KGlobal::locale();
  int pageWidth = p.viewport().width();
  int pospriority = 10;
  int possummary = 60;
  int posdue = pageWidth - 85;  //+ indent;
  int fontHeight = 10;
  int priority=item->priority();
  QRect rect;
  QRect startpoint;
  
  // size of item
  outStr=item->summary();
  int left = possummary+(level*10);
  rect = p.boundingRect(left,mCurrentLinePos+18,
                        (posdue-(left + rect.width() + 5)),-1,WordBreak,outStr);
  if ( !item->description().isEmpty() ) {
    outStr = item->description();
    rect = p.boundingRect(left+20, rect.bottom()+5, pageWidth-(left+10), -1, 
			  WordBreak, outStr);
  }
  // if too big make new page
  if ( rect.bottom() > p.viewport().height()) {
    mCurrentLinePos = 0;
    mPrinter->newPage();
    connect = false;
  }

  // If this is a sub-item, r will not be 0, and we want the LH side of the priority line up
  //to the RH side of the parent item's priority
  if (r) {
    pospriority = r->right() + 1;
  }

  // Priority
  if (priority > 0) {
    outStr.setNum(priority);
    rect = p.boundingRect(pospriority,mCurrentLinePos + 10,
                          5,-1,AlignCenter,outStr);
    // Make it a more reasonable size
    rect.setWidth(18);
    rect.setHeight(18);
    p.drawText(rect,AlignCenter, outStr);
    p.drawRect(rect);
    startpoint = rect; //save for later
  }

  // Connect the dots
  if (level > 0 && connect) {
    int center,bottom,to,endx;
    center = r->left() + (r->width()/2);
    bottom = r->bottom() + 1;
    to = rect.top() + (rect.height()/2);
    endx = rect.left();
    p.moveTo(center,bottom);
    p.lineTo(center,to);
    p.lineTo(endx,to);
  }

  // summary
  outStr=item->summary();
  rect = p.boundingRect(left,rect.top(),
                        (posdue-(left + rect.width() + 5)),-1,WordBreak,outStr);
  QRect newrect;
  p.drawText(rect,WordBreak,outStr,-1,&newrect);
   
  // due
  if (item->hasDueDate()) {
    outStr = local->formatDate(item->dtDue().date(),true);
    rect = p.boundingRect(posdue,mCurrentLinePos, mCurrentLinePos,-1,AlignTop|AlignLeft,outStr);
    p.drawText(rect, mCurrentLinePos, outStr);
  }

  // if terminated, cross it
  if (item->isCompleted()) {
    p.drawLine( 5, (mCurrentLinePos)-fontHeight/2 + 2,
                    pageWidth-5, mCurrentLinePos-fontHeight/2 + 2);
  }

  if ( !item->description().isEmpty() ) {
    mCurrentLinePos=newrect.bottom() + 5;
    outStr = item->description();
    rect = p.boundingRect(left+20, mCurrentLinePos, pageWidth-(left+10), -1, 
			  WordBreak, outStr);
    p.drawText(rect, WordBreak, outStr, -1, &newrect);
  }

  // Set the new line position
  mCurrentLinePos=newrect.bottom() + 10; //set the line position

  // If the item has subitems, we need to call ourselves recursively
  bool conn = true;
  QPtrList<Incidence> l = item->relations();
  Incidence *c;
  for(c=l.first();c;c=l.next()) {
    drawTodo(count, static_cast<Todo *> (c),p,conn,level+1,&startpoint);
    if ( !conn )
      connect = false;
  }
}


///////////////////////////////////////////////////////////////////////////////

void CalPrinter::drawHeader(QPainter &p, const QDate &fd, const QDate &td,
			    const QDate &cd,
			    int width, int height, PrintType pt)
{
  KLocale *local = KGlobal::locale();
  QFont font("helvetica", 18, QFont::Bold);
  p.drawRect(0, 0, width, height);
  p.fillRect(1, 1, 
	     width-2, 
	     height-2, 
	     QBrush(Dense7Pattern));

  p.setFont(font);
  int lineSpacing = p.fontMetrics().lineSpacing();
  QString title;
  QString myOwner(mCalendar->getOwner());

  //  title.sprintf("%s %d Schedule for ",qd.monthName(qd.month()),qd.year());
  //  title += myOwner;

  switch(pt) {
  case TimeTable:
  	break;

  case Todolist:
    title +=  i18n("To-do items:");
   
    p.drawText(5, lineSpacing,title);
    break;
  case Month:
      title += local->monthName(cd.month());
      title += " ";
      title += QString::number(cd.year());
      p.drawText(5, lineSpacing, title );
      break;
  case Week:

    title += local->formatDate(fd);

    p.drawText(5, lineSpacing, title );
    title.truncate(0);

    title += local->formatDate(td);
    p.drawText(5, 2*lineSpacing, title);
    break;
  case Day:
   
    title += local->formatDate(fd,false);
    p.drawText(5, lineSpacing, title );
    
  }
  
  // print previous month for month view, print current for todo, day and week
  switch (pt) {
  case TimeTable: break;
  case Todolist:
  case Week:
  case Day:
    drawSmallMonth(p, QDate(cd.addDays(-cd.day()+1)),
		   width/2+5, 5, /*width/4-10*/100, height-10);
    break;

    drawSmallMonth(p, QDate(cd.addDays(cd.daysInMonth()-cd.day()+1)),
  		 width/2+width/4+5, 5, /*width/4-10*/100, height-10);
  case Month:
    drawSmallMonth(p, QDate(cd.addDays(-cd.day())),
		   width/2+5, 5, /*width/4-10*/100, height-10);
    //print the following month as well
    drawSmallMonth(p, QDate(cd.addDays(cd.daysInMonth()-cd.day()+1)),
  		 width/2+width/4+5, 5, /*width/4-10*/100, height-10);

  }
}

/*
 * This routine draws a header box over the main part of the calendar
 * containing the days of the week.
 */
void CalPrinter::drawDaysOfWeekBox(QPainter &p, const QDate &qd,
				   int x, int y, int width, int height)
{
  KLocale *local = KGlobal::locale();

  p.setFont(QFont("helvetica", 10, QFont::Bold));
  p.drawRect(x, y, width, height);
  p.fillRect(x+1, y+1,
             width-2, height-2,
             QBrush(Dense7Pattern));
  p.drawText(x+5, y, width-10, height, AlignCenter | AlignVCenter,
             local->weekDayName(qd.dayOfWeek()));
}

void CalPrinter::drawDayBox(QPainter &p, const QDate &qd,
			    int x, int y, int width, int height,
			    bool fullDate)
{
  KLocale *local = KGlobal::locale();
  QString dayNumStr;
  QPtrList<Event> eventList;
  QString ampm;

#ifndef KORG_NOPLUGINS
  QString hstring(KOCore::self()->holiday(qd));
#else
  QString hstring;
#endif

  // This has to be localized
  if (fullDate) {
    /*int index;
    dayNumStr= qd.toString();
    index = dayNumStr.find(' ');
    dayNumStr.remove(0, index);
    index = dayNumStr.findRev(' ');
    dayNumStr.truncate(index);*/

    dayNumStr = local->weekDayName(qd.dayOfWeek()) + ' ' + local->monthName(qd.month(), true) + ' ' + QString::number(qd.day());
  } else {
    dayNumStr = QString::number(qd.day());
  }

  p.drawRect(x, y, width, height);
  // p.fillRect(x+1, y+1, width-2,height, QBrush(Dense7Pattern));
  p.drawRect(x, y, width, mSubHeaderHeight);
  p.fillRect(x+1, y+1, width-2, mSubHeaderHeight-2, QBrush(Dense7Pattern));
  if (!hstring.isEmpty()) {
    p.setFont(QFont("helvetica", 8, QFont::Bold, TRUE));

    p.drawText(x+5, y, width-25, mSubHeaderHeight, AlignLeft | AlignVCenter,
	       hstring);
  }
  p.setFont(QFont("helvetica", 10, QFont::Bold));
  p.drawText(x+5, y, width-10, mSubHeaderHeight, AlignRight | AlignVCenter,
	     dayNumStr);

  eventList = mCalendar->events(qd, TRUE);
  eventList.first();
  int count = 1;
  QString outStr;
  Event *currEvent(eventList.first());
  p.setFont(QFont("helvetica", 8));
  int lineSpacing = p.fontMetrics().lineSpacing();

  while (count <= 9 && (currEvent != NULL)) {
    if (currEvent->doesFloat() || currEvent->isMultiDay())
      outStr = currEvent->summary();

    else {
      QTime t1 = currEvent->dtStart().time();

      outStr = local->formatTime(t1);
      outStr += " " + currEvent->summary();

    } // doesFloat

    p.drawText(x+5, y+(lineSpacing*(count+1)), width-10, lineSpacing,
	       AlignLeft|AlignVCenter, outStr);
    currEvent = eventList.next();
    ++count;
  }

  if ( count <= 9 ) {
    QPtrList<Todo> todos = mCalendar->todos( qd );
    Todo *todo;
    for( todo = todos.first(); todo && count <= 9; todo = todos.next() ) {
      QString text;
      if (todo->hasDueDate()) {
        if (!todo->doesFloat()) {
          text += KGlobal::locale()->formatTime(todo->dtDue().time());
          text += " ";
        }
      }
      text += i18n("To-Do: %1").arg(todo->summary());

      p.drawText(x+5, y+(lineSpacing*(count+1)), width-10, lineSpacing,
                AlignLeft|AlignVCenter, text);
      ++count;
    }
  }
}

void CalPrinter::drawTTDayBox(QPainter &p, const QDate &qd,
			    int x, int y, int width, int height,
			    bool fullDate)
{
  KLocale *local = KGlobal::locale();
  QString dayNumStr;
  QPtrList<Event> eventList;
  QString ampm;

#ifndef KORG_NOPLUGINS
  QString hstring(KOCore::self()->holiday(qd));
#else
  QString hstring;
#endif

  // This has to be localized
  if (fullDate) {
    /*int index;
    dayNumStr= qd.toString();
    index = dayNumStr.find(' ');
    dayNumStr.remove(0, index);
    index = dayNumStr.findRev(' ');
    dayNumStr.truncate(index);*/
    dayNumStr = local->weekDayName(qd.dayOfWeek()) + ' ' + local->monthName(qd.month(), true) + ' ' + QString::number(qd.day());
  } else {
    dayNumStr = QString::number(qd.day());
  }

  p.drawRect(x, y, width, mSubHeaderHeight); //draw Rect for Header
  p.fillRect(x+1, y+1, width-2, mSubHeaderHeight-2, QBrush(Dense5Pattern));
  p.setFont(QFont("helvetica", 10, QFont::Bold));
  p.drawText(x+5, y, width, mSubHeaderHeight,
        AlignCenter | AlignVCenter | AlignJustify | WordBreak,
	     dayNumStr);

  p.drawRect(x, y+mSubHeaderHeight, width, height); //draw rect for daily event

  //draw lines for day
  int cury=y+mSubHeaderHeight+height;
  for(int i=1; i<=12;i++){
    cury+=height;
    p.drawLine(x,cury,x+width,cury);
  }
  //draw one straight line to close day vertically
  p.drawLine(x+width,y,x+width,y+mSubHeaderHeight+(13*height));

  p.setFont(QFont("helvetica", 10));
  QBrush oldBrush=p.brush();
  p.setBrush(QBrush(Dense5Pattern));

  eventList = mCalendar->events(qd, TRUE);
  Event *currEvent;

  //Draw all Events for Day
  QString MultiDayStr; //string for storing Multi Day Events
  for (currEvent = eventList.first(); currEvent; currEvent = eventList.next()) {
      if (currEvent->doesFloat() || currEvent->isMultiDay()) {
          if(!MultiDayStr.isNull()) MultiDayStr += ", ";
          MultiDayStr += currEvent->summary(); // add MultiDayevent
          }
      else {
           int startTime = currEvent->dtStart().time().hour();
           int endTime = currEvent->dtEnd().time().hour();
           float minuteInc = height / 60.0;
           if ((startTime >= mStartHour)  && (endTime <= (mStartHour + 12))) {
                startTime -= mStartHour;
                int startMinuteOff = (int) (minuteInc * currEvent->dtStart().time().minute());
                int currentyPos =y+mSubHeaderHeight+height+startMinuteOff+startTime*height;
                endTime -= mStartHour;
                int endMinuteOff = (int) (minuteInc * currEvent->dtEnd().time().minute());
                int eventLenght=endMinuteOff + (endTime - startTime)*height;
                kdDebug(5850) << currEvent->summary() << ": " << " x=" << x << " currY=" << currentyPos << " width=" << width << " lenght=" << eventLenght;
                p.drawRect(x, currentyPos,
                width, eventLenght);
                p.drawText(x,
          		 currentyPos,
          		 width,
          		 eventLenght,
          		 AlignCenter | AlignVCenter | AlignJustify | WordBreak, currEvent->summary());
            }
        }
  }

  p.setBrush(oldBrush);

  // Fill MultiDay Event Box
  if(MultiDayStr.length()!=0)
      p.fillRect(x+1,y+1+mSubHeaderHeight, width-2, height-2, QBrush(Dense5Pattern));
  p.setFont(QFont("helvetica", 10));
  p.drawText(x, y+mSubHeaderHeight, width, height, AlignCenter | AlignVCenter| AlignJustify | WordBreak,
	     MultiDayStr);
}


void CalPrinter::drawDaysOfWeek(QPainter &p, const QDate &qd,
				int width, int /*height*/)
{	
  int offset=mHeaderHeight+5;
  int cellWidth = width/7;
  int cellHeight = mSubHeaderHeight;
  QDate monthDate(QDate(qd.year(), qd.month(),1));

  if (KGlobal::locale()->weekStartsMonday())
    // correct to monday
    monthDate = monthDate.addDays(-(monthDate.dayOfWeek()-1));
  else
    // correct to sunday
    monthDate = monthDate.addDays(-(monthDate.dayOfWeek()%7));

  for (int col = 0; col < 7; col++) {
    drawDaysOfWeekBox(p, monthDate,
		      col*cellWidth, offset,
		      cellWidth, cellHeight);
    monthDate = monthDate.addDays(1);
  }
}

void CalPrinter::drawDay(QPainter &p, const QDate &qd, int width, int height)
{
  int startHour = mStartHour;
  int endHour = 20;
  int offset = mHeaderHeight + 5;
  QPtrList<Event> eventList = mCalendar->events(qd, TRUE);
  Event *currEvent;
 
  p.setFont(QFont("helvetica", 14));
  p.setBrush(QBrush(Dense7Pattern));
  currEvent = eventList.first();
  int allDays = 0;
  while(currEvent) {
    if (currEvent->doesFloat()) {
      p.drawRect(20, offset, width-25, 35);
      p.drawText(30, offset+10, width-40, 30, AlignLeft | AlignTop, 
                 currEvent->summary());
      offset += 40;
      allDays++;
      eventList.remove();
      currEvent = eventList.current();
    } else {
      currEvent = eventList.next();
    }
  }
  startHour += (allDays/2);
  p.setBrush(QBrush());
  int tmpEnd;
  for (currEvent = eventList.first(); currEvent;
       currEvent = eventList.next()) {
    if (currEvent->dtStart().time().hour() < startHour) 
      startHour = currEvent->dtStart().time().hour();
    tmpEnd = currEvent->dtEnd().time().hour();
    if (currEvent->dtEnd().time().minute() > 0)
      tmpEnd++;
    if (tmpEnd > endHour) 
      endHour = tmpEnd;
  }
  int hours = endHour - startHour;
  int cellHeight = (height-offset) / hours; // hour increments.
  int cellWidth = width-80;

  QString numStr;
  for (int i = 0; i < hours; i++) {
    p.drawRect(0, offset+i*cellHeight, 75, cellHeight);
    p.drawLine(37, offset+i*cellHeight+(cellHeight/2),
               75, offset+i*cellHeight+(cellHeight/2));

    if ( !KGlobal::locale()->use12Clock() ) {
      numStr.setNum(i+startHour);
      if (cellHeight > 40) {
        p.setFont(QFont("helvetica", 20, QFont::Bold));
      } else {
        p.setFont(QFont("helvetica", 16, QFont::Bold));
      }
      p.drawText(0, offset+i*cellHeight, 33, cellHeight/2,
                 AlignTop|AlignRight, numStr);
      p.setFont(QFont("helvetica", 14, QFont::Bold));
      p.drawText(37, offset+i*cellHeight, 45, cellHeight/2,
                 AlignTop | AlignLeft, "00");
    } else {
      QTime time( i + startHour, 0 );
      numStr = KGlobal::locale()->formatTime( time );
      p.setFont(QFont("helvetica", 14, QFont::Bold));
      p.drawText(4, offset+i*cellHeight, 70, cellHeight/2,
                 AlignTop|AlignLeft, numStr);
    }

    p.drawRect(80, offset+i*cellHeight,cellWidth, cellHeight);
    p.drawLine(80, offset+i*cellHeight+(cellHeight/2),
               cellWidth+80, offset+i*cellHeight+(cellHeight/2));

  }

  p.setFont(QFont("helvetica", 14));
  p.setBrush(QBrush(Dense7Pattern));
  for (currEvent = eventList.first(); currEvent;
       currEvent = eventList.next()) {
    int startTime = currEvent->dtStart().time().hour();
    int endTime = currEvent->dtEnd().time().hour();
    float minuteInc = cellHeight / 60.0;
    if ((startTime >= startHour)  && (endTime <= (startHour + hours))) {
      startTime -= startHour;
      int startMinuteOff = (int) (minuteInc * 
      currEvent->dtStart().time().minute());
      int endMinuteOff = (int) (minuteInc * currEvent->dtEnd().time().minute());
      int cheight = (int) (minuteInc * 
                    currEvent->dtStart().secsTo(currEvent->dtEnd()) / 60 );
      p.drawRect(80, offset+startMinuteOff+startTime*cellHeight, 
                 cellWidth, cheight);
      p.drawText(85, offset+startMinuteOff+startTime*cellHeight+5, cellWidth-10, 
                 cheight-10, AlignLeft | AlignTop, currEvent->summary());
    }
  }
  p.setBrush(QBrush(NoBrush));
}

void CalPrinter::drawWeek(QPainter &p, const QDate &qd, int width, int height)
{
  QDate weekDate = qd;
  int offset = mHeaderHeight+5;
  int cellWidth = width/2;
  int cellHeight = (height-offset)/3;

  if (KGlobal::locale()->weekStartsMonday())
    // correct to monday
    weekDate = qd.addDays(-(qd.dayOfWeek()-1));
  else
    // correct to sunday
    weekDate = qd.addDays(-(qd.dayOfWeek()%7));

  for (int i = 0; i < 7; i++, weekDate = weekDate.addDays(1)) {
    if (i < 3)
      drawDayBox(p, weekDate, 0, offset+i*cellHeight, 
		 cellWidth, cellHeight, TRUE);
    else
      if ((weekDate.dayOfWeek() == 6 && KGlobal::locale()->weekStartsMonday()) ||
	  (weekDate.dayOfWeek() == 5 && !KGlobal::locale()->weekStartsMonday()))
	drawDayBox(p, weekDate, cellWidth, offset+2*cellHeight, 
		   cellWidth, cellHeight/2, TRUE);
      else if ((weekDate.dayOfWeek() == 7 && KGlobal::locale()->weekStartsMonday()) ||
	       (weekDate.dayOfWeek() == 6 && !KGlobal::locale()->weekStartsMonday()))
	drawDayBox(p, weekDate, cellWidth, offset+2*cellHeight+(cellHeight/2),
		   cellWidth, cellHeight/2, TRUE);
      else
	drawDayBox(p, weekDate, cellWidth, offset+(i%3)*cellHeight,
		   cellWidth, cellHeight, TRUE);
  }
}

void CalPrinter::drawTimeTable(QPainter &p, const QDate &qd, int width, int height)
{
  QDate weekDate = qd;
  int offset = 5;
  int cellWidthTimeline = 40;
  int hoursToPrint = 12;
  int cellWidth = (width-cellWidthTimeline)/6;
  int cellHeight = (height-offset) / (hoursToPrint+1); // print 12 hours + 1 field for daily usage
  int ystartTimeLine =offset+mSubHeaderHeight+cellHeight;

  if (KGlobal::locale()->weekStartsMonday())
    // correct to monday
    weekDate = qd.addDays(-(qd.dayOfWeek()-1));
  else
    // correct to sunday
    weekDate = qd.addDays(-(qd.dayOfWeek()%7));

  // Draw the timeline info on the left site of the page
  QString numStr;
  for (int i = 0; i < hoursToPrint; i++) {
    p.drawRect(0, ystartTimeLine+i*cellHeight, //draw Rect for one hour
          cellWidthTimeline, cellHeight);
    p.drawLine(cellWidthTimeline/2,   //draw line for half an hour
          ystartTimeLine+i*cellHeight+(cellHeight/2),
	       cellWidthTimeline, ystartTimeLine+i*cellHeight+(cellHeight/2));
    numStr.setNum(i+mStartHour);
    p.setFont(QFont("helvetica", 10, QFont::Bold));
    p.drawText(0, ystartTimeLine+i*cellHeight, //draw hour text
          cellWidthTimeline/2, cellHeight/2,
	       AlignTop|AlignRight, numStr);
    p.setFont(QFont("helvetica", 8, QFont::Bold));
    p.drawText(cellWidthTimeline/2+2,  //draw minutes text
          ystartTimeLine+i*cellHeight, cellWidthTimeline/2,
          cellHeight/2, AlignTop | AlignLeft, "00");
  }

  // draw each day
  for (int i = 0; i < 7; i++, weekDate = weekDate.addDays(1)) {
    if (i < 6)
      drawTTDayBox(p, weekDate, cellWidthTimeline+i*cellWidth, offset,
		 cellWidth, cellHeight, TRUE);
  }
}

void CalPrinter::drawMonth(QPainter &p, const QDate &qd,
			   int width, int height)
{
  int weekdayCol;
  int offset = mHeaderHeight+5+mSubHeaderHeight;
  int cellWidth = width/7;
  int cellHeight = (height-offset) / 5;
  QDate monthDate(QDate(qd.year(), qd.month(), 1));

  if (KGlobal::locale()->weekStartsMonday())
    weekdayCol = monthDate.dayOfWeek() - 1;
  else
    weekdayCol = monthDate.dayOfWeek() % 7;
  monthDate = monthDate.addDays(-weekdayCol);

  for (int row = 0; row < (weekdayCol + qd.daysInMonth() - 1 )/7 + 1; row++) {
    for (int col = 0; col < 7; col++) {
      drawDayBox(p, monthDate, col*cellWidth, offset+row*cellHeight,
                 cellWidth, cellHeight);
      monthDate = monthDate.addDays(1);
    }
  }
}


void CalPrinter::drawSmallMonth(QPainter &p, const QDate &qd,
				int x, int y, int width, int height)
{
  bool firstCol = TRUE;
  QDate monthDate(QDate(qd.year(), qd.month(), 1));
  QDate monthDate2;
  int month = monthDate.month();

  // draw the title
  p.setFont(QFont("helvetica", 8, QFont::Bold));
  //  int lineSpacing = p.fontMetrics().lineSpacing();
  p.drawText(x, y, width, height/4, AlignCenter, KGlobal::locale()->monthName(qd.month()));

  int cellWidth = width/7;
  int cellHeight = height/8;
  QString tmpStr;
  KLocale *local = KGlobal::locale();

  if (KGlobal::locale()->weekStartsMonday())
    // correct to monday
    monthDate2 = monthDate.addDays(-(monthDate.dayOfWeek()-1));
  else
    // correct to sunday
    monthDate2 = monthDate.addDays(-(monthDate.dayOfWeek()%7));

  // draw days of week
   p.setFont(QFont("helvetica", 8, QFont::Bold));
  for (int col = 0; col < 7; col++) {
    // tmpStr.sprintf("%c",(const char*)monthDate2.dayName(monthDate2.dayOfWeek()));
    tmpStr=local->weekDayName(monthDate2.dayOfWeek())[0].upper();
    p.drawText(x+col*cellWidth, y+height/4, cellWidth, cellHeight,
	       AlignCenter, tmpStr);
    monthDate2 = monthDate2.addDays(1);
  }

  // draw separator line
  p.drawLine(x, y+height/4+cellHeight, x+width, y+height/4+cellHeight);

  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 7; col++) {
      if (monthDate.month() != month)
	break;
      if (firstCol) {
	firstCol = FALSE;
	if (KGlobal::locale()->weekStartsMonday())
	  col = monthDate.dayOfWeek() - 1;
	else
	  col = monthDate.dayOfWeek() % 7;
      }
      p.drawText(x+col*cellWidth, 
		 y+height/4+cellHeight+(row*cellHeight),
		 cellWidth, cellHeight, AlignCenter, 
		 tmpStr.setNum(monthDate.day()));
      monthDate = monthDate.addDays(1);
    }
  }
}

/****************************************************************************/

CalPrintDialog::CalPrintDialog(KPrinter *p, QWidget *parent, const char *name)
  : QDialog(parent, name, true)
{
  mPrinter = p;

  setCaption(i18n("Print"));
  
  QVBoxLayout *layout = new QVBoxLayout(this, 10);
  
  QGroupBox *rangeGroup = new QGroupBox(this);
  rangeGroup->setTitle(i18n("Date Range"));
  
  QVBoxLayout *layout2 = new QVBoxLayout(rangeGroup, 10);
  layout2->addSpacing(10);
  QHBoxLayout *subLayout2 = new QHBoxLayout();
  layout2->addLayout(subLayout2);

  mFromDateEdit = new KDateEdit(rangeGroup);
//  fromDated->setMinimumHeight(30);
//  fromDated->setMinimumSize(fromDated->sizeHint());
//  fromDated->setDate(fd);
  subLayout2->addWidget(mFromDateEdit);
  
  mToDateEdit = new KDateEdit(rangeGroup);
//  mToDateEdit->setMinimumSize(mToDateEdit->sizeHint());
//  mToDateEdit->setDate(td);
  subLayout2->addWidget(mToDateEdit);
    
  layout->addWidget(rangeGroup);
  
  mTypeGroup = new QButtonGroup(i18n("View Type"), this);
  QVBoxLayout *layout3 = new QVBoxLayout(mTypeGroup, 10);
  layout3->addSpacing(10);
  
  QRadioButton *rButt;
  layout3->addWidget(rButt = new QRadioButton(i18n("Day"), mTypeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintDay()));  
  //  rButt->setEnabled(FALSE);
 
  layout3->addWidget(rButt = new QRadioButton(i18n("Week"), mTypeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintWeek()));  

  layout3->addWidget(rButt = new QRadioButton(i18n("Month"), mTypeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintMonth()));  

  layout3->addWidget(rButt = new QRadioButton(i18n("To-do"), mTypeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintTodo()));

  layout3->addWidget(rButt = new QRadioButton(i18n("Timetable"), mTypeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintTimeTable()));

  layout->addWidget(mTypeGroup);

#if 0
  KSeparator *hLine = new KSeparator( KSeparator::HLine, this);
  layout->addWidget(hLine);
#endif

  QHBoxLayout *subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  mOkButton = new QPushButton(this);
  connect(mOkButton,SIGNAL(clicked()),SLOT(accept()));
  mOkButton->setDefault(true);
  mOkButton->setAutoDefault(true);
  subLayout->addWidget(mOkButton);

  QPushButton *button = new QPushButton(i18n("&Cancel"), this);
  connect(button, SIGNAL(clicked()),SLOT(reject()));
  subLayout->addWidget(button);
}

CalPrintDialog::~CalPrintDialog()
{
}

void CalPrintDialog::setRange(const QDate &from, const QDate &to)
{
  mFromDateEdit->setDate(from);
  mToDateEdit->setDate(to);
}

void CalPrintDialog::setPreview(bool preview)
{
  mOkButton->setText(preview ? i18n("&Preview") : i18n("&Print..."));
}

QDate CalPrintDialog::fromDate() const
{
  return mFromDateEdit->date();
}

QDate CalPrintDialog::toDate() const
{
  return mToDateEdit->date();
}

CalPrinter::PrintType CalPrintDialog::printType() const
{
  return mPrintType;
}

void CalPrintDialog::setPrintDay()
{
  mTypeGroup->setButton(0);
  mPrintType = CalPrinter::Day;
}

void CalPrintDialog::setPrintWeek()
{
  mTypeGroup->setButton(1);
  mPrintType = CalPrinter::Week;
}

void CalPrintDialog::setPrintMonth()
{
  mTypeGroup->setButton(2);
  mPrintType = CalPrinter::Month;
}

void CalPrintDialog::setPrintTodo()
{
  mTypeGroup->setButton(3);
  mPrintType = CalPrinter::Todolist;
}

void CalPrintDialog::setPrintTimeTable()
{
  mTypeGroup->setButton(4);
  mPrintType = CalPrinter::TimeTable;
}
