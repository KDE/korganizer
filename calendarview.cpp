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
#include "komessagebox.h"
#include "exportwebdialog.h"
#include "kocorehelper.h"
#include "incidencechanger.h"
#include "mailscheduler.h"

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
#include <libkcal/htmlexportsettings.h>

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
#include <qvbox.h>

#include <stdlib.h>

using namespace KOrg;

CalendarView::CalendarView( QWidget *parent, const char *name )
  : CalendarViewBase( parent, name ),
    mHistory( 0 ),
    mCalendar( CalendarNull::self() ),
    mChanger( 0 )
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

  QVBox *rightBox = new QVBox( mPanner );
  mNavigatorBar = new NavigatorBar( rightBox );
  mRightFrame = new QWidgetStack( rightBox );
  rightBox->setStretchFactor( mRightFrame, 1 );

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

  connect( mDateNavigator, SIGNAL(incidenceDropped(Incidence*, const QDate&)),
           SLOT( addIncidenceOn( Incidence *, const QDate & ) ) );
  connect( mDateNavigator, SIGNAL(incidenceDroppedMove(Incidence*,const QDate&)),
           SLOT( moveIncidenceTo( Incidence *, const QDate & ) ) );

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

  KDirWatch *messageWatch = new KDirWatch( this );
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
  kdDebug(5850)<<"CalendarView::setCalendar"<<endl;
  mCalendar = cal;

  delete mHistory;
  mHistory = new History( mCalendar );
  connect( mHistory, SIGNAL( undone() ), SLOT( updateView() ) );
  connect( mHistory, SIGNAL( redone() ), SLOT( updateView() ) );

  if ( mChanger ) delete mChanger;
  setIncidenceChanger( new IncidenceChanger( mCalendar, this ) );

  mCalendar->registerObserver( this );

  mDateNavigator->setCalendar( mCalendar );

  mTodoList->setCalendar( mCalendar );
}

void CalendarView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  emit newIncidenceChanger( mChanger );
  connect( mChanger, SIGNAL( incidenceAdded( Incidence* ) ),
           this, SLOT( incidenceAdded( Incidence* ) ) );
  connect( mChanger, SIGNAL( incidenceChanged( Incidence*, Incidence*, int ) ),
           this, SLOT( incidenceChanged( Incidence*, Incidence*, int ) ) );
  connect( mChanger, SIGNAL( incidenceChanged( Incidence*, Incidence* ) ),
           this, SLOT( incidenceChanged( Incidence*, Incidence* ) ) );
  connect( mChanger, SIGNAL( incidenceToBeDeleted( Incidence * ) ),
           this, SLOT( incidenceToBeDeleted( Incidence * ) ) );
  connect( mChanger, SIGNAL( incidenceDeleted( Incidence * ) ),
           this, SLOT( incidenceDeleted( Incidence * ) ) );

  connect( mChanger, SIGNAL( schedule( Scheduler::Method, Incidence*) ),
           this, SLOT( schedule( Scheduler::Method, Incidence*) ) );
}

Calendar *CalendarView::calendar()
{
  if ( mCalendar ) return mCalendar;
  else return CalendarNull::self();
}

KOIncidenceEditor *CalendarView::editorDialog( Incidence *incidence ) const
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
    mCalPrinter = new CalPrinter( this, mCalendar, new KOCoreHelper() );
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
  // FIXME: Move the filter loading and saving to the CalFilter class in libkcal
  QStringList filterList = config->readListEntry("CalendarFilters");
  QString currentFilter = config->readEntry("Current Filter");

  QStringList::ConstIterator it = filterList.begin();
  QStringList::ConstIterator end = filterList.end();
  while(it != end) {
//    kdDebug(5850) << "  filter: " << (*it) << endl;
    CalFilter *filter;
    filter = new CalFilter(*it);
    config->setGroup("Filter_" + (*it));
    filter->setCriteria(config->readNumEntry("Criteria",0));
    filter->setCategoryList(config->readListEntry("CategoryList"));
    filter->setCompletedTimeSpan( config->readNumEntry( "HideTodoDays", 0 ) );
    mFilters.append(filter);

    ++it;
  }
  
  config->setGroup("General");
  int pos = filterList.findIndex( currentFilter );
  mCurrentFilter = 0;
  if ( pos>=0 ) {
    mCurrentFilter = mFilters.at( pos );
  }
  updateFilter();
}

