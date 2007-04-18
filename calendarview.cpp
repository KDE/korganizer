/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998, 1999 Preston Brown <preston.brown@yale.edu>
    Fester Zigterman <F.J.F.ZigtermanRustenburg@student.utwente.nl>
    Ian Dawes <iadawes@globalserve.net>
    Laszlo Boloni <boloni@cs.purdue.edu>

    Copyright (C) 2000-2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (C) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "calendarview.h"

#ifndef KORG_NOPRINTER
#include "calprinter.h"
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
#include "kholidays.h"
#include "mailscheduler.h"

#include <kcal/vcaldrag.h>
#include <kcal/icaldrag.h>
#include <kcal/icalformat.h>
#include <kcal/vcalformat.h>
#include <kcal/scheduler.h>
#include <kcal/calendarlocal.h>
#include <kcal/journal.h>
#include <kcal/calfilter.h>
#include <kcal/attendee.h>
#include <kcal/dndfactory.h>
#include <kcal/freebusy.h>
#include <kcal/filestorage.h>
#include <kcal/calendarresources.h>
#include <kcal/calendarnull.h>
#include <kcal/htmlexportsettings.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kconfig.h>
#include <krun.h>
#include <kdirwatch.h>

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QTimer>
#include <QStackedWidget>
#include <QList>
#include <QFile>
#include <QLayout>
#ifndef KORG_NOSPLITTER
#include <QSplitter>
#endif


//Added by qt3to4:
#include <QByteArray>
#include <QBoxLayout>
#include <QVBoxLayout>

#include <stdlib.h>
#include <assert.h>
#include <kvbox.h>

using namespace KOrg;

CalendarView::CalendarView( QWidget *parent )
  : CalendarViewBase( parent ),
    mHistory( 0 ),
    mCalendar( CalendarNull::self() ),
    mChanger( 0 )
{
  kDebug(5850) << "CalendarView::CalendarView( Calendar )" << endl;

  mViewManager = new KOViewManager( this );
  mDialogManager = new KODialogManager( this );

  mModified = false;
  mReadOnly = false;
  mSelectedIncidence = 0;

  mCalPrinter = 0;

  mNavigator = new DateNavigator( this );
  mDateChecker = new DateChecker( this );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin(0);

#ifndef KORG_NOSPLITTER
  // create the main layout frames.
  mPanner = new QSplitter( Qt::Horizontal, this );
  mPanner->setObjectName( "CalendarView::Panner" );
  topLayout->addWidget( mPanner );

  mLeftSplitter = new QSplitter( Qt::Vertical, mPanner );
  mLeftSplitter->setObjectName( "CalendarView::LeftFrame" );
//  mPanner->setResizeMode( mLeftSplitter, QSplitter::Stretch );

  mDateNavigator = new DateNavigatorContainer( mLeftSplitter );
  mDateNavigator->setObjectName( "CalendarView::DateNavigator" );

//  mLeftSplitter->setResizeMode( mDateNavigator, QSplitter::Stretch );
  mLeftSplitter->setCollapsible( mLeftSplitter->indexOf(mDateNavigator), true );

  mTodoList = new KOTodoView( CalendarNull::self(), mLeftSplitter );
  mTodoList->setObjectName( "todolist" );

  mEventViewer = new KOEventViewer( mLeftSplitter );
  mEventViewer->setObjectName( "EventViewer" );

  KVBox *rightBox = new KVBox( mPanner );
  mNavigatorBar = new NavigatorBar( rightBox );
  mRightFrame = new QStackedWidget( rightBox );
  rightBox->setStretchFactor( mRightFrame, 1 );

  mLeftFrame = mLeftSplitter;
#else
  QWidget *mainBox;
  QWidget *leftFrame;

  if ( KOPrefs::instance()->mVerticalScreen ) {
    mainBox = new KVBox( this );
    leftFrame = new KHBox( mainBox );
  } else {
    mainBox = new KHBox( this );
    leftFrame = new KVBox( mainBox );
  }

  topLayout->addWidget( mainBox );

  mDateNavigator = new KDateNavigator( leftFrame, true,
                                       "CalendarView::DateNavigator",
                                       QDate::currentDate() );
  mTodoList = new KOTodoView( CalendarNull::self(), leftFrame, "todolist" );

  mEventViewer = new KOEventViewer ( leftFrame, "EventViewer" );

  QWidget *rightBox = new QWidget( mainBox );
  QBoxLayout *rightLayout = new QVBoxLayout( rightBox );
  rightLayout->setMargin();

  mNavigatorBar = new NavigatorBar( QDate::currentDate(), rightBox );
  rightLayout->addWidget( mNavigatorBar );

  mRightFrame = new QStackedWidget( rightBox );
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

  connect( mDateChecker, SIGNAL( dayPassed( const QDate & ) ),
           mTodoList, SLOT( dayPassed( const QDate & ) ) );
  connect( mDateChecker, SIGNAL( dayPassed( const QDate & ) ),
           SIGNAL( dayPassed( const QDate & ) ) );
  connect( mDateChecker, SIGNAL( dayPassed( const QDate & ) ),
           mDateNavigator, SLOT( updateToday() ) );

  connect( this, SIGNAL( configChanged() ),
           mDateNavigator, SLOT( updateConfig() ) );

  connect( this, SIGNAL( incidenceSelected(Incidence *) ),
           mEventViewer, SLOT ( setIncidence (Incidence *) ) );

  //TODO: do a pretty Summary,
  QString s;
  s = i18n( "<p><em>No Item Selected</em></p>"
           "<p>Select an event, to-do or journal entry to view its details "
           "here.</p>");

  mEventViewer->setDefaultText( s );
  mEventViewer->setWhatsThis(
                   i18n( "View the details of events, journal entries or to-dos "
                         "selected in KOrganizer's main view here." ) );
  mEventViewer->setIncidence( 0 );

  mViewManager->connectTodoView( mTodoList );
  mViewManager->connectView( mTodoList );

  KOGlobals::self()->
      setHolidays( new KHolidays( KOPrefs::instance()->mHolidays ) );

  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( checkClipboard() ) );

  connect( mTodoList, SIGNAL( incidenceSelected( Incidence * ) ),
           SLOT( processTodoListSelection( Incidence * ) ) );
  disconnect( mTodoList, SIGNAL( incidenceSelected( Incidence * ) ),
           this, SLOT( processMainViewSelection( Incidence * ) ) );

  kDebug(5850) << "CalendarView::CalendarView() done" << endl;
}

