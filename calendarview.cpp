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

#include <stdlib.h>

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
#include "filtereditdialog.h"
#include "kowhatsnextview.h"
#include "kojournalview.h"
#include "journal.h"
#include "calendarview.h"
#include "calendarview.moc"

bool CreateEditorVisitor::visit(Event *)
{
  return false;
}

bool CreateEditorVisitor::visit(Todo *)
{
  return false;
}

bool CreateEditorVisitor::visit(Journal *)
{
  return false;
}


CalendarView::CalendarView(QWidget *parent,const char *name)
  : QWidget(parent,name)
{
  kdDebug() << "CalendarView::CalendarView()" << endl;

  mCurrentView = 0;

  mWhatsNextView = 0;
  mTodoView = 0;
  mAgendaView = 0;
  mMonthView = 0;
  mListView = 0;
  mProjectView = 0;
  mJournalView = 0;

  mModified = false;
  mReadOnly = false;
  mEventsSelected = true;

  mOptionsDialog = 0;
  mSearchDialog = 0;
  mArchiveDialog = 0;
  mFilterEditDialog = 0;
  mExportWebDialog = 0;
  mOutgoingDialog = 0;
  mIncomingDialog = 0;
  
  mCalPrinter = 0;

//  setMinimumSize(620,400);	// make sure we don't get resized too small...

  // Create calendar object, which manages all calendar information associated
  // with this calendar view window.
  mCalendar = new CalendarLocal;
  mCalendar->setHoliday(KOPrefs::instance()->mHoliday);
  mCalendar->setEmail(KOPrefs::instance()->email());
  connect(mCalendar,SIGNAL(calUpdated(Incidence *)),
          SLOT(eventUpdated(Incidence *)));

  mCategoryEditDialog = new CategoryEditDialog();

  QBoxLayout *topLayout = new QVBoxLayout(this);

  // create the main layout frames.
  mPanner = new QSplitter(QSplitter::Horizontal,this,"CalendarView::Panner");
  topLayout->addWidget(mPanner);

  mLeftFrame = new QSplitter(QSplitter::Vertical,mPanner,
                            "CalendarView::LeftFrame");
  mRightFrame = new QWidgetStack(mPanner, "CalendarView::RightFrame");

  mDateNavigator = new KDateNavigator(mLeftFrame, mCalendar, TRUE,
                        "CalendarView::DateNavigator", QDate::currentDate());
  connect(mDateNavigator, SIGNAL(datesSelected(const QDateList)),
          SLOT(selectDates(const QDateList)));
  connect(mDateNavigator,SIGNAL(weekClicked(QDate)),SLOT(selectWeek(QDate)));
  connect(mDateNavigator,SIGNAL(eventDropped(Event *)),
          SLOT(eventAdded(Event *)));
  connect(this, SIGNAL(configChanged()), mDateNavigator, SLOT(updateConfig()));

  mTodoList = new KOTodoView(mCalendar, mLeftFrame, "CalendarView::TodoList");
  connect(mTodoList, SIGNAL(newTodoSignal()),
	  this, SLOT(newTodo()));
  connect(mTodoList, SIGNAL(newSubTodoSignal(Todo *)),
	  this, SLOT(newSubTodo(Todo *)));
  connect(mTodoList, SIGNAL(editTodoSignal(Todo *)),
	  this, SLOT(editTodo(Todo *)));
  connect(mTodoList, SIGNAL(showTodoSignal(Todo *)),
	  this, SLOT(showTodo(Todo *)));
  connect(mTodoList, SIGNAL(deleteTodoSignal(Todo *)),
          this, SLOT(deleteTodo(Todo *)));
  connect(this, SIGNAL(configChanged()), mTodoList, SLOT(updateConfig()));

  mFilters.setAutoDelete(true);

  mFilterView = new KOFilterView(&mFilters,mLeftFrame,"CalendarView::FilterView");
  connect(mFilterView,SIGNAL(filterChanged()),SLOT(updateFilter()));
  connect(mFilterView,SIGNAL(editFilters()),SLOT(editFilters()));
  // Hide filter per default
  mFilterView->hide();

  readSettings();

  setupRollover();

  // We should think about seperating startup settings and configuration change.
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

void CalendarView::createOptionsDialog()
{
  if (!mOptionsDialog) {
    mOptionsDialog = new KOPrefsDialog(this);
    mOptionsDialog->readConfig();
    connect(mOptionsDialog,SIGNAL(configChanged()),SLOT(updateConfig()));
    connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
            mOptionsDialog,SLOT(updateCategories()));
  }
}

