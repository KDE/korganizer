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
#include <qclipboard.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <knotifyclient.h>

#include "koarchivedlg.h"
#include "komailclient.h"
#include "calprinter.h"
#include "exportwebdialog.h"
#include "koprefsdialog.h"
#include "koeventeditor.h"
#include "kotodoeditor.h"
#include "koprefs.h"
#include "koeventviewerdialog.h"
#include "vcaldrag.h"
#include "icalformat.h"
#include "vcalformat.h"
#include "outgoingdialog.h"
#include "incomingdialog.h"
#include "scheduler.h"
#include "calendarlocal.h"
#include "categoryeditdialog.h"
#include "kofilterview.h"
#include "calfilter.h"
#include "koprojectview.h"

#include "calendarview.h"
#include "calendarview.moc"


CalendarView::CalendarView(QWidget *parent,const char *name)
  : QWidget(parent,name)
{
  kdDebug() << "CalendarView::CalendarView()" << endl;

  mCurrentView = 0;
  mTodoView = 0;
  mAgendaView = 0;
  mMonthView = 0;
  mListView = 0;

  mModified=false;
  mReadOnly = false;
  mEventsSelected = true;

  mSearchDialog = 0L;
  mArchiveDialog = 0;

//  setMinimumSize(620,400);	// make sure we don't get resized too small...

  // Create calendar object, which manages all calendar information associated
  // with this calendar view window.
  mCalendar = new CalendarLocal;

  mCalendar->setTopwidget(this);

  mOutgoingDialog = new OutgoingDialog(mCalendar,this);
  connect(mOutgoingDialog,SIGNAL(numMessagesChanged(int)),
          SIGNAL(numOutgoingChanged(int)));
  mIncomingDialog = new IncomingDialog(mCalendar,this);
  connect(mIncomingDialog,SIGNAL(numMessagesChanged(int)),
          SIGNAL(numIncomingChanged(int)));

  mCategoryEditDialog = new CategoryEditDialog();

  QBoxLayout *topLayout = new QVBoxLayout(this);

  // create the main layout frames.
  mPanner = new QSplitter(QSplitter::Horizontal,this,"CalendarView::Panner");
  topLayout->addWidget(mPanner);

  mLeftFrame = new QSplitter(QSplitter::Vertical,mPanner,
                            "CalendarView::LeftFrame");
  mRightFrame = new QWidgetStack(mPanner, "CalendarView::RightFrame");

  mOptionsDialog = new KOPrefsDialog(this);
  mOptionsDialog->readConfig();
  connect(mOptionsDialog,SIGNAL(configChanged()),SLOT(updateConfig()));
  connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
          mOptionsDialog,SLOT(updateCategories()));

  mDateNavigator = new KDateNavigator(mLeftFrame, mCalendar, TRUE,
                        "CalendarView::DateNavigator", QDate::currentDate());

//  if (!filename.isEmpty()) initCalendar(filename);

  mTodoList   = new KOTodoView(mCalendar, mLeftFrame, "CalendarView::TodoList");

  CalFilter *filter = new CalFilter;
  mCalendar->setFilter(filter);
  mFilterView = new KOFilterView(filter,mLeftFrame,"CalendarView::FilterView");
  connect(mFilterView,SIGNAL(filterChanged()),SLOT(updateView()));
  // Hide filter per default
  mFilterView->hide();

  // create the main data display views.
  mTodoView   = new KOTodoView(mCalendar, mRightFrame, "CalendarView::TodoView");
  mRightFrame->addWidget(mTodoView,0);
  mCalendarViews.append(mTodoView);

  mAgendaView = new KOAgendaView(mCalendar, mRightFrame, "CalendarView::AgendaView");
  mRightFrame->addWidget(mAgendaView,1);
  mCalendarViews.append(mAgendaView);

  mListView   = new KOListView(mCalendar, mRightFrame, "CalendarView::ListView");
  mRightFrame->addWidget(mListView,2);
  mCalendarViews.append(mListView);

  mMonthView = new KOMonthView(mCalendar, mRightFrame, "CalendarView::MonthView");
  mRightFrame->addWidget(mMonthView,3);
  mCalendarViews.append(mMonthView);

  mProjectView = new KOProjectView(mCalendar,mRightFrame,
                                   "CalendarView::ProjectView");
  mRightFrame->addWidget(mProjectView,0);
  mCalendarViews.append(mProjectView);

  readCurrentView();