CalendarView::~CalendarView()
{
  kDebug(5850) << "~CalendarView()" << endl;

  mCalendar->unregisterObserver( this );
  qDeleteAll( mFilters );
  qDeleteAll( mExtensions );

  delete mDialogManager;
  delete mViewManager;
  delete mEventViewer;
  kDebug(5850) << "~CalendarView() done" << endl;
}

void CalendarView::setCalendar( Calendar *cal )
{
  kDebug(5850)<<"CalendarView::setCalendar"<<endl;
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


  connect( this, SIGNAL( cancelAttendees( Incidence * ) ),
           mChanger, SLOT( cancelAttendees( Incidence * ) ) );
}

Calendar *CalendarView::calendar()
{
  if ( mCalendar ) return mCalendar;
  else return CalendarNull::self();
}

KOIncidenceEditor *CalendarView::editorDialog( Incidence *incidence ) const
{
  if (mDialogList.contains(incidence) )
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
  kDebug(5850) << "CalendarView::openCalendar(): " << filename << endl;

  if (filename.isEmpty()) {
    kDebug(5850) << "CalendarView::openCalendar(): Error! Empty filename." << endl;
    return false;
  }

  if (!QFile::exists(filename)) {
    kDebug(5850) << "CalendarView::openCalendar(): Error! File '" << filename
              << "' doesn't exist." << endl;
  }

  bool loadedSuccesfully = true;
  if ( !merge ) {
    mCalendar->close();
    CalendarLocal *cl = dynamic_cast<CalendarLocal*>( mCalendar );
    if ( cl ) {
      loadedSuccesfully = cl->load( filename );
    } else {
      CalendarResources *cr = dynamic_cast<CalendarResources*>( mCalendar );
      assert( cr ); // otherwise something is majorly wrong
      // openCalendar called without merge and a filename, what should we do?
      return false;
    }
  } else {
    // merge in a file
    FileStorage storage( mCalendar );
    storage.setFileName( filename );
    loadedSuccesfully = storage.load();
  }

  if ( loadedSuccesfully ) {
    if ( merge )
      setModified( true );
    else {
      setModified( false );
      mViewManager->setDocumentId( filename );
      mTodoList->setDocumentId( filename );
    }
    updateCategories();
    updateView();
    return true;
  } else {
    // while failing to load, the calendar object could
    // have become partially populated.  Clear it out.
    if ( !merge ) mCalendar->close();

    KMessageBox::error(this,i18n("Could not load calendar '%1'.", filename));

    return false;
  }
}

bool CalendarView::saveCalendar( const QString& filename )
{
  kDebug(5850) << "CalendarView::saveCalendar(): " << filename << endl;

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
  kDebug(5850) << "CalendarView::closeCalendar()" << endl;

  // child windows no longer valid
  emit closingDown();

  mCalendar->close();
  setModified( false );
  updateView();
}

void CalendarView::archiveCalendar()
{
  mDialogManager->showArchiveDialog();
}


void CalendarView::readSettings()
{
//  kDebug(5850) << "CalendarView::readSettings()" << endl;

  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::self()->config();

#ifndef KORG_NOSPLITTER
  KConfigGroup geometryConfig( config, "KOrganizer Geometry" );

  QList<int> sizes = geometryConfig.readEntry( "Separator1",QList<int>() );
  if ( sizes.count() != 2 ) {
    sizes << mDateNavigator->minimumSizeHint().width();
    sizes << 300;
  }
  mPanner->setSizes( sizes );

  sizes = geometryConfig.readEntry( "Separator2",QList<int>() );
  mLeftSplitter->setSizes( sizes );
#endif

  mEventViewer->readSettings( config );

  mViewManager->readSettings( config );
  mTodoList->restoreLayout( config, QString( "Todo Layout" ) );

  readFilterSettings( config );

  KConfigGroup viewConfig( config, "Views" );
  int dateCount = viewConfig.readEntry( "ShownDatesCount", 7 );
  if ( dateCount == 7 ) {
    mNavigator->selectWeek();
  } else {
    mNavigator->selectDates( mNavigator->selectedDates().first(), dateCount );
  }
}


void CalendarView::writeSettings()
{
//  kDebug(5850) << "CalendarView::writeSettings" << endl;

  KConfig *config = KOGlobals::self()->config();

#ifndef KORG_NOSPLITTER
  KConfigGroup geometryConfig( config, "KOrganizer Geometry" );

  QList<int> list = mPanner->sizes();
  geometryConfig.writeEntry( "Separator1", list );

  list = mLeftSplitter->sizes();
  geometryConfig.writeEntry( "Separator2", list );
#endif
  mEventViewer->writeSettings( config );
  mViewManager->writeSettings( config );
  mTodoList->saveLayout( config, QString( "Todo Layout" ) );

  KOPrefs::instance()->writeConfig();

  writeFilterSettings( config );

  KConfigGroup viewConfig( config, "Views" );
  viewConfig.writeEntry( "ShownDatesCount", mNavigator->selectedDates().count() );

  config->sync();
}

