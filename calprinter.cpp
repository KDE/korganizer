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
#include <kdated.h>
#include <kmessagebox.h>
#include <kapp.h>

#include "kooptionsdialog.h"
#include "koprefs.h"

#include "calprinter.h"
#include "calprinter.moc"

CalPrinter::CalPrinter(QWidget *par, CalObject *cal)
  : QObject(0L, "CalPrinter")
{
  calendar = cal;
  parent = par;
  printer = new QPrinter;
  cpd = 0L;
  previewProc = new KProcess;
  connect(previewProc, SIGNAL(processExited(KProcess *)), 
	  SLOT(previewCleanup()));
  printer->setOrientation(QPrinter::Landscape);

  updateConfig();
}

CalPrinter::~CalPrinter()
{
  delete printer;
  delete previewProc;
}

void CalPrinter::setupPrinter()
{
  KOOptionsDialog *optionsDlg = new KOOptionsDialog;
  optionsDlg->showPrinterTab();
  connect(optionsDlg, SIGNAL(configChanged()),
	  parent, SLOT(updateConfig()));
//  connect(optionsDlg, SIGNAL(closed(QWidget *)), 
//	  parent, SLOT(cleanWindow(QWidget *)));
  optionsDlg->show(); 
} 

void CalPrinter::preview(PrintType pt, const QDate &fd, const QDate &td)
{
  previewFileName = tmpnam(0L);
  oldOutputToFile = printer->outputToFile();
  oldFileName = printer->outputFileName();

  printer->setOutputToFile(TRUE);
  printer->setOutputFileName(previewFileName);

  cpd = new CalPrintDialog(printer, TRUE, fd, td);
  switch(pt) {
  case Day: 
    cpd->setPrintDay();
    break;
  case Week: 
    cpd->setPrintWeek();
    break;
  case Month: 
    cpd->setPrintMonth();
    break;
  case Todo: 
    cpd->setPrintTodo(); 
    break;
  }

  connect(cpd, SIGNAL(doneSignal(int, QDate, QDate)), 
	  this, SLOT(doPreview(int, QDate, QDate)));
  cpd->show();
}

void CalPrinter::doPreview(int pt, QDate fd, QDate td)
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
  case Todo:
    printTodo(fd, td);
    break;
  }
  
  // restore previous settings that were used before the preview.
  printer->setOutputToFile(oldOutputToFile);
  printer->setOutputFileName(oldFileName);
  
  previewProg = KOPrefs::instance()->mPrintPreview;

  previewProc->clearArguments(); // clear out any old arguments
  *previewProc << previewProg; // program name
  *previewProc << previewFileName; // command line arguments
  if (!previewProc->start()) {
    KMessageBox::error(0,i18n("Could not start %1.").arg(previewProg));
  }
}

void CalPrinter::print(PrintType pt, const QDate &fd, const QDate &td)
{
  cpd = new CalPrintDialog(printer, FALSE, fd, td);
  switch(pt) {
  case Day: 
    cpd->setPrintDay();
    break;
  case Week: 
    cpd->setPrintWeek();
    break;
  case Month: 
    cpd->setPrintMonth();
    break;
  case Todo: 
    cpd->setPrintTodo(); 
    break;
  }

  connect(cpd, SIGNAL(doneSignal(int, QDate, QDate)), 
	  this, SLOT(doPrint(int, QDate, QDate)));
  cpd->show();
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
  case Todo: 
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
    printer->setPrinterName(pName);

  // paper size
  int val;
  val = KOPrefs::instance()->mPaperSize;
  switch(val) {
  case 0: printer->setPageSize(QPrinter::A4); break;
  case 1: printer->setPageSize(QPrinter::B5); break;
  case 2: printer->setPageSize(QPrinter::Letter); break;
  case 3: printer->setPageSize(QPrinter::Legal); break;
  case 4: printer->setPageSize(QPrinter::Executive); break;
  }
 
  // paper orientation
  // ignored for now.
  /*  val = config->readNumEntry("Paper Orientation", 1);
  if (val == 0)
    printer->setOrientation(QPrinter::Portrait);
  else 
    printer->setOrientation(QPrinter::Landscape);
  */

  weekStartsMonday = KOPrefs::instance()->mWeekstart;
  timeAmPm = (KOPrefs::instance()->mTimeFormat ? TRUE : FALSE);

  startHour = KOPrefs::instance()->mDayBegins;
}