void CalendarView::createOutgoingDialog()
{
  if (!mOutgoingDialog) {
    mOutgoingDialog = new OutgoingDialog(mCalendar,this);
    connect(mOutgoingDialog,SIGNAL(numMessagesChanged(int)),
            SIGNAL(numOutgoingChanged(int)));
  }
}

void CalendarView::createIncomingDialog()
{
  if (!mIncomingDialog) {
    mIncomingDialog = new IncomingDialog(mCalendar,this);
    connect(mIncomingDialog,SIGNAL(numMessagesChanged(int)),
            SIGNAL(numIncomingChanged(int)));
    connect(mIncomingDialog,SIGNAL(calendarUpdated()),SLOT(updateView()));
  }
}

void CalendarView::createPrinter()
{
  if (!mCalPrinter) {
    mCalPrinter = new CalPrinter(this, mCalendar);
    connect(this, SIGNAL(configChanged()), mCalPrinter, SLOT(updateConfig()));
  }
}

bool CalendarView::openCalendar(QString filename, bool merge)
{
  kdDebug() << "CalendarView::openCalendar(): " << filename << endl;
  
  if (filename.isEmpty()) {
    kdDebug() << "CalendarView::openCalendar(): Error! Empty filename." << endl;
    return false;
  }

  if (!QFile::exists(filename)) {
    kdDebug() << "CalendarView::openCalendar(): Error! File '" << filename
              << "' doesn't exist." << endl;
  }

  if (!merge) mCalendar->close();
  
  if (mCalendar->load(filename)) {
    if (merge) setModified(true);
    else setModified(false);
    updateView();
    return true;
  } else {
    // while failing to load, the calendar object could
    // have become partially populated.  Clear it out.
    if (!merge) mCalendar->close();
    return false;
  }
}

bool CalendarView::saveCalendar(QString filename)
{
  kdDebug() << "CalendarView::saveCalendar(): " << filename << endl;

  // Store back all unsaved data into calendar object
  mCurrentView->flushView();

  QString e = filename.right(4);

  if (e == ".vcs") {
    int result = KMessageBox::warningContinueCancel(this,
        i18n("Your calendar will be saved in iCalendar format.Use\n"
              "'Export vCalendar' to save in vCalendar format."),
        i18n("Format Conversion"),i18n("Proceed"),"dontaskFormatConversion",
        true);
    if (result != KMessageBox::Continue) return false;
    filename.right(4) = ".ics";
  }
  

  CalFormat *format = new ICalFormat(mCalendar);

  bool success = mCalendar->save(filename,format);
  
  delete format;
  
  if (!success) {
    return false;
  }

  setModified(false);

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
  readCurrentView(&config);

  readFilterSettings(&config);
}

void CalendarView::readCurrentView(KConfig *config)
{
  config->setGroup("General");
  QString view = config->readEntry("Current View");

  if (view == "WhatsNext") showWhatsNextView();
  else if (view == "Month") showMonthView();
  else if (view == "List") showListView();
  else if (view == "Project") showProjectView();
  else if (view == "Journal") showJournalView();
  else showAgendaView();
}

