/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998, 1999
    Preston Brown (preston.brown@yale.edu)
    Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
    Ian Dawes (iadawes@globalserve.net)
    Laszlo Boloni (boloni@cs.purdue.edu)

    Copyright (c) 2000, 2001, 2002, 2003, 2004
    Cornelius Schumacher <schumacher@kde.org>

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

#include "calendarview.h"
#include "mailscheduler.h"

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
#include "kojournaleditor.h"
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
#include "datenavigatorcontainer.h"
#include "kotodoview.h"
#include "datenavigator.h"
#include "resourceview.h"
#include "navigatorbar.h"
#include "history.h"
#include "kogroupware.h"
#include "freebusymanager.h"
#include "komonthview.h"
#include "datechecker.h"

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
#include <libkcal/calendarnull.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kconfig.h>
#include <krun.h>
#include <kdirwatch.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qmultilineedit.h>
#include <qtimer.h>
#include <qwidgetstack.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qlayout.h>
#ifndef KORG_NOSPLITTER
#include <qsplitter.h>
#endif

#include <stdlib.h>

using namespace KOrg;

CalendarView::CalendarView( QWidget *parent, const char *name )
  : CalendarViewBase( parent, name ),
    mHistory( 0 ),
    mCalendar( CalendarNull::self() )
{
  kdDebug(5850) << "CalendarView::CalendarView( Calendar )" << endl;

  mViewManager = new KOViewManager( this );
  mDialogManager = new KODialogManager( this );

  mModified = false;
  mReadOnly = false;
  mSelectedIncidence = 0;

  mCalPrinter = 0;

  mFilters.setAutoDelete( true );

  mExtensions.setAutoDelete( true );

  mNavigator = new DateNavigator( this );
  mDateChecker = new DateChecker( this );

  QBoxLayout *topLayout = new QVBoxLayout( this );

#ifndef KORG_NOSPLITTER
  // create the main layout frames.
  mPanner = new QSplitter( QSplitter::Horizontal, this,
                           "CalendarView::Panner" );
  topLayout->addWidget( mPanner );

  mLeftSplitter = new QSplitter( QSplitter::Vertical, mPanner,
                                 "CalendarView::LeftFrame" );
//  mPanner->setResizeMode( mLeftSplitter, QSplitter::Stretch );

  mDateNavigator = new DateNavigatorContainer( mLeftSplitter,
                                               "CalendarView::DateNavigator" );
//  mLeftSplitter->setResizeMode( mDateNavigator, QSplitter::Stretch );
  mLeftSplitter->setCollapsible( mDateNavigator, true );
  mTodoList = new KOTodoView( CalendarNull::self(), mLeftSplitter, "todolist" );
  mFilterView = new KOFilterView( &mFilters, mLeftSplitter,
                                  "CalendarView::FilterView" );

  QWidget *rightBox = new QWidget( mPanner );
  QBoxLayout *rightLayout = new QVBoxLayout( rightBox );

  mNavigatorBar = new NavigatorBar( rightBox );
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

  mDateNavigator = new KDateNavigator( leftFrame, true,
                                       "CalendarView::DateNavigator",
                                       QDate::currentDate() );
  mTodoList = new KOTodoView( CalendarNull::self(), leftFrame, "todolist" );
  mFilterView = new KOFilterView( &mFilters, leftFrame,
                                  "CalendarView::FilterView" );

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
  connect( mNavigatorBar, SIGNAL( goMonth(int) ),
           mNavigator, SLOT( selectMonth(int) ) );

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
  connect( mDateNavigator, SIGNAL( goMonth(int) ),
           mNavigator, SLOT( selectMonth(int) ) );

  connect( mDateNavigator, SIGNAL( goPrevious() ),
           mNavigator, SLOT( selectPrevious() ) );
  connect( mDateNavigator, SIGNAL( goNext() ),
           mNavigator, SLOT( selectNext() ) );

  connect( mDateNavigator, SIGNAL( datesSelected( const KCal::DateList & ) ),
           mNavigator, SLOT( selectDates( const KCal::DateList & ) ) );

  connect( mDateNavigator, SIGNAL( incidenceDropped( Incidence * ) ),
           SLOT( incidenceAdded( Incidence * ) ) );
  connect( mDateNavigator, SIGNAL( incidenceDroppedMove( Incidence *, Incidence * ) ),
           SLOT( incidenceChanged( Incidence *, Incidence *) ) );

  connect( mDateChecker, SIGNAL( dayPassed( QDate ) ),
           mTodoList, SLOT( dayPassed( QDate ) ) );
  connect( mDateChecker, SIGNAL( dayPassed( QDate ) ),
           SIGNAL( dayPassed( QDate ) ) );
  connect( mDateChecker, SIGNAL( dayPassed( QDate ) ),
           mDateNavigator, SLOT( updateToday() ) );

  connect( this, SIGNAL( configChanged() ),
           mDateNavigator, SLOT( updateConfig() ) );

  mViewManager->connectTodoView( mTodoList );
  mViewManager->connectView( mTodoList );

  connect( mFilterView, SIGNAL( filterChanged() ), SLOT( updateFilter() ) );
  connect( mFilterView, SIGNAL( editFilters() ), SLOT( editFilters() ) );
  // Hide filter per default
  mFilterView->hide();

  KDirWatch *messageWatch = new KDirWatch();
  messageWatch->addDir( locateLocal( "data", "korganizer/income/" ) );
  connect( messageWatch, SIGNAL( dirty( const QString & ) ),
           SLOT( lookForIncomingMessages() ) );

  // We should think about seperating startup settings and configuration change.
  updateConfig();

  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( checkClipboard() ) );

  connect( mTodoList, SIGNAL( incidenceSelected( Incidence * ) ),
           SLOT( processTodoListSelection( Incidence * ) ) );
  disconnect( mTodoList, SIGNAL( incidenceSelected( Incidence * ) ),
           this, SLOT( processMainViewSelection( Incidence * ) ) );

  kdDebug(5850) << "CalendarView::CalendarView() done" << endl;
}

