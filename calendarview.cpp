/*
  $Id$
  
  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.troll.no and http://www.kde.org respectively
  
  Copyright (c) 1997, 1998, 1999
  Preston Brown (preston.brown@yale.edu)
  Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
  Ian Dawes (iadawes@globalserve.net)
  Laszlo Boloni (boloni@cs.purdue.edu)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qcursor.h>
#include <qmultilineedit.h>
#include <qtimer.h>
#include <qwidgetstack.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "koarchivedlg.h"
#include "komailclient.h"
#include "calprinter.h"
#include "exportwebdialog.h"
#include "kooptionsdialog.h"
#include "koeventeditor.h"
#include "kotodoeditor.h"
#include "koprefs.h"
#include "koeventviewerdialog.h"
#include "koarchivedlg.h"

#include "calendarview.h"
#include "calendarview.moc"


CalendarView::CalendarView(QWidget *parent,const char *name) 
  : QWidget(parent,name)
{
  qDebug("CalendarView::CalendarView()");

  currentView = 0;
  todoView = 0;
  agendaView = 0;
  monthView = 0;
  listView = 0;

  mModified=false;
  mReadOnly = false;
  
  searchDlg = 0L;
  mArchiveDialog = 0;

  setMinimumSize(620,400);	// make sure we don't get resized too small...

  // Create calendar object, which manages all calendar information associated
  // with this calendar view window.
  mCalendar = new CalObject;

  mCalendar->setTopwidget(this);

  QBoxLayout *topLayout = new QVBoxLayout(this);

  // create the main layout frames.
  panner = new QSplitter(QSplitter::Horizontal,this,"CalendarView::Panner");
  topLayout->addWidget(panner);

  leftFrame = new QSplitter(QSplitter::Vertical,panner,
                            "CalendarView::LeftFrame");
  rightFrame = new QWidgetStack(panner, "CalendarView::RightFrame");

  mOptionsDialog = new KOOptionsDialog(this);
  connect(mOptionsDialog,SIGNAL(configChanged()),SLOT(updateConfig()));

  dateNavigator = new KDateNavigator(leftFrame, mCalendar, TRUE,
                        "CalendarView::DateNavigator", QDate::currentDate());

//  if (!filename.isEmpty()) initCalendar(filename);

  todoList   = new KOTodoView(mCalendar, leftFrame, "CalendarView::TodoList");

  // create the main data display views.
  todoView   = new KOTodoView(mCalendar, rightFrame, "CalendarView::TodoView");
  rightFrame->addWidget(todoView,0);

  agendaView = new KOAgendaView(mCalendar, rightFrame, "CalendarView::AgendaView");
  rightFrame->addWidget(agendaView,1);
  mCalendarViews.append(agendaView);

  listView   = new KOListView(mCalendar, rightFrame, "CalendarView::ListView");
  rightFrame->addWidget(listView,2);
  mCalendarViews.append(listView);

  monthView = new KOMonthView(mCalendar, rightFrame, "CalendarView::MonthView");
  rightFrame->addWidget(monthView,3);
  mCalendarViews.append(monthView);

  readCurrentView();

/*
  // List classnames of available views
  QObject *obj;
  for(obj=mCalendarViews.first();obj;obj=mCalendarViews.next())
    qDebug("calViews: %s",obj->className());
*/  

  // set up printing object
  calPrinter = new CalPrinter(this, mCalendar);

  // set up web exporting object
  mExportWebDialog = new ExportWebDialog(mCalendar);

  // hook up the signals/slots of all widgets together so communication
  // can happen when things get clicked.
  hookupSignals();

  if (currentView) {
    rightFrame->raiseWidget(currentView);
  } else {
    rightFrame->raiseWidget(todoView);
  }

  goToday();

  changeAgendaView(agendaViewMode);

  setupRollover();

  updateConfig();

  qDebug("CalendarView::CalendarView() done");
}

CalendarView::~CalendarView()
{
  qDebug("~CalendarView()");
  hide();

  // clean up our calender object
  mCalendar->close();  // CS: This seems to cause a "Double QObject deletion"
  delete mCalendar;
  mCalendar = 0;

  qDebug("~CalendarView() done");
}