void CalendarView::writeFilterSettings(KConfig *config)
{
//  kdDebug(5850) << "CalendarView::writeFilterSettings()" << endl;

  QStringList filterList;

  CalFilter *filter = mFilters.first();
  while( filter ) {
//    kdDebug(5850) << " fn: " << filter->name() << endl;
    filterList << filter->name();
    config->setGroup( "Filter_" + filter->name() );
    config->writeEntry( "Criteria", filter->criteria() );
    config->writeEntry( "CategoryList", filter->categoryList() );
    config->writeEntry( "HideTodoDays", filter->completedTimeSpan() );
    filter = mFilters.next();
  }
  config->setGroup( "General" );
  config->writeEntry( "CalendarFilters", filterList);
  config->writeEntry( "Current Filter", mCurrentFilter->name());
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
  checkForFilteredChange( incidence );
}

void CalendarView::incidenceChanged( Incidence *oldIncidence,
                                     Incidence *newIncidence )
{
  incidenceChanged( oldIncidence, newIncidence, KOGlobals::UNKNOWN_MODIFIED );
}

void CalendarView::incidenceChanged( Incidence *oldIncidence,
                                     Incidence *newIncidence, int what )
{
  // FIXME: Make use of the what flag, which indicates which parts of the incidence have changed!
  KOIncidenceEditor *tmp = editorDialog( newIncidence );
  if ( tmp ) {
    kdDebug(5850) << "Incidence modified and open" << endl;
    tmp->modified( what );
  }
  setModified( true );
  history()->recordEdit( oldIncidence, newIncidence );
  changeIncidenceDisplay( newIncidence, KOGlobals::INCIDENCEEDITED );
  updateUnmanagedViews();
  checkForFilteredChange( newIncidence );
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

void CalendarView::checkForFilteredChange( Incidence *incidence )
{
  CalFilter *filter = calendar()->filter();
  if ( filter && !filter->filterIncidence( incidence ) ) {
    // Incidence is filtered and thus not shown in the view, tell the
    // user so that he isn't surprised if his new event doesn't show up
    KMessageBox::information( this, i18n("The incidence \"%1\" is filtered by "
                 "your current filter rules, so it will be hidden and not "
                 "appear in the view.").arg( incidence->summary() ),
                 i18n("Filter applied"), "ChangedIncidenceFiltered" );
  }
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

int CalendarView::msgItemDelete( Incidence *incidence )
{
  return KMessageBox::warningContinueCancel(this,
      i18n("The item \"%1\" will be permanently deleted.").arg( incidence->summary() ),
      i18n("KOrganizer Confirmation"), KGuiItem(i18n("&Delete"),"editdelete"));
}


void CalendarView::edit_cut()
{
  Incidence *incidence = selectedIncidence();

  if ( !incidence || !mChanger ) {
    KNotifyClient::beep();
    return;
  }
  mChanger->cutIncidence( incidence );
}

void CalendarView::edit_copy()
{
  Incidence *incidence = selectedIncidence();

  if (!incidence) {
    KNotifyClient::beep();
    return;
  }
  DndFactory factory( mCalendar );
  if ( !factory.copyIncidence( incidence ) ) {
    KNotifyClient::beep();
  }
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

  // FIXME: use a visitor here
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
    mChanger->addIncidence( pastedEvent );

  } else if ( pastedIncidence->type() == "Todo" ) {
    Todo* pastedTodo = static_cast<Todo*>(pastedIncidence);
    Todo* _selectedTodo = selectedTodo();
    if ( _selectedTodo )
      pastedTodo->setRelatedTo( _selectedTodo );
    mChanger->addIncidence( pastedTodo );
  }
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
}


void CalendarView::newEvent()
{
  kdDebug(5850) << "CalendarView::newEvent()" << endl;
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
  connectIncidenceEditor( eventEditor );
  eventEditor->newEvent( text );
  mDialogManager->connectTypeAhead( eventEditor, viewManager()->agendaView() );
  eventEditor->show();
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QString &attachment )
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  connectIncidenceEditor( eventEditor );
  eventEditor->newEvent( summary, description, attachment );
  eventEditor->show();
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QString &attachment, const QStringList &attendees )
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  connectIncidenceEditor( eventEditor );
  eventEditor->newEvent( summary, description, attachment, attendees );
  eventEditor->show();
}