void CalendarView::writeCurrentView(KConfig *config)
{
  config->setGroup("General");

  QString view;
  if (mCurrentView == mWhatsNextView) view = "WhatsNext";
  else if (mCurrentView == mMonthView) view = "Month";
  else if (mCurrentView == mListView) view = "List";
  else if (mCurrentView == mProjectView) view = "Project";
  else if (mCurrentView == mJournalView) view = "Journal";
  else view = "Agenda";
  
  config->writeEntry("Current View",view);
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

  writeCurrentView(&config);

  if (mAgendaView) {
    mAgendaView->writeSettings(&config);
  }
  if (mProjectView) {
    mProjectView->writeSettings(&config);
  }

  KOPrefs::instance()->writeConfig();

  writeFilterSettings(&config);

  config.sync();
}

void CalendarView::readFilterSettings(KConfig *config)
{
//  kdDebug() << "CalendarView::readFilterSettings()" << endl;

  mFilters.clear();

  config->setGroup("General");
  QStringList filterList = config->readListEntry("CalendarFilters");

  QStringList::ConstIterator it = filterList.begin();
  QStringList::ConstIterator end = filterList.end();
  while(it != end) {
//    kdDebug() << "  filter: " << (*it) << endl;
  
    CalFilter *filter;
    filter = new CalFilter(*it);
    config->setGroup("Filter_" + (*it));
    filter->setInclusionCriteria(config->readNumEntry("Inclusion",0));
    filter->setExclusionCriteria(config->readNumEntry("Exclusion",0));
    mFilters.append(filter);
  
    ++it;
  }
  
  mFilterView->updateFilters();
}

void CalendarView::writeFilterSettings(KConfig *config)
{
//  kdDebug() << "CalendarView::writeFilterSettings()" << endl;

  QStringList filterList;

  CalFilter *filter = mFilters.first();
  while(filter) {
//    kdDebug() << " fn: " << filter->name() << endl;
    filterList << filter->name();
    config->setGroup("Filter_" + filter->name());
    config->writeEntry("Inclusion",filter->inclusionCriteria());
    config->writeEntry("Exclusion",filter->exclusionCriteria());
    filter = mFilters.next();
  }
  config->setGroup("General");
  config->writeEntry("CalendarFilters",filterList);
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
  if (mAgendaView) mAgendaView->slotNextDates();
  // this *appears* to work fine...
  updateView(mDateNavigator->getSelected());
}