bool CalendarView::openCalendar(QString filename)
{
  qDebug("CalendarView::openCalendar()");
  if (initCalendar(filename)) {
    setModified(false);
    updateView();
    return true;
  } else {
    return false;
  }
}


bool CalendarView::mergeCalendar(QString filename)
{
  qDebug("CalendarView::mergeCalendar()");
  if (mCalendar->load(filename)) {
    setModified(false);
    updateView();
    return true;
  } else {
    return false;
  }
}


bool CalendarView::saveCalendar(QString filename)
{
  qDebug("CalendarView::saveCalendar(): %s",filename.latin1());
  mCalendar->save(filename);
  
  // We should check for errors here.

  setModified(false);
  
  return true;
}

void CalendarView::closeCalendar()
{
  qDebug("CalendarView::closeCalendar()");

  // child windows no longer valid
  emit closingDown();

  mCalendar->close();
  setModified(false);
  updateView();
}

void CalendarView::archiveCalendar()
{
  if (!mArchiveDialog) mArchiveDialog = new ArchiveDialog(mCalendar,this);
  mArchiveDialog->show();
  mArchiveDialog->raise();
  
  // Workaround.
  QApplication::restoreOverrideCursor();
}

bool CalendarView::initCalendar(QString filename)
{
  // read the settings from the config file
  readSettings();

  qDebug("CalendarView::initCalendar(): filename: %s",filename.latin1());
  
  QApplication::setOverrideCursor(waitCursor);

  if (!filename.isEmpty()) {
    setModified(false);
    if (!(mCalendar->load(filename))) {
      // while failing to load, the calendar object could 
      // have become partially populated.  Cle1ar it out.
      mCalendar->close();
      QApplication::restoreOverrideCursor();
      return false;
    }
  }

  QApplication::restoreOverrideCursor();
  
  return true;
}


void CalendarView::readSettings()
{
//  qDebug("CalendarView::readSettings()");

  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig config(locate("config", "korganizerrc")); 

  config.setGroup("General");

  QValueList<int> sizes = config.readIntListEntry("Separator1");
  if (sizes.count() == 2) {
    panner->setSizes(sizes);
  }

  sizes = config.readIntListEntry("Separator2");
  if (sizes.count() == 2) {
    leftFrame->setSizes(sizes);
  }

  // Set current view from Entry "Current View"
  readCurrentView();
}

void CalendarView::readCurrentView()
{
  QString str;
  KConfig config(locate("config", "korganizerrc")); 

  config.setGroup("General");
  str = config.readEntry("Current View");
  if (!str.isEmpty()) {
    if (str.compare("KOTodoView") == 0) currentView = 0;
    else {
      currentView = agendaView;
      KOBaseView *obj;
      for(obj=mCalendarViews.first();obj;obj=mCalendarViews.next()) {
        if (str.compare(obj->className()) == 0) currentView = obj;
      }
    }
  }

  config.setGroup("Views");
  agendaViewMode = config.readNumEntry("Agenda View", KOAgendaView::DAY);  
}

void CalendarView::writeSettings()
{
//  qDebug("CalendarView::writeSettings");

  KConfig config(locateLocal("config", "korganizerrc")); 

  config.setGroup("General");

  QValueList<int> list = panner->sizes();
  config.writeEntry("Separator1",list);

  list = leftFrame->sizes();
  config.writeEntry("Separator2",list);

  QString tmpStr;
  if (currentView) tmpStr = currentView->className();
  else tmpStr = "KOTodoView";  
  config.writeEntry("Current View", tmpStr);

  config.setGroup("Views");
  config.writeEntry("Agenda View", agendaView->currentView());

  KOPrefs::instance()->writeConfig();

  config.sync();  
}

void CalendarView::goToday()
{
  QDateList tmpList;
  QDate today(QDate::currentDate());
  tmpList.append(&today);
  dateNavigator->selectDates(tmpList);
  saveSingleDate = QDate::currentDate();
  updateView(dateNavigator->getSelected());
}

void CalendarView::goNext()
{
  // adapt this to work for other views
  agendaView->slotNextDates();
  // this *appears* to work fine...
  updateView(dateNavigator->getSelected());
}