/*
  // List classnames of available views
  QObject *obj;
  for(obj=mCalendarViews.first();obj;obj=mCalendarViews.next())
    kdDebug() << "calViews: " << obj->className() << endl;
*/

  // set up printing object
  mCalPrinter = new CalPrinter(this, mCalendar);

  // set up web exporting object
  mExportWebDialog = new ExportWebDialog(mCalendar);

  // hook up the signals/slots of all widgets together so communication
  // can happen when things get clicked.
  hookupSignals();

  raiseCurrentView();

  goToday();

  changeAgendaView(mAgendaViewMode);

  setupRollover();

  updateConfig();

  connect(QApplication::clipboard(),SIGNAL(dataChanged()),
          SLOT(checkClipboard()));

  kdDebug() << "CalendarView::CalendarView() done" << endl;
}

CalendarView::~CalendarView()
{
  kdDebug() << "~CalendarView()" << endl;
  hide();

  // clean up our calender object
  mCalendar->close();  // CS: This seems to cause a "Double QObject deletion"
  delete mCalendar;
  mCalendar = 0;

  kdDebug() << "~CalendarView() done" << endl;
}


bool CalendarView::openCalendar(QString filename)
{
  kdDebug() << "CalendarView::openCalendar()" << endl;
  if (initCalendar(filename)) {
    setModified(false);
    updateView();
    emit statusMessage(i18n("Opened calendar %1").arg(filename));
    return true;
  } else {
    return false;
  }
}


bool CalendarView::mergeCalendar(QString filename)
{
  kdDebug() << "CalendarView::mergeCalendar()" << endl;
  if (mCalendar->load(filename)) {
    setModified(true);
    updateView();
    return true;
  } else {
    return false;
  }
}


bool CalendarView::saveCalendar(QString filename)
{
  kdDebug() << "CalendarView::saveCalendar(): " << filename << endl;
  mCalendar->save(filename);

  // We should check for errors here.

  setModified(false);

  emit statusMessage(i18n("Saved calendar %1").arg(filename));

  return true;
}

void CalendarView::closeCalendar()
{
  kdDebug() << "CalendarView::closeCalendar()" << endl;

  // child windows no longer valid
  emit closingDown();

  mCalendar->close();
  setModified(false);
  updateView();
}

void CalendarView::archiveCalendar()
{
  if (!mArchiveDialog) {
    mArchiveDialog = new ArchiveDialog(mCalendar,this);
    connect(mArchiveDialog,SIGNAL(eventsDeleted()),SLOT(updateView()));
  }
  mArchiveDialog->show();
  mArchiveDialog->raise();

  // Workaround.
  QApplication::restoreOverrideCursor();
}

bool CalendarView::initCalendar(QString filename)
{
  // read the settings from the config file
  readSettings();

  kdDebug() << "CalendarView::initCalendar(): filename: " << filename << endl;

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
//  kdDebug() << "CalendarView::readSettings()" << endl;

  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig config(locateLocal("config", "korganizerrc"));

  config.setGroup("General");

  QValueList<int> sizes = config.readIntListEntry("Separator1");
  if (sizes.count() == 2) {
    mPanner->setSizes(sizes);
  }

  sizes = config.readIntListEntry("Separator2");
  if (sizes.count() == 3) {
    mLeftFrame->setSizes(sizes);
  }

  // Set current view from Entry "Current View"
  readCurrentView();

  mAgendaView->readSettings();
}

void CalendarView::readCurrentView()
{
  QString str;
  KConfig config(locateLocal("config", "korganizerrc"));

  mCurrentView = mAgendaView;

  config.setGroup("General");
  str = config.readEntry("Current View");
  if (!str.isEmpty()) {
    KOBaseView *obj;
    for(obj=mCalendarViews.first();obj;obj=mCalendarViews.next()) {
      if (str.compare(obj->className()) == 0) mCurrentView = obj;
    }
  }

  config.setGroup("Views");
  mAgendaViewMode = config.readNumEntry("Agenda View", KOAgendaView::DAY);
}