void CalendarView::goPrevious()
{
  // adapt this to work for other views
  if (mAgendaView) mAgendaView->slotPrevDates();
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

void CalendarView::updateConfig()
{
  kdDebug() << "CalendarView::updateConfig()" << endl;
  emit configChanged();

  mCalendar->updateConfig();

  // To make the "fill window" configurations work
  raiseCurrentView();
}

void CalendarView::eventChanged(Event *event)
{
  changeEventDisplay(event,EVENTEDITED);
}

void CalendarView::eventAdded(Event *event)
{
  changeEventDisplay(event,EVENTADDED);
}

void CalendarView::eventToBeDeleted(Event *)
{
  kdDebug() << "CalendarView::eventToBeDeleted(): to be implemented" << endl;
}

void CalendarView::eventDeleted()
{
  changeEventDisplay(0,EVENTDELETED);
}

// most of the changeEventDisplays() right now just call the view's
// total update mode, but they SHOULD be recoded to be more refresh-efficient.
void CalendarView::changeEventDisplay(Event *which, int action)
{
//  kdDebug() << "CalendarView::changeEventDisplay" << endl;

  mDateNavigator->updateView();
  if (mSearchDialog)
    mSearchDialog->updateView();

  if (which) {
    // If there is an event view visible update the display
    mCurrentView->changeEventDisplay(which,action);
// TODO: check, if update needed
//    if (which->getTodoStatus()) {
      mTodoList->updateView();
//    }
  } else {
    mCurrentView->updateView();
  }
}

void CalendarView::updateTodoViews()
{
  kdDebug() << "CalendarView::updateTodoViews()" << endl;

  mTodoList->updateView();
  mCurrentView->updateView();
}

void CalendarView::changeAgendaView( int newView )
{
  if (!mAgendaView) return;

  if (newView == mAgendaView->currentView()) return;

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
  if (!mAgendaView) return;

  int view;

  if( mCurrentView == mAgendaView ) {
    view = mAgendaView->currentView() + 1;
    if ((view >= KOAgendaView::DAY) && ( view < KOAgendaView::LIST))
      changeAgendaView( view );
    else
      changeAgendaView( KOAgendaView::DAY );
  } else {
    changeAgendaView( mAgendaView->currentView() );
    showView( mAgendaView );
  }
}

void CalendarView::showView(KOBaseView *view)
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
  if ((mMonthView && KOPrefs::instance()->mFullViewMonth && mCurrentView == mMonthView) ||
      (mTodoView && KOPrefs::instance()->mFullViewTodo && mCurrentView == mTodoView)) {
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
  if (mTodoView) mTodoView->updateView();
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
  Event *anEvent=0;

  if (mCurrentView->isEventView()) {
    anEvent = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
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
  Event *anEvent=0;

  if (mCurrentView->isEventView()) {
    anEvent = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }
  mCalendar->copyEvent(anEvent);
}

void CalendarView::edit_paste()
{
  Event *pastedEvent;
  QDateList tmpList(FALSE);

  tmpList = mDateNavigator->getSelected();
  pastedEvent = mCalendar->pasteEvent(tmpList.first());
  changeEventDisplay(pastedEvent, EVENTADDED);
}

void CalendarView::edit_options()
{
  createOptionsDialog();
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
  KOEventEditor *eventEditor = getEventEditor();
  eventEditor->newEvent(fromHint,toHint);
  eventEditor->show();
}

void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint, bool allDay)
{
  KOEventEditor *eventEditor = getEventEditor();
  eventEditor->newEvent(fromHint,toHint,allDay);
  eventEditor->show();
}

void CalendarView::newTodo()
{
  KOTodoEditor *todoEditor = getTodoEditor();
  todoEditor->newTodo(QDateTime::currentDateTime().addDays(7),0,true);
  todoEditor->show();
}

void CalendarView::newSubTodo(Todo *parentEvent)
{
  KOTodoEditor *todoEditor = getTodoEditor();
  todoEditor->newTodo(QDateTime::currentDateTime().addDays(7),parentEvent,true);
  todoEditor->show();
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

void CalendarView::editEvent(Event *anEvent)
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

    KOEventEditor *eventEditor = getEventEditor();
    eventEditor->editEvent(anEvent,qd);
    eventEditor->show();
  } else {
    KNotifyClient::beep();
  }
}

void CalendarView::editTodo(Todo *todo)
{
  if (todo) {
    if (todo->isReadOnly()) {
      showTodo(todo);
      return;
    }

    QDateList tmpList(FALSE);
    QDate qd;

    tmpList = mDateNavigator->getSelected();
    qd = *tmpList.first();

    KOTodoEditor *todoEditor = getTodoEditor();
    todoEditor->editTodo(todo,qd);
    todoEditor->show();
  } else {
    KNotifyClient::beep();
  }
}

void CalendarView::showEvent(Event *event)
{
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
  eventViewer->setEvent(event);
  eventViewer->show();
}

void CalendarView::showTodo(Todo *event)
{
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
  eventViewer->setTodo(event);
  eventViewer->show();
}