void CalendarView::goPrevious()
{
  // adapt this to work for other views
  agendaView->slotPrevDates();
  // this *appears* to work fine...
  updateView(dateNavigator->getSelected());
}

void CalendarView::setupRollover()
{
  // right now, this is a single shot (because I am too lazy to code a
  // real one using a real qtimer object).  It will only work for a single
  // day rollover.  I should fix this. :)
  QDate tmpDate = QDate::currentDate().addDays(1);
  QTime tmpTime = QTime(00, 1);
  QDateTime tomorrow(tmpDate, tmpTime);

  QTimer::singleShot(QDateTime::currentDateTime().secsTo(tomorrow)*1000,
		     dateNavigator, SLOT(updateView()));
}


void CalendarView::hookupSignals()
{
  // SIGNAL/SLOTS FOR DATE SYNCHRO    
  connect(listView, SIGNAL(datesSelected(const QDateList)),
	  dateNavigator, SLOT(selectDates(const QDateList)));
  connect(agendaView, SIGNAL(datesSelected(const QDateList)),
	  dateNavigator, SLOT(selectDates(const QDateList)));
  connect(monthView, SIGNAL(datesSelected(const QDateList)),
	  dateNavigator, SLOT(selectDates(const QDateList)));    
  connect(dateNavigator, SIGNAL(datesSelected(const QDateList)),
	  this, SLOT(updateView(const QDateList)));

  connect(dateNavigator,SIGNAL(weekClicked(QDate)),SLOT(selectWeek(QDate)));

  connect(dateNavigator,SIGNAL(eventDropped(KOEvent *)),
          SLOT(eventAdded(KOEvent *)));    

  // SIGNALS/SLOTS FOR LIST VIEW
  connect(listView, SIGNAL(showEventSignal(KOEvent *)),
	  this, SLOT(showEvent(KOEvent *)));
  connect(listView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(listView, SIGNAL(deleteEventSignal(KOEvent *)), 
	  this, SLOT(deleteEvent(KOEvent *)));

  // SIGNALS/SLOTS FOR DAY/WEEK VIEW
  connect(agendaView,SIGNAL(newEventSignal(QDateTime)),
          this, SLOT(newEvent(QDateTime)));
  connect(agendaView,SIGNAL(newEventSignal(QDate)),
          this, SLOT(newEvent(QDate)));
//  connect(agendaView,SIGNAL(newEventSignal()),
//		this, SLOT(newEvent()));
  connect(agendaView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(agendaView, SIGNAL(showEventSignal(KOEvent *)),
	  this, SLOT(showEvent(KOEvent *)));
  connect(agendaView, SIGNAL(deleteEventSignal(KOEvent *)), 
	  this, SLOT(deleteEvent(KOEvent *)));

  // SIGNALS/SLOTS FOR MONTH VIEW
  connect(monthView, SIGNAL(showEventSignal(KOEvent *)),
	  this, SLOT(showEvent(KOEvent *)));
  connect(monthView, SIGNAL(newEventSignal(QDate)),
	  this, SLOT(newEvent(QDate)));
  connect(monthView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(monthView, SIGNAL(deleteEventSignal(KOEvent *)),
	  this, SLOT(deleteEvent(KOEvent *)));

  // SIGNALS/SLOTS FOR TODO VIEW
  connect(todoView, SIGNAL(newTodoSignal()),
	  this, SLOT(newTodo()));
  connect(todoView, SIGNAL(newSubTodoSignal(KOEvent *)),
	  this, SLOT(newSubTodo(KOEvent *)));
  connect(todoView, SIGNAL(showTodoSignal(KOEvent *)),
	  this, SLOT(showTodo(KOEvent *)));
  connect(todoView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(todoView, SIGNAL(deleteEventSignal(KOEvent *)),
          this, SLOT(deleteEvent(KOEvent *)));

  // SIGNALS/SLOTS FOR TODO LIST
  connect(todoList, SIGNAL(newTodoSignal()),
	  this, SLOT(newTodo()));
  connect(todoList, SIGNAL(newSubTodoSignal(KOEvent *)),
	  this, SLOT(newSubTodo(KOEvent *)));
  connect(todoList, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(todoList, SIGNAL(showTodoSignal(KOEvent *)),
	  this, SLOT(showTodo(KOEvent *)));
  connect(todoList, SIGNAL(deleteEventSignal(KOEvent *)),
          this, SLOT(deleteEvent(KOEvent *)));

  // CONFIGURATION SIGNALS/SLOTS
  // need to know about changed in configuration.
  connect(this, SIGNAL(configChanged()), mCalendar, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), agendaView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), monthView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), listView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), calPrinter, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), dateNavigator, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), todoView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), todoList, SLOT(updateConfig()));

  // MISC. SIGNALS/SLOTS
  connect(mCalendar, SIGNAL(calUpdated(KOEvent *)),
          this, SLOT(eventUpdated(KOEvent *)));
}