void CalendarView::writeSettings()
{
//  kdDebug() << "CalendarView::writeSettings" << endl;

  KConfig config(locateLocal("config", "korganizerrc"));

  config.setGroup("General");

  QValueList<int> list = mPanner->sizes();
  config.writeEntry("Separator1",list);

  list = mLeftFrame->sizes();
  config.writeEntry("Separator2",list);

  config.writeEntry("Current View",mCurrentView->className());

  config.setGroup("Views");
  config.writeEntry("Agenda View", mAgendaView->currentView());

  mAgendaView->writeSettings();

  KOPrefs::instance()->writeConfig();

  config.sync();
}

void CalendarView::goToday()
{
  QDateList tmpList;
  QDate today(QDate::currentDate());
  tmpList.append(&today);
  mDateNavigator->selectDates(tmpList);
  mSaveSingleDate = QDate::currentDate();
  updateView(mDateNavigator->getSelected());
}

void CalendarView::goNext()
{
  // adapt this to work for other views
  mAgendaView->slotNextDates();
  // this *appears* to work fine...
  updateView(mDateNavigator->getSelected());
}

void CalendarView::goPrevious()
{
  // adapt this to work for other views
  mAgendaView->slotPrevDates();
  // this *appears* to work fine...
  updateView(mDateNavigator->getSelected());
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
		     mDateNavigator, SLOT(updateView()));
}


void CalendarView::hookupSignals()
{
  // SIGNAL/SLOTS FOR DATE SYNCHRO
  connect(mListView, SIGNAL(datesSelected(const QDateList)),
	  mDateNavigator, SLOT(selectDates(const QDateList)));
  connect(mAgendaView, SIGNAL(datesSelected(const QDateList)),
	  mDateNavigator, SLOT(selectDates(const QDateList)));
  connect(mMonthView, SIGNAL(datesSelected(const QDateList)),
	  mDateNavigator, SLOT(selectDates(const QDateList)));
  connect(mDateNavigator, SIGNAL(datesSelected(const QDateList)),
	  this, SLOT(selectDates(const QDateList)));

  connect(mDateNavigator,SIGNAL(weekClicked(QDate)),SLOT(selectWeek(QDate)));

  connect(mDateNavigator,SIGNAL(eventDropped(KOEvent *)),
          SLOT(eventAdded(KOEvent *)));

  // SIGNALS/SLOTS FOR LIST VIEW
  connect(mListView, SIGNAL(showEventSignal(KOEvent *)),
	  this, SLOT(showEvent(KOEvent *)));
  connect(mListView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(mListView, SIGNAL(deleteEventSignal(KOEvent *)),
	  this, SLOT(deleteEvent(KOEvent *)));
  connect(mListView,SIGNAL(eventsSelected(bool)),
          SLOT(processEventSelection(bool)));

  // SIGNALS/SLOTS FOR DAY/WEEK VIEW
  connect(mAgendaView,SIGNAL(newEventSignal(QDateTime)),
          this, SLOT(newEvent(QDateTime)));
  connect(mAgendaView,SIGNAL(newEventSignal(QDate)),
          this, SLOT(newEvent(QDate)));
//  connect(mAgendaView,SIGNAL(newEventSignal()),
//		this, SLOT(newEvent()));
  connect(mAgendaView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(mAgendaView, SIGNAL(showEventSignal(KOEvent *)),
	  this, SLOT(showEvent(KOEvent *)));
  connect(mAgendaView, SIGNAL(deleteEventSignal(KOEvent *)),
	  this, SLOT(deleteEvent(KOEvent *)));
  connect(mAgendaView,SIGNAL(eventsSelected(bool)),
          SLOT(processEventSelection(bool)));

  // SIGNALS/SLOTS FOR MONTH VIEW
  connect(mMonthView, SIGNAL(showEventSignal(KOEvent *)),
	  this, SLOT(showEvent(KOEvent *)));
  connect(mMonthView, SIGNAL(newEventSignal(QDate)),
	  this, SLOT(newEvent(QDate)));
  connect(mMonthView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(mMonthView, SIGNAL(deleteEventSignal(KOEvent *)),
	  this, SLOT(deleteEvent(KOEvent *)));
  connect(mMonthView,SIGNAL(eventsSelected(bool)),
          SLOT(processEventSelection(bool)));

  // SIGNALS/SLOTS FOR TODO VIEW
  connect(mTodoView, SIGNAL(newTodoSignal()),
	  this, SLOT(newTodo()));
  connect(mTodoView, SIGNAL(newSubTodoSignal(KOEvent *)),
	  this, SLOT(newSubTodo(KOEvent *)));
  connect(mTodoView, SIGNAL(showTodoSignal(KOEvent *)),
	  this, SLOT(showTodo(KOEvent *)));
  connect(mTodoView, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(mTodoView, SIGNAL(deleteEventSignal(KOEvent *)),
          this, SLOT(deleteEvent(KOEvent *)));

  // SIGNALS/SLOTS FOR TODO LIST
  connect(mTodoList, SIGNAL(newTodoSignal()),
	  this, SLOT(newTodo()));
  connect(mTodoList, SIGNAL(newSubTodoSignal(KOEvent *)),
	  this, SLOT(newSubTodo(KOEvent *)));
  connect(mTodoList, SIGNAL(editEventSignal(KOEvent *)),
	  this, SLOT(editEvent(KOEvent *)));
  connect(mTodoList, SIGNAL(showTodoSignal(KOEvent *)),
	  this, SLOT(showTodo(KOEvent *)));
  connect(mTodoList, SIGNAL(deleteEventSignal(KOEvent *)),
          this, SLOT(deleteEvent(KOEvent *)));

  // CONFIGURATION SIGNALS/SLOTS
  // need to know about changed in configuration.
  connect(this, SIGNAL(configChanged()), mCalendar, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), mAgendaView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), mMonthView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), mListView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), mCalPrinter, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), mDateNavigator, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), mTodoView, SLOT(updateConfig()));
  connect(this, SIGNAL(configChanged()), mTodoList, SLOT(updateConfig()));

  // MISC. SIGNALS/SLOTS
  connect(mCalendar,SIGNAL(calUpdated(KOEvent *)),
          SLOT(eventUpdated(KOEvent *)));
  connect(mIncomingDialog,SIGNAL(calendarUpdated()),SLOT(updateView()));
}