void CalendarView::readFilterSettings( KConfig *config )
{
//  kDebug(5850) << "CalendarView::readFilterSettings()" << endl;

  qDeleteAll( mFilters );
  mFilters.clear();

  KConfigGroup generalConfig( config, "General" );
  // FIXME: Move the filter loading and saving to the CalFilter class in libkcal
  QStringList filterList = generalConfig.readEntry ("CalendarFilters" , QStringList() );
  QString currentFilter = generalConfig.readEntry( "Current Filter" );

  QStringList::ConstIterator it = filterList.begin();
  QStringList::ConstIterator end = filterList.end();
  while( it != end ) {
//    kDebug(5850) << "  filter: " << (*it) << endl;
    CalFilter *filter;
    filter = new CalFilter( *it );
    KConfigGroup filterConfig( config, "Filter_" + (*it) );
    filter->setCriteria( filterConfig.readEntry( "Criteria", 0 ) );
    filter->setCategoryList( filterConfig.readEntry( "CategoryList" , QStringList() ) );
    if ( filter->criteria() & KCal::CalFilter::HideTodosWithoutAttendeeInEmailList )
      filter->setEmailList( KOPrefs::instance()->allEmails() );
    filter->setCompletedTimeSpan( filterConfig.readEntry( "HideTodoDays", 0 ) );
    mFilters.append( filter );

    ++it;
  }

  int pos = filterList.indexOf( currentFilter );
  mCurrentFilter = 0;
  if ( pos>=0 ) {
    mCurrentFilter = mFilters.at( pos );
  }
  updateFilter();
}

void CalendarView::writeFilterSettings( KConfig *config )
{
//  kDebug(5850) << "CalendarView::writeFilterSettings()" << endl;

  QStringList filterList;

  foreach ( CalFilter *filter, mFilters ) {
//    kDebug(5850) << " fn: " << filter->name() << endl;
    filterList << filter->name();
    KConfigGroup filterConfig( config, "Filter_" + filter->name() );
    filterConfig.writeEntry( "Criteria", filter->criteria() );
    filterConfig.writeEntry( "CategoryList", filter->categoryList() );
    filterConfig.writeEntry( "HideTodoDays", filter->completedTimeSpan() );
  }
  KConfigGroup generalConfig( config, "General" );
  generalConfig.writeEntry( "CalendarFilters", filterList );
  if ( mCurrentFilter ) {
    generalConfig.writeEntry( "Current Filter", mCurrentFilter->name() );
  } else {
    generalConfig.writeEntry( "Current Filter", QString() );
  }
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
  if ( dynamic_cast<KOMonthView*>( mViewManager->currentView() ) )
    mNavigator->selectNextMonth();
  else
    mNavigator->selectNext();
}

void CalendarView::goPrevious()
{
  if ( dynamic_cast<KOMonthView*>( mViewManager->currentView() ) )
    mNavigator->selectPreviousMonth();
  else
    mNavigator->selectPrevious();
}

void CalendarView::updateConfig( const QByteArray& receiver)
{
  if ( receiver != "korganizer" ) return;
  kDebug(5850) << "CalendarView::updateConfig()" << endl;

  KOGlobals::self()->
    setHolidays( new KHolidays( KOPrefs::instance()->mHolidays ) );

  // Only set a new time zone if it changed. This prevents the window
  // from being modified on start
  KDateTime::Spec newTimeSpec = KOPrefs::instance()->timeSpec();
  if ( mCalendar->viewTimeSpec() != newTimeSpec ) {

    const QString question( i18n("The time zone setting was changed. Do you want to keep the absolute time of "
                                "the items in your calendar, which will show them to be at a different time than "
                                "before, or move them to be at the old time also in the new time zone?") );
    int rc = KMessageBox::questionYesNo( this, question,
                              i18n("Keep Absolute Times?"),
                              KGuiItem(i18n("Keep Times")),
                              KGuiItem(i18n("Move Times")),
                              "calendarKeepAbsoluteTimes");
    if ( rc == KMessageBox::Yes ) {
      // keep the absolute time - note the new viewing time zone in the calendar
      mCalendar->setViewTimeSpec( newTimeSpec );
    } else {
      // only set the new timezone, wihtout shifting events, they will be
      // interpreted as being in the new timezone now
      mCalendar->shiftTimes( mCalendar->viewTimeSpec(), newTimeSpec );
    }
  }
  emit configChanged();
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
    kDebug(5850) << "Incidence modified and open" << endl;
    tmp->modified( what );
  }
  setModified( true );
  history()->recordEdit( oldIncidence, newIncidence );

  // Record completed todos in journals, if enabled. we should to this here in
  // favor of the todolist. users can mark a task as completed in an editor
  // as well.
  if ( newIncidence->type() == "Todo"
    && KOPrefs::instance()->recordTodosInJournals()
    &&  ( what == KOGlobals::COMPLETION_MODIFIED
    || what == KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE ) ) {

      Todo *todo = static_cast<Todo *>(newIncidence);
      if ( todo->isCompleted()
      || what == KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE ) {
        QString timeStr = KGlobal::locale()->formatTime( QTime::currentTime() );
        QString description = i18n( "Todo completed: %1 (%2)" ,
          newIncidence->summary(), timeStr );

        Journal::List journals = calendar()->journals( QDate::currentDate() );
        Journal *journal;

        if ( journals.isEmpty() ) {
          journal = new Journal();
          journal->setDtStart( QDateTime::currentDateTime() );

          QString dateStr = KGlobal::locale()->formatDate( QDate::currentDate() );
          journal->setSummary( i18n("Journal of %1", dateStr ) );
          journal->setDescription( description );

          if ( !mChanger->addIncidence( journal, this ) ) {
            KODialogManager::errorSaveIncidence( this, journal );
            delete journal;
            return;
          }

        } else { // journal list is not empty
          journal = journals.at(0);
          Journal *oldJournal = journal->clone();
          journal->setDescription( journal->description().append( '\n' + description ) );

          if ( !mChanger->changeIncidence( oldJournal, journal ) ) {
            KODialogManager::errorSaveIncidence( this, journal );
            delete journal;
            return;
          }
        }
      }
  }

  changeIncidenceDisplay( newIncidence, KOGlobals::INCIDENCEEDITED );
  updateUnmanagedViews();
  checkForFilteredChange( newIncidence );
}

