/*
  This file is part of KOrganizer.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.troll.no and http://www.kde.org respectively

  Copyright (c) 1997, 1998, 1999
  Preston Brown (preston.brown@yale.edu)
  Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
  Ian Dawes (iadawes@globalserve.net)
  Laszlo Boloni (boloni@cs.purdue.edu)

  Copyright (c) 2000, 2001, 2002
  Cornelius Schumacher <schumacher@kde.org>

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

// $Id$

#include <stdlib.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qmultilineedit.h>
#include <qtimer.h>
#include <qwidgetstack.h>
#include <qclipboard.h>
#include <qptrlist.h>
#include <qclipboard.h>
#include <qfile.h>
#ifndef KORG_NOSPLITTER
#include <qsplitter.h>
#endif

#include <kglobal.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kconfig.h>
#include <krun.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/icalformat.h>
#include <libkcal/vcalformat.h>
#include <libkcal/scheduler.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/journal.h>
#include <libkcal/calfilter.h>
#include <libkcal/attendee.h>
#include <libkcal/dndfactory.h>
#include <libkcal/freebusy.h>

#ifndef KORG_NOMAIL
#include "komailclient.h"
#endif
#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "koeventeditor.h"
#include "kotodoeditor.h"
#include "koprefs.h"
#include "koeventviewerdialog.h"
#include "publishdialog.h"
#include "kofilterview.h"
#include "koglobals.h"
#include "koviewmanager.h"
//ET WORKAROUND
#include "koagendaview.h"
#include "kodialogmanager.h"
#include "outgoingdialog.h"
#include "incomingdialog.h"
#include "statusdialog.h"

#include "calendarview.h"
using namespace KOrg;
#include "calendarview.moc"


CalendarView::CalendarView(QWidget *parent,const char *name)
  : CalendarViewBase(parent,name)
{
  kdDebug() << "CalendarView::CalendarView()" << endl;

  mViewManager = new KOViewManager( this );
  mDialogManager = new KODialogManager( this );

  mModified = false;
  mReadOnly = false;
  mSelectedIncidence = 0;

  mCalPrinter = 0;

  mFilters.setAutoDelete(true);

  // Create calendar object, which manages all calendar information associated
  // with this calendar view window.
  mCalendar = new CalendarLocal(KOPrefs::instance()->mTimeZoneId.local8Bit());
  mCalendar->setOwner(KOPrefs::instance()->fullName());
  mCalendar->setEmail(KOPrefs::instance()->email());

  mCalendar->registerObserver( this );

  // TODO: Make sure that view is updated, when calendar is changed.

  QBoxLayout *topLayout = new QVBoxLayout(this);

#ifndef KORG_NOSPLITTER
  // create the main layout frames.
  mPanner = new QSplitter(QSplitter::Horizontal,this,"CalendarView::Panner");
  topLayout->addWidget(mPanner);

  mLeftSplitter = new QSplitter(QSplitter::Vertical,mPanner,
                            "CalendarView::LeftFrame");
  mPanner->setResizeMode(mLeftSplitter,QSplitter::KeepSize);

  mDateNavigator = new KDateNavigator(mLeftSplitter, mCalendar, TRUE,
                        "CalendarView::DateNavigator", QDate::currentDate());
  mLeftSplitter->setResizeMode(mDateNavigator,QSplitter::KeepSize);
  mTodoList = new KOTodoView(mCalendar, mLeftSplitter, "todolist");
  mFilterView = new KOFilterView(&mFilters,mLeftSplitter,"CalendarView::FilterView");

  mRightFrame = new QWidgetStack(mPanner, "CalendarView::RightFrame");

  mLeftFrame = mLeftSplitter;
#else
  QWidget *mainBox;
  QWidget *leftFrame;

  if ( KOPrefs::instance()->mVerticalScreen ) {
    mainBox = new QVBox( this );
    leftFrame = new QHBox( mainBox );
  } else {
    mainBox = new QHBox( this );
    leftFrame = new QVBox( mainBox );
  }

  topLayout->addWidget( mainBox );

  mDateNavigator = new KDateNavigator(leftFrame, mCalendar, TRUE,
                        "CalendarView::DateNavigator", QDate::currentDate());
  mTodoList = new KOTodoView(mCalendar, leftFrame, "todolist");
  mFilterView = new KOFilterView(&mFilters,leftFrame,"CalendarView::FilterView");

  mRightFrame = new QWidgetStack(mainBox, "CalendarView::RightFrame");

  mLeftFrame = leftFrame;

  if ( KOPrefs::instance()->mVerticalScreen ) {
//    mTodoList->setFixedHeight( 60 );
    mTodoList->setFixedHeight( mDateNavigator->sizeHint().height() );
  }
#endif

  connect(mDateNavigator, SIGNAL(datesSelected(const DateList &)),
          SLOT(selectDates(const DateList &)));
  connect(mDateNavigator,SIGNAL(weekClicked(QDate)),SLOT(selectWeek(QDate)));
  connect(mDateNavigator,SIGNAL(eventDropped(Event *)),
          SLOT(eventAdded(Event *)));
  connect(this, SIGNAL(configChanged()), mDateNavigator, SLOT(updateConfig()));

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

  connect(mFilterView,SIGNAL(filterChanged()),SLOT(updateFilter()));
  connect(mFilterView,SIGNAL(editFilters()),SLOT(editFilters()));
  // Hide filter per default
  mFilterView->hide();

  readSettings();

  setupRollover();

  mMessageTimer = new QTimer();
  connect(mMessageTimer,SIGNAL(timeout()),SLOT(lookForIncomingMessages()));
  mMidnightTimer = new QTimer();
  connect(mMidnightTimer,SIGNAL(timeout()),SLOT(updateView()));
  
  QTime now = QTime::currentTime();
  int secs = 0;
  secs = now.hour()*60*60 + now.minute()*60 + now.second();
  // timer for 1 second after midnoght
  mMidnightTimer->start((24*60*60-secs)*1000+1000,true);
  // We should think about seperating startup settings and configuration change.
  updateConfig();

  connect(QApplication::clipboard(),SIGNAL(dataChanged()),
          SLOT(checkClipboard()));
  connect( mTodoList,SIGNAL( incidenceSelected( Incidence * ) ),
           SLOT( processTodoListSelection( Incidence * ) ) );
  connect(mTodoList,SIGNAL(isModified(bool)),SLOT(setModified(bool)));
  
  kdDebug() << "CalendarView::CalendarView() done" << endl;
}

CalendarView::~CalendarView()
{
  kdDebug() << "~CalendarView()" << endl;

  // clean up our calender object
  mCalendar->close();  // CS: This seems to cause a "Double QObject deletion"
  delete mCalendar;
  delete mDialogManager;
  delete mViewManager;

  kdDebug() << "~CalendarView() done" << endl;
}

KOViewManager *CalendarView::viewManager()
{
  return mViewManager;
}

KODialogManager *CalendarView::dialogManager()
{
  return mDialogManager;
}

QDate CalendarView::startDate()
{
  DateList dates = mDateNavigator->selectedDates();

  return dates.first();
}

QDate CalendarView::endDate()
{
  DateList dates = mDateNavigator->selectedDates();

  return dates.last();
}


void CalendarView::createPrinter()
{
#ifndef KORG_NOPRINTER
  if (!mCalPrinter) {
    mCalPrinter = new CalPrinter(this, mCalendar);
    connect(this, SIGNAL(configChanged()), mCalPrinter, SLOT(updateConfig()));
  }
#endif
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
    else {
      setModified(false);
      mViewManager->setDocumentId( filename );
      mTodoList->setDocumentId( filename );
    }
    updateView();
    return true;
  } else {
    // while failing to load, the calendar object could
    // have become partially populated.  Clear it out.
    if (!merge) mCalendar->close();

    KMessageBox::error(this,i18n("Couldn't load calendar '%1'.").arg(filename));
    
    return false;
  }
}

bool CalendarView::saveCalendar(QString filename)
{
  kdDebug() << "CalendarView::saveCalendar(): " << filename << endl;

  // Store back all unsaved data into calendar object
  mViewManager->currentView()->flushView();

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
  mDialogManager->showArchiveDialog();
}


void CalendarView::readSettings()
{
//  kdDebug() << "CalendarView::readSettings()" << endl;

  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KGlobal::config();

#ifndef KORG_NOSPLITTER
  config->setGroup("KOrganizer Geometry");

  QValueList<int> sizes = config->readIntListEntry("Separator1");
  if (sizes.count() != 2) {
    sizes << mDateNavigator->minimumSizeHint().width();
    sizes << 300;
  }
  mPanner->setSizes(sizes);

  sizes = config->readIntListEntry("Separator2");
  if (sizes.count() == 3) {
    mLeftSplitter->setSizes(sizes);
  }
#endif

  mViewManager->readSettings( config );
  mTodoList->restoreLayout(config,QString("Todo Layout"));

  readFilterSettings(config);
}


void CalendarView::writeSettings()
{
//  kdDebug() << "CalendarView::writeSettings" << endl;

  KConfig *config = KGlobal::config();

#ifndef KORG_NOSPLITTER
  config->setGroup("KOrganizer Geometry");

  QValueList<int> list = mPanner->sizes();
  config->writeEntry("Separator1",list);

  list = mLeftSplitter->sizes();
  config->writeEntry("Separator2",list);
#endif

  mViewManager->writeSettings( config );
  mTodoList->saveLayout(config,QString("Todo Layout"));

  KOPrefs::instance()->writeConfig();

  writeFilterSettings(config);

  config->sync();
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
    filter->setCriteria(config->readNumEntry("Criteria",0));
    filter->setCategoryList(config->readListEntry("CategoryList"));
    mFilters.append(filter);

    ++it;
  }

  if (mFilters.count() == 0) {
    CalFilter *filter = new CalFilter(i18n("Default"));
    mFilters.append(filter);
  }
  mFilterView->updateFilters();
  config->setGroup("FilterView");
  mFilterView->setFiltersEnabled(config->readBoolEntry("FilterEnabled"));
  mFilterView->setSelectedFilter(config->readEntry("Current Filter"));

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
    config->writeEntry("Criteria",filter->criteria());
    config->writeEntry("CategoryList",filter->categoryList());
    filter = mFilters.next();
  }
  config->setGroup("General");
  config->writeEntry("CalendarFilters",filterList);

  config->setGroup("FilterView");
  config->writeEntry("FilterEnabled",mFilterView->filtersEnabled());
  config->writeEntry("Current Filter",mFilterView->selectedFilter()->name());
}


void CalendarView::goToday()
{
  DateList tmpList;
  tmpList.append(QDate::currentDate());
  mDateNavigator->selectDates(tmpList);
  mSaveSingleDate = QDate::currentDate();
  updateView();
}

void CalendarView::goNext()
{
  // adapt this to work for other views
//#if 0
//ET WORKAROUND
  if (mViewManager->currentView() == (KOrg::BaseView*)mViewManager->mAgendaView) mViewManager->mAgendaView->slotNextDates();
//ET TODO adapt selection of daymatrix apropriately
//#endif
  // this *appears* to work fine...
  updateView();
}

void CalendarView::goPrevious()
{
  // adapt this to work for other views
//#if 0
//ET WORKAROUND
  if (mViewManager->currentView() == (KOrg::BaseView*)mViewManager->mAgendaView) mViewManager->mAgendaView->slotPrevDates();
//ET TODO adapt selection of daymatrix apropriately
//#endif
  // this *appears* to work fine...
  updateView();
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

  mMessageTimer->stop();
  if (KOPrefs::instance()->mIntervalCheck) {
    mMessageTimer->start(KOPrefs::instance()->mIntervalCheckTime*60*1000,false);
  }

  mCalendar->setTimeZoneId(KOPrefs::instance()->mTimeZoneId.local8Bit());

  // To make the "fill window" configurations work
  mViewManager->raiseCurrentView();
}


void CalendarView::eventChanged(Event *event)
{
  changeEventDisplay(event,KOGlobals::EVENTEDITED);
}

void CalendarView::eventAdded(Event *event)
{
  changeEventDisplay(event,KOGlobals::EVENTADDED);
}

void CalendarView::eventToBeDeleted(Event *)
{
  kdDebug() << "CalendarView::eventToBeDeleted(): to be implemented" << endl;
}

void CalendarView::eventDeleted()
{
  changeEventDisplay(0,KOGlobals::EVENTDELETED);
}


// most of the changeEventDisplays() right now just call the view's
// total update mode, but they SHOULD be recoded to be more refresh-efficient.
void CalendarView::changeEventDisplay(Event *which, int action)
{
//  kdDebug() << "CalendarView::changeEventDisplay" << endl;

  mDateNavigator->updateView();
  mDialogManager->updateSearchDialog();

  if (which) {
    // If there is an event view visible update the display
    mViewManager->currentView()->changeEventDisplay(which,action);
// TODO: check, if update needed
//    if (which->getTodoStatus()) {
      mTodoList->updateView();
//    }
  } else {
    mViewManager->currentView()->updateView();
  }
}


void CalendarView::updateTodoViews()
{
  kdDebug() << "CalendarView::updateTodoViews()" << endl;

  mTodoList->updateView();
  mViewManager->currentView()->updateView();
}


void CalendarView::updateView(const QDate &start, const QDate &end)
{
  mTodoList->updateView();
  mViewManager->updateView(start, end);
//ET
  mDateNavigator->updateView();
}

void CalendarView::updateView()
{
  // update the current view with the current dates from the date navigator
  DateList tmpList = mDateNavigator->selectedDates();

#if 0
  // if no dates are supplied, we should refresh the mDateNavigator too...
  mDateNavigator->updateView();
#endif
  updateView( tmpList.first(), tmpList.last() );
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

  Incidence *incidence = mViewManager->currentView()->selectedIncidences().first();

  if (mViewManager->currentView()->isEventView()) {
    if ( incidence && incidence->type() == "Event" ) {
      anEvent = static_cast<Event *>(incidence);
    }
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }
  DndFactory factory( mCalendar );
  factory.cutEvent(anEvent);
  changeEventDisplay(anEvent, KOGlobals::EVENTDELETED);
}

void CalendarView::edit_copy()
{
  Event *anEvent=0;

  Incidence *incidence = mViewManager->currentView()->selectedIncidences().first();

  if (mViewManager->currentView()->isEventView()) {
    if ( incidence && incidence->type() == "Event" ) {
      anEvent = static_cast<Event *>(incidence);
    }
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }
  DndFactory factory( mCalendar );
  factory.copyEvent(anEvent);
}

void CalendarView::edit_paste()
{
  QDate date = mDateNavigator->selectedDates().first();

  DndFactory factory( mCalendar );
  Event *pastedEvent = factory.pasteEvent(date);
  changeEventDisplay(pastedEvent, KOGlobals::EVENTADDED);
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
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
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent(fromHint,toHint);
  eventEditor->show();
}

void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint, bool allDay)
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent(fromHint,toHint,allDay);
  eventEditor->show();
}


void CalendarView::newTodo()
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  todoEditor->newTodo(QDateTime::currentDateTime().addDays(7),0,true);
  todoEditor->show();
}

void CalendarView::newSubTodo()
{
  Todo *todo = selectedTodo();
  if ( todo ) newSubTodo( todo );
}

void CalendarView::newSubTodo(Todo *parentEvent)
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  todoEditor->newTodo(QDateTime::currentDateTime().addDays(7),parentEvent,true);
  todoEditor->show();
}

void CalendarView::appointment_new()
{
  DateList tmpList = mDateNavigator->selectedDates();
  QDate from = tmpList.first();
  QDate to = tmpList.last();

  ASSERT(from.isValid());

  newEvent(QDateTime(from, QTime(KOPrefs::instance()->mStartTime,0,0)),
	   QDateTime(to, QTime(KOPrefs::instance()->mStartTime +
                     KOPrefs::instance()->mDefaultDuration,0,0)));
}

void CalendarView::allday_new()
{
  DateList tmpList = mDateNavigator->selectedDates();
  QDate from = tmpList.first();
  QDate to = tmpList.last();

  ASSERT(from.isValid());

  newEvent(QDateTime(from, QTime(12,0,0)),
           QDateTime(to, QTime(12,0,0)), TRUE);
}


void CalendarView::editEvent( Event *event )
{
  kdDebug() << "CalendarView::editEvent()" << endl;

  if ( !event ) return;

  if ( mDialogList.find( event ) != mDialogList.end() ) {
    mDialogList[ event ]->raise();
    mDialogList[ event ]->show();
    return;
  }

  if ( event->isReadOnly() ) {
    showEvent( event );
    return;
  }
  
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  mDialogList.insert( event, eventEditor );
  eventEditor->editEvent( event );
  eventEditor->show();
}

void CalendarView::editTodo( Todo *todo )
{
  if ( !todo ) return;

  if ( mDialogList.find( todo ) != mDialogList.end() ) {
    mDialogList[todo]->raise();
    mDialogList[todo]->show();
    return;
  }

  if ( todo->isReadOnly() ) {
    showTodo( todo );
    return;
  }

  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  mDialogList.insert( todo, todoEditor );
  todoEditor->editTodo( todo );
  todoEditor->show();
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

  Incidence *incidence = mViewManager->currentView()->selectedIncidences().first();

  if (mViewManager->currentView()->isEventView()) {
    if ( incidence && incidence->type() == "Event" ) {
      anEvent = static_cast<Event *>(incidence);
    }
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

  Incidence *incidence = mViewManager->currentView()->selectedIncidences().first();

  if (mViewManager->currentView()->isEventView()) {
    if ( incidence && incidence->type() == "Event" ) {
      anEvent = static_cast<Event *>(incidence);
    }
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

  Incidence *incidence = mViewManager->currentView()->selectedIncidences().first();

  if (mViewManager->currentView()->isEventView()) {
    if ( incidence && incidence->type() == "Event" ) {
      anEvent = static_cast<Event *>(incidence);
    }
  }

  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  deleteEvent(anEvent);
}

void CalendarView::todo_unsub()
{
  Todo *anTodo = selectedTodo();
  if (!anTodo) return;
  if (!anTodo->relatedTo()) return;
  anTodo->relatedTo()->removeRelation(anTodo);
  anTodo->setRelatedTo(0);
  anTodo->setRelatedToUid("");
  setModified(true);
  updateView();
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
        if (!todo->relations().isEmpty()) {
          KMessageBox::sorry(this,i18n("Cannot delete To-Do which has children."),
                         i18n("Delete To-Do"));
        } else {
          calendar()->deleteTodo(todo);
          updateView();
        }
        break;
    } // switch
  } else {
    if (!todo->relations().isEmpty()) {
        KMessageBox::sorry(this,i18n("Cannot delete To-Do which has children."),
                         i18n("Delete To-Do"));
    } else {
      calendar()->deleteTodo(todo);
      updateView();
    }
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
        if (anEvent->organizer()==KOPrefs::instance()->email() && anEvent->attendeeCount()>0)
          schedule(Scheduler::Cancel,anEvent);
        mCalendar->deleteEvent(anEvent);
        changeEventDisplay(anEvent,KOGlobals::EVENTDELETED);
        break;

// Disabled because it does not work
#if 0
      case KMessageBox::No: // just this one
        QDate qd = mDateNavigator->selectedDates().first();
        if (!qd.isValid()) {
          kdDebug() << "no date selected, or invalid date" << endl;
          KNotifyClient::beep();
          return;
        }
        while (!anEvent->recursOn(qd)) qd = qd.addDays(1);
        anEvent->addExDate(qd);
        changeEventDisplay(anEvent, KOGlobals::EVENTEDITED);
        break;
#endif
    } // switch
  } else {
    if (KOPrefs::instance()->mConfirm) {
      switch (msgItemDelete()) {
        case KMessageBox::Continue: // OK
          if (anEvent->organizer()==KOPrefs::instance()->email() && anEvent->attendeeCount()>0)
	    schedule(Scheduler::Cancel,anEvent);
          mCalendar->deleteEvent(anEvent);
          changeEventDisplay(anEvent, KOGlobals::EVENTDELETED);
          break;
      } // switch
    } else {
      if (anEvent->organizer()==KOPrefs::instance()->email() && anEvent->attendeeCount()>0)
        schedule(Scheduler::Cancel,anEvent);
      mCalendar->deleteEvent(anEvent);
      changeEventDisplay(anEvent, KOGlobals::EVENTDELETED);
    }
  } // if-else
}

bool CalendarView::deleteEvent(const QString &uid)
{
    Event *ev = mCalendar->getEvent(uid);
    if (ev) {
        deleteEvent(ev);
        return true;
    } else {
        return false;
    }
}

/*****************************************************************************/