CalendarView::~CalendarView()
{
  kdDebug(5850) << "~CalendarView()" << endl;

  mCalendar->unregisterObserver( this );

  delete mDialogManager;
  delete mViewManager;

  kdDebug(5850) << "~CalendarView() done" << endl;
}

void CalendarView::setCalendar( Calendar *cal )
{
  mCalendar = cal;

  delete mHistory;
  mHistory = new History( mCalendar );
  connect( mHistory, SIGNAL( undone() ), SLOT( updateView() ) );
  connect( mHistory, SIGNAL( redone() ), SLOT( updateView() ) );

  mCalendar->registerObserver( this );

  mDateNavigator->setCalendar( mCalendar );

  mTodoList->setCalendar( mCalendar );
}

Calendar *CalendarView::calendar()
{
  if ( mCalendar ) return mCalendar;
  else return CalendarNull::self();
}

KOViewManager *CalendarView::viewManager()
{
  return mViewManager;
}

KODialogManager *CalendarView::dialogManager()
{
  return mDialogManager;
}

KOIncidenceEditor *CalendarView::editorDialog( Incidence *incidence )
{
  if (mDialogList.find(incidence) != mDialogList.end ())
    return mDialogList[incidence];
  else return 0;
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

  FileStorage storage( mCalendar );
  storage.setFileName( filename );

  if ( storage.load() ) {
    if ( merge ) setModified( true );
    else {
      setModified( false );
      mViewManager->setDocumentId( filename );
      mDialogManager->setDocumentId( filename );
      mTodoList->setDocumentId( filename );
    }
    updateCategories();
    updateView();
    return true;
  } else {
    // while failing to load, the calendar object could
    // have become partially populated.  Clear it out.
    if ( !merge ) mCalendar->close();

    KMessageBox::error(this,i18n("Could not load calendar '%1'.").arg(filename));

    return false;
  }
}