void CalendarView::updateConfig()
{
  kdDebug() << "CalendarView::updateConfig()" << endl;
  emit configChanged();

  raiseCurrentView();
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
  kdDebug() << "CalendarView::eventToBeDeleted(): to be implemented" << endl;
}

void CalendarView::eventDeleted()
{
  changeEventDisplay(0,EVENTDELETED);
}

// most of the changeEventDisplays() right now just call the view's
// total update mode, but they SHOULD be recoded to be more refresh-efficient.
void CalendarView::changeEventDisplay(KOEvent *which, int action)
{
//  kdDebug() << "CalendarView::changeEventDisplay" << endl;

  mDateNavigator->updateView();
  if (mSearchDialog)
    mSearchDialog->updateView();

  if (which) {
    // If there is an event view visible update the display
    mCurrentView->changeEventDisplay(which,action);
    if (which->getTodoStatus()) {
      mTodoList->updateView();
    }
  } else {
    mCurrentView->updateView();
  }
}

void CalendarView::updateTodoViews()
{
  mTodoList->updateView();
  mCurrentView->updateView();
}

void CalendarView::changeAgendaView( int newView )
{
  if (newView == mAgendaView->currentView()) return;

  QPixmap px;

  switch( newView ) {
  case KOAgendaView::DAY: {
    QDateList tmpList(FALSE);
    tmpList = mDateNavigator->getSelected();
    if (mSaveSingleDate != *tmpList.first()) {
      mDateNavigator->selectDates(mSaveSingleDate);
      updateView(mDateNavigator->getSelected());
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
  mAgendaView->slotViewChange( newView );

  adaptNavigationUnits();
}

void CalendarView::nextAgendaView()
{
  int view;

  if( mCurrentView == mAgendaView ) {
    view = mAgendaView->currentView() + 1;
    if ((view >= KOAgendaView::DAY) && ( view < KOAgendaView::LIST))
      changeAgendaView( view );
    else
      changeAgendaView( KOAgendaView::DAY );
  } else {
    changeAgendaView( mAgendaView->currentView() );
    changeView( mAgendaView );
  }
}

void CalendarView::changeView(KOBaseView *view)
{
  if(view == mCurrentView) return;

  mCurrentView = view;

  raiseCurrentView();
  processEventSelection(false);

  updateView(mDateNavigator->getSelected());

  adaptNavigationUnits();
}

void CalendarView::raiseCurrentView()
{
  if ((KOPrefs::instance()->mFullViewMonth && mCurrentView == mMonthView) ||
       KOPrefs::instance()->mFullViewTodo && mCurrentView == mTodoView) {
    mLeftFrame->hide();
  } else {
    mLeftFrame->show();
  }
  mRightFrame->raiseWidget(mCurrentView);
}

void CalendarView::updateView(const QDateList selectedDates)
{
//  kdDebug() << "CalendarView::updateView()" << endl;

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
    mSaveSingleDate = *tmpList.first();
  } else {
    // for sanity, set viewtype to LIST for now...
    numView = KOAgendaView::LIST;
  }

  mCurrentView->selectDates(selectedDates);

  mTodoList->updateView();
  mTodoView->updateView();
}

void CalendarView::updateView()
{
  // update the current view with the current dates from the date navigator
  QDateList tmpList(FALSE); // we want a shallow copy
  tmpList = mDateNavigator->getSelected();

  // if no dates are supplied, we should refresh the mDateNavigator too...
  mDateNavigator->updateView();
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

  if (mCurrentView->isEventView()) {
    anEvent = (mCurrentView->getSelected()).first();
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }
  mCalendar->cutEvent(anEvent);
  changeEventDisplay(anEvent, EVENTDELETED);
}

void CalendarView::edit_copy()
{
  KOEvent *anEvent=0;

  if (mCurrentView->isEventView()) {
    anEvent = (mCurrentView->getSelected()).first();
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }
  mCalendar->copyEvent(anEvent);
}

void CalendarView::edit_paste()
{
  KOEvent *pastedEvent;
  QDateList tmpList(FALSE);

  tmpList = mDateNavigator->getSelected();
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
  kdDebug() << "CalendarView::newEvent()" << endl;
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
  connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
          eventWin,SLOT(updateCategoryConfig()));
  connect(eventWin,SIGNAL(editCategories()),mCategoryEditDialog,SLOT(show()));

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
  connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
          eventWin,SLOT(updateCategoryConfig()));
  connect(eventWin,SIGNAL(editCategories()),mCategoryEditDialog,SLOT(show()));

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
  tmpList = mDateNavigator->getSelected();
  from = *tmpList.first();
  to = *tmpList.last();

  ASSERT(from.isValid());
  if (!from.isValid()) { // mDateNavigator sometimes returns GARBAGE!
    from = QDate::currentDate();
    to = from;
  }

  newEvent(QDateTime(from, QTime(KOPrefs::instance()->mStartTime,0,0)),
	   QDateTime(to, QTime(KOPrefs::instance()->mStartTime +
                     KOPrefs::instance()->mDefaultDuration,0,0)));
}