void CalendarView::newEvent(QDateTime fromHint, QDateTime toHint, bool allDay)
{
  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  connectIncidenceEditor( eventEditor );
  eventEditor->newEvent(fromHint,toHint,allDay);
  mDialogManager->connectTypeAhead( eventEditor, viewManager()->agendaView() );
  eventEditor->show();
}

void CalendarView::newTodo( const QString &text )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo( text );
  todoEditor->show();
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                             const QString &attachment )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo( summary, description, attachment );
  todoEditor->show();
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                            const QString &attachment, const QStringList &attendees )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo( summary, description, attachment, attendees );
  todoEditor->show();
}

void CalendarView::newTodo()
{
  kdDebug(5850) << "CalendarView::newTodo()" << endl;
  QDateTime dtDue;
  bool allday = true;
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
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
  connectIncidenceEditor( todoEditor );
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
  connectIncidenceEditor( todoEditor );
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
  if( todo_unsub (anTodo ) ) {
    updateView();
  }
}

bool CalendarView::todo_unsub( Todo *todo )
{
  bool status;
  if ( !todo || !todo->relatedTo() ) return false;
  Todo *oldTodo = todo->clone();
  if ( mChanger->beginChange( todo ) ) {
    todo->relatedTo()->removeRelation(todo);
    todo->setRelatedTo(0);
    todo->setRelatedToUid("");
    mChanger->changeIncidence( todo, todo, KOGlobals::RELATION_MODIFIED );
    mChanger->endChange( todo );
    setModified(true);
    status = true;
  } else {
    KMessageBox::sorry( this, i18n("Unable to turn sub-to-do into a top-level "
        "to-do, because it cannot be locked.") );
    status = false;
  }
  delete oldTodo;
  return status;
}

bool CalendarView::deleteIncidence( const QString &uid )
{
  Incidence *inc = mCalendar->incidence( uid );
  if ( inc ) {
    deleteIncidence( inc );
    return true;
  } else {
    return false;
  }
}



void CalendarView::toggleAlarm( Incidence *incidence )
{
  if ( !incidence || !mChanger ) {
    kdDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  Incidence*oldincidence = incidence->clone();
  if ( !mChanger->beginChange( incidence ) ) {
    kdDebug(5850) << "Unable to lock incidence " << endl;
    delete oldincidence;
    return;
  }

  Alarm::List alarms = incidence->alarms();
  Alarm::List::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it )
    (*it)->toggleAlarm();
  if (alarms.isEmpty()) {
    // Add an alarm if it didn't have one
    Alarm*alm = incidence->newAlarm();
    alm->setEnabled(true);
  }
  mChanger->changeIncidence( oldincidence, incidence, KOGlobals::ALARM_MODIFIED );
  mChanger->endChange( incidence );
  delete oldincidence;

//  mClickedItem->updateIcons();
}