bool CalendarView::saveCalendar( const QString& filename )
{
  kdDebug(5850) << "CalendarView::saveCalendar(): " << filename << endl;

  // Store back all unsaved data into calendar object
  mViewManager->currentView()->flushView();

  FileStorage storage( mCalendar );
  storage.setFileName( filename );
  storage.setSaveFormat( new ICalFormat );

  bool success = storage.save();

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

  KConfig *config = KOGlobals::self()->config();

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

  KConfig *config = KOGlobals::self()->config();

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


void CalendarView::goDate( const QDate& date )
{
  mNavigator->selectDate( date );
}

void CalendarView::goToday()
{
  mNavigator->selectToday();
}

void CalendarView::goNext()
{
  if (dynamic_cast<KOMonthView*>(mViewManager->currentView() ) )
    mNavigator->selectNextMonth();
  else
    mNavigator->selectNext();
}

void CalendarView::goPrevious()
{
  if (dynamic_cast<KOMonthView*>(mViewManager->currentView() ) )
    mNavigator->selectPreviousMonth();
  else
    mNavigator->selectPrevious();
}

void CalendarView::updateConfig()
{
  kdDebug(5850) << "CalendarView::updateConfig()" << endl;

  emit configChanged();

  QString tz(mCalendar->timeZoneId());
  // Only set a new time zone if it changed. This prevents the window
  // from being modified on start
  if ( tz != KOPrefs::instance()->mTimeZoneId )
    mCalendar->setTimeZoneId(KOPrefs::instance()->mTimeZoneId);
  // To make the "fill window" configurations work
  mViewManager->raiseCurrentView();
}


void CalendarView::incidenceAdded( Incidence *incidence )
{
  setModified( true );
  history()->recordAdd( incidence );
  changeIncidenceDisplay( incidence, KOGlobals::INCIDENCEADDED );
  updateUnmanagedViews();
}

void CalendarView::incidenceChanged( Incidence *oldIncidence,
                                     Incidence *newIncidence )
{
  incidenceChanged( oldIncidence, newIncidence, KOGlobals::UNKNOWN_MODIFIED );
}

void CalendarView::incidenceChanged( Incidence *oldIncidence,
                                     Incidence *newIncidence, int what )
{
  // TODO: Make use of the what flag, which indicates which parts of the incidence have changed!
  KOIncidenceEditor *tmp = editorDialog( newIncidence );
  if ( tmp ) {
    kdDebug(5850) << "Incidence modified and open" << endl;
    tmp->modified( what );
  }
  setModified( true );
  history()->recordEdit( oldIncidence, newIncidence );
//  calendar()->endChange( newIncidence );
  changeIncidenceDisplay( newIncidence, KOGlobals::INCIDENCEEDITED );
  updateUnmanagedViews();
}

void CalendarView::incidenceToBeDeleted( Incidence *incidence )
{
  KOIncidenceEditor *tmp = editorDialog( incidence );
  if (tmp) {
    kdDebug(5850) << "Incidence to be deleted and open in editor" << endl;
    tmp->delayedDestruct();
  }
  setModified( true );
  history()->recordDelete( incidence );
//  changeIncidenceDisplay( incidence, KOGlobals::INCIDENCEDELETED );
  updateUnmanagedViews();
}

void CalendarView::incidenceDeleted( Incidence *incidence )
{
  changeIncidenceDisplay( incidence, KOGlobals::INCIDENCEDELETED );
  updateUnmanagedViews();
}

void CalendarView::startMultiModify( const QString &text )
{
  history()->startMultiModify( text );
}

void CalendarView::endMultiModify()
{
  history()->endMultiModify();
}


void CalendarView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  mDateNavigator->updateView();
  mDialogManager->updateSearchDialog();

  if ( incidence ) {
    // If there is an event view visible update the display
    mViewManager->currentView()->changeIncidenceDisplay( incidence, action );
    if ( mTodoList ) mTodoList->changeIncidenceDisplay( incidence, action );
  } else {
    mViewManager->currentView()->updateView();
    if ( mTodoList ) mTodoList->updateView();
  }
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
      i18n("KOrganizer Confirmation"),KGuiItem(i18n("Delete"),"editdelete"));
}


void CalendarView::edit_cut()
{
  Incidence *incidence = selectedIncidence();

  if (!incidence) {
    KNotifyClient::beep();
    return;
  }
  DndFactory factory( mCalendar );
  if ( incidence->type() == "Event" ) {
    Event *anEvent = static_cast<Event *>(incidence);
    incidenceToBeDeleted( anEvent );
    factory.cutEvent(anEvent);
    incidenceDeleted( anEvent );
  } else if ( incidence->type() == "Todo" ) {
    Todo *anTodo = static_cast<Todo *>(incidence);
    incidenceToBeDeleted( anTodo );
    factory.cutTodo( anTodo );
    incidenceDeleted( anTodo );
  } else {
    KNotifyClient::beep();
  }
}

void CalendarView::edit_copy()
{
  Incidence *incidence = selectedIncidence();

  if (!incidence) {
    KNotifyClient::beep();
    return;
  }
  DndFactory factory( mCalendar );
  if ( incidence->type() == "Event" ) {
    Event *anEvent = static_cast<Event *>(incidence);
    factory.copyEvent( anEvent );
  } else if ( incidence->type() == "Todo" ) {
    Todo *anTodo = static_cast<Todo *>(incidence);
    // TODO: Why should we need to remove the recurrence from a todo when it is copied?
    // Note that this removes the recurrence from the original todo, not only from the todo in the clipboard!
/*    if (anTodo->doesRecur())
      anTodo->recurrence()->unsetRecurs(); // avoid 'forking'*/
    factory.copyTodo( anTodo );
  } else {
    KNotifyClient::beep();
  }

  // Don't clear todo selection when copying, as this is inconsistent with the rest of KDE.
/*  // Clear selection to avoid accidental creation subtodo's.
  // 1) Left todolist
  mTodoList->clearSelection();
  // 2) Fullscreen todolist, test if active
  if ( mViewManager->todoView() )
    mViewManager->todoView()->clearSelection();
*/
}