void CalendarView::incidenceToBeDeleted( Incidence *incidence )
{
  KOIncidenceEditor *tmp = editorDialog( incidence );
  if (tmp) {
    kDebug(5850) << "Incidence to be deleted and open in editor" << endl;
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
    KMessageBox::information( this, i18n("The item \"%1\" is filtered by "
                 "your current filter rules, so it will be hidden and not "
                 "appear in the view.", incidence->summary() ),
                 i18n("Filter Applied"), "ChangedIncidenceFiltered" );
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
    mEventViewer->changeIncidenceDisplay( incidence, action );
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
  return KMessageBox::warningContinueCancel(
    this,
    i18n("The item \"%1\" will be permanently deleted.", incidence->summary() ),
    i18n("KOrganizer Confirmation"),
    KGuiItem( i18n("&Delete"),"edit-delete" ),
    KStandardGuiItem::cancel(),
    QString(),
    KMessageBox::Notify );
}


void CalendarView::edit_cut()
{
  Incidence *incidence = selectedIncidence();

  if ( !incidence || !mChanger ) {
    KNotification::beep();
    return;
  }
  mChanger->cutIncidence( incidence );
}

void CalendarView::edit_copy()
{
  Incidence *incidence = selectedIncidence();

  if (!incidence) {
    KNotification::beep();
    return;
  }
  DndFactory factory( mCalendar );
  if ( !factory.copyIncidence( incidence ) ) {
    KNotification::beep();
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
      if ( (pastedEvent->floats() && aView->selectedIsAllDay()) ||
           (!pastedEvent->floats() && ! aView->selectedIsAllDay()) ) {
        pastedEvent->setDtEnd(KDateTime( endDT, KOPrefs::instance()->timeSpec() ));
      }
    }
    mChanger->addIncidence( pastedEvent, this );

  } else if ( pastedIncidence->type() == "Todo" ) {
    Todo* pastedTodo = static_cast<Todo*>(pastedIncidence);
    Todo* _selectedTodo = selectedTodo();
    if ( _selectedTodo )
      pastedTodo->setRelatedTo( _selectedTodo );
    mChanger->addIncidence( pastedTodo, this );
  }
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
}


void CalendarView::dateTimesForNewEvent( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( !startDt.isValid() ) {
    // Default start is the first selected date with the preferred time as set
    // in the config dlg.
    if ( !startDt.date().isValid() ) {
      startDt.setDate( mNavigator->selectedDates().first() );
    }
    if ( !startDt.time().isValid() ) {
      startDt.setTime( KOPrefs::instance()->mStartTime.time() );
    }
  }
  if ( !endDt.isValid() ) {
    int addSecs = ( KOPrefs::instance()->mDefaultDuration.time().hour()*3600 ) +
                  ( KOPrefs::instance()->mDefaultDuration.time().minute()*60 );
    endDt = startDt.addSecs( addSecs );
  }
  mViewManager->currentView()->eventDurationHint( startDt, endDt, allDay );
}

KOEventEditor *CalendarView::newEventEditor( const QDateTime &startDtParam,
     const QDateTime &endDtParam, bool allDayParam)
{
  // let the current view change the default start/end datetime
  bool allDay = allDayParam;
  QDateTime startDt( startDtParam ), endDt( endDtParam );
  // Adjust the start/end date times (i.e. replace invalid values by defaults,
  // and let the view adjust the type.
  dateTimesForNewEvent( startDt, endDt, allDay );

  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent();
  connectIncidenceEditor( eventEditor );
  eventEditor->setDates( startDt, endDt, allDay );
  mDialogManager->connectTypeAhead( eventEditor, viewManager()->agendaView() );
  return eventEditor;
}




void CalendarView::newEvent()
{
  kDebug(5850) << "CalendarView::newEvent()" << endl;
  newEvent( QDateTime(), QDateTime() );
}

void CalendarView::newEvent( const QDate &dt )
{
  QDateTime startDt( dt, KOPrefs::instance()->mStartTime.time() );
  return newEvent( QDateTime( dt ), QDateTime() );
}

void CalendarView::newEvent( const QDateTime &startDt )
{
  return newEvent( startDt, QDateTime() );
}

void CalendarView::newEvent( const QDateTime &startDt, const QDateTime &endDt,
                             bool allDay )
{
  KOEventEditor *eventEditor = newEventEditor( startDt, endDt, allDay );
  eventEditor->show();
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QStringList &attachments, const QStringList &attendees )
{
  KOEventEditor *eventEditor = newEventEditor();
  eventEditor->setTexts( summary, description );
  // if attach or attendee list is empty, these methods don't do anything, so
  // it's save to call them in every case
  eventEditor->addAttachments( attachments );
  eventEditor->addAttendees( attendees );
  eventEditor->show();
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                            const QStringList &attachments, const QStringList &attendees )
{
  kDebug(5850) << k_funcinfo << endl;
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  todoEditor->setTexts( summary, description );
  todoEditor->addAttachments( attachments );
  todoEditor->addAttendees( attendees );
  todoEditor->setDates( QDateTime(), false );
  todoEditor->show();
}