void CalendarView::updateConfig()
{
  qDebug("CalendarView::updateConfig()");
  emit configChanged();
}

void CalendarView::eventChanged(KOEvent *event)
{
  changeEventDisplay(event,EVENTEDITED);
}

void CalendarView::eventAdded(KOEvent *event)
{
  changeEventDisplay(event,EVENTADDED);
}

void CalendarView::eventToBeDeleted(KOEvent *)
{
  qDebug("CalendarView::eventToBeDeleted(): to be implemented");
}

void CalendarView::eventDeleted()
{
  changeEventDisplay(0,EVENTDELETED);
}

// most of the changeEventDisplays() right now just call the view's
// total update mode, but they SHOULD be recoded to be more refresh-efficient.
void CalendarView::changeEventDisplay(KOEvent *which, int action)
{
//  qDebug("CalendarView::changeEventDisplay");

  dateNavigator->updateView();
  if (searchDlg)
    searchDlg->updateView();

  if (which) {
    // If there is an event view visible update the display
    if (currentView) currentView->changeEventDisplay(which,action);
    if (which->getTodoStatus()) {
      if (!currentView) todoView->updateView();
      todoList->updateView();
    }
  } else {
    if (currentView) currentView->updateView();
  }
}

void CalendarView::updateTodoViews()
{
  todoList->updateView();
  if (!currentView) todoView->updateView();
  else currentView->updateView();
}

void CalendarView::changeAgendaView( int newView )
{
  if (newView == agendaView->currentView()) return;

  QPixmap px;
  
  switch( newView ) {
  case KOAgendaView::DAY: {
    QDateList tmpList(FALSE);
    tmpList = dateNavigator->getSelected();
    if (saveSingleDate != *tmpList.first()) {
      dateNavigator->selectDates(saveSingleDate);
      updateView(dateNavigator->getSelected());
    }    
    break;
  }
  // if its a workweek view, calculate the dates and emit
  case KOAgendaView::WORKWEEK:
    break;
    // if its a week view, calculate the dates and emit
  case KOAgendaView::WEEK:
    break;
    // if its a list view, update the list properties.
  case KOAgendaView::LIST:
    // we want to make sure that this is up to date.
    break;
  }
  agendaView->slotViewChange( newView );

  adaptNavigationUnits();
}

void CalendarView::nextAgendaView()
{
  int view;
  
  if( currentView == agendaView ) {
    view = agendaView->currentView() + 1;
    if ((view >= KOAgendaView::DAY) && ( view < KOAgendaView::LIST))
      changeAgendaView( view );
    else 
      changeAgendaView( KOAgendaView::DAY );
  } else {
    changeAgendaView( agendaView->currentView() );
    changeView( agendaView );
  }
}

void CalendarView::changeView(KOBaseView *view)
{
  if(view == currentView) return;

  currentView = view;

  if (currentView) rightFrame->raiseWidget(currentView);
  else rightFrame->raiseWidget(todoView);

  updateView(dateNavigator->getSelected());

  adaptNavigationUnits();
}

