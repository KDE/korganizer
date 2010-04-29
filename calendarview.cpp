/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998, 1999
    Preston Brown (preston.brown@yale.edu)
    Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
    Ian Dawes (iadawes@globalserve.net)
    Laszlo Boloni (boloni@cs.purdue.edu)

    Copyright (c) 2000, 2001, 2002, 2003, 2004
    Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include "komailclient.h"
#include "multiagendaview.h"

#include <libkcal/calhelper.h>
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
#include <qwhatsthis.h>

#include <stdlib.h>
#include <assert.h>

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

  mFilters.setAutoDelete( true );

  mExtensions.setAutoDelete( true );

  mDateNavigator = new DateNavigator( this );
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

  mDateNavigatorContainer = new DateNavigatorContainer( mLeftSplitter,
                                               "CalendarView::DateNavigator" );

//  mLeftSplitter->setResizeMode( mDateNavigatorContainer, QSplitter::Stretch );
  mLeftSplitter->setCollapsible( mDateNavigatorContainer, true );
  mTodoList = new KOTodoView( CalendarNull::self(), mLeftSplitter, "todolist" );

  mEventViewer = new KOEventViewer( CalendarNull::self(), mLeftSplitter,"EventViewer" );

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

  mDateNavigatorContainer = new KDateNavigator( leftFrame, true,
                                       "CalendarView::DateNavigator",
                                       QDate::currentDate() );
  mTodoList = new KOTodoView( CalendarNull::self(), leftFrame, "todolist" );

  mEventViewer = new KOEventViewer ( CalendarNull::self(), leftFrame, "EventViewer" );

  QWidget *rightBox = new QWidget( mainBox );
  QBoxLayout *rightLayout = new QVBoxLayout( rightBox );

  mNavigatorBar = new NavigatorBar( QDate::currentDate(), rightBox );
  rightLayout->addWidget( mNavigatorBar );

  mRightFrame = new QWidgetStack( rightBox );
  rightLayout->addWidget( mRightFrame );

  mLeftFrame = leftFrame;

  if ( KOPrefs::instance()->mVerticalScreen ) {
    // mTodoList->setFixedHeight( 60 );
    mTodoList->setFixedHeight( mDateNavigatorContainer->sizeHint().height() );
  }
#endif

  // Signals emited by mDateNavigator
  connect( mDateNavigator, SIGNAL( datesSelected( const KCal::DateList &, const QDate & ) ),
           SLOT( showDates( const KCal::DateList &, const QDate & ) ) );

  // Signals emited by mNavigatorBar
  connect( mNavigatorBar, SIGNAL( prevYearClicked() ),
           mDateNavigator, SLOT( selectPreviousYear() ) );
  connect( mNavigatorBar, SIGNAL( nextYearClicked() ),
           mDateNavigator, SLOT( selectNextYear() ) );
  connect( mNavigatorBar, SIGNAL( prevMonthClicked() ),
           mDateNavigator, SLOT( selectPreviousMonth() ) );
  connect( mNavigatorBar, SIGNAL( nextMonthClicked() ),
           mDateNavigator, SLOT( selectNextMonth() ) );
  connect( mNavigatorBar, SIGNAL( monthSelected(int) ),
           mDateNavigator, SLOT( selectMonth(int) ) );
  connect( mNavigatorBar, SIGNAL( yearSelected(int)),
           mDateNavigator, SLOT(selectYear(int)) );


  // Signals emited by mDateNavigatorContainer
  connect( mDateNavigatorContainer, SIGNAL( weekClicked( const QDate & ) ),
           this, SLOT( selectWeek( const QDate & ) ) );
  connect( mDateNavigatorContainer, SIGNAL( prevMonthClicked(const QDate &, const QDate &, const QDate &) ),
           mDateNavigator, SLOT( selectPreviousMonth(const QDate &, const QDate &, const QDate &) ) );
  connect( mDateNavigatorContainer, SIGNAL( nextMonthClicked(const QDate &, const QDate &, const QDate &) ),
           mDateNavigator, SLOT( selectNextMonth(const QDate &, const QDate &, const QDate &) ) );
  connect( mDateNavigatorContainer, SIGNAL( prevYearClicked() ),
           mDateNavigator, SLOT( selectPreviousYear() ) );
  connect( mDateNavigatorContainer, SIGNAL( nextYearClicked() ),
           mDateNavigator, SLOT( selectNextYear() ) );
  connect( mDateNavigatorContainer, SIGNAL( monthSelected(int) ),
           mDateNavigator, SLOT( selectMonth(int) ) );
  connect( mDateNavigatorContainer, SIGNAL(yearSelected(int)),
           mDateNavigator, SLOT(selectYear(int)) );
  connect( mDateNavigatorContainer, SIGNAL( goPrevious() ),
           mDateNavigator, SLOT( selectPrevious() ) );
  connect( mDateNavigatorContainer, SIGNAL( goNext() ),
           mDateNavigator, SLOT( selectNext() ) );

  connect( mDateNavigatorContainer, SIGNAL( datesSelected( const KCal::DateList & ) ),
           mDateNavigator, SLOT( selectDates( const KCal::DateList & ) ) );

  connect( mDateNavigatorContainer, SIGNAL(incidenceDropped(Incidence*, const QDate&)),
           SLOT( addIncidenceOn( Incidence *, const QDate & ) ) );
  connect( mDateNavigatorContainer, SIGNAL(incidenceDroppedMove(Incidence*,const QDate&)),
           SLOT( moveIncidenceTo( Incidence *, const QDate & ) ) );

  connect( mDateChecker, SIGNAL( dayPassed( const QDate & ) ),
           mTodoList, SLOT( dayPassed( const QDate & ) ) );
  connect( mDateChecker, SIGNAL( dayPassed( const QDate & ) ),
           SIGNAL( dayPassed( const QDate & ) ) );
  connect( mDateChecker, SIGNAL( dayPassed( const QDate & ) ),
           mDateNavigatorContainer, SLOT( updateToday() ) );

  connect( this, SIGNAL( configChanged() ),
           mDateNavigatorContainer, SLOT( updateConfig() ) );

  connect( this, SIGNAL( incidenceSelected(Incidence *, const QDate &) ),
           mEventViewer, SLOT ( setIncidence (Incidence *, const QDate &) ) );

  //TODO: do a pretty Summary,
  QString s;
  s = i18n( "<p><em>No Item Selected</em></p>"
           "<p>Select an event, to-do or journal entry to view its details "
           "here.</p>");

  mEventViewer->setDefaultText( s );
  QWhatsThis::add( mEventViewer,
                   i18n( "View the details of events, journal entries or to-dos "
                         "selected in KOrganizer's main view here." ) );
  mEventViewer->setIncidence( 0, QDate() );

  mViewManager->connectTodoView( mTodoList );
  mViewManager->connectView( mTodoList );

  KOGlobals::self()->
      setHolidays( new KHolidays( KOPrefs::instance()->mHolidays ) );

  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( checkClipboard() ) );

  connect( mTodoList, SIGNAL( incidenceSelected( Incidence *,const QDate & ) ),
           SLOT( processTodoListSelection( Incidence *,const QDate & ) ) );
  disconnect( mTodoList, SIGNAL( incidenceSelected( Incidence *,const QDate & ) ),
           this, SLOT( processMainViewSelection( Incidence *,const QDate & ) ) );

  kdDebug(5850) << "CalendarView::CalendarView() done" << endl;
}