void CalendarView::appointment_show()
{
  Event *anEvent = 0;

  if (mCurrentView->isEventView()) {
    anEvent = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  showEvent(anEvent);
}

void CalendarView::appointment_edit()
{
  Event *anEvent = 0;

  if (mCurrentView->isEventView()) {
    anEvent = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  editEvent(anEvent);
}

void CalendarView::appointment_delete()
{
  Event *anEvent = 0;

  if (mCurrentView->isEventView()) {
    anEvent = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
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
  Todo *aTodo;
  KOTodoView *todoList2 = (mCurrentView->isEventView() ? mTodoList : mTodoView);
//  TodoView *todoList2 = (viewMode == TODOVIEW ? mTodoView : mTodoList);

  if (!todoList2) {
    KNotifyClient::beep();
    return;
  }

  aTodo = (todoList2->selectedTodos()).first();
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

void CalendarView::deleteTodo(Todo *todo)
{
  if (!todo) {
    KNotifyClient::beep();
    return;
  }
  if (KOPrefs::instance()->mConfirm) {
    switch (msgItemDelete()) {
      case KMessageBox::Continue: // OK
        mCalendar->deleteTodo(todo);
        updateView();
        break;
    } // switch
  } else {
    mCalendar->deleteTodo(todo);
    updateView();
  }
}

void CalendarView::deleteEvent(Event *anEvent)
{
  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  if (anEvent->recurrence()->doesRecur()) {
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
          mCalendar->deleteEvent(anEvent);
          changeEventDisplay(anEvent, EVENTDELETED);
          break;
      } // switch
    } else {
      mCalendar->deleteEvent(anEvent);
      changeEventDisplay(anEvent, EVENTDELETED);
    }
  } // if-else
}

bool CalendarView::deleteEvent(const QString &VUID)
{
    Event *ev = mCalendar->getEvent(VUID);
    if (ev) {
        deleteEvent(ev);
        return true;
    } else {
        return false;
    }
}

/*****************************************************************************/

void CalendarView::action_search()
{
  if (!mSearchDialog) {
    mSearchDialog = new SearchDialog(mCalendar);
    connect(mSearchDialog,SIGNAL(showEventSignal(Event *)),
	    SLOT(showEvent(Event *)));
    connect(mSearchDialog,SIGNAL(editEventSignal(Event *)),
	    SLOT(editEvent(Event *)));
    connect(mSearchDialog,SIGNAL(deleteEventSignal(Event *)),
	    SLOT(deleteEvent(Event *)));
    connect(this,SIGNAL(closingDown()),mSearchDialog,SLOT(reject()));
  }
  // make sure the widget is on top again
  mSearchDialog->show();
  mSearchDialog->raise();
}

void CalendarView::action_mail()
{
  KOMailClient mailClient;

  Incidence *incidence = currentSelection();

  if (!incidence) {
    KMessageBox::sorry(this,i18n("Can't generate mail:\nNo event selected."));
    return;
  }
  if(incidence->attendeeCount() == 0 ) {
    KMessageBox::sorry(this,
                       i18n("Can't generate mail:\nNo attendees defined.\n"));
    return;
  }

  mailClient.mailAttendees(currentSelection());

#if 0
  Event *anEvent = 0;
  if (mCurrentView->isEventView()) {
    anEvent = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
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
#endif
}


void CalendarView::showWhatsNextView()
{
  if (!mWhatsNextView) {
    mWhatsNextView = new KOWhatsNextView(mCalendar,mRightFrame,
                                         "CalendarView::WhatsNextView");
    mRightFrame->addWidget(mWhatsNextView,0);
  }
  
  showView(mWhatsNextView);
}

void CalendarView::showListView()
{
  if (!mListView) {
    mListView = new KOListView(mCalendar, mRightFrame, "CalendarView::ListView");
    mRightFrame->addWidget(mListView,2);

    connect(mListView, SIGNAL(datesSelected(const QDateList)),
	    mDateNavigator, SLOT(selectDates(const QDateList)));

    connect(mListView, SIGNAL(showEventSignal(Event *)),
	    this, SLOT(showEvent(Event *)));
    connect(mListView, SIGNAL(editEventSignal(Event *)),
	    this, SLOT(editEvent(Event *)));
    connect(mListView, SIGNAL(deleteEventSignal(Event *)),
	    this, SLOT(deleteEvent(Event *)));
    connect(mListView,SIGNAL(eventsSelected(bool)),
            SLOT(processEventSelection(bool)));

    connect(this, SIGNAL(configChanged()), mListView, SLOT(updateConfig()));
  }
  
  showView(mListView);
}

void CalendarView::showAgendaView()
{
  if (!mAgendaView) {
    mAgendaView = new KOAgendaView(mCalendar, mRightFrame, "CalendarView::AgendaView");
    mRightFrame->addWidget(mAgendaView,1);

    connect(mAgendaView, SIGNAL(datesSelected(const QDateList)),
            mDateNavigator, SLOT(selectDates(const QDateList)));

    // SIGNALS/SLOTS FOR DAY/WEEK VIEW
    connect(mAgendaView,SIGNAL(newEventSignal(QDateTime)),
            this, SLOT(newEvent(QDateTime)));
    connect(mAgendaView,SIGNAL(newEventSignal(QDate)),
            this, SLOT(newEvent(QDate)));
//  connect(mAgendaView,SIGNAL(newEventSignal()),
//		this, SLOT(newEvent()));
    connect(mAgendaView, SIGNAL(editEventSignal(Event *)),
	    this, SLOT(editEvent(Event *)));
    connect(mAgendaView, SIGNAL(showEventSignal(Event *)),
            this, SLOT(showEvent(Event *)));
    connect(mAgendaView, SIGNAL(deleteEventSignal(Event *)),
            this, SLOT(deleteEvent(Event *)));
    connect(mAgendaView,SIGNAL(eventsSelected(bool)),
            SLOT(processEventSelection(bool)));

    connect(this, SIGNAL(configChanged()), mAgendaView, SLOT(updateConfig()));

    mAgendaView->readSettings();
  }
  
  showView(mAgendaView);
}

void CalendarView::showDayView()
{
  showAgendaView();
  changeAgendaView(KOAgendaView::DAY);
}

void CalendarView::showWorkWeekView()
{
  showAgendaView();
  changeAgendaView(KOAgendaView::WORKWEEK);
}

void CalendarView::showWeekView()
{
  showAgendaView();
  changeAgendaView(KOAgendaView::WEEK);
}

void CalendarView::showMonthView()
{
  if (!mMonthView) {
    mMonthView = new KOMonthView(mCalendar, mRightFrame, "CalendarView::MonthView");
    mRightFrame->addWidget(mMonthView,0);

    connect(mMonthView, SIGNAL(datesSelected(const QDateList)),
            mDateNavigator, SLOT(selectDates(const QDateList)));

    // SIGNALS/SLOTS FOR MONTH VIEW
    connect(mMonthView, SIGNAL(showEventSignal(Event *)),
            this, SLOT(showEvent(Event *)));
    connect(mMonthView, SIGNAL(newEventSignal(QDate)),
            this, SLOT(newEvent(QDate)));
    connect(mMonthView, SIGNAL(editEventSignal(Event *)),
            this, SLOT(editEvent(Event *)));
    connect(mMonthView, SIGNAL(deleteEventSignal(Event *)),
            this, SLOT(deleteEvent(Event *)));
    connect(mMonthView,SIGNAL(eventsSelected(bool)),
            SLOT(processEventSelection(bool)));

    connect(this, SIGNAL(configChanged()), mMonthView, SLOT(updateConfig()));
  }

  showView(mMonthView);
}

void CalendarView::showTodoView()
{
  if (!mTodoView) {
    mTodoView = new KOTodoView(mCalendar, mRightFrame, "CalendarView::TodoView");
    mRightFrame->addWidget(mTodoView,0);

    // SIGNALS/SLOTS FOR TODO VIEW
    connect(mTodoView, SIGNAL(newTodoSignal()),
            this, SLOT(newTodo()));
    connect(mTodoView, SIGNAL(newSubTodoSignal(Todo *)),
            this, SLOT(newSubTodo(Todo *)));
    connect(mTodoView, SIGNAL(showTodoSignal(Todo *)),
            this, SLOT(showTodo(Todo *)));
    connect(mTodoView, SIGNAL(editTodoSignal(Todo *)),
            this, SLOT(editTodo(Todo *)));
    connect(mTodoView, SIGNAL(deleteTodoSignal(Todo *)),
            this, SLOT(deleteTodo(Todo *)));

    connect(this, SIGNAL(configChanged()), mTodoView, SLOT(updateConfig()));
  }
  
  showView(mTodoView);
}

void CalendarView::showProjectView()
{
  if (!mProjectView) {
    mProjectView = new KOProjectView(mCalendar,mRightFrame,
                                     "CalendarView::ProjectView");
    mRightFrame->addWidget(mProjectView,0);
    
    mProjectView->readSettings();
  }

  showView(mProjectView);
}

void CalendarView::showJournalView()
{
  if (!mJournalView) {
    mJournalView = new KOJournalView(mCalendar,mRightFrame,
                                     "CalendarView::JournalView");
    mRightFrame->addWidget(mJournalView,0);
  }

  showView(mJournalView);
}


void CalendarView::schedule_outgoing()
{
  createOutgoingDialog();

  mOutgoingDialog->show();
  mOutgoingDialog->raise();
}

void CalendarView::schedule_incoming()
{
  createIncomingDialog();

  mIncomingDialog->show();
  mIncomingDialog->raise();
}

void CalendarView::schedule_publish()
{
  Event *event = 0;

  if (mCurrentView->isEventView()) {
    event = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
  }

  if (!event) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  createOutgoingDialog();
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
  Event *event = 0;

  if (mCurrentView->isEventView()) {
    event = dynamic_cast<Event *>((mCurrentView->getSelected()).first());
  }

  if (!event) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  createOutgoingDialog();
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

// TODO: Check, if this function is still needed
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
  createPrinter();

  mCalPrinter->setupPrinter();
}

void CalendarView::print()
{
  createPrinter();

  QDateList tmpDateList(FALSE);

  tmpDateList = mDateNavigator->getSelected();
  mCalPrinter->print(CalPrinter::Month,
		    *tmpDateList.first(), *tmpDateList.last());
}

void CalendarView::printPreview()
{
  kdDebug() << "CalendarView::printPreview()" << endl;

  createPrinter();

  QDateList tmpDateList(FALSE);

  tmpDateList = mDateNavigator->getSelected();

  mCurrentView->printPreview(mCalPrinter,*tmpDateList.first(),
                            *tmpDateList.last());
}

void CalendarView::exportWeb()
{
  if (!mExportWebDialog) {
    mExportWebDialog = new ExportWebDialog(mCalendar);
  }

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
  delete format;
}

void CalendarView::exportVCalendar()
{
  if (mCalendar->journalList().count() > 0) {
    int result = KMessageBox::warningContinueCancel(this,
        i18n("The journal entries can not be exported to a vCalendar file."),
        i18n("Data Loss Warning"),i18n("Proceed"),"dontaskVCalExport",
        true);
    if (result != KMessageBox::Continue) return;
  }

  QString filename = KFileDialog::getSaveFileName("vcalout.vcs",i18n("*.vcs|VCaldendars"),this);

  // Force correct extension
  if (filename.right(4) != ".vcs") filename += ".vcs";

  CalFormat *format = new VCalFormat(mCalendar);
  mCalendar->save(filename,format);
  delete format;
}

void CalendarView::eventUpdated(Incidence *)
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
    showAgendaView();
  }
}

void CalendarView::editCategories()
{
  mCategoryEditDialog->show();
}

void CalendarView::editFilters()
{
//  kdDebug() << "CalendarView::editFilters()" << endl;

  CalFilter *filter = mFilters.first();
  while(filter) {
    kdDebug() << " Filter: " << filter->name() << endl;
    filter = mFilters.next();
  }  

  if (!mFilterEditDialog) {
    mFilterEditDialog = new FilterEditDialog(&mFilters,this);
    connect(mFilterEditDialog,SIGNAL(filterChanged()),
            SLOT(filterEdited()));
  }
  mFilterEditDialog->show();
  mFilterEditDialog->raise();
}

void CalendarView::showFilter(bool visible)
{
  if (visible) mFilterView->show();
  else mFilterView->hide();
}

void CalendarView::updateFilter()
{
  CalFilter *filter = mFilterView->selectedFilter();
  if (mFilterView->filtersEnabled()) filter->setEnabled(true);
  else filter->setEnabled(false);
  mCalendar->setFilter(filter);
  updateView();
}

void CalendarView::filterEdited()
{
  mFilterView->updateFilters();
  updateFilter();
}

KOEventEditor *CalendarView::getEventEditor()
{
  KOEventEditor *eventEditor = new KOEventEditor(mCalendar);

  connect(eventEditor,SIGNAL(eventAdded(Event *)),SLOT(eventAdded(Event *)));
  connect(eventEditor,SIGNAL(eventChanged(Event *)),SLOT(eventChanged(Event *)));
  connect(eventEditor,SIGNAL(eventDeleted()),SLOT(eventDeleted()));

  connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
          eventEditor,SLOT(updateCategoryConfig()));
  connect(eventEditor,SIGNAL(editCategories()),mCategoryEditDialog,SLOT(show()));

  connect(this,SIGNAL(closingDown()),eventEditor,SLOT(reject()));

  return eventEditor;
}