void CalendarView::updateView(const QDateList selectedDates)
{
  QDateList tmpList(false);
  tmpList = selectedDates;

  int numView;
  QPixmap px;

  // if there are 5 dates and the first is a monday, we have a workweek.
  if ((tmpList.count() == 5) &&
      (tmpList.first()->dayOfWeek() == 1) &&
      (tmpList.first()->daysTo(*tmpList.last()) == 4)) {
    numView = KOAgendaView::WORKWEEK;
  // if there are 7 dates and the first date is a monday, we have a regular week.
  } else if ((tmpList.count() == 7) &&
	   (tmpList.first()->dayOfWeek() ==
           (KGlobal::locale()->weekStartsMonday() ? 1 : 7)) &&
	   (tmpList.first()->daysTo(*tmpList.last()) == 6)) {
    numView = KOAgendaView::WEEK;

  } else if (tmpList.count() == 1) {
    numView = KOAgendaView::DAY;
    saveSingleDate = *tmpList.first();

  } else {
    // for sanity, set viewtype to LIST for now...
    numView = KOAgendaView::LIST;

  }

  if (currentView) currentView->selectDates(selectedDates);

  todoList->updateView();
  todoView->updateView();
}

void CalendarView::updateView()
{
  // update the current view with the current dates from the date navigator
  QDateList tmpList(FALSE); // we want a shallow copy
  tmpList = dateNavigator->getSelected();

  // if no dates are supplied, we should refresh the dateNavigator too...
  dateNavigator->updateView();
  updateView(tmpList);
}

  
int CalendarView::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(this,
      i18n("This item will be permanently deleted."),
      i18n("KOrganizer Confirmation"),i18n("Delete"));
}


void CalendarView::edit_cut()
{
  KOEvent *anEvent=0;

  if (currentView) anEvent = (currentView->getSelected()).first();

  if (!anEvent) {
    qApp->beep();
    return;
  }
  mCalendar->cutEvent(anEvent);
  changeEventDisplay(anEvent, EVENTDELETED);
}

void CalendarView::edit_copy()
{
  KOEvent *anEvent=0;

  if (currentView) anEvent = (currentView->getSelected()).first();
  
  if (!anEvent) {
    qApp->beep();
    return;
  }
  mCalendar->copyEvent(anEvent);
}

void CalendarView::edit_paste()
{
  KOEvent *pastedEvent;
  QDateList tmpList(FALSE);

  tmpList = dateNavigator->getSelected();
  pastedEvent = mCalendar->pasteEvent(tmpList.first());
  changeEventDisplay(pastedEvent, EVENTADDED);
}

void CalendarView::edit_options()
{
  mOptionsDialog->readConfig();
  mOptionsDialog->show();
}


void CalendarView::newEvent()
{
  qDebug("CalendarView::newEvent()");
  newEvent(QDate::currentDate());
}

void CalendarView::newEvent(QDateTime fh)
{
  newEvent(fh,
           QDateTime(fh.addSecs(3600*KOPrefs::instance()->mDefaultDuration)));
}

void CalendarView::newEvent(QDate dt)
{
  newEvent(QDateTime(dt, QTime(0,0,0)),
           QDateTime(dt, QTime(0,0,0)), TRUE);
}

void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint)
{
  // create empty event win
  KOEventEditor *eventWin = new KOEventEditor(mCalendar);

  // put in date hint
  eventWin->newEvent(fromHint,toHint);

  // connect the win for changed events
  connect(eventWin,SIGNAL(eventAdded(KOEvent *)),SLOT(eventAdded(KOEvent *)));
  connect(eventWin,SIGNAL(categoryConfigChanged()),
          mOptionsDialog,SLOT(updateCategories()));

  connect(this,SIGNAL(closingDown()),eventWin,SLOT(reject()));

  // show win
  eventWin->show();
}

void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint, bool allDay)
{
  // create empty event win
  KOEventEditor *eventWin = new KOEventEditor(mCalendar);

  // put in date hint
  eventWin->newEvent(fromHint,toHint,allDay);

  // connect the win for changed events
  connect(eventWin,SIGNAL(eventAdded(KOEvent *)),SLOT(eventAdded(KOEvent *)));
  connect(eventWin,SIGNAL(categoryConfigChanged()),
          mOptionsDialog,SLOT(updateCategories()));

  connect(this,SIGNAL(closingDown()),eventWin,SLOT(reject()));

  // show win
  eventWin->show();
}