void CalendarView::edit_paste()
{
// If in agenda view, use the selected time and date from there.
// In all other cases, paste the event on the first day of the
// selection in the day matrix on the left

  QDate date;
  // create an invalid time to check if we got a new time for the eevent
  QTime time(-1,-1);
  QDateTime startDT, endDT;
  bool useEndTime = false;

  KOAgendaView *aView = mViewManager->agendaView();
  if (aView && aView->selectionStart().isValid()) {
      date = aView->selectionStart().date();
    startDT = aView->selectionStart();
    endDT = aView->selectionEnd();
    useEndTime = !aView->selectedIsSingleCell();
    if (!aView->selectedIsAllDay()) {
        time = aView->selectionStart().time();
    }

  } else {
    date = mNavigator->selectedDates().first();
  }

  DndFactory factory( mCalendar );
  Incidence *pastedIncidence;
  if (time.isValid())
    pastedIncidence = factory.pasteIncidence( date, &time );
  else
    pastedIncidence = factory.pasteIncidence( date );
  if ( !pastedIncidence ) return;

  if (pastedIncidence->type() == "Event" ) {

    Event* pastedEvent = static_cast<Event*>(pastedIncidence);
    // only use selected area if event is of the same type (all-day or non-all-day
    // as the current selection is
    if ( aView && endDT.isValid() && useEndTime ) {
      if ( (pastedEvent->doesFloat() && aView->selectedIsAllDay()) ||
           (!pastedEvent->doesFloat() && ! aView->selectedIsAllDay()) ) {
        pastedEvent->setDtEnd(endDT);
      }
    }
    incidenceAdded( pastedEvent );

  } else if ( pastedIncidence->type() == "Todo" ) {
    Todo* pastedTodo = static_cast<Todo*>(pastedIncidence);
    Todo* _selectedTodo = selectedTodo();
    if ( _selectedTodo )
      pastedTodo->setRelatedTo( _selectedTodo );
    incidenceAdded( pastedTodo );
  }
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
}


void CalendarView::newEvent()
{
  kdDebug() << "CalendarView::newEvent()" << endl;
  QDate date = mNavigator->selectedDates().first();
  QTime startTime = KOPrefs::instance()->mStartTime.time();
  QDateTime startDt( date, startTime );
  QTime defaultDuration( KOPrefs::instance()->mDefaultDuration.time() );
  QTime endTime( startTime.addSecs( defaultDuration.hour()*3600 +
     defaultDuration.minute()*60 + defaultDuration.second() ) );
  QDateTime endDt( date, endTime );
  bool allDay = false;

  // let the current view change the default start/end datetime
  mViewManager->currentView()->eventDurationHint( startDt, endDt, allDay );

  if ( allDay ) {
    newEvent( startDt, endDt, true );
  } else {
    newEvent( startDt, endDt );
  }
}

void CalendarView::newEvent(QDateTime fh)
{
  QTime defaultDuration( KOPrefs::instance()->mDefaultDuration.time() );
  QDateTime endTime = fh.addSecs( defaultDuration.hour()*3600 +
     defaultDuration.minute()*60 + defaultDuration.second() );
  newEvent( fh, endTime );
}

void CalendarView::newEvent(QDate dt)
{
  QTime startTime = KOPrefs::instance()->mStartTime.time();
  QTime defaultDuration( KOPrefs::instance()->mDefaultDuration.time() );
  QTime endTime = startTime.addSecs( defaultDuration.hour()*3600 +
                  defaultDuration.minute()*60 + defaultDuration.second() );
  newEvent(QDateTime(dt, startTime),
           QDateTime(dt, endTime), true);
}

void CalendarView::newEvent( const QString &text )
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent( text );
  mDialogManager->connectTypeAhead( eventEditor, viewManager()->agendaView() );
  eventEditor->show();
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QString &attachment )
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent( summary, description, attachment );
  eventEditor->show();
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QString &attachment, const QStringList &attendees )
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent( summary, description, attachment, attendees );
  eventEditor->show();
}

void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint, bool allDay)
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent(fromHint,toHint,allDay);
  mDialogManager->connectTypeAhead( eventEditor, viewManager()->agendaView() );
  eventEditor->show();
}

void CalendarView::newTodo( const QString &text )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  todoEditor->newTodo( text );
  todoEditor->show();
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                             const QString &attachment )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  todoEditor->newTodo( summary, description, attachment );
  todoEditor->show();
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                            const QString &attachment, const QStringList &attendees )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  todoEditor->newTodo( summary, description, attachment, attendees );
  todoEditor->show();
}

void CalendarView::newTodo()
{
  kdDebug() << "CalendarView::newTodo()" << endl;
  QDateTime dtDue;
  bool allday = true;
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  if ( mViewManager->currentView()->isEventView() ) {
    dtDue.setDate( mNavigator->selectedDates().first() );
    QDateTime dtDummy = QDateTime::currentDateTime();
    mViewManager->currentView()->
      eventDurationHint( dtDue , dtDummy , allday );
  }
  else
    dtDue = QDateTime::currentDateTime().addDays( 7 );
  todoEditor->newTodo(dtDue,0,allday);
  todoEditor->show();
}