void CalendarView::action_mail()
{
#ifndef KORG_NOMAIL
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

  Calendar *cal_tmp = new CalendarLocal();
  Event *event = 0;
  Event *ev = 0;
  if ( incidence && incidence->type() == "Event" ) {
    event = static_cast<Event *>(incidence);
    ev = new Event(*event);
    cal_tmp->addEvent(ev);
  }
  ICalFormat mForm(cal_tmp);
  QString attachment = mForm.toString();
  if (ev) delete(ev);
  delete(cal_tmp);

  mailClient.mailAttendees(currentSelection(), attachment);

#endif

#if 0
  Event *anEvent = 0;
  if (mViewManager->currentView()->isEventView()) {
    anEvent = dynamic_cast<Event *>((mViewManager->currentView()->selectedIncidences()).first());
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


void CalendarView::schedule_publish(Incidence *incidence)
{
  Event *event = 0;

	if (incidence == 0) {
    incidence = mViewManager->currentView()->selectedIncidences().first();
//    if (mViewManager->currentView()->isEventView()) {
//      if ( incidence && incidence->type() == "Event" ) {
//        event = static_cast<Event *>(incidence);
//      }
//    }
  }
  if ( incidence && incidence->type() == "Event" ) {
    event = static_cast<Event *>(incidence);
  }

  if (!event) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  PublishDialog *publishdlg = new PublishDialog();
  if (event->attendeeCount()>0) {
    QPtrList<Attendee> attendees = event->attendees();
    attendees.first();
    while ( attendees.current()!=0 ) {
     publishdlg->addAttendee(attendees.current());
     attendees.next();
   }
  }
  if ( publishdlg->exec() == QDialog::Accepted ) {
    OutgoingDialog *dlg = mDialogManager->outgoingDialog();
    Event *ev = new Event(*event);
    ev->clearAttendees();
    if (!dlg->addMessage(ev,Scheduler::Publish,publishdlg->addresses())) {
      delete(ev);
    }
  }
  delete publishdlg;
}

void CalendarView::schedule_request(Incidence *incidence)
{
  schedule(Scheduler::Request,incidence);
}

void CalendarView::schedule_refresh(Incidence *incidence)
{
  schedule(Scheduler::Refresh,incidence);
}

void CalendarView::schedule_cancel(Incidence *incidence)
{
  schedule(Scheduler::Cancel,incidence);
}

void CalendarView::schedule_add(Incidence *incidence)
{
  schedule(Scheduler::Add,incidence);
}

void CalendarView::schedule_reply(Incidence *incidence)
{
  schedule(Scheduler::Reply,incidence);
}

void CalendarView::schedule_counter(Incidence *incidence)
{
  schedule(Scheduler::Counter,incidence);
}

void CalendarView::schedule_declinecounter(Incidence *incidence)
{
  schedule(Scheduler::Declinecounter,incidence);
}

void CalendarView::schedule_publish_freebusy()
{
  QDateTime start = QDateTime::currentDateTime();
  QDateTime end = start.addDays(30);

  FreeBusy *freebusy = new FreeBusy(mCalendar, start, end);
  freebusy->setOrganizer(KOPrefs::instance()->email());

  kdDebug() << "calendarview: schedule_publish_freebusy: startDate: "
     << KGlobal::locale()->formatDateTime( start ) << " End Date: " 
     << KGlobal::locale()->formatDateTime( end ) << endl;
  
  PublishDialog *publishdlg = new PublishDialog();
  if ( publishdlg->exec() == QDialog::Accepted ) {
    OutgoingDialog *dlg = mDialogManager->outgoingDialog();
    if (!dlg->addMessage(freebusy,Scheduler::Publish,publishdlg->addresses())) {
         delete(freebusy);
    }
  }
  delete publishdlg;
}

void CalendarView::schedule(Scheduler::Method method, Incidence *incidence)
{
  Event *event = 0;

	if (incidence == 0) {
    incidence = mViewManager->currentView()->selectedIncidences().first();
//    if (mViewManager->currentView()->isEventView()) {
//      if ( incidence && incidence->type() == "Event" ) {
//        event = static_cast<Event *>(incidence);
//      }
//    }
  }
  if ( incidence && incidence->type() == "Event" ) {
    event = static_cast<Event *>(incidence);
  }

  if (!event) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  Event *ev = new Event(*event);

  if( event->attendeeCount() == 0 && method != Scheduler::Publish ) {
    KMessageBox::sorry(this,i18n("The event has no attendees."));
    return;
  }

  if (method == Scheduler::Reply ) {
    Attendee *me = event->attendeeByMails(KOPrefs::instance()->mAdditionalMails,KOPrefs::instance()->email());
    //Attendee *me = event->attendeeByMail(KOPrefs::instance()->email());
    if (!me) {
      KMessageBox::sorry(this,i18n("Could not find your attendee entry. Please check the emails."));
      return;
    }
    if (me->status()==Attendee::NeedsAction && me->RSVP()) {
      StatusDialog *statdlg = new StatusDialog(this);
      if (!statdlg->exec()==QDialog::Accepted) return;
      me->setStatus( statdlg->status() );
      delete(statdlg);
      //me->setRSVP(false);
    }
    Attendee *menew = new Attendee(*me);
    ev->clearAttendees();
    ev->addAttendee(menew,false);
  }

  OutgoingDialog *dlg = mDialogManager->outgoingDialog();
  if ( !dlg->addMessage(ev,method) ) delete(ev);
}

void CalendarView::openAddressbook()
{
  KRun::runCommand("kaddressbook");
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
#if 0
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
#endif
}

void CalendarView::printSetup()
{
#ifndef KORG_NOPRINTER
  createPrinter();

  mCalPrinter->setupPrinter();
#endif
}

void CalendarView::print()
{
#ifndef KORG_NOPRINTER
  createPrinter();

  DateList tmpDateList = mDateNavigator->selectedDates();
  mCalPrinter->print(CalPrinter::Month,
		     tmpDateList.first(), tmpDateList.last());
#endif
}

void CalendarView::printPreview()
{
#ifndef KORG_NOPRINTER
  kdDebug() << "CalendarView::printPreview()" << endl;

  createPrinter();

  DateList tmpDateList = mDateNavigator->selectedDates();

  mViewManager->currentView()->printPreview(mCalPrinter,tmpDateList.first(),
                             tmpDateList.last());
#endif
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

  DateList week;

  int n = 7;
  if (mViewManager->currentView()->currentDateCount() == 5) n = 5;

  int i;
  for(i=0;i<n;++i) {
    QDate date = weekstart.addDays(i);
    week.append(date);
  }
  mDateNavigator->selectDates(week);

  updateView( week.first(), week.last() );
}

void CalendarView::adaptNavigationUnits()
{
  if (mViewManager->currentView()->isEventView()) {
    int days = mViewManager->currentView()->currentDateCount();
    if (days == 1) {
      emit changeNavStringPrev(i18n("&Previous Day"));
      emit changeNavStringNext(i18n("&Next Day"));
    } else {
      emit changeNavStringPrev(i18n("&Previous Week"));
      emit changeNavStringNext(i18n("&Next Week"));
    }
  }
}

void CalendarView::processMainViewSelection( Incidence *incidence )
{
  if ( incidence ) mTodoList->clearSelection();
  processIncidenceSelection( incidence );
}

void CalendarView::processTodoListSelection( Incidence *incidence )
{
  if ( incidence && mViewManager->currentView() ) {
    mViewManager->currentView()->clearSelection();
  }
  processIncidenceSelection( incidence );
}

void CalendarView::processIncidenceSelection( Incidence *incidence )
{
  if ( incidence == mSelectedIncidence ) return;
  
  mSelectedIncidence = incidence;

  emit incidenceSelected( mSelectedIncidence );

  if ( incidence && incidence->type() == "Event" ) {
    Event *event = static_cast<Event *>( incidence );
    
    if ( event->organizer() == KOPrefs::instance()->email() ) {
      emit organizerEventsSelected( true );
    } else {
      emit organizerEventsSelected(false);
    }
    
    if (event->attendeeByMails( KOPrefs::instance()->mAdditionalMails,
                                KOPrefs::instance()->email() ) ) {
      emit groupEventsSelected( true );
    } else {
      emit groupEventsSelected(false);
    }
  } else {
    emit organizerEventsSelected(false);
    emit groupEventsSelected(false);
  }

  if  ( incidence && incidence->type() == "Todo" ) {
    emit todoSelected( true );
  } else {
    emit todoSelected( false );
  }
}


void CalendarView::checkClipboard()
{
#ifndef KORG_NODND
  if (VCalDrag::canDecode(QApplication::clipboard()->data())) {
    kdDebug() << "CalendarView::checkClipboard() true" << endl;
    emit pasteEnabled(true);
  } else {
    kdDebug() << "CalendarView::checkClipboard() false" << endl;
    emit pasteEnabled(false);
  }
#endif
}

void CalendarView::selectDates(const DateList &selectedDates)
{
//  kdDebug() << "CalendarView::selectDates()" << endl;
  if (mViewManager->currentView()->isEventView()) {
    updateView( selectedDates.first(), selectedDates.last() );
  } else {
    mViewManager->showAgendaView();
  }
}

void CalendarView::editFilters()
{
//  kdDebug() << "CalendarView::editFilters()" << endl;

  CalFilter *filter = mFilters.first();
  while(filter) {
    kdDebug() << " Filter: " << filter->name() << endl;
    filter = mFilters.next();
  }

  mDialogManager->showFilterEditDialog(&mFilters);
}

void CalendarView::showFilter(bool visible)
{
  if (visible) mFilterView->show();
  else mFilterView->hide();
}

void CalendarView::updateFilter()
{
  CalFilter *filter = mFilterView->selectedFilter();
  if (filter) {
    if (mFilterView->filtersEnabled()) filter->setEnabled(true);
    else filter->setEnabled(false);
    mCalendar->setFilter(filter);
    updateView();
  }
}

void CalendarView::filterEdited()
{
  mFilterView->updateFilters();
  updateFilter();
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

  QPtrList<Event> events = mCalendar->getAllEvents();  
  for(uint i=0; i<events.count(); ++i) {
    events.at(i)->setOrganizer(KOPrefs::instance()->email());
    events.at(i)->recreate();
    events.at(i)->setReadOnly(false);
  }

  QPtrList<Todo> todos = mCalendar->getTodoList();
  for(uint i=0; i<todos.count(); ++i) {
    todos.at(i)->setOrganizer(KOPrefs::instance()->email());
    todos.at(i)->recreate();
    todos.at(i)->setReadOnly(false);
  }

  QPtrList<Journal> journals = mCalendar->journalList();
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

QWidgetStack *CalendarView::viewStack()
{
  return mRightFrame;
}

QWidget *CalendarView::leftFrame()
{
  return mLeftFrame;
}

KDateNavigator *CalendarView::dateNavigator()
{
  return mDateNavigator;
}

void CalendarView::addView(KOrg::BaseView *view)
{
  mViewManager->addView(view);
}

void CalendarView::showView(KOrg::BaseView *view)
{
  mViewManager->showView(view);
}

Incidence *CalendarView::currentSelection()
{
  return mViewManager->currentSelection();
}

void CalendarView::toggleExpand()
{
  if ( mLeftFrame->isHidden() ) {
    mLeftFrame->show();
    emit calendarViewExpanded( false );
  } else {
    mLeftFrame->hide();
    emit calendarViewExpanded( true );
  }
}

void CalendarView::calendarModified( bool modified, Calendar * )
{
  setModified( modified );
}

Todo *CalendarView::selectedTodo()
{
  Incidence *incidence = currentSelection();
  if ( incidence && incidence->type() == "Todo" ) {
    return static_cast<Todo *>( incidence );
  }

  incidence = mTodoList->selectedIncidences().first();
  if ( incidence && incidence->type() == "Todo" ) {
    return static_cast<Todo *>( incidence );
  }

  return 0;
}

void CalendarView::dialogClosing(Incidence *in)
{
  mDialogList.remove(in);
}

void CalendarView::showIncidence()
{
  Incidence *incidence = currentSelection();
  if ( !incidence ) incidence = mTodoList->selectedIncidences().first();
  if ( incidence ) {
    ShowIncidenceVisitor v;
    v.act( incidence, this );
  }
}

void CalendarView::editIncidence()
{
  Incidence *incidence = currentSelection();
  if ( !incidence ) incidence = mTodoList->selectedIncidences().first();
  if ( incidence ) {
    EditIncidenceVisitor v;
    v.act( incidence, this );
  }
}

void CalendarView::deleteIncidence()
{
  Incidence *incidence = currentSelection();
  if ( !incidence ) incidence = mTodoList->selectedIncidences().first();
  if ( incidence ) {
    DeleteIncidenceVisitor v;
    v.act( incidence, this );
  }
}


void CalendarView::lookForOutgoingMessages()
{
  OutgoingDialog *ogd = mDialogManager->outgoingDialog();
  ogd->loadMessages();
}

void CalendarView::lookForIncomingMessages()
{
  IncomingDialog *icd = mDialogManager->incomingDialog();
  icd->retrieve();
}

void CalendarView::purgeCompleted()
{
  int result = KMessageBox::warningContinueCancel(this,
      i18n("Delete all completed To-Dos?"),i18n("Purge To-Dos"),i18n("Purge"));

  if (result == KMessageBox::Continue) {
    QPtrList<Todo> todoCal;
    QPtrList<Incidence> rel;
    Todo *aTodo, *rTodo;
    Incidence *rIncidence;
    bool childDelete = false;
    bool deletedOne = true;
    while (deletedOne) {
      todoCal.clear();
      todoCal = calendar()->getTodoList();
      deletedOne = false;
      for (aTodo = todoCal.first(); aTodo; aTodo = todoCal.next()) {
        if (aTodo->isCompleted()) {
          rel = aTodo->relations();
          if (!rel.isEmpty()) {
            for (rIncidence=rel.first(); rIncidence; rIncidence=rel.next()){
              if (rIncidence->type()=="Todo") {
                rTodo = static_cast<Todo*>(rIncidence);
                if (!rTodo->isCompleted()) childDelete = true;
              }
            }
          }
          else {
            calendar()->deleteTodo(aTodo);
            deletedOne = true;
          }
        }
      }
    }
    if (childDelete) {
      KMessageBox::sorry(this,i18n("Cannot purge To-Do which has uncompleted children."),
                         i18n("Delete To-Do"));
    }
    updateView();
  }
}