void CalPrinter::printDay(const QDate &fd, const QDate &td)
{
  QPainter p;
  QDate curDay, fromDay, toDay;

  printer->setOrientation(QPrinter::Portrait);

  fromDay = fd;
  curDay = fd;
  toDay = td;

  p.begin(printer);
  // the painter initially begins at 72 dpi per the Qt docs. 
  // we want half-inch margins.
  margin = 36;
  p.setViewport(margin, margin, 
		p.viewport().width()-margin, 
		p.viewport().height()-margin);
  pageWidth = p.viewport().width();
  pageHeight = p.viewport().height();
  headerHeight = 72;
  subHeaderHeight = 20;

  do {
    drawHeader(p, curDay,toDay,curDay,
	       pageWidth, headerHeight, Day);
    drawDay(p, curDay, pageWidth, pageHeight);
    curDay = curDay.addDays(1);
    if (curDay <= toDay)
      printer->newPage();
  } while (curDay <= toDay);
  
  p.end();
}

void CalPrinter::printWeek(const QDate &fd, const QDate &td)
{
  QPainter p;
  QDate curWeek, fromWeek, toWeek;

  printer->setOrientation(QPrinter::Portrait);

  if (weekStartsMonday) {
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

  p.begin(printer);
  // the painter initially begins at 72 dpi per the Qt docs. 
  // we want half-inch margins.
  margin = 36;
  p.setViewport(margin, margin, 
		p.viewport().width()-margin, 
		p.viewport().height()-margin);
  pageWidth = p.viewport().width();
  pageHeight = p.viewport().height();
  headerHeight = 72;
  subHeaderHeight = 20;

  curWeek = fromWeek.addDays(6);
  do {
     drawHeader(p, fd, td,
	       curWeek,
	       pageWidth, headerHeight, Week);
    drawWeek(p, curWeek, pageWidth, pageHeight);
    curWeek = curWeek.addDays(7);
    if (curWeek <= toWeek)
      printer->newPage();
  } while (curWeek <= toWeek);
  
  p.end();
}

void CalPrinter::printMonth(const QDate &fd, const QDate &td)
{
  QPainter p;
  QDate curMonth, fromMonth, toMonth;

  printer->setOrientation(QPrinter::Landscape);

  fromMonth = fd.addDays(-(fd.day()-1));
  toMonth = td.addDays(td.daysInMonth()-td.day());

  p.begin(printer);
  // the painter initially begins at 72 dpi per the Qt docs. 
  // we want half-inch margins.
  margin = 36;
  p.setViewport(margin, margin, 
		p.viewport().width()-margin, 
		p.viewport().height()-margin);
  pageWidth = p.viewport().width();
  pageHeight = p.viewport().height();
  headerHeight = 72;
  subHeaderHeight = 20;

  curMonth = fromMonth;
  do {
    drawHeader(p, fromMonth, 
	       toMonth, curMonth,
	       pageWidth, headerHeight, Month);
    drawDaysOfWeek(p, curMonth, pageWidth, pageHeight);
    drawMonth(p, curMonth, pageWidth, pageHeight);
    curMonth = curMonth.addDays(fromMonth.daysInMonth());
    if (fromMonth <= toMonth)
      printer->newPage();
  } while (curMonth <= toMonth);
  
  p.end();
}

void CalPrinter::printTodo(const QDate &fd, const QDate &td)
{
  KLocale *local = KGlobal::locale();
  QPainter p;
  QList<KOEvent> todoList;

  printer->setOrientation(QPrinter::Portrait);

  p.begin(printer);
  pageWidth = p.viewport().width();
  pageHeight = p.viewport().height();
  headerHeight = pageHeight/7 - 20;

  int pospriority = 10;
  int possummary = 50;
  int posdue = pageWidth - 100;
  int lineSpacing = 15;
  int fontHeight = 10;

  drawHeader(p, fd, td, fd, pageWidth, headerHeight, Todo);

  todoList = calendar->getTodoList();
  todoList.first();
  int count = 1;
  QString outStr;
 
  p.setFont(QFont("helvetica", 10));
  lineSpacing = p.fontMetrics().lineSpacing();
  // draw the headers
  p.setFont(QFont("helvetica", 10, QFont::Bold));
  outStr += i18n("Priority");
  
  p.drawText(pospriority, headerHeight - 2,
	     outStr);
  outStr.truncate(0);
  outStr += i18n("Summary");
  
  p.drawText(possummary, headerHeight - 2,
		 outStr);
  outStr.truncate(0);
  outStr += i18n("Due");
 
  p.drawText(posdue,  headerHeight - 2,
		 outStr);  
  p.setFont(QFont("helvetica", 10));

  fontHeight =  p.fontMetrics().height();
  for(int cprior = 1; cprior <= 6; cprior++) {
    KOEvent *currEvent(todoList.first());
    while (currEvent != NULL) {
      QDate due = currEvent->getDtEnd().date();
      QDate start = currEvent->getDtStart().date();
      // if it is not to start yet, skip.
      if ( (!start.isValid()) && (start >= td) ) {
	currEvent = todoList.next();
	continue;
      }      
      // priority
      int priority = currEvent->getPriority();
      // 6 is the lowest priority (the unspecified one)
      if ((priority != cprior) || ((cprior==6) && (priority==0))) {
	currEvent = todoList.next();
	continue;
      }
      if (priority > 0) {
	  outStr.setNum(priority);
	 
	  p.drawText(pospriority, (lineSpacing*count)+headerHeight,
		     outStr);
      }
      // summary
      outStr=currEvent->getSummary();
     
      p.drawText(possummary, (lineSpacing*count)+headerHeight,
		 outStr);
      // due
      if (currEvent->hasDueDate()){
      outStr = local->formatDate(due);
      p.drawText(posdue, (lineSpacing*count)+headerHeight,
		 outStr);
      }
      // if terminated, cross it
      int status = currEvent->getStatus();
      if (status == KOEvent::COMPLETED) {
	  p.drawLine( 5, (lineSpacing*count)+headerHeight-fontHeight/2 + 2, 
		      pageWidth-5, (lineSpacing*count)+headerHeight-fontHeight/2 + 2);
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

  p.drawRect(0, 0, width, height);
  p.fillRect(1, 1, 
	     width-2, 
	     height-2, 
	     QBrush(Dense3Pattern));

  p.setFont(QFont("helvetica", 24, QFont::Bold));
  int lineSpacing = p.fontMetrics().lineSpacing();
  QString title;
  QString myOwner(calendar->getOwner());

  //  title.sprintf("%s %d Schedule for ",qd.monthName(qd.month()),qd.year());
  //  title += myOwner;

  switch(pt) {
  case Todo:
    title +=  i18n("To-Do items:");
   
    p.drawText(5, lineSpacing,title);
    break;
  case Month:
  case Week:
    
    title += local->formatDate(fd);
   
    p.drawText(5, lineSpacing, title );
    title.truncate(0);
   
    title+= local->formatDate(td);
    p.drawText(5, 2*lineSpacing, title);
    break;
  case Day:
   
    title =+ local->formatDate(fd,false);
    p.drawText(5, lineSpacing, title );
    
  }
  
  // print previous month for month view, print current for todo, day and week
  switch (pt) {
  case Todo:
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
             QBrush(Dense3Pattern));
  p.drawText(x+5, y, width-10, height, AlignCenter | AlignVCenter,
             local->WeekDayName(qd.dayOfWeek()));
}

void CalPrinter::drawDayBox(QPainter &p, const QDate &qd,
			    int x, int y, int width, int height, 
			    bool fullDate)
{
  KLocale *local = KGlobal::locale();
  QString dayNumStr;
  QList<KOEvent> eventList;
  QString ampm;

  QString hstring(calendar->getHolidayForDate(qd));

  if (fullDate) {
    int index;
    dayNumStr= qd.toString();
    index = dayNumStr.find(' ');
    dayNumStr.remove(0, index);
    index = dayNumStr.findRev(' ');
    dayNumStr.truncate(index);
    dayNumStr = local->WeekDayName(qd.dayOfWeek()) + dayNumStr;
  } else {
    dayNumStr = QString::number(qd.day());
  }

  p.drawRect(x, y, width, height);
  // p.fillRect(x+1, y+1, width-2,height, QBrush(Dense3Pattern));
  p.drawRect(x, y, width, subHeaderHeight);
  p.fillRect(x+1, y+1, width-2, subHeaderHeight-2, QBrush(Dense4Pattern));
  if (!hstring.isEmpty()) {
    p.setFont(QFont("helvetica", 8, QFont::Bold, TRUE));

    p.drawText(x+5, y, width-25, subHeaderHeight, AlignLeft | AlignVCenter,
	       hstring);
  }
  p.setFont(QFont("helvetica", 10, QFont::Bold));
  p.drawText(x+5, y, width-10, subHeaderHeight, AlignRight | AlignVCenter, 
	     dayNumStr);

  eventList = calendar->getEventsForDate(qd, TRUE);
  eventList.first();
  int count = 1;
  QString outStr;
  KOEvent *currEvent(eventList.first());
  p.setFont(QFont("helvetica", 8));
  int lineSpacing = p.fontMetrics().lineSpacing();

  while (count <= 9 && (currEvent != NULL)) {
    if (currEvent->doesFloat() || currEvent->isMultiDay())
      outStr += currEvent->getSummary();
    
    else {
      QTime t1 = currEvent->getDtStart().time();
      
      if (timeAmPm) local->use12Clock();
      outStr = local->formatTime(t1);
      outStr += currEvent->getSummary();
  
    } // doesFloat
     
    p.drawText(x+5, y+(lineSpacing*(count+1)), width-10, lineSpacing, 
	       AlignLeft|AlignVCenter, outStr);
    currEvent = eventList.next();
    ++count;
  }
}

void CalPrinter::drawDaysOfWeek(QPainter &p, const QDate &qd, 
				int width, int height)
{	
  int offset=headerHeight+5;
  int cellWidth = width/7;
  int cellHeight = subHeaderHeight;
  QDate monthDate(QDate(qd.year(), qd.month(),1));
  
  if (weekStartsMonday)
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
  int offset = headerHeight + 5;
  int cellWidth = width-80; 
  int cellHeight = (height-offset) / 12; // 12 hour increments.
  
  QString numStr;
  for (int i = 0; i < 12; i++) {
    p.drawRect(0, offset+i*cellHeight, 75, cellHeight);
    p.drawLine(37, offset+i*cellHeight+(cellHeight/2), 
	       75, offset+i*cellHeight+(cellHeight/2));
    numStr.setNum(i+startHour);
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
  QList<KOEvent> eventList = calendar->getEventsForDate(qd, TRUE);
  KOEvent *currEvent;
  p.setBrush(QBrush(Dense4Pattern));
  for (currEvent = eventList.first(); currEvent;
       currEvent = eventList.next()) {
    int startTime = currEvent->getDtStart().time().hour();
    int endTime = currEvent->getDtEnd().time().hour();
    float minuteInc = cellHeight / 60.0;
    if ((startTime >= startHour)  && 
	(endTime <= (startHour + 12))) {
      startTime -= startHour;
      int startMinuteOff = (int) (minuteInc * 
	currEvent->getDtStart().time().minute());
      endTime -= startHour;
      int endMinuteOff = (int) (minuteInc * 
	currEvent->getDtEnd().time().minute());
      p.drawRect(80, offset+startMinuteOff+startTime*cellHeight, 
		 cellWidth, endMinuteOff + (endTime - startTime)*cellHeight);
      p.drawText(85, 
		 offset+startMinuteOff+startTime*cellHeight+5,
		 cellWidth-10, 
		 endMinuteOff + (endTime - startTime)*cellHeight-10,
		 AlignLeft | AlignTop, currEvent->getSummary());
    }
  }
  p.setBrush(QBrush(NoBrush));
}

void CalPrinter::drawWeek(QPainter &p, const QDate &qd, int width, int height)
{
  QDate weekDate = qd;
  int offset = headerHeight+5;
  int cellWidth = width/2;
  int cellHeight = (height-offset)/3;

  if (weekStartsMonday)
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
      if ((weekDate.dayOfWeek() == 6 && weekStartsMonday) ||
	  (weekDate.dayOfWeek() == 5 && !weekStartsMonday))
	drawDayBox(p, weekDate, cellWidth, offset+2*cellHeight, 
		   cellWidth, cellHeight/2, TRUE);
      else if ((weekDate.dayOfWeek() == 7 && weekStartsMonday) ||
	       (weekDate.dayOfWeek() == 6 && !weekStartsMonday))
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
  bool firstCol = TRUE;
  int offset = headerHeight+5+subHeaderHeight;
  int cellWidth = width/7;
  int cellHeight = (height-offset) / 5;
  QDate monthDate(QDate(qd.year(), qd.month(), 1));
  int month = monthDate.month();

  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 7; col++) {
      if (monthDate.month() != month)
	break;
      if (firstCol) {
	firstCol = FALSE;
	if (weekStartsMonday)
	  col = monthDate.dayOfWeek() - 1;
	else
	  col = monthDate.dayOfWeek() % 7;
      }
      drawDayBox(p, monthDate, 
		 col*cellWidth, offset+row*cellHeight,
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
  p.drawText(x, y, width, height/4, AlignCenter, qd.monthName(qd.month()));

  int cellWidth = width/7;
  int cellHeight = height/8;
  QString tmpStr;
  KLocale *local = KGlobal::locale();

  if (weekStartsMonday)
    // correct to monday
    monthDate2 = monthDate.addDays(-(monthDate.dayOfWeek()-1)); 
  else
    // correct to sunday
    monthDate2 = monthDate.addDays(-(monthDate.dayOfWeek()%7)); 

  // draw days of week
  for (int col = 0; col < 7; col++) {
    // tmpStr.sprintf("%c",(const char*)monthDate2.dayName(monthDate2.dayOfWeek()));
    tmpStr=local->WeekDayName(monthDate2.dayOfWeek());
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
	if (weekStartsMonday)
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

CalPrintDialog::CalPrintDialog(QPrinter *p, bool preview, const QDate &fd, 
			       const QDate &td, QWidget *parent, 
			       const char *name)
  : QDialog(parent, name, FALSE)
{
  printer = p;
  
  QVBoxLayout *layout = new QVBoxLayout(this, 10);
  
  QGroupBox *rangeGroup = new QGroupBox(this);
  rangeGroup->setTitle(i18n("Date Range"));
  
  QVBoxLayout *layout2 = new QVBoxLayout(rangeGroup, 10);
  layout2->addSpacing(10);
  QHBoxLayout *subLayout2 = new QHBoxLayout();
  layout2->addLayout(subLayout2);
  
  fromDated = new KDateEdit(rangeGroup);
  fromDated->setMinimumHeight(30);
  fromDated->setMinimumSize(fromDated->sizeHint());
  fromDated->setDate(fd);
  subLayout2->addWidget(fromDated);
  
  toDated = new KDateEdit(rangeGroup);
  toDated->setMinimumSize(toDated->sizeHint());
  toDated->setDate(td);
  subLayout2->addWidget(toDated);
    
  layout->addWidget(rangeGroup);
  
  typeGroup = new QButtonGroup(i18n("View Type"), this);
  QVBoxLayout *layout3 = new QVBoxLayout(typeGroup, 10);
  layout3->addSpacing(10);
  
  QRadioButton *rButt;
  layout3->addWidget(rButt = new QRadioButton(i18n("Day"), typeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintDay()));  
  //  rButt->setEnabled(FALSE);
 
  layout3->addWidget(rButt = new QRadioButton(i18n("Week"), typeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintWeek()));  

  layout3->addWidget(rButt = new QRadioButton(i18n("Month"), typeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintMonth()));  

  layout3->addWidget(rButt = new QRadioButton(i18n("To-Do"), typeGroup));
  rButt->setMinimumHeight(rButt->sizeHint().height()-5);
  connect(rButt,  SIGNAL(clicked()), this, SLOT(setPrintTodo()));  
  
  layout->addWidget(typeGroup);

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::HLine|QFrame::Sunken);
  hLine->setFixedHeight(hLine->sizeHint().height());
  layout->addWidget(hLine);

  QHBoxLayout *subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  QPushButton *button = new QPushButton((preview ? i18n("&Preview")
					 : i18n("&Print")), 
					this);
  connect(button, SIGNAL(clicked()),
	  this, SLOT(accept()));
  button->setDefault(TRUE);
  button->setAutoDefault(TRUE);
  button->setFixedSize(button->sizeHint());
  subLayout->addWidget(button);

  button = new QPushButton(i18n("&Cancel"), this);
  connect(button, SIGNAL(clicked()),
	  this, SLOT(reject()));
  button->setFixedSize(button->sizeHint());
  subLayout->addWidget(button);

  adjustSize();

  layout->activate();
}

CalPrintDialog::~CalPrintDialog()
{
}

inline QDate CalPrintDialog::getFrom() const
{
  return fromDated->getDate();
}

inline QDate CalPrintDialog::getTo() const
{
  return toDated->getDate();
}

inline const CalPrinter::PrintType CalPrintDialog::getPrintType() const
{
  return pt;
}

void CalPrintDialog::setPrintDay()
{
  typeGroup->setButton(0);
  pt = CalPrinter::Day;
}

void CalPrintDialog::setPrintWeek()
{
  typeGroup->setButton(1);
  pt = CalPrinter::Week;
}

void CalPrintDialog::setPrintMonth()
{
  typeGroup->setButton(2);
  pt = CalPrinter::Month;
}

void CalPrintDialog::setPrintTodo()
{
  typeGroup->setButton(3);
  pt = CalPrinter::Todo;
}

void CalPrintDialog::accept()
{
  hide();
  emit doneSignal(pt, fromDated->getDate(), toDated->getDate());
  delete this;
}

void CalPrintDialog::reject()
{
  hide();
  delete this;
}
