/*
  This file is part of KOrganizer.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.troll.no and http://www.kde.org respectively

  Copyright (c) 1997, 1998, 1999
  Preston Brown (preston.brown@yale.edu)
  Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
  Ian Dawes (iadawes@globalserve.net)
  Laszlo Boloni (boloni@cs.purdue.edu)

  Copyright (c) 2000, 2001, 2002, 2003
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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdlib.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qmultilineedit.h>
#include <qtimer.h>
#include <qwidgetstack.h>
#include <qptrlist.h>
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
#include <kdirwatch.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/icaldrag.h>
#include <libkcal/icalformat.h>
#include <libkcal/vcalformat.h>
#include <libkcal/scheduler.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/journal.h>
#include <libkcal/calfilter.h>
#include <libkcal/attendee.h>
#include <libkcal/dndfactory.h>
#include <libkcal/freebusy.h>
#include <libkcal/filestorage.h>
#include <libkcal/calendarresources.h>
#include <libkcal/qtopiaformat.h>

#ifndef KORG_NOMAIL
#include "komailclient.h"
#endif
#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif
#include "koeventeditor.h"
#include "kotodoeditor.h"
#include "koprefs.h"
#include "koeventviewerdialog.h"
#include "publishdialog.h"
#include "kofilterview.h"
#include "koglobals.h"
#include "koviewmanager.h"
#include "koagendaview.h"
#include "kodialogmanager.h"
#include "outgoingdialog.h"
#include "incomingdialog.h"
#include "statusdialog.h"
#include "kdatenavigator.h"
#include "kotodoview.h"
#include "datenavigator.h"
#include "resourceview.h"
#include "navigatorbar.h"
#include "history.h"

#include "calendarview.h"

using namespace KOrg;

CalendarView::CalendarView( Calendar *calendar,
                            QWidget *parent, const char *name )
  : CalendarViewBase( parent, name ),
    mCalendar( calendar )
{
  kdDebug(5850) << "CalendarView::CalendarView( Calendar )" << endl;

  mHistory = new History( calendar );
  connect( mHistory, SIGNAL( undone() ), SLOT( updateView() ) );
  connect( mHistory, SIGNAL( redone() ), SLOT( updateView() ) );

  mViewManager = new KOViewManager( this );
  mDialogManager = new KODialogManager( this );

  mModified = false;
  mReadOnly = false;
  mSelectedIncidence = 0;

  mCalPrinter = 0;

  mFilters.setAutoDelete(true);

  mExtensions.setAutoDelete( true );

  mCalendar->registerObserver( this );

  // TODO: Make sure that view is updated, when calendar is changed.

  mStorage = new FileStorage( mCalendar );

  mNavigator = new DateNavigator( this );

  QBoxLayout *topLayout = new QVBoxLayout(this);

#ifndef KORG_NOSPLITTER
  // create the main layout frames.
  mPanner = new QSplitter(QSplitter::Horizontal,this,"CalendarView::Panner");
  topLayout->addWidget(mPanner);

  mLeftSplitter = new QSplitter(QSplitter::Vertical,mPanner,
                            "CalendarView::LeftFrame");
  mPanner->setResizeMode(mLeftSplitter,QSplitter::KeepSize);

  mDateNavigator = new KDateNavigator(mLeftSplitter, mCalendar, TRUE,
                        "CalendarView::DateNavigator", QDate::currentDate() );
  mLeftSplitter->setResizeMode(mDateNavigator,QSplitter::KeepSize);
  mTodoList = new KOTodoView(mCalendar, mLeftSplitter, "todolist");
  mFilterView = new KOFilterView(&mFilters,mLeftSplitter,"CalendarView::FilterView");

  QWidget *rightBox = new QWidget( mPanner );
  QBoxLayout *rightLayout = new QVBoxLayout( rightBox );

  mNavigatorBar = new NavigatorBar( QDate::currentDate(), rightBox );
  rightLayout->addWidget( mNavigatorBar );

  mRightFrame = new QWidgetStack( rightBox );
  rightLayout->addWidget( mRightFrame, 1 );

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

  QWidget *rightBox = new QWidget( mainBox );
  QBoxLayout *rightLayout = new QVBoxLayout( rightBox );

  mNavigatorBar = new NavigatorBar( QDate::currentDate(), rightBox );
  rightLayout->addWidget( mNavigatorBar );

  mRightFrame = new QWidgetStack( rightBox );
  rightLayout->addWidget( mRightFrame );

  mLeftFrame = leftFrame;

  if ( KOPrefs::instance()->mVerticalScreen ) {
//    mTodoList->setFixedHeight( 60 );
    mTodoList->setFixedHeight( mDateNavigator->sizeHint().height() );
  }
#endif

  connect( mNavigator, SIGNAL( datesSelected( const KCal::DateList & ) ),
           SLOT( showDates( const KCal::DateList & ) ) );
  connect( mNavigator, SIGNAL( datesSelected( const KCal::DateList & ) ),
           mDateNavigator, SLOT( selectDates( const KCal::DateList & ) ) );

  connect( mNavigatorBar, SIGNAL( goPrevYear() ),
           mNavigator, SLOT( selectPreviousYear() ) );
  connect( mNavigatorBar, SIGNAL( goNextYear() ),
           mNavigator, SLOT( selectNextYear() ) );
  connect( mNavigatorBar, SIGNAL( goPrevMonth() ),
           mNavigator, SLOT( selectPreviousMonth() ) );
  connect( mNavigatorBar, SIGNAL( goNextMonth() ),
           mNavigator, SLOT( selectNextMonth() ) );

  connect( mNavigator, SIGNAL( datesSelected( const KCal::DateList & ) ),
           mNavigatorBar, SLOT( selectDates( const KCal::DateList & ) ) );

  connect( mDateNavigator, SIGNAL( weekClicked( const QDate & ) ),
           mNavigator, SLOT( selectWeek( const QDate & ) ) );

  connect( mDateNavigator, SIGNAL( goPrevYear() ),
           mNavigator, SLOT( selectPreviousYear() ) );
  connect( mDateNavigator, SIGNAL( goNextYear() ),
           mNavigator, SLOT( selectNextYear() ) );
  connect( mDateNavigator, SIGNAL( goPrevMonth() ),
           mNavigator, SLOT( selectPreviousMonth() ) );
  connect( mDateNavigator, SIGNAL( goNextMonth() ),
           mNavigator, SLOT( selectNextMonth() ) );

  connect( mDateNavigator, SIGNAL( goPrevious() ),
           mNavigator, SLOT( selectPrevious() ) );
  connect( mDateNavigator, SIGNAL( goNext() ),
           mNavigator, SLOT( selectNext() ) );

  connect( mDateNavigator, SIGNAL( datesSelected( const KCal::DateList & ) ),
           mNavigator, SLOT( selectDates( const KCal::DateList & ) ) );

  connect( mDateNavigator, SIGNAL( eventDropped( Event * ) ),
           SLOT( eventAdded( Event *) ) );

  connect(mDateNavigator,SIGNAL(dayPassed(QDate)),SLOT(updateView()));

  connect( this, SIGNAL( configChanged() ),
           mDateNavigator, SLOT( updateConfig() ) );

  connect( mTodoList, SIGNAL( newTodoSignal() ),
	   SLOT( newTodo() ) );
  connect( mTodoList, SIGNAL( newSubTodoSignal( Todo *) ),
	   SLOT( newSubTodo( Todo * ) ) );
  connect( mTodoList, SIGNAL( editTodoSignal( Todo * ) ),
	   SLOT( editTodo( Todo * ) ) );
  connect( mTodoList, SIGNAL( showTodoSignal( Todo * ) ),
	   SLOT( showTodo( Todo *) ) );
  connect( mTodoList, SIGNAL( deleteTodoSignal( Todo *) ),
           SLOT( deleteTodo( Todo *) ) );
  connect( this, SIGNAL( configChanged()), mTodoList, SLOT( updateConfig() ) );
  connect( mTodoList, SIGNAL( purgeCompletedSignal() ),
           SLOT( purgeCompleted() ) );
  connect( mTodoList, SIGNAL( todoModifiedSignal( Todo *, int ) ),
	   SLOT( todoModified( Todo *, int ) ) );

  connect( mFilterView, SIGNAL( filterChanged() ), SLOT( updateFilter() ) );
  connect( mFilterView, SIGNAL( editFilters() ), SLOT( editFilters() ) );
  // Hide filter per default
  mFilterView->hide();

  readSettings();

  KDirWatch *messageWatch = new KDirWatch();
  messageWatch->addDir(locateLocal("data","korganizer/income/"));
  connect (messageWatch,SIGNAL(dirty(const QString &)),SLOT(lookForIncomingMessages()));

  // We should think about seperating startup settings and configuration change.
  updateConfig();

  connect(QApplication::clipboard(),SIGNAL(dataChanged()),
          SLOT(checkClipboard()));
  connect( mTodoList,SIGNAL( incidenceSelected( Incidence * ) ),
           SLOT( processTodoListSelection( Incidence * ) ) );
  connect(mTodoList,SIGNAL(isModified(bool)),SLOT(setModified(bool)));

  kdDebug(5850) << "CalendarView::CalendarView() done" << endl;
}

CalendarView::~CalendarView()
{
  kdDebug(5850) << "~CalendarView()" << endl;

  delete mDialogManager;
  delete mViewManager;

  kdDebug(5850) << "~CalendarView() done" << endl;
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
  DateList dates = mNavigator->selectedDates();

  return dates.first();
}

QDate CalendarView::endDate()
{
  DateList dates = mNavigator->selectedDates();

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


bool CalendarView::openCalendar(const QString& filename, bool merge)
{
  kdDebug(5850) << "CalendarView::openCalendar(): " << filename << endl;

  if (filename.isEmpty()) {
    kdDebug(5850) << "CalendarView::openCalendar(): Error! Empty filename." << endl;
    return false;
  }

  if (!QFile::exists(filename)) {
    kdDebug(5850) << "CalendarView::openCalendar(): Error! File '" << filename
              << "' doesn't exist." << endl;
  }

  if (!merge) mCalendar->close();

  mStorage->setFileName( filename );

  if ( mStorage->load() ) {
    if ( merge ) setModified( true );
    else {
      setModified( false );
      mViewManager->setDocumentId( filename );
      mDialogManager->setDocumentId( filename );
      mTodoList->setDocumentId( filename );
    }
    updateView();
    return true;
  } else {
    // while failing to load, the calendar object could
    // have become partially populated.  Clear it out.
    if ( !merge ) mCalendar->close();

    KMessageBox::error(this,i18n("Couldn't load calendar '%1'.").arg(filename));

    return false;
  }
}

bool CalendarView::saveCalendar( const QString& filename )
{
  kdDebug(5850) << "CalendarView::saveCalendar(): " << filename << endl;

  // Store back all unsaved data into calendar object
  mViewManager->currentView()->flushView();

  mStorage->setFileName( filename );
  mStorage->setSaveFormat( new ICalFormat );

  bool success = mStorage->save();

  if ( !success ) {
    return false;
  }

  return true;
}

void CalendarView::closeCalendar()
{
  kdDebug(5850) << "CalendarView::closeCalendar()" << endl;

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
//  kdDebug(5850) << "CalendarView::readSettings()" << endl;

  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::config();

#ifndef KORG_NOSPLITTER
  config->setGroup("KOrganizer Geometry");

  QValueList<int> sizes = config->readIntListEntry("Separator1");
  if (sizes.count() != 2) {
    sizes << mDateNavigator->minimumSizeHint().width();
    sizes << 300;
  }
  mPanner->setSizes(sizes);

  sizes = config->readIntListEntry("Separator2");
  mLeftSplitter->setSizes(sizes);
#endif

  mViewManager->readSettings( config );
  mTodoList->restoreLayout(config,QString("Todo Layout"));

  readFilterSettings(config);

  config->setGroup( "Views" );
  int dateCount = config->readNumEntry( "ShownDatesCount", 7 );
  if ( dateCount == 5 ) mNavigator->selectWorkWeek();
  else if ( dateCount == 7 ) mNavigator->selectWeek();
  else mNavigator->selectDates( dateCount );
}


void CalendarView::writeSettings()
{
//  kdDebug(5850) << "CalendarView::writeSettings" << endl;

  KConfig *config = KOGlobals::config();

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

  config->setGroup( "Views" );
  config->writeEntry( "ShownDatesCount", mNavigator->selectedDates().count() );

  config->sync();
}

void CalendarView::readFilterSettings(KConfig *config)
{
//  kdDebug(5850) << "CalendarView::readFilterSettings()" << endl;

  mFilters.clear();

  config->setGroup("General");
  QStringList filterList = config->readListEntry("CalendarFilters");

  QStringList::ConstIterator it = filterList.begin();
  QStringList::ConstIterator end = filterList.end();
  while(it != end) {
//    kdDebug(5850) << "  filter: " << (*it) << endl;

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

  mFilterView->blockSignals(true);
  mFilterView->setFiltersEnabled(config->readBoolEntry("FilterEnabled"));
  mFilterView->setSelectedFilter(config->readEntry("Current Filter"));
  mFilterView->blockSignals(false);
  // We do it manually to avoid it being done twice by the above calls
  updateFilter();
}

void CalendarView::writeFilterSettings(KConfig *config)
{
//  kdDebug(5850) << "CalendarView::writeFilterSettings()" << endl;

  QStringList filterList;

  CalFilter *filter = mFilters.first();
  while(filter) {
//    kdDebug(5850) << " fn: " << filter->name() << endl;
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
  mNavigator->selectToday();
}

void CalendarView::goNext()
{
  mNavigator->selectNext();
}

void CalendarView::goPrevious()
{
  mNavigator->selectPrevious();
}

void CalendarView::updateConfig()
{
  kdDebug(5850) << "CalendarView::updateConfig()" << endl;

  emit configChanged();

  mCalendar->setTimeZoneId(KOPrefs::instance()->mTimeZoneId);

  // To make the "fill window" configurations work
  mViewManager->raiseCurrentView();
}


void CalendarView::eventChanged( Event *oldEvent, Event *newEvent )
{
  changeEventDisplay( newEvent, KOGlobals::EVENTEDITED );

  mHistory->recordEdit( oldEvent, newEvent );
}

void CalendarView::eventAdded( Event *event )
{
  changeEventDisplay( event, KOGlobals::EVENTADDED );

  mHistory->recordAdd( event );
}

void CalendarView::eventToBeDeleted( Event * )
{
  kdDebug(5850) << "CalendarView::eventToBeDeleted(): to be implemented"
                << endl;
}

void CalendarView::eventDeleted()
{
  changeEventDisplay( 0, KOGlobals::EVENTDELETED );
}


void CalendarView::todoChanged( Todo *oldTodo, Todo *newTodo )
{
  updateTodoViews();

  mHistory->recordEdit( oldTodo, newTodo );
}

void CalendarView::todoAdded( Todo *todo )
{
  updateTodoViews();

  mHistory->recordAdd( todo );
}


// most of the changeEventDisplays() right now just call the view's
// total update mode, but they SHOULD be recoded to be more refresh-efficient.
void CalendarView::changeEventDisplay(Event *which, int action)
{
//  kdDebug(5850) << "CalendarView::changeEventDisplay" << endl;

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
  kdDebug(5850) << "CalendarView::updateTodoViews()" << endl;

  mTodoList->updateView();
  mViewManager->currentView()->updateView();
}


void CalendarView::updateView(const QDate &start, const QDate &end)
{
  mTodoList->updateView();
  mViewManager->updateView(start, end);
  mDateNavigator->updateView();
}

void CalendarView::updateView()
{
  DateList tmpList = mNavigator->selectedDates();

  // We assume that the navigator only selects consecutive days.
  updateView( tmpList.first(), tmpList.last() );
}

void CalendarView::updateUnmanagedViews()
{
  mDateNavigator->updateDayMatrix();
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
// If in agenda view, use the selected time and date from there.
// In all other cases, paste the event on the first day of the
// selection in the day matrix on the left

  QDate date;
  // create an invalid time to check if we got a new time for the eevent
  QTime time(-1,-1);

  KOAgendaView *aView = mViewManager->agendaView();
  if (aView && aView->selectionStart().isValid()) {
      date = aView->selectionStart().date();
      if (!aView->selectedIsAllDay())
        time = aView->selectionStart().time();
  } else {
    date = mNavigator->selectedDates().first();
  }

  DndFactory factory( mCalendar );
  Event *pastedEvent;
  if (time.isValid())
    pastedEvent = factory.pasteEvent( date, &time );
  else
    pastedEvent = factory.pasteEvent( date );
  changeEventDisplay( pastedEvent, KOGlobals::EVENTADDED );
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
}


void CalendarView::newEvent()
{
  // TODO: Replace this code by a common eventDurationHint of KOBaseView.
  KOAgendaView *aView = mViewManager->agendaView();
  if (aView) {
    if (aView->selectionStart().isValid()) {
      if (aView->selectedIsAllDay()) {
        newEvent(aView->selectionStart(),aView->selectionEnd(),true);
      } else {
        newEvent(aView->selectionStart(),aView->selectionEnd());
      }
      return;
    }
  }

  QDate date = mNavigator->selectedDates().first();

  newEvent( QDateTime( date, QTime( KOPrefs::instance()->mStartTime, 0, 0 ) ),
	    QDateTime( date, QTime( KOPrefs::instance()->mStartTime +
                       KOPrefs::instance()->mDefaultDuration, 0, 0 ) ) );
}

void CalendarView::newEvent(QDateTime fh)
{
  newEvent(fh,
           QDateTime(fh.addSecs(3600*KOPrefs::instance()->mDefaultDuration)));
}

void CalendarView::newEvent(QDate dt)
{
  newEvent(QDateTime(dt, QTime(0,0,0)),
           QDateTime(dt, QTime(0,0,0)), true);
}

void CalendarView::newEvent( const QString &text )
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent( text );
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

void CalendarView::newFloatingEvent()
{
  DateList tmpList = mNavigator->selectedDates();
  QDate date = tmpList.first();

  newEvent( QDateTime( date, QTime( 12, 0, 0 ) ),
            QDateTime( date, QTime( 12, 0, 0 ) ), true );
}


void CalendarView::editEvent( Event *event )
{
  kdDebug(5850) << "CalendarView::editEvent()" << endl;

  if ( !event ) return;

  if ( mDialogList.find( event ) != mDialogList.end() ) {
    kdDebug(5850) << "CalendarView::editEvent() in List" << endl;
    mDialogList[ event ]->reload();
    mDialogList[ event ]->raise();
    mDialogList[ event ]->show();
    return;
  }

  if ( event->isReadOnly() ) {
    showEvent( event );
    return;
  }

  kdDebug(5850) << "CalendarView::editEvent() new EventEditor" << endl;
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  mDialogList.insert( event, eventEditor );
  eventEditor->editEvent( event );
  eventEditor->show();
}

void CalendarView::editTodo( Todo *todo )
{
  if ( !todo ) return;
  kdDebug(5850) << "CalendarView::editTodo" << endl;

  if ( mDialogList.find( todo ) != mDialogList.end() ) {
    kdDebug(5850) << "Already in the list " << endl;
    mDialogList[todo]->reload();
    mDialogList[todo]->raise();
    mDialogList[todo]->show();
    return;
  }

  if ( todo->isReadOnly() ) {
    showTodo( todo );
    return;
  }

  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  kdDebug(5850) << "New editor" << endl;
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

void CalendarView::todoModified (Todo *event, int changed)
{
  if (mDialogList.find (event) != mDialogList.end ()) {
    kdDebug(5850) << "Todo modified and open" << endl;
    KOTodoEditor* temp = (KOTodoEditor *) mDialogList[event];
    temp->modified (changed);

  }

  mViewManager->updateView();
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
    QDate itemDate = mViewManager->currentSelectionDate();
    kdDebug(5850) << "Recurrence-Date: " << itemDate.toString() << endl;
    int km;
    if (!itemDate.isValid()) {
      kdDebug(5850) << "Date Not Valid" << endl;
      km = KMessageBox::warningContinueCancel(this,
        i18n("This event recurs over multiple dates. "
             "Are you sure you want to delete this event "
             "and all its recurrences?"),
             i18n("KOrganizer Confirmation"),i18n("Delete All"));
    } else {
      km = KMessageBox::warningYesNoCancel(this,
        i18n("This event recurs over multiple dates. "
             "Do you want to delete all it's recurrences, "
             "or only the current one on %1?" )
             .arg( KGlobal::locale()->formatDate(itemDate)),
             i18n("KOrganizer Confirmation"),i18n("Delete Current"),
             i18n("Delete All"));
    }
    switch(km) {

      case KMessageBox::No: // Continue // all
        if (anEvent->organizer()==KOPrefs::instance()->email() && anEvent->attendeeCount()>0)
          schedule(Scheduler::Cancel,anEvent);
        mCalendar->deleteEvent(anEvent);
        changeEventDisplay(anEvent,KOGlobals::EVENTDELETED);
        break;

// Disabled because it does not work (doesn't seem to be true anymore)
#if 1
      case KMessageBox::Yes: // just this one
        //QDate qd = mNavigator->selectedDates().first();
        //if (!qd.isValid()) {
        //  kdDebug(5850) << "no date selected, or invalid date" << endl;
        //  KNotifyClient::beep();
        //  return;
        //}
        //while (!anEvent->recursOn(qd)) qd = qd.addDays(1);
        if (itemDate!=QDate(1,1,1) || itemDate.isValid()) {
          anEvent->addExDate(itemDate);
          int duration = anEvent->recurrence()->duration();
          if ( duration > 0 ) {
            anEvent->recurrence()->setDuration( duration - 1 );
          }
          changeEventDisplay(anEvent, KOGlobals::EVENTEDITED);
        }
        break;
#endif
    }
  } else {
    if (KOPrefs::instance()->mConfirm) {
      switch (msgItemDelete()) {
        case KMessageBox::Continue: // OK
          if ( anEvent->organizer() == KOPrefs::instance()->email() &&
               anEvent->attendeeCount() > 0 ) {
	    schedule( Scheduler::Cancel,anEvent );
          }
          mHistory->recordDelete( anEvent );
          mCalendar->deleteEvent( anEvent );
          changeEventDisplay( anEvent, KOGlobals::EVENTDELETED );
          break;
      }
    } else {
      if ( anEvent->organizer() == KOPrefs::instance()->email() &&
           anEvent->attendeeCount() > 0 ) {
        schedule(Scheduler::Cancel,anEvent);
      }
      mHistory->recordDelete( anEvent );
      mCalendar->deleteEvent( anEvent );
      changeEventDisplay( anEvent, KOGlobals::EVENTDELETED );
    }
  }
}

bool CalendarView::deleteEvent(const QString &uid)
{
    Event *ev = mCalendar->event(uid);
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

  CalendarLocal cal_tmp;
  Event *event = 0;
  Event *ev = 0;
  if ( incidence && incidence->type() == "Event" ) {
    event = static_cast<Event *>(incidence);
    ev = new Event(*event);
    cal_tmp.addEvent(ev);
  }
  ICalFormat mForm;
  QString attachment = mForm.toString( &cal_tmp );
  delete(ev);

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
  Todo *todo = 0;

  if (incidence == 0) {
    incidence = mViewManager->currentView()->selectedIncidences().first();
    if (incidence == 0) {
      incidence = mTodoList->selectedIncidences().first();
    }
  }
  if ( incidence && incidence->type() == "Event" ) {
    event = static_cast<Event *>(incidence);
  } else {
    if ( incidence && incidence->type() == "Todo" ) {
      todo = static_cast<Todo *>(incidence);
    }
  }

  if (!event && !todo) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  PublishDialog *publishdlg = new PublishDialog();
  if (incidence->attendeeCount()>0) {
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      publishdlg->addAttendee( *it );
    }
  }
  bool send = true;
  if ( KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail ) {
    if ( publishdlg->exec() != QDialog::Accepted )
      send = false;
  }
  if ( send ) {
    OutgoingDialog *dlg = mDialogManager->outgoingDialog();
    if ( event ) {
      Event *ev = new Event(*event);
      ev->registerObserver(0);
      ev->clearAttendees();
      if (!dlg->addMessage(ev,Scheduler::Publish,publishdlg->addresses())) {
	delete(ev);
      }
    } else {
      if ( todo ) {
	Todo *ev = new Todo(*todo);
	ev->registerObserver(0);
	ev->clearAttendees();
	if (!dlg->addMessage(ev,Scheduler::Publish,publishdlg->addresses())) {
	  delete(ev);
	}
      }
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

void CalendarView::schedule_publish_freebusy(int daysToPublish)
{
  QDateTime start = QDateTime::currentDateTime();
  QDateTime end = start.addDays(daysToPublish);

  FreeBusy *freebusy = new FreeBusy(mCalendar, start, end);
  freebusy->setOrganizer(KOPrefs::instance()->email());

  kdDebug(5850) << "calendarview: schedule_publish_freebusy: startDate: "
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
  Todo *todo = 0;

  if (incidence == 0) {
    incidence = mViewManager->currentView()->selectedIncidences().first();
    if (incidence == 0) {
      incidence = mTodoList->selectedIncidences().first();
    }
  }
  if ( incidence && incidence->type() == "Event" ) {
    event = static_cast<Event *>(incidence);
  }
  if ( incidence && incidence->type() == "Todo" ) {
    todo = static_cast<Todo *>(incidence);
  }

  if (!event && !todo) {
    KMessageBox::sorry(this,i18n("No event selected."));
    return;
  }

  if( incidence->attendeeCount() == 0 && method != Scheduler::Publish ) {
    KMessageBox::sorry(this,i18n("The event has no attendees."));
    return;
  }

  Event *ev = 0;
  if (event) ev = new Event(*event);
  Todo *to = 0;
  if (todo) to = new Todo(*todo);

  if (method == Scheduler::Reply || method == Scheduler::Refresh) {
    Attendee *me = incidence->attendeeByMails(KOPrefs::instance()->mAdditionalMails,KOPrefs::instance()->email());
    if (!me) {
      KMessageBox::sorry(this,i18n("Could not find your attendee entry. Please check the emails."));
      return;
    }
    if (me->status()==Attendee::NeedsAction && me->RSVP() && method==Scheduler::Reply) {
      StatusDialog *statdlg = new StatusDialog(this);
      if (!statdlg->exec()==QDialog::Accepted) return;
      me->setStatus( statdlg->status() );
      delete(statdlg);
    }
    Attendee *menew = new Attendee(*me);
    if (ev) {
      ev->clearAttendees();
      ev->addAttendee(menew,false);
    } else {
      if (to) {
	todo->clearAttendees();
	todo->addAttendee(menew,false);
      }
    }
  }

  OutgoingDialog *dlg = mDialogManager->outgoingDialog();
  if (ev) {
    if ( !dlg->addMessage(ev,method) ) delete(ev);
  } else {
    if (to) {
      if ( !dlg->addMessage(to,method) ) delete(to);
    }
  }
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

  DateList tmpDateList = mNavigator->selectedDates();
  mCalPrinter->print(CalPrinter::Month,
		     tmpDateList.first(), tmpDateList.last());
#endif
}

void CalendarView::printPreview()
{
#ifndef KORG_NOPRINTER
  kdDebug(5850) << "CalendarView::printPreview()" << endl;

  createPrinter();

  DateList tmpDateList = mNavigator->selectedDates();

  mViewManager->currentView()->printPreview(mCalPrinter,tmpDateList.first(),
                             tmpDateList.last());
#endif
}

void CalendarView::exportICalendar()
{
  QString filename = KFileDialog::getSaveFileName("icalout.ics",i18n("*.ics|ICalendars"),this);

  // Force correct extension
  if (filename.right(4) != ".ics") filename += ".ics";

  FileStorage storage( mCalendar, filename, new ICalFormat );
  storage.save();
}

void CalendarView::exportVCalendar()
{
  if (mCalendar->journals().count() > 0) {
    int result = KMessageBox::warningContinueCancel(this,
        i18n("The journal entries can not be exported to a vCalendar file."),
        i18n("Data Loss Warning"),i18n("Proceed"),"dontaskVCalExport",
        true);
    if (result != KMessageBox::Continue) return;
  }

  QString filename = KFileDialog::getSaveFileName("vcalout.vcs",i18n("*.vcs|VCalendars"),this);

  // Force correct extension
  if (filename.right(4) != ".vcs") filename += ".vcs";

  FileStorage storage( mCalendar, filename, new VCalFormat );
  storage.save();
}

void CalendarView::eventUpdated(Incidence *)
{
  setModified();
  // Don't call updateView here. The code, which has caused the update of the
  // event is responsible for updating the view.
//  updateView();
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
    return;
  } else {
    if  ( incidence && incidence->type() == "Todo" ) {
      emit todoSelected( true );
      Todo *event = static_cast<Todo *>( incidence );
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
      return;
    } else {
     emit todoSelected( false );
     emit organizerEventsSelected(false);
     emit groupEventsSelected(false);
    }
    return;
  }

/*  if  ( incidence && incidence->type() == "Todo" ) {
    emit todoSelected( true );
  } else {
    emit todoSelected( false );
  }*/
}