void CalendarView::allday_new()
{

  QDate from, to;
  QDateList tmpList(FALSE);
  tmpList = mDateNavigator->getSelected();

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
  if(anEvent) {
    if (anEvent->isReadOnly()) {
      showEvent(anEvent);
      return;
    }

    QDateList tmpList(FALSE);
    QDate qd;

    tmpList = mDateNavigator->getSelected();
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
      connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
              eventWin,SLOT(updateCategoryConfig()));
      connect(eventWin,SIGNAL(editCategories()),
              mCategoryEditDialog,SLOT(show()));
      connect(this, SIGNAL(closingDown()),
              eventWin, SLOT(reject()));
      eventWin->show();
    }
  } else {
    KNotifyClient::beep();
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

void CalendarView::appointment_show()
{
  KOEvent *anEvent = 0;

  if (mCurrentView->isEventView()) {
    anEvent = (mCurrentView->getSelected()).first();
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  showEvent(anEvent);
}

void CalendarView::appointment_edit()
{
  KOEvent *anEvent = 0;

  if (mCurrentView->isEventView()) {
    anEvent = (mCurrentView->getSelected()).first();
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  editEvent(anEvent);
}

void CalendarView::appointment_delete()
{
  KOEvent *anEvent = 0;

  if (mCurrentView->isEventView()) {
    anEvent = (mCurrentView->getSelected()).first();
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  deleteEvent(anEvent);
}

// Is this function needed anymore? It could just call the deleteTodo slot of
// the KOTodoView, couldn't it?
void CalendarView::action_deleteTodo()
{
  KOEvent *aTodo;
  KOTodoView *todoList2 = (mCurrentView->isEventView() ? mTodoList : mTodoView);
//  TodoView *todoList2 = (viewMode == TODOVIEW ? mTodoView : mTodoList);

  aTodo = (todoList2->getSelected()).first();
  if (!aTodo) {
    KNotifyClient::beep();
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
    KNotifyClient::beep();
    return;
  }

  // At the moment we don't handle recurrence for todos
  if (!anEvent->getTodoStatus() && anEvent->doesRecur()) {
    switch(KMessageBox::warningContinueCancel(this,
        i18n("This event recurs over multiple dates.\n"
             "Are you sure you want to delete this event "
             "and all its recurrences?"),
             i18n("KOrganizer Confirmation"),i18n("&Continue"))) {

      case KMessageBox::Continue: // all
        mCalendar->deleteEvent(anEvent);
        changeEventDisplay(anEvent,EVENTDELETED);
        break;

// Disabled because it does not work
#if 0
      case KMessageBox::No: // just this one
        QDate qd;
        QDateList tmpList(FALSE);
        tmpList = mDateNavigator->getSelected();
        qd = *(tmpList.first());
        if (!qd.isValid()) {
          kdDebug() << "no date selected, or invalid date" << endl;
          KNotifyClient::beep();
          return;
        }
        while (!anEvent->recursOn(qd)) qd = qd.addDays(1);
        anEvent->addExDate(qd);
        changeEventDisplay(anEvent, EVENTEDITED);
        break;
#endif
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
  if (!mSearchDialog) {
    mSearchDialog = new SearchDialog(mCalendar);
    connect(mSearchDialog,SIGNAL(showEventSignal(KOEvent *)),
	    SLOT(showEvent(KOEvent *)));
    connect(mSearchDialog,SIGNAL(editEventSignal(KOEvent *)),
	    SLOT(editEvent(KOEvent *)));
    connect(mSearchDialog,SIGNAL(deleteEventSignal(KOEvent *)),
	    SLOT(deleteEvent(KOEvent *)));
    connect(this,SIGNAL(closingDown()),mSearchDialog,SLOT(reject()));
  }
  // make sure the widget is on top again
  mSearchDialog->show();
  mSearchDialog->raise();
}

void CalendarView::action_mail()
{
  KOMailClient mailobject;

  KOEvent *anEvent = 0;
  if (mCurrentView->isEventView()) {
    anEvent = (mCurrentView->getSelected()).first();
  }

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
  changeView(mListView);
}

void CalendarView::view_day()
{
  changeView(mAgendaView);
  changeAgendaView(KOAgendaView::DAY);
}

void CalendarView::view_workweek()
{
  changeView(mAgendaView);
  changeAgendaView(KOAgendaView::WORKWEEK);
}

void CalendarView::view_week()
{
  changeView(mAgendaView);
  changeAgendaView(KOAgendaView::WEEK);
}

void CalendarView::view_month()
{
  changeView(mMonthView);
}

void CalendarView::view_todolist()
{
  changeView(mTodoView);
}

void CalendarView::view_project()
{
  changeView(mProjectView);
}

void CalendarView::schedule_outgoing()
{
  mOutgoingDialog->show();
  mOutgoingDialog->raise();
}

void CalendarView::schedule_incoming()
{
  mIncomingDialog->show();
  mIncomingDialog->raise();
}

void CalendarView::schedule_publish()
{
  KOEvent *event = 0;

  if (mCurrentView->isEventView()) {
    event = (mCurrentView->getSelected()).first();
  }

  if (!event) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  mOutgoingDialog->addMessage(event,Scheduler::Publish,"dummy@nowhere.nil");
}

void CalendarView::schedule_request()
{
  schedule(Scheduler::Request);
}

void CalendarView::schedule_refresh()
{
  schedule(Scheduler::Refresh);
}

void CalendarView::schedule_cancel()
{
  schedule(Scheduler::Cancel);
}

void CalendarView::schedule_add()
{
  schedule(Scheduler::Add);
}

void CalendarView::schedule_reply()
{
  schedule(Scheduler::Reply);
}

void CalendarView::schedule_counter()
{
  schedule(Scheduler::Counter);
}

void CalendarView::schedule_declinecounter()
{
  schedule(Scheduler::Declinecounter);
}

void CalendarView::schedule(Scheduler::Method method)
{
  KOEvent *event = 0;

  if (mCurrentView->isEventView()) {
    event = (mCurrentView->getSelected()).first();
  }

  if (!event) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  mOutgoingDialog->addMessage(event,method);
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

  pidFile.setName(tmpStr);

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
  mCalPrinter->setupPrinter();
}

void CalendarView::print()
{
  QDateList tmpDateList(FALSE);

  tmpDateList = mDateNavigator->getSelected();
  mCalPrinter->print(CalPrinter::Month,
		    *tmpDateList.first(), *tmpDateList.last());
}

void CalendarView::printPreview()
{
  kdDebug() << "CalendarView::printPreview()" << endl;

  QDateList tmpDateList(FALSE);

  tmpDateList = mDateNavigator->getSelected();

  mCurrentView->printPreview(mCalPrinter,*tmpDateList.first(),
                            *tmpDateList.last());
}

void CalendarView::exportWeb()
{
  mExportWebDialog->show();
  mExportWebDialog->raise();
}

void CalendarView::exportICalendar()
{
  QString filename = KFileDialog::getSaveFileName("icalout.ics",i18n("*.ics|ICalendars"),this);

  // Force correct extension
  if (filename.right(4) != ".ics") filename += ".ics";

  CalFormat *format = new ICalFormat(mCalendar);
  mCalendar->save(filename,format);
}

void CalendarView::exportVCalendar()
{
  QString filename = KFileDialog::getSaveFileName("vcalout.vcs",i18n("*.vcs|VCaldendars"),this);

  // Force correct extension
  if (filename.right(4) != ".vcs") filename += ".vcs";

  CalFormat *format = new VCalFormat(mCalendar);
  mCalendar->save(filename,format);
}

void CalendarView::eventUpdated(KOEvent *)
{
  setModified();
  // Don't call updateView here. The code, which has caused the update of the
  // event is responsible for updating the view.
//  updateView();
}

void CalendarView::selectWeek(QDate weekstart)
{
//  kdDebug() << "CalendarView::selectWeek(): " << weekstart.toString() << endl;

  QDateList week;

  int n = 7;
  if (mCurrentView->currentDateCount() == 5) n = 5;

  int i;
  for(i=0;i<n;++i) {
    QDate date = weekstart.addDays(i);
    week.append(&date);
  }
  mDateNavigator->selectDates(week);

  updateView(week);
}

void CalendarView::adaptNavigationUnits()
{
  if (mCurrentView->isEventView()) {
    int days = mCurrentView->currentDateCount();
    if (days == 1) {
      emit changeNavStringPrev(i18n("&Previous Day"));
      emit changeNavStringNext(i18n("&Next Day"));
    } else {
      emit changeNavStringPrev(i18n("&Previous Week"));
      emit changeNavStringNext(i18n("&Next Week"));
    }
  }
}

void CalendarView::processEventSelection(bool selected)
{
  // Do nothing, if state hasn't changed
// Disabled because initial state wasn't propagated correctly
  if (mEventsSelected == selected) return;

  mEventsSelected = selected;
  emit eventsSelected(mEventsSelected);
}

void CalendarView::emitEventsSelected()
{
  emit eventsSelected(mEventsSelected);
}

void CalendarView::checkClipboard()
{
  if (VCalDrag::canDecode(QApplication::clipboard()->data())) {
    kdDebug() << "CalendarView::checkClipboard() true" << endl;
    emit pasteEnabled(true);
  } else {
    kdDebug() << "CalendarView::checkClipboard() false" << endl;
    emit pasteEnabled(false);
  }
}

void CalendarView::selectDates(const QDateList selectedDates)
{
//  kdDebug() << "CalendarView::selectDates()" << endl;
  if (mCurrentView->isEventView()) {
    updateView(selectedDates);
  } else {
    changeView(mAgendaView);
  }
}

void CalendarView::editCategories()
{
  mCategoryEditDialog->show();
}

void CalendarView::showFilter(bool visible)
{
  if (visible) mFilterView->show();
  else mFilterView->hide();
}