void CalendarView::newTodo()
{
  KOTodoEditor *todoWin = new KOTodoEditor( mCalendar );
  todoWin->newTodo(QDateTime::currentDateTime().addDays(7),0,true);

  // connect the win for changed events
  connect(todoWin,SIGNAL(todoAdded(KOEvent *)),SLOT(updateTodoViews()));
  connect(todoWin,SIGNAL(categoryConfigChanged()),
          mOptionsDialog,SLOT(updateCategories()));

  connect(this, SIGNAL(closingDown()),
	  todoWin, SLOT(reject()));

  // show win
  todoWin->show();
}

void CalendarView::newSubTodo(KOEvent *parentEvent)
{
  KOTodoEditor *todoWin = new KOTodoEditor( mCalendar );
  todoWin->newTodo(QDateTime::currentDateTime().addDays(7),parentEvent,true);

  // connect the win for changed events
  connect(todoWin,SIGNAL(todoAdded(KOEvent *)),SLOT(updateTodoViews()));
  connect(todoWin,SIGNAL(categoryConfigChanged()),
          mOptionsDialog,SLOT(updateCategories()));

  connect(this, SIGNAL(closingDown()),
	  todoWin, SLOT(reject()));

  // show win
  todoWin->show();
}

void CalendarView::appointment_new()
{
  QDate from, to;

  QDateList tmpList(FALSE);
  tmpList = dateNavigator->getSelected();
  from = *tmpList.first();
  to = *tmpList.last();
  
  ASSERT(from.isValid());
  if (!from.isValid()) { // dateNavigator sometimes returns GARBAGE!
    from = QDate::currentDate();
    to = from;
  }

  qDebug("StartTime: %d",KOPrefs::instance()->mStartTime);

  newEvent(QDateTime(from, QTime(KOPrefs::instance()->mStartTime,0,0)),
	   QDateTime(to, QTime(KOPrefs::instance()->mStartTime +
                     KOPrefs::instance()->mDefaultDuration,0,0)));
}

void CalendarView::allday_new()
{

  QDate from, to;
  QDateList tmpList(FALSE);
  tmpList = dateNavigator->getSelected();

  from = *tmpList.first();
  to = *tmpList.last();

  ASSERT(from.isValid());
  if (!from.isValid()) {
    from = QDate::currentDate();
    to = from;
  }
  
  newEvent(QDateTime(from, QTime(12,0,0)),
           QDateTime(to, QTime(12,0,0)), TRUE);
}

void CalendarView::editEvent(KOEvent *anEvent)
{
  QDateList tmpList(FALSE);
  QDate qd;

  tmpList = dateNavigator->getSelected();
  if(anEvent) {
    qd = *tmpList.first();
    if (anEvent->getTodoStatus()) {
      // this is a todo
      KOTodoEditor *eventWin = new KOTodoEditor(mCalendar );
      eventWin->editTodo(anEvent, qd);
      // connect for changed events
      connect(eventWin,SIGNAL(todoChanged(KOEvent *)),
              SLOT(updateTodoViews()));
      connect(eventWin,SIGNAL(todoDeleted()),
              SLOT(updateTodoViews()));
      connect(eventWin,SIGNAL(categoryConfigChanged()),
              mOptionsDialog,SLOT(updateCategories()));
      connect(this, SIGNAL(closingDown()),
              eventWin, SLOT(reject()));
      eventWin->show();
    } else { // this is an event
      KOEventEditor *eventWin = new KOEventEditor(mCalendar );
      eventWin->editEvent(anEvent, qd);
      // connect the win for changed events
      connect(eventWin,SIGNAL(eventChanged(KOEvent *)),
              SLOT(eventChanged(KOEvent *)));
      connect(eventWin,SIGNAL(eventDeleted()),
              SLOT(eventDeleted()));
      connect(eventWin,SIGNAL(categoryConfigChanged()),
              mOptionsDialog,SLOT(updateCategories()));
      connect(this, SIGNAL(closingDown()),
              eventWin, SLOT(reject()));
      eventWin->show();
    }
  } else {
    qApp->beep();
  }
}

void CalendarView::showEvent(KOEvent *event)
{
  if (event->getTodoStatus()) showTodo(event);
  else {
    KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
    eventViewer->setEvent(event);
    eventViewer->show();
  }
}

void CalendarView::showTodo(KOEvent *event)
{
  if (!event->getTodoStatus()) showEvent(event);
  else {
    KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
    eventViewer->setTodo(event);
    eventViewer->show();
  }
}