void CalendarView::checkClipboard()
{
#ifndef KORG_NODND
  if (ICalDrag::canDecode(QApplication::clipboard()->data())) {
    kdDebug(5850) << "CalendarView::checkClipboard() true" << endl;
    emit pasteEnabled(true);
  } else {
    kdDebug(5850) << "CalendarView::checkClipboard() false" << endl;
    emit pasteEnabled(false);
  }
#endif
}

void CalendarView::showDates(const DateList &selectedDates)
{
//  kdDebug(5850) << "CalendarView::selectDates()" << endl;

  if ( mViewManager->currentView() ) {
    updateView( selectedDates.first(), selectedDates.last() );
  } else {
    mViewManager->showAgendaView();
  }
}

void CalendarView::editFilters()
{
//  kdDebug(5850) << "CalendarView::editFilters()" << endl;

  CalFilter *filter = mFilters.first();
  while(filter) {
    kdDebug(5850) << " Filter: " << filter->name() << endl;
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

  Event::List events = mCalendar->events();
  for(uint i=0; i<events.count(); ++i) {
    (*events.at(i))->setOrganizer(KOPrefs::instance()->email());
    (*events.at(i))->recreate();
    (*events.at(i))->setReadOnly(false);
  }

  Todo::List todos = mCalendar->todos();
  for(uint i=0; i<todos.count(); ++i) {
    (*todos.at(i))->setOrganizer(KOPrefs::instance()->email());
    (*todos.at(i))->recreate();
    (*todos.at(i))->setReadOnly(false);
  }

  Journal::List journals = mCalendar->journals();
  for(uint i=0; i<journals.count(); ++i) {
    (*journals.at(i))->setOrganizer(KOPrefs::instance()->email());
    (*journals.at(i))->recreate();
    (*journals.at(i))->setReadOnly(false);
  }

  updateView();
}

void CalendarView::showIntro()
{
  kdDebug(5850) << "To be implemented." << endl;
}

QWidgetStack *CalendarView::viewStack()
{
  return mRightFrame;
}

QWidget *CalendarView::leftFrame()
{
  return mLeftFrame;
}

DateNavigator *CalendarView::dateNavigator()
{
  return mNavigator;
}

void CalendarView::addView(KOrg::BaseView *view)
{
  mViewManager->addView(view);
}

void CalendarView::showView(KOrg::BaseView *view)
{
  mViewManager->showView(view);
}

void CalendarView::addExtension( CalendarViewExtension::Factory *factory )
{
  CalendarViewExtension *extension = factory->create( mLeftSplitter );

  mExtensions.append( extension );
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

void CalendarView::showIncidence(Incidence *incidence)
{
  if ( incidence ) {
    ShowIncidenceVisitor v;
    v.act( incidence, this );
  }
}

void CalendarView::editIncidence(Incidence *incidence)
{
  if ( incidence ) {
    EditIncidenceVisitor v;
    v.act( incidence, this );
  }
}

void CalendarView::deleteIncidence(Incidence *incidence)
{
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
    Todo::List todoCal;
    Incidence::List rel;
    bool childDelete = false;
    bool deletedOne = true;
    while (deletedOne) {
      todoCal.clear();
      todoCal = calendar()->todos();
      deletedOne = false;
      Todo::List::ConstIterator it;
      for ( it = todoCal.begin(); it != todoCal.end(); ++it ) {
        Todo *aTodo = *it;
        if (aTodo->isCompleted()) {
          rel = aTodo->relations();
          if (!rel.isEmpty()) {
            Incidence::List::ConstIterator it2;
            for ( it2 = rel.begin(); it2 != rel.end(); ++it2 ) {
              Incidence *rIncidence = *it2;
              if (rIncidence->type()=="Todo") {
                Todo *rTodo = static_cast<Todo*>(rIncidence);
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

void CalendarView::slotCalendarChanged()
{
  kdDebug(5850) << "CalendarView::slotCalendarChanged()" << endl;
}

NavigatorBar *CalendarView::navigatorBar()
{
  return mNavigatorBar;
}

void CalendarView::importQtopia( const QString &categories,
                                 const QString &datebook,
                                 const QString &todolist )
{
  QtopiaFormat qtopiaFormat;
  if ( !categories.isEmpty() ) qtopiaFormat.load( mCalendar, categories );
  if ( !datebook.isEmpty() ) qtopiaFormat.load( mCalendar, datebook );
  if ( !todolist.isEmpty() ) qtopiaFormat.load( mCalendar, todolist );
  updateView();
}

#include "calendarview.moc"
