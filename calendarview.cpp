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

#include <sys/types.h>
#include <signal.h>

#include <qfiledlg.h>
#include <qcursor.h>
#include <qmlined.h>
#include <qmsgbox.h>
#include <qtimer.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kstdaccel.h>
#include <kfiledialog.h>

#include "misc.h"
#include "version.h"
#include "koarchivedlg.h"
#include "komailclient.h"
#include "calprinter.h"
#include "aboutdlg.h"
#include "exportwebdialog.h"

#include "calendarview.h"
#include "calendarview.moc"	

#define AGENDABUTTON 0x10
#define NOACCEL 0

CalendarView::CalendarView(QString filename, QWidget *parent, const char *name ) 
  : QWidget( parent, name )
{
  qDebug("CalendarView::CalendarView()");

  currentView = 0;
  todoView = 0;
  agendaView = 0;
  monthView = 0;
  listView = 0;

  mModified=false;
  
  searchDlg = 0L;
//  setMinimumSize(600,400);	// make sure we don't get resized too small...

  // Create calendar object, which contains all calendar information associated
  // with this calendar view window.
  mCalendar = new CalObject;

  mCalendar->setTopwidget(this);

  QBoxLayout *topLayout = new QVBoxLayout(this);

  // create the main layout frames.
  panner = new QSplitter(QSplitter::Horizontal, this, "CalendarView::Panner");
  topLayout->addWidget(panner);

  leftFrame = new QFrame(panner, "CalendarView::LeftFrame");
  rightFrame = new QWidgetStack(panner, "CalendarView::RightFrame");

  optionsDlg = new OptionsDialog("KOrganizer Configuration Options",
				 this);
  connect(optionsDlg, SIGNAL(configChanged()),
	  this, SLOT(updateConfig()));

  QVBoxLayout *layoutLeftFrame = new QVBoxLayout(leftFrame, 1, -1,
						 "CalendarView::layoutLeftFrame");
  dateNavFrame = new QFrame(leftFrame, "CalendarView::DateNavFrame");
  dateNavFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);

  dateNavigator = new KDateNavigator(dateNavFrame, mCalendar, TRUE,
                        "CalendarView::DateNavigator", QDate::currentDate());
  dateNavigator->move(2,2);
  dateNavigator->resize(160, 150);

  dateNavFrame->resize(dateNavigator->width()+4, dateNavigator->height()+4);
  dateNavFrame->setMinimumSize(dateNavFrame->size());
  dateNavFrame->setFixedHeight(dateNavigator->height());
  
  layoutLeftFrame->addWidget(dateNavFrame);
  layoutLeftFrame->addSpacing(5);

  if (!filename.isEmpty()) initCalendar(filename);

  // create the main data display views.
  todoList   = new KOTodoView(mCalendar, leftFrame, "CalendarView::TodoList");
  layoutLeftFrame->addWidget(todoList);

  todoView   = new KOTodoView(mCalendar, rightFrame, "CalendarView::TodoView");
  rightFrame->addWidget(todoView,0);

  agendaView = new KOAgendaView(mCalendar, rightFrame, "CalendarView::AgendaView");
  rightFrame->addWidget(agendaView,1);//  layout2->addWidget(agendaView);
  mCalendarViews.append(agendaView);

  listView   = new KOListView(mCalendar, rightFrame, "CalendarView::ListView");
  rightFrame->addWidget(listView,2);//  layout3->addWidget(listView);
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
  // setup toolbar, menubar and status bar, NOTE: this must be done
  // after the widget creation, because setting the menubar, toolbar
  // or statusbar will generate a call to updateRects, which assumes
  // that all of them are around.

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


bool CalendarView::setFile(QString filename)
{
  if (initCalendar(filename)) {
    setModified(false);
    updateView();
    return true;
  } else {
    return false;
  }
}


bool CalendarView::mergeFile(QString filename)
{
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
  mCalendar->save(filename);
  
  // We should check for errors here.

  setModified(false);
  
  return true;
}

void CalendarView::closeCalendar()
{
  mCalendar->close();
  fileName = "";
  setModified(false);
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
      // have become partially populated.  Clear it out.
      mCalendar->close();
      fileName = "";
      return false;
    }
  }

  KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 
  config.setGroup("General");
  config.writeEntry("Current Calendar (2.0)", fileName);

  QApplication::restoreOverrideCursor();
  
  return true;
}