void CalendarView::newTodo()
{
  kDebug(5850) << k_funcinfo << endl;
  QDateTime dtDue;
  bool allday = true;
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  if ( mViewManager->currentView()->isEventView() ) {
    dtDue.setDate( mNavigator->selectedDates().first() );
    QDateTime dtDummy = QDateTime::currentDateTime();
    mViewManager->currentView()->
      eventDurationHint( dtDue, dtDummy, allday );
    todoEditor->setDates( dtDue, allday );
  }
  todoEditor->show();
}

void CalendarView::newTodo( const QDate &date )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  todoEditor->setDates( QDateTime( date, QTime::currentTime() ), true );
  todoEditor->show();
}

void CalendarView::newJournal()
{
  kDebug(5850) << "CalendarView::newJournal()" << endl;
  newJournal( QString::null, QDate() );
}

void CalendarView::newJournal( const QDate &date )
{
  newJournal( QString::null, date );
}

void CalendarView::newJournal( const QString &text, const QDate &date )
{
  KOJournalEditor *journalEditor = mDialogManager->getJournalEditor();
  connectIncidenceEditor( journalEditor );
  journalEditor->newJournal();
  journalEditor->setTexts( text );
  if ( !date.isValid() ) {
    journalEditor->setDate( mNavigator->selectedDates().first() );
  } else {
    journalEditor->setDate( date );
  }
  journalEditor->show();
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
  todoEditor->newTodo();
  todoEditor->setDates( QDateTime(), false, parentEvent );
  todoEditor->show();
}

void CalendarView::newFloatingEvent()
{
  DateList tmpList = mNavigator->selectedDates();
  QDate date = tmpList.first();

  newEvent( QDateTime( date, QTime( 12, 0, 0 ) ),
            QDateTime( date, QTime( 12, 0, 0 ) ), true );
}

bool CalendarView::addIncidence( const QString &ical )
{
  kDebug(5850) << "CalendarView::addIncidence:\n" << ical << endl;
  ICalFormat format;
  format.setTimeSpec( mCalendar->timeSpec() );
  Incidence *incidence = format.fromString( ical );
  if ( !incidence ) return false;
  if ( !mChanger->addIncidence( incidence, this ) ) {
    delete incidence;
    return false;
  }
  return true;
}

void CalendarView::appointment_show()
{
  Incidence *incidence = selectedIncidence();
  if (incidence)
    showIncidence( incidence );
  else
    KNotification::beep();
}

void CalendarView::appointment_edit()
{
  Incidence *incidence = selectedIncidence();
  if (incidence)
    editIncidence( incidence );
  else
    KNotification::beep();
}

void CalendarView::appointment_delete()
{
  Incidence *incidence = selectedIncidence();
  if (incidence)
    deleteIncidence( incidence );
  else
    KNotification::beep();
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
  bool status= false;
  if ( !todo || !todo->relatedTo() ) return false;

  if ( mChanger->beginChange( todo ) ) {
      Todo *oldTodo = todo->clone();
      todo->setRelatedTo(0);
      mChanger->changeIncidence( oldTodo, todo, KOGlobals::RELATION_MODIFIED );
      mChanger->endChange( todo );
      delete oldTodo;
      setModified(true);
      status = true;
  }
  if ( ! status ) {
    KMessageBox::sorry( this, i18n("Unable to turn sub-to-do into a top-level "
        "to-do, because it cannot be locked.") );
  }

  return status;
}

bool CalendarView::makeSubTodosIndependents ( )
{
  bool  status = false;
  Todo *anTodo = selectedTodo();

  if( makeSubTodosIndependents( anTodo ) ) {
    updateView();
    status = true;
  }
  return status;
}

bool CalendarView::makeSubTodosIndependents ( Todo *todo )
{
  if( !todo || todo->relations().isEmpty() ) return false;

  startMultiModify ( i18n( "Make sub-to-dos independent" ) );
  Incidence::List subTodos( todo->relations() );
  Incidence::List::Iterator it;
  Incidence *aIncidence;
  Todo *aTodo;

  for ( it= subTodos.begin(); it != subTodos.end(); ++it ) {
    aIncidence = *it;
    if( aIncidence && aIncidence->type() == "Todo" ) {
      aTodo = static_cast<Todo*>( aIncidence );
      todo_unsub ( aTodo );
    }
  }
  endMultiModify();
  return true;
}

bool CalendarView::deleteIncidence( const QString &uid, bool force )
{
  Incidence *inc = mCalendar->incidence( uid );
  if ( inc ) {
    deleteIncidence( inc, force );
    return true;
  } else {
    return false;
  }
}