CalendarView::~CalendarView()
{
  kdDebug(5850) << "~CalendarView()" << endl;

  mCalendar->unregisterObserver( this );

  delete mDialogManager;
  delete mViewManager;
  delete mEventViewer;
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

  mDateNavigatorContainer->setCalendar( mCalendar );

  mTodoList->setCalendar( mCalendar );

  mEventViewer->setCalendar( mCalendar );
}

void CalendarView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  emit newIncidenceChanger( mChanger );
  connect( mChanger, SIGNAL( incidenceAdded( Incidence* ) ),
           this, SLOT( incidenceAdded( Incidence* ) ) );
  connect( mChanger, SIGNAL( incidenceChanged( Incidence*, Incidence*, KOGlobals::WhatChanged ) ),
           this, SLOT( incidenceChanged( Incidence*, Incidence*, KOGlobals::WhatChanged ) ) );
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

QPair<ResourceCalendar *, QString> CalendarView::viewSubResourceCalendar()
{
  QPair<ResourceCalendar *, QString> p( 0, QString() );
  KOrg::BaseView *cV = mViewManager->currentView();
  if ( cV && cV == mViewManager->multiAgendaView() ) {
    cV = mViewManager->multiAgendaView()->selectedAgendaView();
  }
  if ( cV ) {
    p = qMakePair( cV->resourceCalendar(), cV->subResourceCalendar() );
  }
  return p;
}

KOIncidenceEditor *CalendarView::editorDialog( Incidence *incidence ) const
{
  if (mDialogList.find(incidence) != mDialogList.end ())
    return mDialogList[incidence];
  else return 0;
}

QDate CalendarView::activeDate( bool fallbackToToday )
{
  KOrg::BaseView *curView = mViewManager->currentView();
  if ( curView ) {
    if ( curView->selectionStart().isValid() ) {
      return curView->selectionStart().date();
    }

    // Try the view's selectedDates()
    if ( !curView->selectedIncidenceDates().isEmpty() ) {
      if ( curView->selectedIncidenceDates().first().isValid() ) {
        return curView->selectedIncidenceDates().first();
      }
    }
  }

  // When all else fails, use the navigator start date, or today.
  if ( fallbackToToday ) {
    return QDate::currentDate();
  } else {
    return mDateNavigator->selectedDates().first();
  }
}

QDate CalendarView::activeIncidenceDate()
{
  KOrg::BaseView *curView = mViewManager->currentView();
  if ( curView ) {
    DateList dates = curView->selectedIncidenceDates();
    if ( !dates.isEmpty() ) {
      return dates.first();
    }
  }

  return QDate();
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
  setModified( false );
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
  config->setGroup( "KOrganizer Geometry" );

  QValueList<int> sizes = config->readIntListEntry( "Separator1" );
  if ( sizes.count() != 2 ) {
    sizes << mDateNavigatorContainer->minimumSizeHint().width();
    sizes << 300;
  }
  mPanner->setSizes( sizes );

  sizes = config->readIntListEntry( "Separator2" );
  mLeftSplitter->setSizes( sizes );
#endif

  mEventViewer->readSettings( config );

  mViewManager->readSettings( config );
  mTodoList->restoreLayout( config, QString( "Todo Layout" ) );

  readFilterSettings( config );

  config->setGroup( "Views" );
  int dateCount = config->readNumEntry( "ShownDatesCount", 7 );
  if ( dateCount == 7 ) mDateNavigator->selectWeek();
  else mDateNavigator->selectDates( mDateNavigator->selectedDates().first(), dateCount );
}


void CalendarView::writeSettings()
{
//  kdDebug(5850) << "CalendarView::writeSettings" << endl;

  KConfig *config = KOGlobals::self()->config();

#ifndef KORG_NOSPLITTER
  config->setGroup( "KOrganizer Geometry" );

  QValueList<int> list = mPanner->sizes();
  config->writeEntry( "Separator1", list );

  list = mLeftSplitter->sizes();
  config->writeEntry( "Separator2", list );
#endif
  mEventViewer->writeSettings( config );
  mViewManager->writeSettings( config );
  mTodoList->saveLayout( config, QString( "Todo Layout" ) );

  KOPrefs::instance()->writeConfig();

  writeFilterSettings( config );

  config->setGroup( "Views" );
  config->writeEntry( "ShownDatesCount", mDateNavigator->selectedDates().count() );

  config->sync();
}

void CalendarView::readFilterSettings( KConfig *config )
{
//  kdDebug(5850) << "CalendarView::readFilterSettings()" << endl;

  mFilters.clear();

  config->setGroup( "General" );
  // FIXME: Move the filter loading and saving to the CalFilter class in libkcal
  QStringList filterList = config->readListEntry ("CalendarFilters" );
  QString currentFilter = config->readEntry( "Current Filter" );

  QStringList::ConstIterator it = filterList.begin();
  QStringList::ConstIterator end = filterList.end();
  while( it != end ) {
//    kdDebug(5850) << "  filter: " << (*it) << endl;
    CalFilter *filter;
    filter = new CalFilter( *it );
    config->setGroup( "Filter_" + (*it) );
    filter->setCriteria( config->readNumEntry( "Criteria", 0 ) );
    filter->setCategoryList( config->readListEntry( "CategoryList" ) );
    if ( filter->criteria() & KCal::CalFilter::HideTodosWithoutAttendeeInEmailList )
      filter->setEmailList( KOPrefs::instance()->allEmails() );
    filter->setCompletedTimeSpan( config->readNumEntry( "HideTodoDays", 0 ) );
    mFilters.append( filter );

    ++it;
  }

  config->setGroup( "General" );
  int pos = filterList.findIndex( currentFilter );
  mCurrentFilter = 0;
  if ( pos>=0 ) {
    mCurrentFilter = mFilters.at( pos );
  }
  updateFilter();
}

void CalendarView::writeFilterSettings( KConfig *config )
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
  config->writeEntry( "CalendarFilters", filterList );
  if ( mCurrentFilter ) {
    config->writeEntry( "Current Filter", mCurrentFilter->name() );
  } else {
    config->writeEntry( "Current Filter", QString::null );
  }
}


void CalendarView::goDate( const QDate &date )
{
  mDateNavigator->selectDate( date );
}

void CalendarView::showDate( const QDate &date )
{
  int dateCount = mDateNavigator->datesCount();
  if ( dateCount == 7 ) {
    mDateNavigator->selectWeek( date );
  } else {
    mDateNavigator->selectDates( date, dateCount );
  }
}

void CalendarView::goToday()
{
  mDateNavigator->selectToday();
}

void CalendarView::goNext()
{
  if ( dynamic_cast<KOMonthView*>( mViewManager->currentView() ) ) {
    mDateNavigator->selectNextMonth();
  } else {
    mDateNavigator->selectNext();
  }
}

void CalendarView::goPrevious()
{
  if ( dynamic_cast<KOMonthView*>( mViewManager->currentView() ) ) {
    mDateNavigator->selectPreviousMonth();
  } else {
    mDateNavigator->selectPrevious();
  }
}