void CalendarView::appointment_edit()
{
  KOEvent *anEvent = 0;
  
  if (currentView) anEvent = (currentView->getSelected()).first();

  if (!anEvent) {
    qApp->beep();
    return;
  }

  editEvent(anEvent);
}

void CalendarView::appointment_delete()
{
  KOEvent *anEvent = 0;

  if (currentView) anEvent = (currentView->getSelected()).first();

  if (!anEvent) {
    qApp->beep();
    return;
  }

  deleteEvent(anEvent);
}

// Is this function needed anymore? It could just call the deleteTodo slot of
// the KOTodoView, couldn't it?
void CalendarView::action_deleteTodo()
{
  KOEvent *aTodo;
  KOTodoView *todoList2 = (currentView ? todoList : todoView); 
//  TodoView *todoList2 = (viewMode == TODOVIEW ? todoView : todoList); 

  aTodo = todoList2->getSelected();
  if (!aTodo) {
    qApp->beep();
    return;
  }
  
  // disable deletion for now, because it causes a crash.
  return;
  
  if (KOPrefs::instance()->mConfirm) {
    switch(msgItemDelete()) {
      case KMessageBox::Continue: // OK
        mCalendar->deleteTodo(aTodo);
        // If there would be a removeTodo() function in KOTodoView we would call
        // it here... (before the mCalendar->deleteTodo call actually)
        todoList2->updateView();
        break;
    } // switch
  } else {
    mCalendar->deleteTodo(aTodo);
    // If there would be a removeTodo() function in KOTodoView we would call
    // it here... (before the mCalendar->deleteTodo call actually)
    todoList2->updateView();
  }
}

void CalendarView::deleteEvent(KOEvent *anEvent)
{
  if (!anEvent) {
    qApp->beep();
    return;
  }

  // At the moment we don't handle recurrence for todos
  if (!anEvent->getTodoStatus() && anEvent->doesRecur()) {

  switch(KMessageBox::warningYesNoCancel(this,
				i18n("This event recurs over multiple dates.\n"
				     "Are you sure you want to delete the\n"
				     "selected event, or just this instance?\n"),
				i18n("KOrganizer Confirmation"),
				i18n("&All"), i18n("&This"))) {

    case KMessageBox::Yes: // all
      mCalendar->deleteEvent(anEvent);
      changeEventDisplay(anEvent, EVENTDELETED);
      break;

    case KMessageBox::No: // just this one
      {
        QDate qd;
        QDateList tmpList(FALSE);
        tmpList = dateNavigator->getSelected();
        qd = *(tmpList.first());
        if (!qd.isValid()) {
          debug("no date selected, or invalid date");
          qApp->beep();
          return;
        }
        while (!anEvent->recursOn(qd))
          qd.addDays(1);
        anEvent->addExDate(qd);
        changeEventDisplay(anEvent, EVENTDELETED);
        break;
      }

    } // switch
  } else {
    if (KOPrefs::instance()->mConfirm) {
      switch (msgItemDelete()) {
        case KMessageBox::Continue: // OK
          if (anEvent->getTodoStatus()) {
            mCalendar->deleteTodo(anEvent);
          } else {
            mCalendar->deleteEvent(anEvent);
          }
          changeEventDisplay(anEvent, EVENTDELETED);
          break;
      } // switch
    } else {
      mCalendar->deleteEvent(anEvent);
      changeEventDisplay(anEvent, EVENTDELETED);
    }
  } // if-else
}

/*****************************************************************************/

void CalendarView::action_search()
{
  if (!searchDlg) {
    searchDlg = new SearchDialog(mCalendar);
    connect(searchDlg,SIGNAL(showEventSignal(KOEvent *)),
	    SLOT(showEvent(KOEvent *)));
    connect(searchDlg,SIGNAL(editEventSignal(KOEvent *)),
	    SLOT(editEvent(KOEvent *)));
    connect(searchDlg,SIGNAL(deleteEventSignal(KOEvent *)), 
	    SLOT(deleteEvent(KOEvent *)));
    connect(this,SIGNAL(closingDown()),searchDlg,SLOT(reject()));
  }
  // make sure the widget is on top again
  searchDlg->show();
  searchDlg->raise();
}