KOTodoEditor *CalendarView::getTodoEditor()
{
  KOTodoEditor *todoEditor = new KOTodoEditor(mCalendar);

  connect(mCategoryEditDialog,SIGNAL(categoryConfigChanged()),
          todoEditor,SLOT(updateCategoryConfig()));
  connect(todoEditor,SIGNAL(editCategories()),mCategoryEditDialog,SLOT(show()));

  connect(todoEditor,SIGNAL(todoAdded(Todo *)),SLOT(updateTodoViews()));
  connect(todoEditor,SIGNAL(todoChanged(Todo *)),SLOT(updateTodoViews()));
  connect(todoEditor,SIGNAL(todoDeleted()),SLOT(updateTodoViews()));

  connect(this, SIGNAL(closingDown()),todoEditor,SLOT(reject()));

  return todoEditor;
}

Incidence *CalendarView::currentSelection()
{
  if (!mCurrentView) return 0;
  
  return mCurrentView->getSelected().first();
}

void CalendarView::takeOverEvent()
{
  Incidence *incidence = currentSelection();

  if (!incidence) return;
  
  incidence->setOrganizer(KOPrefs::instance()->email());
  incidence->recreate();
  incidence->setReadOnly(false);

  updateView();
}

void CalendarView::takeOverCalendar()
{
  // TODO: Create Calendar::allIncidences() function and use it here

  QList<Event> events = mCalendar->getAllEvents();  
  for(uint i=0; i<events.count(); ++i) {
    events.at(i)->setOrganizer(KOPrefs::instance()->email());
    events.at(i)->recreate();
    events.at(i)->setReadOnly(false);
  }
  
  QList<Todo> todos = mCalendar->getTodoList();
  for(uint i=0; i<todos.count(); ++i) {
    todos.at(i)->setOrganizer(KOPrefs::instance()->email());
    todos.at(i)->recreate();
    todos.at(i)->setReadOnly(false);
  }
  
  QList<Journal> journals = mCalendar->journalList();
  for(uint i=0; i<journals.count(); ++i) {
    journals.at(i)->setOrganizer(KOPrefs::instance()->email());
    journals.at(i)->recreate();
    journals.at(i)->setReadOnly(false);
  }
  
  updateView();
}

void CalendarView::showIntro()
{
  kdDebug() << "To be implemented." << endl;
}