void CalendarView::updateConfig( const QCString& receiver)
{
  if ( receiver != "korganizer" ) return;
  kdDebug(5850) << "CalendarView::updateConfig()" << endl;

  KOGlobals::self()->
    setHolidays( new KHolidays( KOPrefs::instance()->mHolidays ) );

  QString tz(  mCalendar->timeZoneId() );
  // Only set a new time zone if it changed. This prevents the window
  // from being modified on start
  if ( tz != KOPrefs::instance()->mTimeZoneId ) {

    const QString question( i18n("The timezone setting was changed. Do you want to keep the absolute time of "
                                "the items in your calendar, which will show them to be at a different time than "
                                "before, or move them to be at the old time also in the new timezone?") );
    int rc = KMessageBox::questionYesNo( this, question,
                              i18n("Keep Absolute Times?"),
                              KGuiItem(i18n("Keep Times")),
                              KGuiItem(i18n("Move Times")),
                              "calendarKeepAbsoluteTimes");
    if ( rc == KMessageBox::Yes ) {
      // user wants us to shift
      mCalendar->setTimeZoneIdViewOnly( KOPrefs::instance()->mTimeZoneId );
    } else {
      // only set the new timezone, wihtout shifting events, they will be
      // interpreted as being in the new timezone now
      mCalendar->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    }
  }
  emit configChanged();

  //switch beetween merged, side by side and tabbed agenda if needed
  mViewManager->updateMultiCalendarDisplay();

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
                                     Incidence *newIncidence,
                                     KOGlobals::WhatChanged modification )
{
  KOIncidenceEditor *tmp = editorDialog( newIncidence );
  if ( tmp ) {
    kdDebug(5850) << "Incidence modified and open" << endl;
    tmp->modified();
  }
  setModified( true );
  history()->recordEdit( oldIncidence, newIncidence );

  // Record completed todos in journals, if enabled. we should to this here in
  // favour of the todolist. users can mark a task as completed in an editor
  // as well.
  if ( newIncidence->type() == "Todo" &&
       KOPrefs::instance()->recordTodosInJournals() &&
       ( modification == KOGlobals::COMPLETION_MODIFIED ||
         modification == KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE ) ) {

      Todo *todo = static_cast<Todo *>(newIncidence);
      if ( todo->isCompleted() ||
           modification == KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE ) {
        QString timeStr = KGlobal::locale()->formatTime( QTime::currentTime() );
        QString description = i18n( "To-do completed: %1 (%2)" ).arg(
          newIncidence->summary() ).arg( timeStr );

        Journal::List journals = calendar()->journals( QDate::currentDate() );
        Journal *journal;

        if ( journals.isEmpty() ) {
          journal = new Journal();
          journal->setDtStart( QDateTime::currentDateTime() );

          QString dateStr = KGlobal::locale()->formatDate( QDate::currentDate() );
          journal->setSummary( i18n("Journal of %1").arg( dateStr ) );
          journal->setDescription( description );

          //TODO: recorded to-dos should save into the standard resource always
          if ( !mChanger->addIncidence( journal, 0, QString(), this ) ) {
            KODialogManager::errorSaveIncidence( this, journal );
            delete journal;
            return;
          }

        } else { // journal list is not empty
          journal = *(journals.at(0));
          Journal *oldJournal = journal->clone();
          journal->setDescription( journal->description().append( "\n" + description ) );

          if ( !mChanger->changeIncidence( oldJournal, journal,
                                           KOGlobals::DESCRIPTION_MODIFIED, this ) ) {
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
    KMessageBox::information( this, i18n("The item \"%1\" is filtered by "
                 "your current filter rules, so it will be hidden and not "
                 "appear in the view.").arg( incidence->summary() ),
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
  mDateNavigatorContainer->updateView();
  mDialogManager->updateSearchDialog();

  if ( incidence ) {
    // If there is an event view visible update the display
    mViewManager->currentView()->changeIncidenceDisplay( incidence, action );
    if ( mTodoList ) mTodoList->changeIncidenceDisplay( incidence, action );
    mEventViewer->changeIncidenceDisplay( incidence, activeDate( true ), action );
  } else {
    mViewManager->currentView()->updateView();
    if ( mTodoList ) mTodoList->updateView();
  }
}


void CalendarView::updateView(const QDate &start, const QDate &end)
{
  mTodoList->updateView();
  mViewManager->updateView(start, end);
  mDateNavigatorContainer->updateView();
}

void CalendarView::updateView()
{
  DateList tmpList = mDateNavigator->selectedDates();

  // We assume that the navigator only selects consecutive days.
  updateView( tmpList.first(), tmpList.last() );
}

void CalendarView::updateUnmanagedViews()
{
  mDateNavigatorContainer->updateDayMatrix();
}

int CalendarView::msgItemDelete( Incidence *incidence )
{
  return KMessageBox::warningContinueCancel(this,
      i18n("The item \"%1\" will be permanently deleted.").arg( incidence->summary() ),
      i18n("KOrganizer Confirmation"), KGuiItem(i18n("&Delete"),"editdelete"));
}


void CalendarView::edit_cut()
{
  Incidence *incidence = incToSendToClipboard( true );

  if ( !incidence || !mChanger ) {
    KNotifyClient::beep();
    return;
  }

  Incidence::List incidences;
  int km = KMessageBox::Yes;

  if ( !incidence->relations().isEmpty() &&
       incidence->type() == "Todo" ) { // Only todos (yet?)
    km = KMessageBox::questionYesNoCancel( this,
                                           i18n("The item \"%1\" has sub-to-dos. "
                                                "Do you want to cut just this item and "
                                                "make all its sub-to-dos independent, or "
                                                "cut the to-do with all its sub-to-dos?"
                                             ).arg( incidence->summary() ),
                                           i18n("KOrganizer Confirmation"),
                                           i18n("Cut Only This"),
                                           i18n("Cut All"));
  }

  if ( km == KMessageBox::Yes ) { // only one
    incidences.append( incidence );
    makeChildrenIndependent( incidence );
  } else if ( km == KMessageBox::No ) { // all
    // load incidence + children + grandchildren...
    getIncidenceHierarchy( incidence, incidences );
  }

  if ( km != KMessageBox::Cancel ) {
    mChanger->cutIncidences( incidences, this );
  }
}

void CalendarView::edit_copy()
{
  Incidence *incidence = incToSendToClipboard( false );

  if ( !incidence ) {
    KNotifyClient::beep();
    return;
  }

  Incidence::List incidences;
  int km = KMessageBox::Yes;

  if ( !incidence->relations().isEmpty() &&
       incidence->type() == "Todo" ) { // only todos.
    km = KMessageBox::questionYesNoCancel( this,
                                           i18n("The item \"%1\" has sub-to-dos. "
                                                "Do you want to copy just this item or "
                                                "copy the to-do with all its sub-to-dos?"
                                             ).arg( incidence->summary() ),
                                           i18n("KOrganizer Confirmation"),
                                           i18n("Copy Only This"),
                                           i18n("Copy All"));
  }

  if ( km == KMessageBox::Yes ) { // only one
    incidences.append( incidence );
  } else if ( km == KMessageBox::No ) { // all
    // load incidence + children + grandchildren...
    getIncidenceHierarchy( incidence, incidences );
  }

  if ( km != KMessageBox::Cancel ) {
    DndFactory factory( mCalendar );
    if ( !factory.copyIncidences( incidences ) ) {
      KNotifyClient::beep();
    }
  }
}

Incidence* CalendarView::incToSendToClipboard( bool cut )
{
  Incidence *originalInc = selectedIncidence();

  if ( originalInc && originalInc->doesRecur() &&
       originalInc->type() == "Event" ) { // temporary, until recurring to-dos are fixed

    Incidence *inc;
    KOGlobals::WhichOccurrences chosenOption;
    if ( cut ) {
      inc = singleOccurrenceOrAll( originalInc, KOGlobals::CUT, chosenOption, QDate(), true );
    } else {
      // The user is copying, the original incidence can't be changed
      // we can only dissociate a copy
      Incidence *originalIncSaved = originalInc->clone();
      inc = singleOccurrenceOrAll( originalIncSaved, KOGlobals::COPY, chosenOption, QDate(), false );

      // no dissociation, no need to leak our clone
      if ( chosenOption == KOGlobals::ALL ) {
        inc = originalInc;
        delete originalIncSaved;
      }

      // no need to leak our clone
      if ( chosenOption == KOGlobals::NONE ) {
        delete originalIncSaved;
      }
    }

    return inc;
  } else {
    return originalInc;
  }
}

void CalendarView::edit_paste()
{
// If in agenda and month view, use the selected time and date from there.
// In all other cases, use the navigator's selected date.

  QDate date;          // null dates are invalid, that's what we want
  bool timeSet = false;// flag denoting if the time has been set.
  QTime time;          // null dates are valid, so rely on the timeSet flag
  QDateTime endDT;     // null datetimes are invalid, that's what we want
  bool useEndTime = false;

  KOrg::BaseView *curView = mViewManager->currentView();
  if ( !curView ) {
    return;
  }

  KOAgendaView *aView = mViewManager->agendaView();
  KOMonthView *mView = mViewManager->monthView();
  if ( curView == mViewManager->multiAgendaView() ) {
    aView = mViewManager->multiAgendaView()->selectedAgendaView();
    curView = aView;
  }

  if ( curView == aView && aView->selectionStart().isValid() ) {
    date = aView->selectionStart().date();
    endDT = aView->selectionEnd();
    useEndTime = !aView->selectedIsSingleCell();
    if ( !aView->selectedIsAllDay() ) {
      time = aView->selectionStart().time();
      timeSet = true;
    }
  } else if ( curView == mView && mView->selectionStart().isValid() ) {
    date = mView->selectionStart().date();
  } else if ( !mDateNavigator->selectedDates().isEmpty() &&
              curView->supportsDateNavigation() ) {
    // default to the selected date from the navigator
    date = mDateNavigator->selectedDates().first();
  }

  if ( !date.isValid() && curView->supportsDateNavigation() ) {
    KMessageBox::sorry(
      this,
      i18n( "Paste failed: unable to determine a valid target date." ) );
    return;
  }

  DndFactory factory( mCalendar );
  Incidence::List pastedIncidences;
  if ( timeSet && time.isValid() ) {
    pastedIncidences = factory.pasteIncidences( date, &time );
  } else {
    pastedIncidences = factory.pasteIncidences( date );
  }

  Incidence::List::Iterator it;
  for ( it = pastedIncidences.begin(); it != pastedIncidences.end(); ++it ) {
    QPair<ResourceCalendar *, QString>p = viewSubResourceCalendar();

    // FIXME: use a visitor here
    if ( ( *it )->type() == "Event" ) {
      Event *pastedEvent = static_cast<Event*>( *it );
      // only use selected area if event is of the same type
      // (all-day or non-all-day) as the current selection is
      if ( aView && endDT.isValid() && useEndTime ) {
        if ( ( pastedEvent->doesFloat() && aView->selectedIsAllDay() ) ||
             ( !pastedEvent->doesFloat() && !aView->selectedIsAllDay() ) ) {
          pastedEvent->setDtEnd( endDT );
        }
      }

      // KCal supports events with relations, but korganizer doesn't
      // so unset it. It can even come from other application.
      pastedEvent->setRelatedTo( 0 );
      pastedEvent->setRelatedToUid( QString() );

      mChanger->addIncidence( pastedEvent, p.first, p.second, this );

    } else if ( ( *it )->type() == "Todo" ) {
      Todo *pastedTodo = static_cast<Todo*>( *it );
      Todo *_selectedTodo = selectedTodo();

      // if we are cutting a hierarchy only the root
      // should be son of _selectedTodo
      if ( _selectedTodo && !pastedTodo->relatedTo() ) {
        pastedTodo->setRelatedTo( _selectedTodo );
      }
      mChanger->addIncidence( pastedTodo, p.first, p.second, this );
    }
  }
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
}

void CalendarView::dateTimesForNewEvent( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  mViewManager->currentView()->eventDurationHint( startDt, endDt, allDay );

  if ( !startDt.isValid() || !endDt.isValid() ) {
    startDt.setDate( activeDate( true ) );
    startDt.setTime( KOPrefs::instance()->mStartTime.time() );

    int addSecs = ( KOPrefs::instance()->mDefaultDuration.time().hour() * 3600 ) +
                  ( KOPrefs::instance()->mDefaultDuration.time().minute() * 60 );

    endDt = startDt.addSecs( addSecs );
  }
}

KOEventEditor *CalendarView::newEventEditor( ResourceCalendar *res, const QString &subRes,
                                             const QDateTime &startDtParam,
                                             const QDateTime &endDtParam, bool allDayParam )
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
  eventEditor->setResource( res, subRes );
  eventEditor->setDates( startDt, endDt, allDay );
  mDialogManager->connectTypeAhead( eventEditor, dynamic_cast<KOrg::AgendaView*>(viewManager()->currentView()) );
  return eventEditor;
}

void CalendarView::newEvent()
{
  KOrg::BaseView *currentView = mViewManager->currentView();
  if ( currentView == mViewManager->multiAgendaView() ) {
    currentView = mViewManager->multiAgendaView()->selectedAgendaView();
  }

  newEvent( currentView->resourceCalendar(),
            currentView->subResourceCalendar() );
}

void CalendarView::newEvent( ResourceCalendar *res, const QString &subRes )
{
  kdDebug(5850) << "CalendarView::newEvent()" << endl;
  newEvent( res, subRes, QDateTime(), QDateTime() );
}

void CalendarView::newEvent( ResourceCalendar *res, const QString &subRes,
                             const QDate &dt )
{
  QDateTime startDt( dt, KOPrefs::instance()->mStartTime.time() );
  newEvent( res, subRes, QDateTime( dt ), QDateTime() );
}

void CalendarView::newEvent( ResourceCalendar *res, const QString &subRes,
                             const QDateTime &startDt )
{
  newEvent( res, subRes, startDt, QDateTime() );
}

void CalendarView::newEvent( ResourceCalendar *res, const QString &subRes,
                             const QDateTime &startDt, const QDateTime &endDt,
                             bool allDay )
{
  KOEventEditor *eventEditor = newEventEditor( res, subRes,
                                               startDt, endDt, allDay );
  eventEditor->show();
}

void CalendarView::newEvent( ResourceCalendar *res, const QString &subRes,
                             const QString &summary, const QString &description,
                             const QStringList &attachments, const QStringList &attendees,
                             const QStringList &attachmentMimetypes, bool inlineAttachment )
{
  KOEventEditor *eventEditor = newEventEditor( res, subRes );
  eventEditor->setTexts( summary, description );
  // if attach or attendee list is empty, these methods don't do anything, so
  // it's safe to call them in every case
  eventEditor->addAttachments( attachments, attachmentMimetypes, inlineAttachment );
  eventEditor->addAttendees( attendees );
  eventEditor->show();
}

void CalendarView::newTodo( ResourceCalendar *res, const QString &subRes,
                            const QString &summary, const QString &description,
                            const QStringList &attachments, const QStringList &attendees,
                            const QStringList &attachmentMimetypes,
                            bool inlineAttachment, bool isTask )
{
  kdDebug(5850) << k_funcinfo << endl;
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  todoEditor->setResource( res, subRes );
  todoEditor->setTexts( summary, description );
  todoEditor->addAttachments( attachments, attachmentMimetypes, inlineAttachment );
  todoEditor->addAttendees( attendees );
  todoEditor->selectCreateTask( isTask );
  todoEditor->show();
}

void CalendarView::newTodo()
{
  KOrg::BaseView *currentView = mViewManager->currentView();
  if ( currentView == mViewManager->multiAgendaView() ) {
    currentView = mViewManager->multiAgendaView()->selectedAgendaView();
  }

  newTodo( currentView->resourceCalendar(),
           currentView->subResourceCalendar() );
}

void CalendarView::newTodo( ResourceCalendar *res, const QString &subRes )
{
  kdDebug(5850) << k_funcinfo << endl;
  QDateTime dtDue;
  bool allday = true;
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  todoEditor->setResource( res, subRes );
  if ( mViewManager->currentView()->isEventView() ) {
    dtDue.setDate( mDateNavigator->selectedDates().first() );
    QDateTime dtDummy = QDateTime::currentDateTime();
    mViewManager->currentView()->eventDurationHint( dtDue, dtDummy, allday );
    todoEditor->setDates( dtDue, allday );
  }
  todoEditor->show();
}

void CalendarView::newTodo( ResourceCalendar *res, const QString &subRes,
                            const QDate &date )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  todoEditor->setResource( res, subRes );
  todoEditor->setDates( QDateTime( date, QTime::currentTime() ), true );
  todoEditor->show();
}

void CalendarView::newJournal()
{
  KOrg::BaseView *currentView = mViewManager->currentView();
  if ( currentView == mViewManager->multiAgendaView() ) {
    currentView = mViewManager->multiAgendaView()->selectedAgendaView();
  }

  newJournal( currentView->resourceCalendar(),
              currentView->subResourceCalendar() );
}

void CalendarView::newJournal( ResourceCalendar *res, const QString &subRes )
{
  kdDebug(5850) << "CalendarView::newJournal()" << endl;
  newJournal( res, subRes, QString::null, QDate() );
}

void CalendarView::newJournal( ResourceCalendar *res, const QString &subRes,
                               const QDate &date)
{
  newJournal( res, subRes, QString::null, date );
}

void CalendarView::newJournal( ResourceCalendar *res, const QString &subRes,
                               const QString &text, const QDate &date )
{
  KOJournalEditor *journalEditor = mDialogManager->getJournalEditor();
  connectIncidenceEditor( journalEditor );
  journalEditor->newJournal();
  journalEditor->setResource( res, subRes );
  journalEditor->setTexts( text );
  if ( !date.isValid() ) {
    journalEditor->setDate( mDateNavigator->selectedDates().first() );
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

bool CalendarView::addIncidence( const QString &ical )
{
  kdDebug(5850) << "CalendarView::addIncidence:\n" << ical << endl;
  ICalFormat format;
  format.setTimeZone( mCalendar->timeZoneId(), true );
  Incidence *incidence = format.fromString( ical );
  if ( !incidence ) return false;
  if ( !mChanger->addIncidence( incidence, 0, QString(), this ) ) {
    delete incidence;
    return false;
  }
  return true;
}

void CalendarView::appointment_show()
{
  Incidence *incidence = selectedIncidence();
  if ( incidence ) {
    showIncidence( incidence, activeIncidenceDate() );
  } else {
    KNotifyClient::beep();
  }
}

void CalendarView::appointment_edit()
{
  Incidence *incidence = selectedIncidence();
  if ( incidence ) {
    editIncidence( incidence, activeIncidenceDate() );
  } else {
    KNotifyClient::beep();
  }
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
  if( incidence_unsub ( anTodo ) ) {
    updateView();
  }
}

bool CalendarView::incidence_unsub( Incidence *inc )
{
  bool status = false;
  if ( !inc || !inc->relatedTo() ) {
    return false;
  }

  if ( mChanger->beginChange( inc, 0, QString() ) ) {
      Incidence *oldInc = inc->clone();
      inc->setRelatedTo( 0 );
      mChanger->changeIncidence( oldInc, inc, KOGlobals::RELATION_MODIFIED, this );
      mChanger->endChange( inc, 0, QString() );
      delete oldInc;
      setModified(true);
      status = true;
  }
  if ( ! status ) {
    KMessageBox::sorry( this, i18n("Unable to turn sub-to-do into a top-level "
        "to-do, because it cannot be locked.") );
  }

  return status;
}

bool CalendarView::makeSubTodosIndependent ( )
{
  bool  status = false;
  Todo *aTodo = selectedTodo();

  if ( makeChildrenIndependent( aTodo ) ) {
    updateView();
    status = true;
  }
  return status;
}

bool CalendarView::makeChildrenIndependent ( Incidence *inc )
{
  if ( !inc || inc->relations().isEmpty() ) {
    return false;
  }

  startMultiModify ( i18n( "Make sub-to-dos independent" ) );
  Incidence::List subIncs( inc->relations() );
  Incidence::List::Iterator it;

  for ( it= subIncs.begin(); it != subIncs.end(); ++it ) {
    incidence_unsub ( *it );
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
    kdDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }
  Incidence*oldincidence = incidence->clone();
  if ( !mChanger->beginChange( incidence, 0, QString() ) ) {
    kdDebug(5850) << "Unable to lock incidence " << endl;
    delete oldincidence;
    return;
  }

  Alarm::List alarms = incidence->alarms();
  Alarm::List::ConstIterator it;
  for ( it = alarms.begin(); it != alarms.end(); ++it ) {
    (*it)->toggleAlarm();
  }
  if ( alarms.isEmpty() ) {
    // Add an alarm if it didn't have one
    Alarm *alm = incidence->newAlarm();
    alm->setType( Alarm::Display );
    alm->setEnabled( true );
    int duration; // in secs
    switch( KOPrefs::instance()->mReminderTimeUnits ) {
    default:
    case 0: // mins
      duration = KOPrefs::instance()->mReminderTime * 60;
      break;
    case 1: // hours
      duration = KOPrefs::instance()->mReminderTime * 60 * 60;
      break;
    case 2: // days
      duration = KOPrefs::instance()->mReminderTime * 60 * 60 * 24;
      break;
    }
    if ( incidence->type() == "Event" ) {
      alm->setStartOffset( KCal::Duration( -duration ) );
    } else {
      alm->setEndOffset( KCal::Duration( -duration ) );
    }
  }
  mChanger->changeIncidence( oldincidence, incidence, KOGlobals::ALARM_MODIFIED, this );
  mChanger->endChange( incidence, 0, QString() );
  delete oldincidence;

//  mClickedItem->updateIcons();
}

void CalendarView::dissociateOccurrence( Incidence *incidence, const QDate &date )
{
  if ( !incidence || !mChanger ) {
    kdDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }

  QPair<ResourceCalendar *, QString>p =
    CalHelper::incSubResourceCalendar( calendar(), incidence );

  if ( !mChanger->beginChange( incidence, p.first, p.second ) ) {
    kdDebug(5850) << "Unable to lock incidence " << endl;
    return;
  }
  startMultiModify( i18n("Dissociate occurrence") );
  Incidence*oldincidence = incidence->clone();

  Incidence* newInc = mCalendar->dissociateOccurrence( incidence, date, true );

  if ( newInc ) {
    mChanger->changeIncidence( oldincidence, incidence, KOGlobals::NOTHING_MODIFIED, this );
    mChanger->addIncidence( newInc, p.first, p.second, this );
  } else {
    KMessageBox::sorry( this, i18n("Dissociating the occurrence failed."),
      i18n("Dissociating Failed") );
  }
  mChanger->endChange( incidence, p.first, p.second );
  endMultiModify();
  delete oldincidence;
}

void CalendarView::dissociateFutureOccurrence( Incidence *incidence, const QDate &date )
{
  if ( !incidence || !mChanger ) {
    kdDebug(5850) << "CalendarView::toggleAlarm() called without having a clicked item" << endl;
    return;
  }

  QPair<ResourceCalendar *, QString>p =
    CalHelper::incSubResourceCalendar( calendar(), incidence );

  if ( !mChanger->beginChange( incidence, p.first, p.second ) ) {
    kdDebug(5850) << "Unable to lock incidence " << endl;
    return;
  }
  startMultiModify( i18n("Dissociate future occurrences") );
  Incidence*oldincidence = incidence->clone();

  Incidence* newInc = mCalendar->dissociateOccurrence( incidence, date, true );
  if ( newInc ) {
    mChanger->changeIncidence( oldincidence, incidence, KOGlobals::NOTHING_MODIFIED, this );
    mChanger->addIncidence( newInc, p.first, p.second, this );
  } else {
    KMessageBox::sorry( this, i18n("Dissociating the future occurrences failed."),
      i18n("Dissociating Failed") );
  }
  endMultiModify();
  mChanger->endChange( incidence, p.first, p.second );
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
      KMessageBox::error( this, i18n("Unable to publish the item '%1'").arg( incidence->summary() ) );
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

void CalendarView::schedule_forward( Incidence *incidence )
{
  if ( !incidence ) {
    incidence = selectedIncidence();
  }

  if ( !incidence ) {
    KMessageBox::information(
      this,
      i18n( "No item selected." ),
      i18n( "Forwarding" ),
      "ForwardNoEventSelected" );
    return;
  }

  PublishDialog publishdlg;
  if ( publishdlg.exec() == QDialog::Accepted ) {
    QString recipients = publishdlg.addresses();
    if ( incidence->organizer().isEmpty() ) {
      incidence->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                       KOPrefs::instance()->email() ) );
    }

    ICalFormat format;
    QString messageText = format.createScheduleMessage( incidence, Scheduler::Request );
    KOMailClient mailer;
    if ( mailer.mailTo( incidence, recipients, messageText ) ) {
      KMessageBox::information(
        this,
        i18n( "The item information was successfully sent." ),
        i18n( "Forwarding" ),
        "IncidenceForwardSuccess" );
    } else {
      KMessageBox::error(
        this,
        i18n( "Unable to forward the item '%1'" ).arg( incidence->summary() ),
        i18n( "Forwarding Error" ) );
    }
  }
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
  if ( scheduler.performTransaction( incidence, method ) ) {
    KMessageBox::information( this, i18n("The groupware message for item '%1'"
                                "was successfully sent.\nMethod: %2")
                                .arg( incidence->summary() )
                                .arg( Scheduler::methodName( method ) ),
                              i18n("Sending Free/Busy"),
                              "FreeBusyPublishSuccess" );
  } else {
    KMessageBox::error( this, i18n("Groupware message sending failed. "
                        "%2 is request/reply/add/cancel/counter/etc.",
                        "Unable to send the item '%1'.\nMethod: %2")
                        .arg( incidence->summary() )
                        .arg( Scheduler::methodName( method ) ) );
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
  KOCoreHelper helper;
  CalPrinter printer( this, mCalendar, &helper );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  KOrg::BaseView *currentView = mViewManager->currentView();

  CalPrinterBase::PrintType printType = CalPrinterBase::Month;
  if ( currentView ) printType = currentView->printType();

  DateList tmpDateList = mDateNavigator->selectedDates();
  Incidence::List selectedIncidences;
  if ( mViewManager->currentView() ) {
    selectedIncidences = mViewManager->currentView()->selectedIncidences();
  }
  printer.print( printType, tmpDateList.first(), tmpDateList.last(), selectedIncidences );
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
  if ( !filename.isEmpty() )
  {
    // Force correct extension
    if (filename.right(4) != ".ics") filename += ".ics";
    if ( QFile( filename ).exists() ) {
      if ( KMessageBox::No == KMessageBox::warningYesNo(
             this,
             i18n( "Do you want to overwrite %1?").arg(filename) ) ) {
	      return;
      }
    }
    FileStorage storage( mCalendar, filename, new ICalFormat );
    storage.save();
  }
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
  if ( !filename.isEmpty() )
  {
    // TODO: I don't like forcing extensions:
    // Force correct extension
    if (filename.right(4) != ".vcs") filename += ".vcs";
    if ( QFile( filename ).exists() ) {
      if ( KMessageBox::No == KMessageBox::warningYesNo(
             this,
             i18n( "Do you want to overwrite %1?").arg(filename ) ) ) {
             return;
      }
    }
    FileStorage storage( mCalendar, filename, new VCalFormat );
    storage.save();
  }
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

void CalendarView::processMainViewSelection( Incidence *incidence, const QDate &date )
{
  if ( incidence ) mTodoList->clearSelection();
  processIncidenceSelection( incidence, date );
}

void CalendarView::processTodoListSelection( Incidence *incidence, const QDate &date )
{
  if ( incidence && mViewManager->currentView() ) {
    mViewManager->currentView()->clearSelection();
  }
  processIncidenceSelection( incidence, date );
}

void CalendarView::processIncidenceSelection( Incidence *incidence, const QDate &date )
{
  if ( incidence != mSelectedIncidence ) {
    // This signal also must be emitted if incidence is 0
    emit incidenceSelected( incidence, date );
  }

  if ( !incidence ) {
   mSelectedIncidence = incidence;
   return;
  }
  if ( incidence == mSelectedIncidence ) {
    if ( !incidence->doesRecur() || mSaveDate == date ) {
      return;
    }
  }

  mSelectedIncidence = incidence;
  mSaveDate = date;

  emit incidenceSelected( mSelectedIncidence, date );
  bool organizerEvents = false;
  bool groupEvents = false;
  bool todo = false;
  bool subtodo = false;

  organizerEvents = KOPrefs::instance()->thatIsMe( incidence->organizer().email() );
  groupEvents = incidence->attendeeByMails( KOPrefs::instance()->allEmails() );

  if ( incidence->type() == "Todo" ) {
    todo = true;
    subtodo = ( incidence->relatedTo() != 0 );
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

void CalendarView::showDates( const DateList &selectedDates, const QDate &preferredMonth )
{
  mDateNavigatorContainer->selectDates( selectedDates, preferredMonth );
  mNavigatorBar->selectDates( selectedDates );

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
  while( filter ) {
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
  kdDebug(5850) << "To be implemented." << endl;
}

void CalendarView::showDateNavigator( bool show )
{
  if( show )
    mDateNavigatorContainer->show();
  else
    mDateNavigatorContainer->hide();
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
  mChanger->endChange( in, 0, QString() );
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
  showIncidence( selectedIncidence(), activeIncidenceDate() );
}

void CalendarView::editIncidence()
{
  editIncidence( selectedIncidence(), activeIncidenceDate() );
}

bool CalendarView::editIncidence( const QString &uid )
{
  return editIncidence( mCalendar->incidence( uid ), QDate() );
}

bool CalendarView::editIncidence( const QString &uid, const QDate &date )
{
  return editIncidence( mCalendar->incidence( uid ), date );
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

void CalendarView::pasteIncidence()
{
  edit_paste();
}

void CalendarView::showIncidence( Incidence *incidence, const QDate &date )
{
  if ( !incidence ) {
    return;
  }

  KOEventViewerDialog *eventViewer = new KOEventViewerDialog( calendar(), this );
  eventViewer->setIncidence( incidence, date );
  eventViewer->show();
}

bool CalendarView::editIncidence( Incidence *incidence, const QDate &date, bool isCounter )
{
  kdDebug(5850) << "CalendarView::editEvent()" << endl;

  CalendarResources *stdcal = dynamic_cast<CalendarResources *>( mCalendar );
  if( stdcal && !stdcal->hasCalendarResources() ) {
    KMessageBox::sorry(
      this,
      i18n( "No resources found. We can not edit the item." ) );
    return false;
  }

  // FIXME: This is a nasty hack, since we need to set a parent for the
  //        resource selection dialog. However, we don't have any UI methods
  //        in the calendar, only in the CalendarResources::DestinationPolicy
  //        So we need to type-cast it and extract it from the CalendarResources
  QWidget *tmpparent = 0;
  if ( stdcal ) {
    tmpparent = stdcal->dialogParentWidget();
    stdcal->setDialogParentWidget( this );
  }

  if ( !incidence ) {
    kdDebug(5850) << "Empty Incidence" << endl;
    KNotifyClient::beep();
    return false;
  }

  if ( !mChanger ) {
    kdDebug(5850) << "Empty Changer" << endl;
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
    showIncidence( incidence, date );
    return true;
  }

  QPair<ResourceCalendar *, QString>p =
    CalHelper::incSubResourceCalendar( calendar(), incidence );

  Incidence *savedIncidence = incidence->clone();
  Incidence *incToChange;

  if ( incidence->doesRecur() ) {
    KOGlobals::WhichOccurrences chosenOption;
    incToChange = singleOccurrenceOrAll( incidence, KOGlobals::EDIT, chosenOption, date );
  } else {
    incToChange = incidence;
  }

  // If the user pressed cancel incToChange is 0
  if ( incToChange ) {
    if ( !isCounter && !mChanger->beginChange( incToChange, p.first, p.second ) ) {
      warningChangeFailed( incToChange );
      showIncidence( incToChange, date );

      return false;
    }

    kdDebug(5850) << "CalendarView::editIncidence() new IncidenceEditor" << endl;
    KOIncidenceEditor *incidenceEditor = mDialogManager->getEditor( incToChange );
    connectIncidenceEditor( incidenceEditor );

    mDialogList.insert( incToChange, incidenceEditor );
    if ( incidence != incToChange ) {
      incidenceEditor->setRecurringIncidence( savedIncidence, incidence );
    }
    incidenceEditor->setResource( p.first, p.second );
    incidenceEditor->editIncidence( incToChange, date, mCalendar );
    incidenceEditor->show();
    return true;
  } else {
    return false;
  }
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
  mChanger->deleteIncidence ( todo, this );
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
      mChanger->deleteIncidence( todo, this );
    return;
  }

  /* Ok, this to-do has sub-to-dos, ask what to do */
  int km = KMessageBox::No;
  if ( !force ) {
    km = KMessageBox::questionYesNoCancel(
      this,
      i18n("The item \"%1\" has sub-to-dos. "
           "Do you want to delete just this item and "
           "make all its sub-to-dos independent, or "
           "delete the to-do with all its sub-to-dos?"
        ).arg( todo->summary() ),
      i18n("KOrganizer Confirmation"),
      i18n("Delete Only This"),
      i18n("Delete All"));
  }
  startMultiModify( i18n("Deleting sub-to-dos" ) );
  // Delete only the father
  if( km == KMessageBox::Yes ) {
    // Instead of making a subto-do independent, why not relate
    // it to it's dead father's parent?
    makeChildrenIndependent ( todo );
    mChanger->deleteIncidence( todo, this );
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
      KNotifyClient::beep();
    }
    return;
  }
  if ( incidence->isReadOnly() ) {
    if ( !force ) {
      KMessageBox::information( this, i18n("The item \"%1\" is marked read-only "
                                "and cannot be deleted; it probably belongs to "
                                "a read-only calendar resource.")
                                .arg(incidence->summary()),
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
    kdDebug(5850) << "Recurrence-Date: " << itemDate.toString() << endl;
    int km = KMessageBox::Ok;
    if ( !force ) {
      if ( !itemDate.isValid() ) {
        kdDebug(5850) << "Date Not Valid" << endl;
        km = KMessageBox::warningContinueCancel(this,
          i18n("The calendar item \"%1\" recurs over multiple dates; "
               "are you sure you want to delete it "
               "and all its recurrences?").arg( incidence->summary() ),
               i18n("KOrganizer Confirmation"), i18n("Delete All") );
      } else {
        km = KOMessageBox::fourBtnMsgBox( this, QMessageBox::Warning,
          i18n("The calendar item \"%1\" recurs over multiple dates. "
               "Do you want to delete only the current one on %2, only all "
               "future recurrences, or all its recurrences?" )
               .arg( incidence->summary() )
               .arg( KGlobal::locale()->formatDate(itemDate)),
               i18n("KOrganizer Confirmation"), i18n("Delete C&urrent"),
               i18n("Delete &Future"),
               i18n("Delete &All"));
      }
    }

    QPair<ResourceCalendar *, QString>p =
      CalHelper::incSubResourceCalendar( calendar(), incidence );

    switch(km) {
      case KMessageBox::Ok: // Continue // all
      case KMessageBox::Continue:
        mChanger->deleteIncidence( incidence, this );
        break;

      case KMessageBox::Yes: // just this one
        if ( mChanger->beginChange( incidence, p.first, p.second ) ) {
          Incidence *oldIncidence = incidence->clone();
          incidence->recurrence()->addExDate( itemDate );
          mChanger->changeIncidence( oldIncidence, incidence,
                                     KOGlobals::RECURRENCE_MODIFIED_ONE_ONLY, this );
          mChanger->endChange( incidence, p.first, p.second );
          delete oldIncidence;
        }
        break;
      case KMessageBox::No: // all future items
        if ( mChanger->beginChange( incidence, p.first, p.second ) ) {
          Incidence *oldIncidence = incidence->clone();
          Recurrence *recur = incidence->recurrence();
          recur->setEndDate( itemDate.addDays(-1) );
          mChanger->changeIncidence( oldIncidence, incidence,
                                     KOGlobals::RECURRENCE_MODIFIED_ALL_FUTURE,  this );
          mChanger->endChange( incidence, p.first, p.second );
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
      mChanger->deleteIncidence( incidence, this );
      processIncidenceSelection( 0, QDate() );
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
      if ( !mChanger->deleteIncidence( todo, this ) )
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
      KMessageBox::information( this, i18n("Unable to purge to-dos with "
                                "uncompleted children."), i18n("Delete To-do"),
                                "UncompletedChildrenPurgeTodos" );
    }
  }
}

void CalendarView::warningChangeFailed( Incidence *incidence )
{
  if ( incidence ) {
    KMessageBox::sorry(
      this,
      i18n( "Unable to edit \"%1\" because it is locked by another process." ).
      arg( incidence->summary() ) );
  }
}

void CalendarView::editCanceled( Incidence *incidence )
{
  mCalendar->endChange( incidence );
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
    KMessageBox::sorry(this, i18n("Unable to copy the item to %1.")
                       .arg( dt.toString() ), i18n("Copying Failed") );
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

  QPair<ResourceCalendar *, QString>p = viewSubResourceCalendar();

  if ( !mChanger->addIncidence( incidence, p.first, p.second, this ) ) {
    KODialogManager::errorSaveIncidence( this, incidence );
    delete incidence;
  }
}

void CalendarView::moveIncidenceTo( Incidence *incmove, const QDate &dt )
{
  if ( !incmove || !mChanger ) {
    KMessageBox::sorry( this, i18n("Unable to move the item to %1.")
                        .arg( dt.toString() ), i18n("Moving Failed") );
    return;
  }
  Incidence *incidence = mCalendar->incidence( incmove->uid() );
  if ( !incidence ) {
    addIncidenceOn( incidence, dt );
    return;
  }

  Incidence *oldIncidence = incidence->clone();
  QPair<ResourceCalendar *, QString>p = viewSubResourceCalendar();

  if ( !mChanger->beginChange( incidence, p.first, p.second ) ) {
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
  mChanger->changeIncidence( oldIncidence, incidence, KOGlobals::DATE_MODIFIED,this );
  mChanger->endChange( incidence, p.first, p.second );
  delete oldIncidence;
}

void CalendarView::resourcesChanged()
{
  mViewManager->resourcesChanged();
  mDateNavigatorContainer->setUpdateNeeded();
  updateView();
}

Incidence* CalendarView::singleOccurrenceOrAll( Incidence *inc,
                                                KOGlobals::OccurrenceAction userAction,
                                                KOGlobals::WhichOccurrences &chosenOption,
                                                const QDate &itemDate,
                                                const bool commitToCalendar )
{

  // temporary, until recurring to-dos are fixed
  if ( inc->type() != "Event" ) {
    chosenOption = KOGlobals::ALL;
    return inc;
  }

  Incidence *incToReturn = 0;
  Incidence *incSaved = 0;
  KOGlobals::WhatChanged whatChanged;

  bool dissociationOccurred = false;
  const QDate &dt = itemDate.isValid() ? itemDate : activeIncidenceDate();

  QString dialogTitle;
  QString dialogText;

  if ( userAction == KOGlobals::CUT ) {
    dialogTitle = i18n( "Cutting Recurring Item" );

    dialogText = i18n("The item you try to cut is a recurring item. Do you want to cut "
                       "only this single occurrence, only future items, "
                       "or all items in the recurrence?");

  } else if ( userAction == KOGlobals::COPY ) {
    dialogTitle = i18n( "Copying Recurring Item" );

    dialogText = i18n("The item you try to copy is a recurring item. Do you want to copy "
                       "only this single occurrence, only future items, "
                       "or all items in the recurrence?");
  } else {
    dialogTitle = i18n( "Changing Recurring Item" );

    dialogText = i18n( "The item you try to change is a recurring item. Shall the changes "
                       "be applied only to this single occurrence, only to the future items, "
                       "or to all items in the recurrence?" );
  }

  int res = KOMessageBox::fourBtnMsgBox( this, QMessageBox::Question,
            dialogText,
            dialogTitle,
            i18n("Only &This Item"), i18n("Only &Future Items"), i18n("&All Occurrences") );
  switch ( res ) {
    case KMessageBox::Ok: // All occurrences
      incToReturn = inc;
      chosenOption = KOGlobals::ALL;
      break;
    case KMessageBox::Yes: { // Just this occurrence
      // Dissociate this occurrence:
      // create clone of event, set relation to old event, set cloned event
      // for mActionItem, add exception date to old event, changeIncidence
      // for the old event, remove the recurrence from the new copy and then just
      // go on with the newly adjusted mActionItem and let the usual code take
      // care of the new time!

      chosenOption = KOGlobals::ONLY_THIS_ONE;
      whatChanged  = KOGlobals::RECURRENCE_MODIFIED_ONE_ONLY;
      startMultiModify( i18n("Dissociate event from recurrence") );
      incSaved = inc->clone();
      incToReturn = mCalendar->dissociateOccurrence( inc, dt );
      if ( incToReturn ) {
        dissociationOccurred = true;
      } else {
        KMessageBox::sorry( this, i18n("Unable to add the exception item to the "
            "calendar. No change will be done."), i18n("Error Occurred") );
        incToReturn = 0;
      }

      break; }
    case KMessageBox::No/*Future*/: { // All future occurrences
      // Dissociate this occurrence:
      // create clone of event, set relation to old event, set cloned event
      // for mActionItem, add recurrence end date to old event, changeIncidence
      // for the old event, adjust the recurrence for the new copy and then just
      // go on with the newly adjusted mActionItem and let the usual code take
      // care of the new time!
      chosenOption = KOGlobals::ONLY_FUTURE;
      whatChanged  = KOGlobals::RECURRENCE_MODIFIED_ALL_FUTURE;
      startMultiModify( i18n("Split future recurrences") );
      incSaved = inc->clone();
      incToReturn = mCalendar->dissociateOccurrence( inc, dt, false );
      if ( incToReturn ) {
        dissociationOccurred = true;
      } else {
        KMessageBox::sorry( this, i18n("Unable to add the future items to the "
            "calendar. No change will be done."), i18n("Error Occurred") );

        incToReturn = 0;
      }

      break; }
    default:
      chosenOption = KOGlobals::NONE;
  }

  if ( dissociationOccurred && commitToCalendar ) {
    QPair<ResourceCalendar *, QString>p = viewSubResourceCalendar();
    mChanger->addIncidence( incToReturn, p.first, p.second, this );
    mChanger->changeIncidence( incSaved, inc, whatChanged, this );
  }

  return incToReturn;
}

void CalendarView::selectWeek( const QDate &date )
{
  if ( KOPrefs::instance()->mWeekNumbersShowWork    &&
       mViewManager->agendaIsSelected()             &&
       mViewManager->agendaMode()  == KOViewManager::AGENDA_WORK_WEEK ) {
    mDateNavigator->selectWorkWeek( date );
  } else {
    mDateNavigator->selectWeek( date );
  }
}

void CalendarView::getIncidenceHierarchy( Incidence *inc,
                                          Incidence::List &children )
{
  // protecion against looping hierarchies
  if ( inc && !children.contains( inc ) ) {
    Incidence::List::ConstIterator it;
    Incidence::List immediateChildren = inc->relations();
    for ( it = immediateChildren.constBegin(); it != immediateChildren.constEnd(); ++it ) {
      getIncidenceHierarchy( *it, children );
    }
    children.append( inc );
  }
}

#include "calendarview.moc"