void CalendarView::readSettings()
{
//  qDebug("CalendarView::readSettings()");

  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 

  config.setGroup("General");

  QStringList strlist = config.readListEntry("Separator");
  if (!strlist.isEmpty()){
    QValueList<int> sizes;
    QStringList::Iterator it;
    for( it = strlist.begin(); it != strlist.end(); ++it ) {
      sizes.append((*it).toInt());
    }
    panner->setSizes(sizes);
  }

  // Set current view from Entry "Current View"
  readCurrentView();

  config.setGroup("Views");
  agendaViewMode = config.readNumEntry("Agenda View", KOAgendaView::DAY);

  config.setGroup( "Colors" );
  if( config.readBoolEntry( "DefaultColors", TRUE ) == TRUE )
  {
    optionsDlg->setColorDefaults();
    optionsDlg->applyColorDefaults();
  }

  config.sync();
}

void CalendarView::readCurrentView()
{
  QString str;
  KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 

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
}

void CalendarView::writeSettings()
{
//  qDebug("CalendarView::writeSettings");

  KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 

  QString tmpStr;
  config.setGroup("General");

  QStringList strlist;
  QValueList<int> list = panner->sizes();
  QValueList<int>::Iterator it;
  for( it = list.begin(); it != list.end(); ++it ) {
    strlist.append(QString::number(*it));
  }
  config.writeEntry("Separator", strlist);

  if (currentView) tmpStr = currentView->className();
  else tmpStr = "KOTodoView";  
  config.writeEntry("Current View", tmpStr);

  config.setGroup("Views");
  config.writeEntry("Agenda View", agendaView->currentView());

  config.sync();
}

void CalendarView::goToday()
{
  dateNavigator->selectDates(QDate::currentDate());
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
    

  // SIGNALS/SLOTS FOR LIST VIEW
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
  connect(agendaView, SIGNAL(deleteEventSignal(KOEvent *)), 
	  this, SLOT(deleteEvent(KOEvent *)));

  // SIGNALS/SLOTS FOR MONTH VIEW
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
  emit configChanged();
}

// most of the changeEventDisplays() right now just call the view's
// total update mode, but they SHOULD be recoded to be more refresh-efficient.
void CalendarView::changeEventDisplay(KOEvent *which, int action)
{
  dateNavigator->updateView();
  if (searchDlg)
    searchDlg->updateView();

  // If there is an event view visible update the display
  if (currentView) currentView->changeEventDisplay(which,action);
  if (which->getTodoStatus()) {
    if (!currentView) todoView->changeEventDisplay(which,action);
    todoList->changeEventDisplay(which,action);
  }
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
}

void CalendarView::updateView(const QDateList selectedDates)
{
  QDateList tmpList(FALSE);
  tmpList = selectedDates;
  int numView;
  KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 
  config.setGroup("Time & Date");
  bool weekStartsMonday = config.readBoolEntry("Week Starts Monday", TRUE);
  QPixmap px;

  // if there are 5 dates and the first is a monday, we have a workweek.
  if ((tmpList.count() == 5) &&
      (tmpList.first()->dayOfWeek() == 1) &&
      (tmpList.first()->daysTo(*tmpList.last()) == 4)) {
    numView = KOAgendaView::WORKWEEK;
  // if there are 7 dates and the first date is a monday, we have a regular week.
  } else if ((tmpList.count() == 7) &&
	   (tmpList.first()->dayOfWeek() == (weekStartsMonday ? 1 : 7)) &&
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

  
int CalendarView::msgCalModified()
{
  // returns:
  //  0: "Yes"
  //  1: "No"
  //  2: "Cancel"
  return QMessageBox::warning(this,
			      i18n("KOrganizer Warning"),
			      i18n("This calendar has been modified.\n"
				   "Would you like to save it?"),
			      i18n("&Yes"), i18n("&No"), i18n("&Cancel"));
}

int CalendarView::msgItemDelete()
{
  // returns:
  //  0: "OK"
  //  1: "Cancel"
  return QMessageBox::warning(this,
			      i18n("KOrganizer Confirmation"),
			      i18n("This item will be permanently deleted."),
			      i18n("&OK"), i18n("&Cancel"));
}


void CalendarView::edit_cut()
{
  KOEvent *anEvent;

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
  KOEvent *anEvent;

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
  // Don´t know why this is necessary, but the options dialog doesn´t show
  // correctly, when called for the first time, unless it is resized here.
  // This is a workaround, the dialog should be fixed.
  optionsDlg->resize(600,460);

  optionsDlg->show();
}


/****************************************************************************/
void CalendarView::newEvent()
{
  qDebug("CalendarView::newEvent()");
//  QDate date;
//  date = QDate::currentDate();
  newEvent(QDate::currentDate());
}
void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint)
{
  // create empty event win
  EditEventWin *eventWin = new EditEventWin( mCalendar );

  // put in date hint
  eventWin->newEvent(fromHint, toHint);


  // connect the win for changed events
  connect(eventWin, SIGNAL(eventChanged(KOEvent *, int)), 
  	  SLOT(changeEventDisplay(KOEvent *, int)));
  connect(this, SIGNAL(closingDown()),
	  eventWin, SLOT(cancel()));


  // show win
  eventWin->show();
}

void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint, bool allDay)
{
  // create empty event win
  EditEventWin *eventWin = new EditEventWin( mCalendar );

  // put in date hint
  eventWin->newEvent(fromHint, toHint, allDay);

  // connect the win for changed events
  connect(eventWin, SIGNAL(eventChanged(KOEvent *, int)), 
  	  SLOT(changeEventDisplay(KOEvent *, int)));
  connect(this, SIGNAL(closingDown()),
	  eventWin, SLOT(cancel()));

  // show win
  eventWin->show();
}

