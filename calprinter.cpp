/* CalPrinter.cpp
 * Copyright (c) 1998 Preston Brown
 *
 * $Id$
 */

#include <math.h>

#include <qpainter.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include <klocale.h>
#include <kstddirs.h>
#include <kdateedit.h>
#include <kseparator.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kapp.h>
#include <kdebug.h>

#include "koprefsdialog.h"
#include "koprefs.h"
#include "todo.h"

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

  mPreviewFile = 0;

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
  }

  if (mPrintDialog->exec() == QDialog::Accepted) {
    doPreview(mPrintDialog->printType(),mPrintDialog->fromDate(),
              mPrintDialog->toDate());
  }
}

void CalPrinter::doPreview(int pt, QDate fd, QDate td)
{
  bool oldOutputToFile = mPrinter->outputToFile();
  QString oldFileName = mPrinter->outputFileName();

  mPreviewFile = new KTempFile;
  mPreviewFile->setAutoDelete(true);
  mPrinter->setOutputToFile(true);
  mPrinter->setOutputFileName(mPreviewFile->name());

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
  }

  // restore previous settings that were used before the preview.
  mPrinter->setOutputToFile(oldOutputToFile);
  mPrinter->setOutputFileName(oldFileName);
  
  QString previewProg = KOPrefs::instance()->mPrintPreview;

  KProcess *previewProc = new KProcess;
  connect(previewProc, SIGNAL(processExited(KProcess *)), 
	  SLOT(previewCleanup(KProcess *)));

  previewProc->clearArguments(); // clear out any old arguments
  *previewProc << previewProg; // program name
  *previewProc << mPreviewFile->name(); // command line arguments
  if (!previewProc->start()) {
    KMessageBox::error(0,i18n("Could not start %1.").arg(previewProg));
  }
}

void CalPrinter::previewCleanup(KProcess *process)
{
  delete process;
  delete mPreviewFile;
  mPreviewFile = 0;
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
  }

  if (mPrintDialog->exec() == QDialog::Accepted) {
    doPrint(mPrintDialog->printType(),mPrintDialog->fromDate(),
            mPrintDialog->toDate());
  }
}

void CalPrinter::doPrint(int pt, QDate fd, QDate td)
{
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
  }
}

///////////////////////////////////////////////////////////////////////////////

void CalPrinter::updateConfig()
{
  // printer name
  QString pName = KOPrefs::instance()->mPrinter;
  if (!pName.isEmpty())
    mPrinter->setPrinterName(pName);

  // paper size
  int val;
  val = KOPrefs::instance()->mPaperSize;
  switch(val) {
    case 0: mPrinter->setPageSize(KPrinter::A4); break;
    case 1: mPrinter->setPageSize(KPrinter::B5); break;
    case 2: mPrinter->setPageSize(KPrinter::Letter); break;
    case 3: mPrinter->setPageSize(KPrinter::Legal); break;
    case 4: mPrinter->setPageSize(KPrinter::Executive); break;
  }
 
  // paper orientation
  // ignored for now.
  /*  val = config->readNumEntry("Paper Orientation", 1);
  if (val == 0)
    mPrinter->setOrientation(KPrinter::Portrait);
  else 
    mPrinter->setOrientation(KPrinter::Landscape);
  */

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
     drawHeader(p, fd, td,
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
    curMonth = curMonth.addDays(fromMonth.daysInMonth());
    if (fromMonth <= toMonth)
      mPrinter->newPage();
  } while (curMonth <= toMonth);

  p.end();
}