void CalendarView::dissociateOccurrence( Incidence *incidence, const QDate &date )
{
  if ( !incidence || !mChanger ) {
    kdDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  Incidence*oldincidence = incidence->clone();
  if ( !mChanger->beginChange( incidence ) ) {
    kdDebug(5850) << "Unable to lock incidence " << endl;
    delete oldincidence;
    return;
  }

  Incidence* newInc = mCalendar->dissociateOccurrence( incidence, date, true );

  if ( newInc ) {
    startMultiModify( i18n("Dissociate occurrence") );
    mChanger->changeIncidence( oldincidence, incidence );
    mChanger->addIncidence( newInc );
    endMultiModify();
  } else {
    KMessageBox::sorry( this, i18n("Dissociating the occurrence failed."),
      i18n("Dissociating failed") );
  }
  mChanger->endChange( incidence );
  delete oldincidence;
}

void CalendarView::dissociateFutureOccurrence( Incidence *incidence, const QDate &date )
{
  if ( !incidence || !mChanger ) {
    kdDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  Incidence*oldincidence = incidence->clone();
  if ( !mChanger->beginChange( incidence ) ) {
    kdDebug(5850) << "Unable to lock incidence " << endl;
    delete oldincidence;
    return;
  }

  Incidence* newInc = mCalendar->dissociateOccurrence( incidence, date, true );
  if ( newInc ) {
    startMultiModify( i18n("Dissociate future occurrences") );
    mChanger->changeIncidence( oldincidence, incidence );
    mChanger->addIncidence( newInc );
    endMultiModify();
  } else {
    KMessageBox::sorry( this, i18n("Dissociating the future occurrences failed."),
      i18n("Dissociating failed") );
  }
  mChanger->endChange( incidence );
  delete oldincidence;
}


/*****************************************************************************/


void CalendarView::schedule_publish(Incidence *incidence)
{
  if (incidence == 0)
    incidence = selectedIncidence();

  if (!incidence) {
    KMessageBox::information( this, i18n("No incidence selected."),
                              "PublishNoEventSelected" );
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
    Incidence *inc = incidence->clone();
    inc->registerObserver( 0 );
    inc->clearAttendees();
    if (!dlg->addMessage( inc, Scheduler::Publish, publishdlg->addresses() )) {
      delete( inc );
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
  freebusy->setOrganizer( Person( KOPrefs::instance()->fullName(),
                      KOPrefs::instance()->email() ) );

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
  if ( !incidence ) {
    incidence = selectedIncidence();
  }

  if ( !incidence ) {
    KMessageBox::sorry( this, i18n("No incidence selected."),
                        "ScheduleNoEventSelected" );
    return;
  }

  if( incidence->attendeeCount() == 0 && method != Scheduler::Publish ) {
    KMessageBox::information( this, i18n("The incidence has no attendees."),
                              "ScheduleNoIncidences" );
    return;
  }

  Incidence *inc = incidence->clone();

  if ( method == Scheduler::Reply || method == Scheduler::Refresh ) {
    Attendee *me = incidence->attendeeByMails(KOPrefs::instance()->allEmails());
    if ( !me ) {
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
    if ( inc ) {
      inc->clearAttendees();
      inc->addAttendee( menew, false );
    }
  }

  OutgoingDialog *dlg = mDialogManager->outgoingDialog();
  if (inc ) {
    if ( !dlg->addMessage( inc, method ) ) delete(inc);
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

void CalendarView::exportWeb()
{
  // FIXME: Get rid of the settings object. When can I delete it???
  HTMLExportSettings *settings = new HTMLExportSettings( "KOrganizer" );
  // Manually read in the config, because parametrized kconfigxt objects don't 
  // seem to load the config theirselves
  if ( settings ) settings->readConfig();
  ExportWebDialog *dlg = new ExportWebDialog( settings, this );
  connect( dlg,  SIGNAL( exportHTML( HTMLExportSettings* ) ), 
           this, SIGNAL( exportHTML( HTMLExportSettings* ) ) );
  dlg->show();
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
    organizerEvents = KOPrefs::instance()->thatIsMe( incidence->organizer().email() );
    groupEvents = incidence->attendeeByMails( KOPrefs::instance()->allEmails() );

    if ( incidence && incidence->type() == "Todo" ) {
      todo = true;
      subtodo = ( incidence->relatedTo() != 0 );
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
  kdDebug(5850) << "CalendarView::editFilters()" << endl;

  CalFilter *filter = mFilters.first();
  while(filter) {
    kdDebug(5850) << " Filter: " << filter->name() << endl;
    filter = mFilters.next();
  }

  mDialogManager->showFilterEditDialog(&mFilters);
}

/** Filter configuration changed 
*/
void CalendarView::updateFilter()
{
  QStringList filters;
  CalFilter *filter;
  
  int pos = mFilters.find( mCurrentFilter );
  if ( pos < 0 ) {
    mCurrentFilter = 0;
  }
  
  filters << i18n("No filter");
  for ( filter = mFilters.first(); filter; filter = mFilters.next() ) {
    filters << filter->name();
  }

  emit newFilterListSignal( filters );
  // account for the additional "No filter" at the beginning! if the
  // filter is not in the list, pos == -1...
  emit selectFilterSignal( pos+1 );
  mCalendar->setFilter( mCurrentFilter );
  updateView();
}

/** A different filter was selected
*/
void CalendarView::filterActivated( int filterNo )
{
  CalFilter *newFilter = 0;
  if ( filterNo > 0 && filterNo <= int(mFilters.count()) ) {
    newFilter = mFilters.at( filterNo-1 );
  }
  if ( newFilter != mCurrentFilter ) {
    mCurrentFilter = newFilter;
    mCalendar->setFilter( mCurrentFilter );
    updateView();
  }
}

void CalendarView::takeOverEvent()
{
  Incidence *incidence = currentSelection();

  if (!incidence) return;

  incidence->setOrganizer( Person( KOPrefs::instance()->fullName(),
                           KOPrefs::instance()->email() ) );
  incidence->recreate();
  incidence->setReadOnly(false);

  updateView();
}

void CalendarView::takeOverCalendar()
{
  Incidence::List incidences = mCalendar->rawIncidences();
  Incidence::List::Iterator it;

  for ( it = incidences.begin(); it != incidences.end(); it++ ) {
    (*it)->setOrganizer( Person( KOPrefs::instance()->fullName(),
                         KOPrefs::instance()->email() ) );
    (*it)->recreate();
    (*it)->setReadOnly(false);
  }
  updateView();
}

void CalendarView::showIntro()
{
  kdDebug(5850) << "To be implemented." << endl;
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

void CalendarView::dialogClosing( Incidence *in )
{
  // FIXME: this doesn't work, because if it's a new incidence, it's not locked!
  mChanger->endChange( in );
  mDialogList.remove( in );
}

Incidence *CalendarView::currentSelection()
{
  return mViewManager->currentSelection();
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

void CalendarView::showIncidence( Incidence *incidence )
{
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog( this );
  eventViewer->setIncidence( incidence );
  eventViewer->show();
}

bool CalendarView::editIncidence( Incidence *incidence )
{
  kdDebug(5850) << "CalendarView::editEvent()" << endl;

  if ( !incidence || !mChanger ) {
    KNotifyClient::beep();
    return false;
  }
  KOIncidenceEditor *tmp = editorDialog( incidence );
  if ( tmp ) {
    kdDebug(5850) << "CalendarView::editIncidence() in List" << endl;
    tmp->reload();
    tmp->raise();
    tmp->show();
    return true;
  }

  if ( incidence->isReadOnly() ) {
    showIncidence( incidence );
    return true;
  }

  if ( !mChanger->beginChange( incidence ) ) {
    warningChangeFailed( incidence );
    showIncidence( incidence );
    return false;
  }

  kdDebug(5850) << "CalendarView::editIncidence() new IncidenceEditor" << endl;
  KOIncidenceEditor *incidenceEditor = mDialogManager->getEditor( incidence );
  connectIncidenceEditor( incidenceEditor );

  mDialogList.insert( incidence, incidenceEditor );
  incidenceEditor->editIncidence( incidence );
  incidenceEditor->show();
  return true;
}

void CalendarView::deleteTodoIncidence ( Todo *todo )
{
  Todo *subtodo;
  int i;
  
  if ( !todo ) return ;
 
  // it a simple todo, ask and delete.
  if (todo->relations().isEmpty() ) {
    bool doDelete = true;
    if (KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( todo ) == KMessageBox::Continue );
    }
    if ( doDelete )
      mChanger->deleteIncidence( todo );
    return;
  }

  /* Ok, this todo has sub-to-dos, ask what to do */
  int km=KMessageBox::questionYesNoCancel( this,
                                i18n("The incidence \"%1\" has sub-to-dos. "
                                     "Do you want to delete just this item and "
                                     "make all its sub-to-dos independent, or "
                                     "delete the to-do with all its sub-to-dos?"
                                ).arg( todo->summary() ),
                                i18n("KOrganizer Confirmation"), 
                                i18n("Delete only this "),
                                i18n("Delete all"));
  // Delete only the father
  if( km == KMessageBox::Yes ) {
    // do all sub-to-dos independents
    for( i=todo->relations().count()-1; i>=0; i--) {
      subtodo= static_cast<Todo*>(todo->relations()[i]);
      todo_unsub( subtodo );
    }
    
  } else if ( km == KMessageBox::No ) {
    // Delete all 
    // do all sub-to-dos independents
    // we have to hide the delete confirmation for each itemDate
    bool saveConfirm=KOPrefs::instance()->mConfirm;
    KOPrefs::instance()->mConfirm = false;
    
    for( i=todo->relations().count()-1; i>=0; i--) {
      subtodo= static_cast<Todo*>(todo->relations()[i]);
      todo_unsub (subtodo);
      deleteIncidence( subtodo );
    }
    //Restore the Confirm option
    KOPrefs::instance()->mConfirm=saveConfirm;
  }
  mChanger->deleteIncidence( todo );
}

void CalendarView::deleteIncidence(Incidence *incidence)
{
  if ( !incidence || !mChanger ) {
    KNotifyClient::beep();
    return;
  }
  if ( incidence->isReadOnly() ) {
    KMessageBox::information( this, i18n("The item \"%1\" is marked read-only "
                              "and cannot be deleted; it probably belongs to "
                              "a read-only calendar resource.")
                              .arg(incidence->summary()),
                              i18n("Removing not possible"),
                              "deleteReadOnlyIncidence" );
    return;
  }

  CanDeleteIncidenceVisitor v;

  // Let the visitor do special things for special incidence types.
  // e.g. todos with children cannot be deleted, so act(..) returns false
  if ( !v.act( incidence, this ) )
    return;
  //If it is a todo, there are specific delete function 
  
  if ( incidence && incidence->type()=="Todo" ) {
    deleteTodoIncidence( static_cast<Todo*>(incidence) );
    return;
  }
  
  if ( incidence->doesRecur() ) {
    QDate itemDate = mViewManager->currentSelectionDate();
    kdDebug(5850) << "Recurrence-Date: " << itemDate.toString() << endl;
    int km;
    if ( !itemDate.isValid() ) {
      kdDebug(5850) << "Date Not Valid" << endl;
      km = KMessageBox::warningContinueCancel(this,
        i18n("The incidence \"%1\" recurs over multiple dates; "
             "are you sure you want to delete this event "
             "and all its recurrences?").arg( incidence->summary() ),
             i18n("KOrganizer Confirmation"), i18n("Delete All") );
    } else {
      km = KOMessageBox::fourBtnMsgBox( this, QMessageBox::Warning,
        i18n("The incidence \"%1\" recurs over multiple dates. "
             "Do you want to delete only the current one on %2, only all "
             "future recurrences, or all its recurrences?" )
             .arg( incidence->summary() )
             .arg( KGlobal::locale()->formatDate(itemDate)),
             i18n("KOrganizer Confirmation"), i18n("Delete C&urrent"),
             i18n("Delete &Future"),
             i18n("Delete &All"));
    }
    switch(km) {
      case KMessageBox::Ok: // Continue // all
      case KMessageBox::Continue:
        mChanger->deleteIncidence( incidence );
        break;

      case KMessageBox::Yes: // just this one
        if ( mChanger->beginChange( incidence ) ) {
          Incidence *oldIncidence = incidence->clone();
          incidence->addExDate( itemDate );
          mChanger->changeIncidence( oldIncidence, incidence );
          mChanger->endChange( incidence );
          delete oldIncidence;
        }
        break;
      case KMessageBox::No: // all future items
        if ( mChanger->beginChange( incidence ) ) {
          Incidence *oldIncidence = incidence->clone();
          Recurrence *recur = incidence->recurrence();
          recur->setEndDate( itemDate.addDays(-1) );
          mChanger->changeIncidence( oldIncidence, incidence );
          mChanger->endChange( incidence );
          delete oldIncidence;
        }
        break;
    }
  } else {
    bool doDelete = true;
    if (KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( incidence ) == KMessageBox::Continue );
    }
    if ( doDelete )
      mChanger->deleteIncidence( incidence );
  }
}

void CalendarView::connectIncidenceEditor( KOIncidenceEditor *editor )
{
  connect( this, SIGNAL( newIncidenceChanger( IncidenceChangerBase* ) ),
           editor, SLOT( setIncidenceChanger( IncidenceChangerBase* ) ) );
  editor->setIncidenceChanger( mChanger );
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

bool CalendarView::purgeCompletedSubTodos( Todo* todo, bool &allPurged )
{
  if ( !todo ) return true;
  bool deleteThisTodo = true;
  Incidence::List subTodos( todo->relations() );
  Incidence *aIncidence;
  Todo *aTodo;
  Incidence::List::Iterator it;
  for ( it = subTodos.begin(); it != subTodos.end(); ++it ) {
    aIncidence = *it;
    if ( aIncidence && aIncidence->type()=="Todo" ) {
      aTodo = static_cast<Todo*>( aIncidence );
      deleteThisTodo &= purgeCompletedSubTodos( aTodo, allPurged );
    }
  }

  if ( deleteThisTodo ) {
    if ( todo->isCompleted() ) {
      if ( !mChanger->deleteIncidence( todo ) )
        allPurged = false;
    } else {
      deleteThisTodo = false;
    }
  } else {
    if ( todo->isCompleted() ) {
      allPurged = false;
    }
  }
  return deleteThisTodo;
}

void CalendarView::purgeCompleted()
{
  int result = KMessageBox::warningContinueCancel(this,
      i18n("Delete all completed to-dos?"),i18n("Purge To-dos"),i18n("Purge"));

  if (result == KMessageBox::Continue) {
    bool allDeleted = true;
    startMultiModify( i18n("Purging completed to-dos") );
    Todo::List todos = calendar()->rawTodos();
    Todo::List rootTodos;
    Todo::List::ConstIterator it;
    for ( it = todos.begin(); it != todos.end(); ++it ) {
      Todo *aTodo = *it;
      if ( aTodo && !aTodo->relatedTo() )
        rootTodos.append( aTodo );
    }
    // now that we have a list of all root todos, check them and their children
    for ( it = rootTodos.begin(); it != rootTodos.end(); ++it ) {
      purgeCompletedSubTodos( *it, allDeleted );
    }
    endMultiModify();
    if ( !allDeleted ) {
      KMessageBox::information( this, i18n("Unable to purge to-do items with "
                                "uncompleted children."), i18n("Delete To-do"),
                                "UncompletedChildrenPurgeTodos" );
    }
  }
}

void CalendarView::slotCalendarChanged()
{
  kdDebug(5850) << "CalendarView::slotCalendarChanged()" << endl;

  updateView();
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
  KMessageBox::sorry( this, i18n("Unable to edit incidence: "
                                 "it is locked by another process.") );
}

void CalendarView::editCanceled( Incidence *i )
{
  mCalendar->endChange( i );
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

void CalendarView::addIncidenceOn( Incidence *incadd, const QDate &dt )
{
  if ( !incadd || !mChanger ) {
    KMessageBox::sorry(this, i18n("Unable to copy the incidence to %1.")
                       .arg( dt.toString() ), i18n("Copying failed") );
    return;
  }
  Incidence *incidence = mCalendar->incidence( incadd->uid() );
  if ( !incidence ) incidence = incadd;
  // Create a copy of the incidence, since the incadd doesn't belong to us.
  incidence = incidence->clone();
  incidence->recreate();

  if ( incidence->type() == "Event" ) {
    Event *event = static_cast<Event*>(incidence);

    // Adjust date
    QDateTime start = event->dtStart();
    QDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } else if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>(incidence);
    QDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }

  if ( !mChanger->addIncidence( incidence ) ) {
    KODialogManager::errorSaveIncidence( this, incidence );
    delete incidence;
  }
}

void CalendarView::moveIncidenceTo( Incidence *incmove, const QDate &dt )
{
  if ( !incmove || !mChanger ) {
    KMessageBox::sorry( this, i18n("Unable to move the incidence to  %1.")
                        .arg( dt.toString() ), i18n("Moving failed") );
    return;
  }
  Incidence *incidence = mCalendar->incidence( incmove->uid() );
  if ( !incidence ) {
    addIncidenceOn( incidence, dt );
    return;
  }
  Incidence *oldIncidence = incidence->clone();
  if ( !mChanger->beginChange( incidence ) ) {
    delete oldIncidence;
    return;
  }

  if ( incidence->type() == "Event" ) {
    Event *event = static_cast<Event*>(incidence);

    // Adjust date
    QDateTime start = event->dtStart();
    QDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } else if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>(incidence);
    QDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }
  mChanger->changeIncidence( oldIncidence, incidence );
  mChanger->endChange( incidence );
  delete oldIncidence;
}

#include "calendarview.moc"