void CalendarView::newTodo( QDate date )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  todoEditor->newTodo( QDateTime( date, QTime::currentTime() ), 0, true );
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
  KOIncidenceEditor*tmp = editorDialog( event );
  if (tmp) {
    kdDebug(5850) << "CalendarView::editEvent() in List" << endl;
    tmp->reload();
    tmp->raise();
    tmp->show();
    return;
  }

  if ( event->isReadOnly() ) {
    showEvent( event );
    return;
  }

  if ( !mCalendar->beginChange( event ) ) {
    warningChangeFailed( event );
    return;
  }

  kdDebug(5850) << "CalendarView::editEvent() new EventEditor" << endl;
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  mDialogList.insert( event, eventEditor );
  eventEditor->editIncidence( event );
  eventEditor->show();
}

void CalendarView::editTodo( Todo *todo )
{
  if ( !todo ) return;
  kdDebug(5850) << "CalendarView::editTodo" << endl;

  KOIncidenceEditor *tmp = editorDialog( todo );
  if (tmp) {
    kdDebug(5850) << "Already in the list " << endl;
    tmp->reload();
    tmp->raise();
    tmp->show();
    return;
  }

  if ( todo->isReadOnly() ) {
    showTodo( todo );
    return;
  }

  if ( !mCalendar->beginChange( todo ) ) {
    warningChangeFailed( todo );
    return;
  }

  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  kdDebug(5850) << "New editor" << endl;
  mDialogList.insert( todo, todoEditor );
  todoEditor->editIncidence( todo );
  todoEditor->show();
}