void CalendarView::toggleAlarm( Incidence *incidence )
{
  if ( !incidence || !mChanger ) {
    kDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  Incidence*oldincidence = incidence->clone();
  if ( !mChanger->beginChange( incidence ) ) {
    kDebug(5850) << "Unable to lock incidence " << endl;
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
    kDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  if ( !mChanger->beginChange( incidence ) ) {
    kDebug(5850) << "Unable to lock incidence " << endl;
    return;
  }
  startMultiModify( i18n("Dissociate occurrence") );
  Incidence*oldincidence = incidence->clone();

  Incidence* newInc = mCalendar->dissociateOccurrence( incidence, date, true );

  if ( newInc ) {
    // TODO: Use the same resource instead of asking again!
    mChanger->changeIncidence( oldincidence, incidence );
    mChanger->addIncidence( newInc, this );
  } else {
    KMessageBox::sorry( this, i18n("Dissociating the occurrence failed."),
      i18n("Dissociating Failed") );
  }
  mChanger->endChange( incidence );
  endMultiModify();
  delete oldincidence;
}

void CalendarView::dissociateFutureOccurrence( Incidence *incidence, const QDate &date )
{
  if ( !incidence || !mChanger ) {
    kDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  if ( !mChanger->beginChange( incidence ) ) {
    kDebug(5850) << "Unable to lock incidence " << endl;
    return;
  }
  startMultiModify( i18n("Dissociate future occurrences") );
  Incidence*oldincidence = incidence->clone();

  Incidence* newInc = mCalendar->dissociateOccurrence( incidence, date, true );
  if ( newInc ) {
    // TODO: Use the same resource instead of asking again!
    mChanger->changeIncidence( oldincidence, incidence );
    mChanger->addIncidence( newInc, this );
  } else {
    KMessageBox::sorry( this, i18n("Dissociating the future occurrences failed."),
      i18n("Dissociating Failed") );
  }
  endMultiModify();
  mChanger->endChange( incidence );
  delete oldincidence;
}


/*****************************************************************************/


void CalendarView::schedule_publish(Incidence *incidence)
{
  if (incidence == 0)
    incidence = selectedIncidence();

  if (!incidence) {
    KMessageBox::information( this, i18n("No item selected."),
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
  if ( publishdlg->exec() == QDialog::Accepted ) {
    Incidence *inc = incidence->clone();
    inc->registerObserver( 0 );
    inc->clearAttendees();

    // Send the mail
    KCal::MailScheduler scheduler( mCalendar );
    if ( scheduler.publish( incidence, publishdlg->addresses() ) ) {
      KMessageBox::information( this, i18n("The item information was successfully sent."),
                                i18n("Publishing"), "IncidencePublishSuccess" );
    } else {
      KMessageBox::error( this, i18n("Unable to publish the item '%1'", incidence->summary() ) );
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
  KDateTime start = KDateTime::currentUtcDateTime().toTimeSpec(mCalendar->timeSpec());
  KDateTime end = start.addDays(daysToPublish);

  FreeBusy *freebusy = new FreeBusy(mCalendar, start, end);
  freebusy->setOrganizer( Person( KOPrefs::instance()->fullName(),
                      KOPrefs::instance()->email() ) );

  kDebug(5850) << "calendarview: schedule_publish_freebusy: startDate: "
     << KGlobal::locale()->formatDateTime( start ) << " End Date: "
     << KGlobal::locale()->formatDateTime( end ) << endl;

  PublishDialog *publishdlg = new PublishDialog();
  if ( publishdlg->exec() == QDialog::Accepted ) {
    // Send the mail
    KCal::MailScheduler scheduler( mCalendar );
    if ( scheduler.publish( freebusy, publishdlg->addresses() ) ) {
      KMessageBox::information( this, i18n("The free/busy information was successfully sent."),
                                i18n("Sending Free/Busy"), "FreeBusyPublishSuccess" );
    } else {
      KMessageBox::error( this, i18n("Unable to publish the free/busy data.") );
    }
  }
  delete freebusy;
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
    KMessageBox::sorry( this, i18n("No item selected."),
                        "ScheduleNoEventSelected" );
    return;
  }

  if( incidence->attendeeCount() == 0 && method != Scheduler::Publish ) {
    KMessageBox::information( this, i18n("The item has no attendees."),
                              "ScheduleNoIncidences" );
    return;
  }

  Incidence *inc = incidence->clone();
  inc->registerObserver( 0 );
  inc->clearAttendees();

  // Send the mail
  KCal::MailScheduler scheduler( mCalendar );
  if ( !scheduler.performTransaction( incidence, method ) ) {
    KMessageBox::information( this, i18n("The groupware message for item '%1'"
                                "was successfully sent.\nMethod: %2",
                                  incidence->summary() ,
                                  Scheduler::methodName( method ) ),
                              i18n("Sending Free/Busy"),
                              "FreeBusyPublishSuccess" );
  } else {
    KMessageBox::error( this, i18nc("Groupware message sending failed. "
                        "%2 is request/reply/add/cancel/counter/etc.",
                        "Unable to send the item '%1'.\nMethod: %2",
                          incidence->summary() ,
                          Scheduler::methodName( method ) ) );
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
  QString filename = KFileDialog::getSaveFileName(KUrl("icalout.ics"),i18n("*.ics|ICalendars"),this);

  // Force correct extension
  if (filename.right(4) != ".ics") filename += ".ics";

  FileStorage storage( mCalendar, filename, new ICalFormat );
  storage.save();
}

void CalendarView::exportVCalendar()
{
  if (mCalendar->journals().count() > 0) {
    int result = KMessageBox::warningContinueCancel(
      this,
      i18n("The journal entries can not be exported to a vCalendar file."),
      i18n("Data Loss Warning"),
      KGuiItem(i18n("Proceed")),
      KStandardGuiItem::cancel(),
      QString( "dontaskVCalExport" ),
      KMessageBox::Notify );
    if (result != KMessageBox::Continue) return;
  }

  QString filename = KFileDialog::getSaveFileName(KUrl("vcalout.vcs"),i18n("*.vcs|vCalendars"),this);

	// TODO: I don't like forcing extensions:
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
  if (ICalDrag::canDecode(QApplication::clipboard()->mimeData())) {
    kDebug(5850) << "CalendarView::checkClipboard() true" << endl;
    emit pasteEnabled(true);
  } else {
    kDebug(5850) << "CalendarView::checkClipboard() false" << endl;
    emit pasteEnabled(false);
  }
#endif
}

void CalendarView::showDates(const DateList &selectedDates)
{
//  kDebug(5850) << "CalendarView::selectDates()" << endl;

  if ( mViewManager->currentView() ) {
    updateView( selectedDates.first(), selectedDates.last() );
  } else {
    mViewManager->showAgendaView();
  }
}

void CalendarView::editFilters()
{
  kDebug(5850) << "CalendarView::editFilters()" << endl;

  foreach ( CalFilter *filter, mFilters ) {
    if ( filter ) kDebug(5850) << " Filter: " << filter->name() << endl;
  }

  mDialogManager->showFilterEditDialog(&mFilters);
}

/** Filter configuration changed
*/
void CalendarView::updateFilter()
{
  QStringList filters;
  CalFilter *filter;

  int pos = mFilters.indexOf( mCurrentFilter );
  if ( pos < 0 ) {
    mCurrentFilter = 0;
  }

  filters << i18n("No filter");
  foreach ( filter, mFilters ) {
    if ( filter ) filters << filter->name();
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
  emit filterChanged();
}

QString CalendarView::currentFilterName() const
{
  if ( mCurrentFilter) {
    return mCurrentFilter->name();
  } else return i18n("No filter");
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

  for ( it = incidences.begin(); it != incidences.end(); ++it ) {
    (*it)->setOrganizer( Person( KOPrefs::instance()->fullName(),
                         KOPrefs::instance()->email() ) );
    (*it)->recreate();
    (*it)->setReadOnly(false);
  }
  updateView();
}

void CalendarView::showIntro()
{
  kDebug(5850) << "To be implemented." << endl;
}

void CalendarView::showDateNavigator( bool show )
{
  if( show )
    mDateNavigator->show();
  else
    mDateNavigator->hide();
}

void CalendarView::showTodoView( bool show )
{
  if( show )
    mTodoList->show();
  else
    mTodoList->hide();
}

void CalendarView::showEventViewer( bool show )
{
  if( show )
    mEventViewer->show();
  else
    mEventViewer->hide();
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
  kDebug(5850) << "CalendarView::editIncidence()" << endl;
  return editIncidence( mCalendar->incidence( uid ) );
}

bool CalendarView::showIncidence( const QString& uid )
{
  Incidence *incidence = mCalendar->incidence( uid );
  if ( !incidence )
    return false;
  showIncidence( incidence );
  return true;
}

bool CalendarView::showIncidenceContext( const QString& uid )
{
  Incidence *incidence = mCalendar->incidence( uid );
  if ( !incidence )
    return false;
  showIncidenceContext( incidence );
  return true;
}

void CalendarView::deleteIncidence()
{
  deleteIncidence( selectedIncidence() );
}

void CalendarView::cutIncidence(Incidence *)
{
  edit_cut();
}

void CalendarView::copyIncidence(Incidence *)
{
  edit_copy();
}

void CalendarView::showIncidence( Incidence *incidence )
{
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog( this );
  eventViewer->setIncidence( incidence );
  eventViewer->show();
}

void CalendarView::showIncidenceContext( Incidence *incidence )
{
  if ( dynamic_cast<KCal::Event *>( incidence ) ) {
    if ( !viewManager()->currentView()->inherits( "KOEventView" ) )
      viewManager()->showAgendaView();
    // just select the appropriate date
    mNavigator->selectWeek( incidence->dtStart().date() );
    return;
  } else if ( dynamic_cast<KCal::Journal *>( incidence ) ) {
    if ( !viewManager()->currentView()->inherits( "KOJournalView" ) )
      viewManager()->showJournalView();
  } else if ( dynamic_cast<KCal::Todo *>( incidence ) ) {
    if ( !viewManager()->currentView()->inherits( "KOTodoView" ) )
      viewManager()->showTodoView();
  }
  Incidence::List list;
  list.append( incidence );
  viewManager()->currentView()->showIncidences( list );
}

bool CalendarView::editIncidence( Incidence *incidence )
{
  kDebug(5850) << "CalendarView::editEvent()" << endl;

  if ( !incidence || !mChanger ) {
    KNotification::beep();
    return false;
  }
  KOIncidenceEditor *tmp = editorDialog( incidence );
  if ( tmp ) {
    kDebug(5850) << "CalendarView::editIncidence() in List" << endl;
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

  kDebug(5850) << "CalendarView::editIncidence() new IncidenceEditor" << endl;
  KOIncidenceEditor *incidenceEditor = mDialogManager->getEditor( incidence );
  connectIncidenceEditor( incidenceEditor );

  mDialogList.insert( incidence, incidenceEditor );
  incidenceEditor->editIncidence( incidence );
  incidenceEditor->show();
  return true;
}

void CalendarView::deleteSubTodosIncidence ( Todo *todo )
{
  if( !todo ) return;

  Incidence::List subTodos( todo->relations() );
  Incidence::List::Iterator it;
  Incidence *aIncidence;
  Todo *aTodo;

  for ( it= subTodos.begin(); it != subTodos.end(); ++it ) {
    aIncidence = *it;
    if( aIncidence && aIncidence->type() == "Todo" ) {
      aTodo = static_cast<Todo*>( aIncidence );
      deleteSubTodosIncidence ( aTodo );
    }
  }
  mChanger->deleteIncidence ( todo );
}

void CalendarView::deleteTodoIncidence ( Todo *todo, bool force )
{
  if ( !todo ) return ;

  // it a simple todo, ask and delete it.
  if (todo->relations().isEmpty() ) {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( todo ) == KMessageBox::Continue );
    }
    if ( doDelete )
      mChanger->deleteIncidence( todo );
    return;
  }

  /* Ok, this to-do has sub-to-dos, ask what to do */
  int km = KMessageBox::No;
  if ( !force ) {
    km=KMessageBox::questionYesNoCancel( this,
                                i18n("The item \"%1\" has sub-to-dos. "
                                     "Do you want to delete just this item and "
                                     "make all its sub-to-dos independent, or "
                                     "delete the to-do with all its sub-to-dos?"
                               , todo->summary() ),
                                i18n("KOrganizer Confirmation"),
                                KGuiItem(i18n("Delete Only This")),
                                KGuiItem(i18n("Delete All")));
  }
  startMultiModify( i18n("Deleting sub-to-dos" ) );
  // Delete only the father
  if( km == KMessageBox::Yes ) {

    makeSubTodosIndependents ( todo );
    mChanger->deleteIncidence( todo );
  } else if ( km == KMessageBox::No ) {
    // Delete all
    // we have to hide the delete confirmation for each itemDate
    deleteSubTodosIncidence ( todo );
  }
  endMultiModify();
}

void CalendarView::deleteIncidence(Incidence *incidence, bool force)
{
  if ( !incidence || !mChanger ) {
    if ( !force ) {
      KNotification::beep();
    }
    return;
  }
  if ( incidence->isReadOnly() ) {
    if ( !force ) {
      KMessageBox::information( this, i18n("The item \"%1\" is marked read-only "
                                "and cannot be deleted; it probably belongs to "
                                "a read-only calendar resource.",
                                 incidence->summary()),
                                i18n("Removing not possible"),
                                "deleteReadOnlyIncidence" );
    }
    return;
  }

  CanDeleteIncidenceVisitor v;

  // Let the visitor do special things for special incidence types.
  // e.g. todos with children cannot be deleted, so act(..) returns false
  if ( !v.act( incidence, this ) )
    return;
  //If it is a todo, there are specific delete function

  if ( incidence && incidence->type()=="Todo" ) {
    deleteTodoIncidence( static_cast<Todo*>(incidence), force );
    return;
  }

  if ( incidence->doesRecur() ) {
    QDate itemDate = mViewManager->currentSelectionDate();
    kDebug(5850) << "Recurrence-Date: " << itemDate.toString() << endl;
    int km = KMessageBox::Ok;
    if ( !force ) {
      if ( !itemDate.isValid() ) {
        kDebug(5850) << "Date Not Valid" << endl;
        km = KMessageBox::warningContinueCancel(
          this,
          i18n("The calendar item \"%1\" recurs over multiple dates; "
               "are you sure you want to delete it "
               "and all its recurrences?", incidence->summary() ),
          i18n("KOrganizer Confirmation"),
          KGuiItem(i18n("Delete All")) );
      } else {
        km = KOMessageBox::fourBtnMsgBox(
          this,
          QMessageBox::Warning,
          i18n("The calendar item \"%1\" recurs over multiple dates. "
               "Do you want to delete only the current one on %2, only all "
               "future recurrences, or all its recurrences?",
            incidence->summary() ,
            KGlobal::locale()->formatDate( itemDate ) ),
          i18n("KOrganizer Confirmation"),
          KGuiItem(i18n("Delete C&urrent")),
          KGuiItem(i18n("Delete &Future")),
          KGuiItem(i18n("Delete &All")) );
      }
    }
    switch(km) {
      case KMessageBox::Ok: // Continue // all
      case KMessageBox::Continue:
        mChanger->deleteIncidence( incidence );
        break;

      case KMessageBox::Yes: // just this one
        if ( mChanger->beginChange( incidence ) ) {
          Incidence *oldIncidence = incidence->clone();
          incidence->recurrence()->addExDate( itemDate );
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
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( incidence ) == KMessageBox::Continue );
    }
    if ( doDelete ) {
      mChanger->deleteIncidence( incidence );
      processIncidenceSelection( 0 );
    }
  }
}

void CalendarView::connectIncidenceEditor( KOIncidenceEditor *editor )
{
  connect( this, SIGNAL( newIncidenceChanger( IncidenceChangerBase* ) ),
           editor, SLOT( setIncidenceChanger( IncidenceChangerBase* ) ) );
  editor->setIncidenceChanger( mChanger );
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
  int result = KMessageBox::warningContinueCancel(
    this,
    i18n("Delete all completed to-dos?"),
    i18n("Purge To-dos"),
    KGuiItem(i18n("Purge")) );

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
      KMessageBox::information( this, i18n("Unable to purge to-dos with "
                                "uncompleted children."), i18n("Delete To-do"),
                                "UncompletedChildrenPurgeTodos" );
    }
  }
}

void CalendarView::slotCalendarChanged()
{
  kDebug(5850) << "CalendarView::slotCalendarChanged()" << endl;

  updateView();
}

void CalendarView::warningChangeFailed( Incidence * )
{
  KMessageBox::sorry( this, i18n("Unable to edit item: "
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
  QStringList allCats( calendar()->categories() );
  allCats.sort();
  QStringList categories( KOPrefs::instance()->mCustomCategories );
  for ( QStringList::ConstIterator si = allCats.constBegin(); si != allCats.constEnd(); ++si ) {
    if ( !categories.contains( *si )  ) {
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
    KMessageBox::sorry(this, i18n("Unable to copy the item to %1.",
                         dt.toString() ), i18n("Copying Failed") );
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
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } else if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>(incidence);
    KDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }

  if ( !mChanger->addIncidence( incidence, this ) ) {
    KODialogManager::errorSaveIncidence( this, incidence );
    delete incidence;
  }
}

void CalendarView::moveIncidenceTo( Incidence *incmove, const QDate &dt )
{
  if ( !incmove || !mChanger ) {
    KMessageBox::sorry( this, i18n("Unable to move the item to  %1.",
                          dt.toString() ), i18n("Moving Failed") );
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
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } else if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>(incidence);
    KDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }
  mChanger->changeIncidence( oldIncidence, incidence );
  mChanger->endChange( incidence );
  delete oldIncidence;
}

#include "calendarview.moc"