void CalendarView::action_mail()
{
  KOMailClient mailobject;

  KOEvent *anEvent = 0;
  if (currentView) anEvent = (currentView->getSelected()).first();

  if (!anEvent) {
    KMessageBox::sorry(this,i18n("Can't generate mail:\nNo event selected."));
    return;
  }
  if(anEvent->attendeeCount() == 0 ) {
    KMessageBox::sorry(this,
                       i18n("Can't generate mail:\nNo attendees defined.\n"));
    return;
  }

  mailobject.emailEvent(anEvent);
}


void CalendarView::view_list()
{
  changeView(listView);
}

void CalendarView::view_day()
{
  changeView(agendaView);
  changeAgendaView(KOAgendaView::DAY);
}

void CalendarView::view_workweek()
{
  changeView(agendaView);
  changeAgendaView(KOAgendaView::WORKWEEK);
}

void CalendarView::view_week()
{
  changeView(agendaView);
  changeAgendaView(KOAgendaView::WEEK);
}

void CalendarView::view_month()
{
  changeView(monthView);
}

void CalendarView::view_todolist()
{
  changeView(0);
}

void CalendarView::setModified(bool modified)
{
  if (mModified != modified) {
    mModified = modified;
    emit modifiedChanged(mModified);
  }
}

bool CalendarView::isReadOnly()
{
  return mReadOnly;
}

void CalendarView::setReadOnly(bool readOnly)
{
  if (mReadOnly != readOnly) {
    mReadOnly = readOnly;
    emit readOnlyChanged(mReadOnly);
  }
}

bool CalendarView::isModified()
{
  return mModified;
}

void CalendarView::signalAlarmDaemon()
{
  QFile pidFile;
  QString tmpStr;
  pid_t pid;
  char pidStr[25];

  tmpStr = locateLocal("appdata", "alarm.pid");

  pidFile.setName(tmpStr.data());

  // only necessary if the file actually is opened
  if(pidFile.open(IO_ReadOnly)) {
    pidFile.readLine(pidStr, 24);
    pidFile.close();
    pid = atoi(pidStr);
    if (pid > 0)
      kill(pid, SIGHUP);
  }
}

void CalendarView::printSetup()
{
  calPrinter->setupPrinter();
}

void CalendarView::print() 
{
  QDateList tmpDateList(FALSE);

  tmpDateList = dateNavigator->getSelected();
  calPrinter->print(CalPrinter::Month,
		    *tmpDateList.first(), *tmpDateList.last());
}

void CalendarView::printPreview()
{
  qDebug("CalendarView::printPreview()");
  
  QDateList tmpDateList(FALSE);

  tmpDateList = dateNavigator->getSelected();

  if (currentView) currentView->printPreview(calPrinter,*tmpDateList.first(),
                                             *tmpDateList.last());
  else calPrinter->preview(CalPrinter::Todo,
                           *tmpDateList.first(), *tmpDateList.last());
}

void CalendarView::exportWeb()
{
  mExportWebDialog->show();
  mExportWebDialog->raise();
}

void CalendarView::eventUpdated(KOEvent *)
{
  setModified();
}

void CalendarView::selectWeek(QDate weekstart)
{
//  qDebug("CalendarView::selectWeek(): %s",weekstart.toString().latin1());

  QDateList week;

  // Determine number of days for a week. If current view is work week, than
  // n is 5. This does not work at the moment.
  int n = 7;
  if (currentView) {
    // maxDatesHint is not the correct function to find out the number of dates
    // of a view.
    if (currentView->currentDateCount() == 5) n = 5;
  }

  int i;
  for(i=0;i<n;++i) {
    QDate date = weekstart.addDays(i);
    week.append(&date);
  }
  dateNavigator->selectDates(week);
  updateView(week);
}

void CalendarView::adaptNavigationUnits()
{
  if(currentView) {
    int days = currentView->currentDateCount();
    if (days == 1) {
      emit changeNavStringPrev(i18n("&Previous Day"));
      emit changeNavStringNext(i18n("&Next Day"));
    } else {
      emit changeNavStringPrev(i18n("&Previous Week"));
      emit changeNavStringNext(i18n("&Next Week"));
    }
  }
}