void CalPrinter::printTodo(const QDate &fd, const QDate &td)
{
  KLocale *local = KGlobal::locale();
  QPainter p;

  mPrinter->setOrientation(KPrinter::Portrait);

  p.begin(mPrinter);
  int pageWidth = p.viewport().width();
  int pageHeight = p.viewport().height();
  mHeaderHeight = pageHeight/7 - 20;

  int pospriority = 10;
  int possummary = 50;
  int posdue = pageWidth - 100;
  int lineSpacing = 15;
  int fontHeight = 10;

  drawHeader(p, fd, td, fd, pageWidth, mHeaderHeight, Todolist);

  QList<Todo> todoList = mCalendar->getTodoList();
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
      if (priority > 0) {
	  outStr.setNum(priority);
	 
	  p.drawText(pospriority, (lineSpacing*count)+mHeaderHeight,
		     outStr);
      }
      // summary
      outStr=currEvent->summary();
     
      p.drawText(possummary, (lineSpacing*count)+mHeaderHeight,
		 outStr);
      // due
      if (currEvent->hasDueDate()){
        outStr = local->formatDate(currEvent->dtDue().date());
        p.drawText(posdue, (lineSpacing*count)+mHeaderHeight,
		 outStr);
      }
      // if terminated, cross it
      if (currEvent->isCompleted()) {
	  p.drawLine( 5, (lineSpacing*count)+mHeaderHeight-fontHeight/2 + 2, 
		      pageWidth-5, (lineSpacing*count)+mHeaderHeight-fontHeight/2 + 2);
      }
      currEvent = todoList.next();
      ++count;
    }
  }
  p.end();
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
  case Todolist:
    title +=  i18n("To-Do items:");
   
    p.drawText(5, lineSpacing,title);
    break;
  case Month:
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
  QList<Event> eventList;
  QString ampm;

  QString hstring(mCalendar->getHolidayForDate(qd));

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

  eventList = mCalendar->getEventsForDate(qd, TRUE);
  eventList.first();
  int count = 1;
  QString outStr;
  Event *currEvent(eventList.first());
  p.setFont(QFont("helvetica", 8));
  int lineSpacing = p.fontMetrics().lineSpacing();

  while (count <= 9 && (currEvent != NULL)) {
    if (currEvent->doesFloat() || currEvent->isMultiDay())
      outStr += currEvent->summary();
    
    else {
      QTime t1 = currEvent->dtStart().time();
      
      outStr = local->formatTime(t1);
      outStr += currEvent->summary();
  
    } // doesFloat
     
    p.drawText(x+5, y+(lineSpacing*(count+1)), width-10, lineSpacing, 
	       AlignLeft|AlignVCenter, outStr);
    currEvent = eventList.next();
    ++count;
  }
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
  int offset = mHeaderHeight + 5;
  int cellWidth = width-80;
  int cellHeight = (height-offset) / 12; // 12 hour increments.

  QString numStr;
  for (int i = 0; i < 12; i++) {
    p.drawRect(0, offset+i*cellHeight, 75, cellHeight);
    p.drawLine(37, offset+i*cellHeight+(cellHeight/2),
	       75, offset+i*cellHeight+(cellHeight/2));
    numStr.setNum(i+mStartHour);
    p.setFont(QFont("helvetica", 20, QFont::Bold));
    p.drawText(0, offset+i*cellHeight, 33, cellHeight/2,
	       AlignTop|AlignRight, numStr);
    p.setFont(QFont("helvetica", 14, QFont::Bold));
    p.drawText(37, offset+i*cellHeight, 45, cellHeight/2,
	       AlignTop | AlignLeft, "00");
    p.drawRect(80, offset+i*cellHeight,
	       cellWidth, cellHeight);
    p.drawLine(80, offset+i*cellHeight+(cellHeight/2),
    	       cellWidth+80, offset+i*cellHeight+(cellHeight/2));

  }

  p.setFont(QFont("helvetica", 14));
  QList<Event> eventList = mCalendar->getEventsForDate(qd, TRUE);
  Event *currEvent;
  p.setBrush(QBrush(Dense7Pattern));
  for (currEvent = eventList.first(); currEvent;
       currEvent = eventList.next()) {
    int startTime = currEvent->dtStart().time().hour();
    int endTime = currEvent->dtEnd().time().hour();
    float minuteInc = cellHeight / 60.0;
    if ((startTime >= mStartHour)  && 
	(endTime <= (mStartHour + 12))) {
      startTime -= mStartHour;
      int startMinuteOff = (int) (minuteInc * 
	currEvent->dtStart().time().minute());
      endTime -= mStartHour;
      int endMinuteOff = (int) (minuteInc * 
	currEvent->dtEnd().time().minute());
      p.drawRect(80, offset+startMinuteOff+startTime*cellHeight, 
		 cellWidth, endMinuteOff + (endTime - startTime)*cellHeight);
      p.drawText(85, 
		 offset+startMinuteOff+startTime*cellHeight+5,
		 cellWidth-10, 
		 endMinuteOff + (endTime - startTime)*cellHeight-10,
		 AlignLeft | AlignTop, currEvent->summary());
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

  layout3->addWidget(rButt = new QRadioButton(i18n("To-Do"), mTypeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintTodo()));  
  
  layout->addWidget(mTypeGroup);

  KSeparator *hLine = new KSeparator( KSeparator::HLine, this);
  layout->addWidget(hLine);

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
  mOkButton->setText(preview ? i18n("&Preview") : i18n("&Print"));
}

QDate CalPrintDialog::fromDate() const
{
  return mFromDateEdit->getDate();
}

QDate CalPrintDialog::toDate() const
{
  return mToDateEdit->getDate();
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