void CalendarView::newTodo()
{
  TodoEventWin *todoWin = new TodoEventWin( mCalendar );
  todoWin->newEvent(QDateTime::currentDateTime().addDays(7),0,true);

  // connect the win for changed events
  connect(todoWin, SIGNAL(eventChanged(KOEvent *, int)), 
  	  SLOT(changeEventDisplay(KOEvent *, int)));
  connect(this, SIGNAL(closingDown()),
	  todoWin, SLOT(cancel()));

  // show win
  todoWin->show();
}

void CalendarView::newSubTodo(KOEvent *parentEvent)
{
  TodoEventWin *todoWin = new TodoEventWin( mCalendar );
  todoWin->newEvent(QDateTime::currentDateTime().addDays(7),parentEvent,true);

  // connect the win for changed events
  connect(todoWin, SIGNAL(eventChanged(KOEvent *, int)), 
  	  SLOT(changeEventDisplay(KOEvent *, int)));
  connect(this, SIGNAL(closingDown()),
	  todoWin, SLOT(cancel()));

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

  KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 
  config.setGroup("Time & Date");
  QString confStr(config.readEntry("Default Start Time", "12:00"));
  int pos = confStr.find(':');
  if (pos >= 0)
    confStr.truncate(pos);
  int fmt = confStr.toUInt();

  newEvent(QDateTime(from, QTime(fmt,0,0)),
	   QDateTime(to, QTime(fmt+1,0,0)));
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
  if(anEvent!=0)    {
      qd = *tmpList.first();
        EventWin *eventWin;
        if (anEvent->getTodoStatus()) {
            // this is a todo
            eventWin = new TodoEventWin(mCalendar );
            eventWin->editEvent(anEvent, qd);
            connect(eventWin, SIGNAL(eventChanged(KOEvent *, int)),
                    todoList, SLOT(updateView()));
            connect(eventWin, SIGNAL(eventChanged(KOEvent *, int)),
                    todoView, SLOT(updateView()));
        } else { // this is an event
            eventWin = new EditEventWin(mCalendar );
            eventWin->editEvent(anEvent, qd);
        }
        // connect for changed events, common for todos and events
        connect(eventWin, SIGNAL(eventChanged(KOEvent *, int)),
                SLOT(changeEventDisplay(KOEvent *, int)));
        connect(this, SIGNAL(closingDown()),
                eventWin, SLOT(cancel()));

        eventWin->show();
    } else {
        qApp->beep();
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
  
  KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 
  config.setGroup("General");
  if (config.readBoolEntry("Confirm Deletes") == TRUE) {
    switch(msgItemDelete()) {
    case 0: // OK
      mCalendar->deleteTodo(aTodo);
      todoList2->changeEventDisplay(aTodo, EVENTDELETED);
      break;

    } // switch
  } else {
    mCalendar->deleteTodo(aTodo);
    todoList2->changeEventDisplay(aTodo, EVENTDELETED);
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

  switch ( QMessageBox::warning(this,
				i18n("KOrganizer Confirmation"),
				i18n("This event recurs over multiple dates.\n"
				     "Are you sure you want to delete the\n"
				     "selected event, or just this instance?\n"),
				i18n("&All"), i18n("&This"),
				i18n("&Cancel"))) {

    case 0: // all
      mCalendar->deleteEvent(anEvent);
      changeEventDisplay(anEvent, EVENTDELETED);
      break;

    case 1: // just this one
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
    KConfig config(KGlobal::dirs()->findResource("config", "korganizerrc")); 
    config.setGroup("General");
    if (config.readBoolEntry("Confirm Deletes") == TRUE) {
      switch (msgItemDelete()) {
      case 0: // OK
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
    connect(searchDlg, SIGNAL(closed(QWidget *)),
	    this, SLOT(cleanWindow(QWidget *)));
    connect(searchDlg, SIGNAL(editEventSignal(KOEvent *)),
	    this, SLOT(editEvent(KOEvent *)));
    connect(searchDlg, SIGNAL(deleteEventSignal(KOEvent *)), 
	    this, SLOT(deleteEvent(KOEvent *)));
    connect(this, SIGNAL(closingDown()),
	      searchDlg, SLOT(cancel()));
  }
  // make sure the widget is on top again
  searchDlg->hide();
  searchDlg->show();
  
}

void CalendarView::action_mail()
{
  KOEvent *anEvent;
  KoMailClient mailobject(mCalendar);

  if (currentView) anEvent = (currentView->getSelected()).first();

  if (!anEvent) {
    qApp->beep();
    return;
  }
  if(anEvent->attendeeCount() == 0 ) {
    qApp->beep();
    QMessageBox::warning(this,i18n("KOrganizer error"),
			 i18n("Can't generate mail:\n No attendees defined!\n"));
    return;
  }
  mailobject.emailEvent(anEvent,this);
}


void CalendarView::help_contents()
{
  kapp->invokeHTMLHelp("korganizer/korganizer.html","");
}

void CalendarView::help_about()
{
  AboutDialog *ad = new AboutDialog(this, "AboutDialog");
  ad->show();
  delete ad;
}

void CalendarView::help_postcard()
{
  PostcardDialog *pcd = new PostcardDialog(this, "PostcardDialog");
  pcd->show();
  delete pcd;
}

void CalendarView::view_list()
{
  changeView(listView);
}

void CalendarView::view_day()
{
  QDateList tmpList(FALSE);
  tmpList = dateNavigator->getSelected();
  if (saveSingleDate != *tmpList.first()) {
    dateNavigator->selectDates(saveSingleDate);
    updateView(dateNavigator->getSelected());
  }

  agendaView->slotViewChange( KOAgendaView::DAY );
  changeView(agendaView);
}

void CalendarView::view_workweek()
{
  agendaView->slotViewChange(KOAgendaView::WORKWEEK);
  changeView(agendaView);
}

void CalendarView::view_week()
{
  agendaView->slotViewChange(KOAgendaView::WEEK );
  changeView(agendaView);
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
  else calPrinter->print(CalPrinter::Todo,
                         *tmpDateList.first(), *tmpDateList.last());

/*
  switch(viewMode) {
  case AGENDAVIEW:
    if (tmpDateList.count() == 1)
      calPrinter->preview(CalPrinter::Day,
			  *tmpDateList.first(), *tmpDateList.last());
    else if (tmpDateList.count() <= 7)
      calPrinter->preview(CalPrinter::Week,
			  *tmpDateList.first(), *tmpDateList.last());
    else
      calPrinter->preview(CalPrinter::Month,
			  *tmpDateList.first(), *tmpDateList.last());
    break;
  case LISTVIEW:
    calPrinter->preview(CalPrinter::Day,
			*tmpDateList.first(), *tmpDateList.last());
    break;
  case MONTHVIEW:
  default:
    calPrinter->preview(CalPrinter::Month, 
			*tmpDateList.first(), *tmpDateList.last());
    break;
  }
*/
}

void CalendarView::exportWeb()
{
  mExportWebDialog->show();
  mExportWebDialog->raise();
}

void CalendarView::cleanWindow(QWidget *widget)
{
  widget->hide();

  // some widgets are stored in CalendarView
  if (widget == searchDlg)
    searchDlg = NULL;
}

void CalendarView::eventUpdated(KOEvent *)
{
  setModified();
}