void CalendarView::editJournal( Journal *journal )
{
  if ( !journal ) return;
  kdDebug(5850) << "CalendarView::editJournal" << endl;

  KOIncidenceEditor *tmp = editorDialog( journal );
  if ( tmp ) {
    kdDebug(5850) << "Already in the list " << endl;
    tmp->reload();
    tmp->raise();
    tmp->show();
    return;
  }

  if ( journal->isReadOnly() ) {
    showJournal( journal );
    return;
  }

  if ( !mCalendar->beginChange( journal ) ) {
    warningChangeFailed( journal );
    return;
  }

  KOJournalEditor *journalEditor = mDialogManager->getJournalEditor();
  kdDebug(5850) << "New editor" << endl;
  mDialogList.insert( journal, journalEditor );
  journalEditor->editIncidence( journal );
  journalEditor->show();
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

void CalendarView::showJournal(Journal *journal)
{
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
  eventViewer->setJournal(journal);
  eventViewer->show();
}

void CalendarView::appointment_show()
{
  Incidence *incidence = selectedIncidence();
  if (incidence)
    showIncidence( incidence );
  else
    KNotifyClient::beep();
}

void CalendarView::appointment_edit()
{
  Incidence *incidence = selectedIncidence();
  if (incidence)
    editIncidence( incidence );
  else
    KNotifyClient::beep();
}

void CalendarView::appointment_delete()
{
  Incidence *incidence = selectedIncidence();
  if (incidence)
    deleteIncidence( incidence );
  else
    KNotifyClient::beep();
}

void CalendarView::todo_unsub()
{
  Todo *anTodo = selectedTodo();
  if (!anTodo) return;
  if (!anTodo->relatedTo()) return;
  Todo *oldTodo = anTodo->clone();
  anTodo->relatedTo()->removeRelation(anTodo);
  anTodo->setRelatedTo(0);
  anTodo->setRelatedToUid("");
  incidenceChanged( oldTodo, anTodo );
  delete oldTodo;
  setModified(true);
  updateView();
}

void CalendarView::deleteTodo(Todo *todo)
{
  if ( !todo ) {
    KNotifyClient::beep();
    return;
  }
  if (KOPrefs::instance()->mConfirm && (!KOPrefs::instance()->mUseGroupwareCommunication ||
                                        KOPrefs::instance()->thatIsMe( todo->organizer() ))) {
    switch (msgItemDelete()) {
      case KMessageBox::Continue: // OK
        if (!todo->relations().isEmpty()) {
          KMessageBox::sorry(this,i18n("Cannot delete To-Do which has children."),
                         i18n("Delete To-Do"));
        } else {
          bool doDelete = true;
          if( KOPrefs::instance()->mUseGroupwareCommunication ) {
            doDelete = KOGroupware::instance()->sendICalMessage( this, KCal::Scheduler::Cancel, todo, true );
          }
          if( doDelete ) {
            incidenceToBeDeleted( todo );
            calendar()->deleteTodo(todo);
            incidenceDeleted( todo );
          }
        }
        break;
    } // switch
  } else {
    if (!todo->relations().isEmpty()) {
        KMessageBox::sorry(this,i18n("Cannot delete To-Do which has children."),
                         i18n("Delete To-Do"));
    } else {
            bool doDelete = true;
      if( KOPrefs::instance()->mUseGroupwareCommunication ) {
        doDelete = KOGroupware::instance()->sendICalMessage( this, KCal::Scheduler::Cancel, todo, true );
      }
      if( doDelete ) {
        incidenceToBeDeleted( todo );
        calendar()->deleteTodo(todo);
        incidenceDeleted( todo );
      }
    }
  }
}

void CalendarView::deleteJournal(Journal *journal)
{
  if ( !journal ) {
    KNotifyClient::beep();
    return;
  }
  if (KOPrefs::instance()->mConfirm && (!KOPrefs::instance()->mUseGroupwareCommunication ||
                                        KOPrefs::instance()->thatIsMe( journal->organizer() ))) {
    switch (msgItemDelete()) {
      case KMessageBox::Continue: // OK
        bool doDelete = true;
        if( KOPrefs::instance()->mUseGroupwareCommunication ) {
          doDelete = KOGroupware::instance()->sendICalMessage( this, KCal::Scheduler::Cancel, journal, true );
        }
        if( doDelete ) {
          incidenceToBeDeleted( journal );
          calendar()->deleteJournal( journal );
          incidenceDeleted( journal );
        }
        break;
    } // switch
  } else {
      bool doDelete = true;
      if( KOPrefs::instance()->mUseGroupwareCommunication ) {
        doDelete = KOGroupware::instance()->sendICalMessage( this, KCal::Scheduler::Cancel, journal, true );
      }
      if( doDelete ) {
        incidenceToBeDeleted( journal );
        calendar()->deleteJournal( journal );
        incidenceDeleted( journal );
      }
    }
}

void CalendarView::deleteEvent(Event *anEvent)
{
  if (!anEvent) {
    KNotifyClient::beep();
    return;
  }

  if (anEvent->doesRecur()) {
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
    bool doDelete = true;
    switch(km) {
      case KMessageBox::No: // Continue // all
      case KMessageBox::Continue:
        if (KOPrefs::instance()->thatIsMe( anEvent->organizer() ) && anEvent->attendeeCount()>0
            && !KOPrefs::instance()->mUseGroupwareCommunication) {
          schedule(Scheduler::Cancel,anEvent);
        } else if( KOPrefs::instance()->mUseGroupwareCommunication ) {
          doDelete = KOGroupware::instance()->sendICalMessage( this, KCal::Scheduler::Cancel, anEvent, true );
        }
        if( doDelete ) {
          incidenceToBeDeleted( anEvent );
          mCalendar->deleteEvent(anEvent);
          incidenceDeleted( anEvent );
        }
        break;

      case KMessageBox::Yes: // just this one
        if ( itemDate.isValid()) {
          Event*oldEvent = anEvent->clone();
          anEvent->addExDate(itemDate);
          incidenceChanged( oldEvent, anEvent );
        }
        break;
      // TODO_RK: Find a proper dialogbox with four buttons, then change the 9999
      // to the actual code of the "delete only future items" button
      case 9999: // all future items
        Recurrence *recur = anEvent->recurrence();
        if ( recur ) {
          Event*oldEvent = anEvent->clone();
          recur->setEndDate( itemDate.addDays(-1) );
          incidenceChanged( oldEvent, anEvent );
        }
        break;
    }
  } else {
    bool userIsOrganizer = KOPrefs::instance()->thatIsMe( anEvent->organizer() );
    if (KOPrefs::instance()->mConfirm && (!KOPrefs::instance()->mUseGroupwareCommunication ||
                                          userIsOrganizer)) {
      bool doDelete = true;
      switch (msgItemDelete()) {
        case KMessageBox::Continue: // OK
          incidenceToBeDeleted( anEvent );
          if ( userIsOrganizer &&
               anEvent->attendeeCount() > 0 &&
               !KOPrefs::instance()->mUseGroupwareCommunication ) {
            schedule( Scheduler::Cancel,anEvent );
          } else if( KOPrefs::instance()->mUseGroupwareCommunication ) {
            doDelete = KOGroupware::instance()->sendICalMessage( this, KCal::Scheduler::Cancel, anEvent, true );
          }
          if( doDelete ) {
            mCalendar->deleteEvent( anEvent );
            incidenceDeleted( anEvent );
          }
          break;
      }
    } else {
      bool doDelete = true;
      if ( userIsOrganizer &&
           anEvent->attendeeCount() > 0 &&
           !KOPrefs::instance()->mUseGroupwareCommunication ) {
        schedule(Scheduler::Cancel,anEvent);
      }else if( KOPrefs::instance()->mUseGroupwareCommunication ) {
        doDelete = KOGroupware::instance()->sendICalMessage( this, KCal::Scheduler::Cancel, anEvent, true );
      }
      if( doDelete ) {
        incidenceToBeDeleted( anEvent );
        mCalendar->deleteEvent( anEvent );
        incidenceDeleted( anEvent );
      }
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



void CalendarView::toggleAlarm( Incidence *incidence )
{
  if ( !incidence ) {
    kdDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  Incidence*oldincidence = incidence->clone();

// TODO: deal correctly with multiple alarms
  Alarm::List alarms = incidence->alarms();
  Alarm::List::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it )
    (*it)->toggleAlarm();
  if (alarms.isEmpty()) {
    // Add an alarm if it didn't have one
    Alarm*alm = incidence->newAlarm();
    alm->setEnabled(true);
  }
  emit incidenceChanged( oldincidence, incidence );
  delete oldincidence;

//  mClickedItem->updateIcons();
}

/*****************************************************************************/

void CalendarView::action_mail()
{
#ifndef KORG_NOMAIL
  KOMailClient mailClient;

  Incidence *incidence = currentSelection();

  if (!incidence) {
    KMessageBox::sorry(this,i18n("Cannot generate mail:\nNo event selected."));
    return;
  }
  if(incidence->attendeeCount() == 0 ) {
    KMessageBox::sorry(this,
                       i18n("Cannot generate mail:\nNo attendees defined.\n"));
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
    KMessageBox::sorry(this,i18n("Cannot generate mail:\nNo event selected."));
    return;
  }
  if(anEvent->attendeeCount() == 0 ) {
    KMessageBox::sorry(this,
                       i18n("Cannot generate mail:\nNo attendees defined.\n"));
    return;
  }

  mailobject.emailEvent(anEvent);
#endif
}


void CalendarView::schedule_publish(Incidence *incidence)
{
  Event *event = 0;
  Todo *todo = 0;
  if (incidence == 0)
    incidence = selectedIncidence();

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
    } else  if ( todo ) {
      Todo *ev = new Todo(*todo);
      ev->registerObserver(0);
      ev->clearAttendees();
      if (!dlg->addMessage(ev,Scheduler::Publish,publishdlg->addresses())) {
        delete(ev);
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

void CalendarView::mailFreeBusy( int daysToPublish )
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

void CalendarView::uploadFreeBusy()
{
  KOGroupware::instance()->freeBusyManager()->publishFreeBusy();
}

void CalendarView::schedule(Scheduler::Method method, Incidence *incidence)
{
  Event *event = 0;
  Todo *todo = 0;
  if (incidence == 0) {
    incidence = selectedIncidence();
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
    Attendee *me = incidence->attendeeByMails(KOPrefs::instance()->allEmails());
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
    if (to) delete(to);
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

void CalendarView::deleteAttendee( Incidence *incidence )
{
  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( KMessageBox::questionYesNo( this, i18n("Some attendees were removed "
       "from the incidence. Shall cancel messages be sent to these attendees?"),
       i18n( "Attendees removed" ) ) == KMessageBox::Yes ) {
      // don't use KOGroupware::sendICalMessage here, because that asks just
      // a very general question "Other people are involved, send message to
      // them?", which isn't helpful at all in this situation. Afterwards, it
      // would only call the MailScheduler::performTransaction, so do this
      // manually.
      // FIXME: Groupware schedulling should be factored out to it's own class
      //        anyway
      KCal::MailScheduler scheduler( mCalendar );
      scheduler.performTransaction( incidence, Scheduler::Cancel );
    }
  } else {
    schedule_cancel( incidence );
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

  KOrg::BaseView *currentView = mViewManager->currentView();

  CalPrinter::PrintType printType = CalPrinter::Month;

  if ( currentView ) printType = currentView->printType();

  DateList tmpDateList = mNavigator->selectedDates();
  mCalPrinter->print( printType, tmpDateList.first(), tmpDateList.last() );
#endif
}

void CalendarView::printPreview()
{
#ifndef KORG_NOPRINTER
  kdDebug(5850) << "CalendarView::printPreview()" << endl;

  createPrinter();

  DateList tmpDateList = mNavigator->selectedDates();

  mViewManager->currentView()->printPreview( mCalPrinter, tmpDateList.first(),
                                             tmpDateList.last() );
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

  QString filename = KFileDialog::getSaveFileName("vcalout.vcs",i18n("*.vcs|vCalendars"),this);

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
  bool organizerEvents = false;
  bool groupEvents = false;
  bool todo = false;
  bool subtodo = false;

  if ( incidence ) {
    organizerEvents = KOPrefs::instance()->thatIsMe( incidence->organizer() );
    groupEvents = incidence->attendeeByMails( KOPrefs::instance()->allEmails() );
    if ( incidence && incidence->type() == "Event" ) {
//      Event *event = static_cast<Event *>( incidence );
    } else if  ( incidence && incidence->type() == "Todo" ) {
      Todo *event = static_cast<Todo *>( incidence );
      todo = true;
      subtodo = (event->relatedTo() != 0);
    }
  }
  emit todoSelected( todo );
  emit subtodoSelected( subtodo );
  emit organizerEventsSelected( organizerEvents );
  emit groupEventsSelected( groupEvents );
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
  Incidence::List incidences = mCalendar->rawIncidences();
  Incidence::List::Iterator it;

  for ( it = incidences.begin(); it != incidences.end(); it++ ) {
    (*it)->setOrganizer(KOPrefs::instance()->email());
    (*it)->recreate();
    (*it)->setReadOnly(false);
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
  showLeftFrame( mLeftFrame->isHidden() );
}

void CalendarView::showLeftFrame(bool show)
{
  if (show) {
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
  incidence = 0;

  Incidence::List selectedIncidences = mTodoList->selectedIncidences();
  if ( !selectedIncidences.isEmpty() ) incidence = selectedIncidences.first();
  if ( incidence && incidence->type() == "Todo" ) {
    return static_cast<Todo *>( incidence );
  }

  return 0;
}

void CalendarView::dialogClosing(Incidence *in)
{
  mDialogList.remove(in);
}

Incidence* CalendarView::selectedIncidence()
{
  Incidence *incidence = currentSelection();
  if ( !incidence ) {
    Incidence::List selectedIncidences = mTodoList->selectedIncidences();
    if ( !selectedIncidences.isEmpty() )
      incidence = selectedIncidences.first();
  }
  return incidence;
}

void CalendarView::showIncidence()
{
  showIncidence( selectedIncidence() );
}

void CalendarView::editIncidence()
{
  editIncidence( selectedIncidence() );
}

bool CalendarView::editIncidence( const QString& uid )
{
  kdDebug(5850) << "CalendarView::editIncidence()" << endl;
  return editIncidence( mCalendar->incidence( uid ) );
}

void CalendarView::deleteIncidence()
{
  deleteIncidence( selectedIncidence() );
}

void CalendarView::showIncidence(Incidence *incidence)
{
  if ( incidence ) {
    ShowIncidenceVisitor v;
    v.act( incidence, this );
  }
}

bool CalendarView::editIncidence(Incidence *incidence)
{
  if ( incidence ) {
    EditIncidenceVisitor v;
    v.act( incidence, this );
    return true;
  }
  return false;
}

void CalendarView::deleteIncidence(Incidence *incidence)
{
  if ( incidence && !incidence->isReadOnly() ) {
    DeleteIncidenceVisitor v;
    v.act( incidence, this );
  }
/* @TODO: Enable this warning message after 3.3
  if ( incidence && incidence->isReadOnly() ) {
    KMessageBox::information( this, TODO_I18N("The item \"%1\" is marked read-only and cannot be deleted. Probably it belongs to a read-only calendar resource.").arg(incidence->summary()), TODO_I18N("Removing not possible"), "deleteReadOnlyIncidence" );
  }
*/
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
    startMultiModify( i18n("Purging completed todos") );
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
            incidenceToBeDeleted( aTodo );
            calendar()->deleteTodo( aTodo );
            incidenceDeleted( aTodo );
            deletedOne = true;
          }
        }
      }
    }
    endMultiModify();
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

  updateView();
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

void CalendarView::warningChangeFailed( Incidence * )
{
  KMessageBox::sorry( this, i18n("Unable to edit incidence. "
                                 "It's locked by another process.") );
}

void CalendarView::editCanceled( Incidence *i )
{
  mCalendar->endChange( i );
}

void CalendarView::recurTodo( Todo *todo )
{
  if (!todo) return;

  if ( todo->doesRecur() ) {
    Recurrence *r = todo->recurrence();
    QDateTime endDateTime = r->endDateTime();
    QDateTime nextDate = r->getNextDateTime( todo->dtDue() );

    if ( ( r->duration() == -1 || ( nextDate.isValid() && endDateTime.isValid()
                                    && nextDate <= endDateTime ) ) ) {
      todo->setDtDue( nextDate );
      while ( !todo->recursAt( todo->dtDue() ) ||
               todo->dtDue() <= QDateTime::currentDateTime() ) {
        todo->setDtDue( r->getNextDateTime( todo->dtDue() ) );
      }

      todo->setCompleted( false );
      todo->setRevision( todo->revision() + 1 );

      return;
    }
  }
  todo->setCompleted( QDateTime::currentDateTime() );
  // incidenceChanged(todo) should be emitted by caller.
}

void CalendarView::showErrorMessage( const QString &msg )
{
  KMessageBox::error( this, msg );
}

void CalendarView::updateCategories()
{
  QStringList allCats( calendar()->incidenceCategories() );
  allCats.sort();
  QStringList categories( KOPrefs::instance()->mCustomCategories );
  for ( QStringList::ConstIterator si = allCats.constBegin(); si != allCats.constEnd(); ++si ) {
    if ( categories.find( *si ) == categories.end() ) {
      categories.append( *si );
    }
  }
  KOPrefs::instance()->mCustomCategories = categories;
  KOPrefs::instance()->writeConfig();
  // Make the category editor update the list!
  emit categoriesChanged();
}

#include "calendarview.moc"
