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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "calendarview.h"
#include "calprinter.h"
#include "datechecker.h"
#include "datenavigator.h"
#include "datenavigatorcontainer.h"
#include "exportwebdialog.h"
#include "kocorehelper.h"
#include "kodaymatrix.h"
#include "kodialogmanager.h"
#include "koeventviewerdialog.h"
#include "koglobals.h"
#include "kohelper.h"
#include "koprefs.h"
#include "koviewmanager.h"
#include "history.h"
#include "htmlexportsettings.h"
#include "navigatorbar.h"
#include "views/agendaview/koagendaview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/todoview/kotodoview.h"

#include <calendarsupport/calendaradaptor.h>
#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/collectiongeneralpage.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/dndfactory.h>
#include <calendarsupport/freebusymanager.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/next/incidenceviewer.h>

#include <incidenceeditor-ng/incidencedefaults.h>
#include <incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditor-ng/incidencedialogfactory.h>

#include <libkdepim/pimmessagebox.h>

#include <Akonadi/CollectionPropertiesDialog>
#include <Akonadi/Control>

#include <KCalCore/CalFilter>
#include <KCalCore/FileStorage>
#include <KCalCore/ICalFormat>
#include <KCalCore/VCalFormat>

#include <KCalUtils/ICalDrag>
#include <KCalUtils/Stringify>

#include <KHolidays/Holidays>

#include <KFileDialog>
#include <KNotification>
#include <KRun>
#include <KVBox>

#include <QApplication>
#include <QClipboard>
#include <QItemSelectionModel>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

CalendarView::CalendarView( QWidget *parent )
  : CalendarViewBase( parent ),
    mHistory( 0 ),
    mCalendar( 0 ),
    mChanger( 0 ),
    mSplitterSizesValid( false )
{
  Akonadi::Control::widgetNeedsAkonadi( this );

  mViewManager = new KOViewManager( this );
  mDialogManager = new KODialogManager( this );

  mReadOnly = false;

  mCalPrinter = 0;

  mDateNavigator = new DateNavigator( this );
  mDateChecker = new DateChecker( this );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( 0 );

  // create the main layout frames.
  mPanner = new QSplitter( Qt::Horizontal, this );
  mPanner->setObjectName( "CalendarView::Panner" );
  topLayout->addWidget( mPanner );

  mLeftSplitter = new QSplitter( Qt::Vertical, mPanner );
  mLeftSplitter->setObjectName( "CalendarView::LeftFrame" );

  mDateNavigatorContainer = new DateNavigatorContainer( mLeftSplitter );
  mDateNavigatorContainer->setObjectName( "CalendarView::DateNavigator" );

  mTodoList = new KOTodoView( true/*sidebar*/, mLeftSplitter );
  mTodoList->setObjectName( "todolist" );

  mEventViewerBox = new KVBox( mLeftSplitter );
  mEventViewerBox->setMargin( 0 );
  mEventViewer = new CalendarSupport::IncidenceViewer( mCalendar, mEventViewerBox );
  mEventViewer->setObjectName( "EventViewer" );

  KVBox *rightBox = new KVBox( mPanner );
  rightBox->layout()->setMargin( 0 );
  mNavigatorBar = new NavigatorBar( rightBox );
  mRightFrame = new QStackedWidget( rightBox );
  rightBox->setStretchFactor( mRightFrame, 1 );

  mLeftFrame = mLeftSplitter;
  mLeftFrame->installEventFilter( this );

  // Signals emitted by mDateNavigator
  connect( mDateNavigator, SIGNAL(datesSelected(KCalCore::DateList,QDate)),
           SLOT(showDates(KCalCore::DateList,QDate)) );

  connect( mDateNavigatorContainer, SIGNAL(newEventSignal(QDate)),
           SLOT(newEvent(QDate)) );
  connect( mDateNavigatorContainer, SIGNAL(newTodoSignal(QDate)),
           SLOT(newTodo(QDate)) );
  connect( mDateNavigatorContainer, SIGNAL(newJournalSignal(QDate)),
           SLOT(newJournal(QDate)) );

  // Signals emitted by mNavigatorBar
  connect( mNavigatorBar, SIGNAL(prevYearClicked()),
           mDateNavigator, SLOT(selectPreviousYear()) );
  connect( mNavigatorBar, SIGNAL(nextYearClicked()),
           mDateNavigator, SLOT(selectNextYear()) );
  connect( mNavigatorBar, SIGNAL(prevMonthClicked()),
           mDateNavigator, SLOT(selectPreviousMonth()) );
  connect( mNavigatorBar, SIGNAL(nextMonthClicked()),
           mDateNavigator, SLOT(selectNextMonth()) );
  connect( mNavigatorBar, SIGNAL(monthSelected(int)),
           mDateNavigator, SLOT(selectMonth(int)) );
  connect( mNavigatorBar, SIGNAL(yearSelected(int)),
           mDateNavigator, SLOT(selectYear(int)) );

  // Signals emitted by mDateNavigatorContainer
  connect( mDateNavigatorContainer, SIGNAL(weekClicked(QDate,QDate)),
           SLOT(selectWeek(QDate,QDate)) );

  connect( mDateNavigatorContainer, SIGNAL(prevMonthClicked(QDate,QDate,QDate)),
           mDateNavigator, SLOT(selectPreviousMonth(QDate,QDate,QDate)) );
  connect( mDateNavigatorContainer, SIGNAL(nextMonthClicked(QDate,QDate,QDate)),
           mDateNavigator, SLOT(selectNextMonth(QDate,QDate,QDate)) );
  connect( mDateNavigatorContainer, SIGNAL(prevYearClicked()),
           mDateNavigator, SLOT(selectPreviousYear()) );
  connect( mDateNavigatorContainer, SIGNAL(nextYearClicked()),
           mDateNavigator, SLOT(selectNextYear()) );
  connect( mDateNavigatorContainer, SIGNAL(monthSelected(int)),
           mDateNavigator, SLOT(selectMonth(int)) );
  connect( mDateNavigatorContainer, SIGNAL(yearSelected(int)),
           mDateNavigator, SLOT(selectYear(int)) );
  connect( mDateNavigatorContainer, SIGNAL(goPrevious()),
           mDateNavigator, SLOT(selectPrevious()) );
  connect( mDateNavigatorContainer, SIGNAL(goNext()),
           mDateNavigator, SLOT(selectNext()) );

  connect( mDateNavigatorContainer, SIGNAL(datesSelected(KCalCore::DateList,QDate)),
           mDateNavigator, SLOT(selectDates(KCalCore::DateList,QDate)) );

  connect( mViewManager, SIGNAL(datesSelected(KCalCore::DateList)),
           mDateNavigator, SLOT(selectDates(KCalCore::DateList)) );

  connect( mDateNavigatorContainer, SIGNAL(incidenceDropped(Akonadi::Item,QDate)),
           SLOT(addIncidenceOn(Akonadi::Item,QDate)) );
  connect( mDateNavigatorContainer, SIGNAL(incidenceDroppedMove(Akonadi::Item,QDate)),
           SLOT(moveIncidenceTo(Akonadi::Item,QDate)) );

  connect( mDateChecker, SIGNAL(dayPassed(QDate)),
           mTodoList, SLOT(dayPassed(QDate)) );
  connect( mDateChecker, SIGNAL(dayPassed(QDate)),
           SIGNAL(dayPassed(QDate)) );
  connect( mDateChecker, SIGNAL(dayPassed(QDate)),
           mDateNavigatorContainer, SLOT(updateToday()) );

  connect( this, SIGNAL(configChanged()),
           mDateNavigatorContainer, SLOT(updateConfig()) );

  connect( this, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           mEventViewer, SLOT(setIncidence(Akonadi::Item,QDate)) );

  //TODO: do a pretty Summary,
  QString s;
  s = i18n( "<p><em>No Item Selected</em></p>"
            "<p>Select an event, to-do or journal entry to view its details "
            "here.</p>" );

  mEventViewer->setDefaultMessage( s );
  mEventViewer->setWhatsThis(
                   i18n( "View the details of events, journal entries or to-dos "
                         "selected in KOrganizer's main view here." ) );
  mEventViewer->setIncidence( Akonadi::Item(), QDate() );

  mViewManager->connectTodoView( mTodoList );
  mViewManager->connectView( mTodoList );

  KOGlobals::self()->
    setHolidays( new KHolidays::HolidayRegion( KOPrefs::instance()->mHolidays ) );

  connect( QApplication::clipboard(), SIGNAL(dataChanged()),
           SLOT(checkClipboard()) );

  connect( mTodoList, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           this, SLOT(processTodoListSelection(Akonadi::Item,QDate)) );
  disconnect( mTodoList, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
              this, SLOT(processMainViewSelection(Akonadi::Item,QDate)) );

  {
    static bool pageRegistered = false;

    if ( !pageRegistered ) {
      Akonadi::CollectionPropertiesDialog::registerPage(
        new CalendarSupport::CollectionGeneralPageFactory );
      pageRegistered = true;
    }
  }
}

CalendarView::~CalendarView()
{
  mCalendar->unregisterObserver( this );
  qDeleteAll( mFilters );
  qDeleteAll( mExtensions );
//
  delete mDialogManager;
  delete mViewManager;
  delete mEventViewer;
  delete mHistory;
}

void CalendarView::setCalendar( CalendarSupport::Calendar *cal )
{
  mCalendar = cal;
  CalendarSupport::FreeBusyManager::self()->setCalendar( cal );

  mHistory->deleteLater();
  mHistory = new History( mCalendar, this );
  connect( mHistory, SIGNAL(undone()), SLOT(updateView()) );
  connect( mHistory, SIGNAL(redone()), SLOT(updateView()) );

  setIncidenceChanger(
    new CalendarSupport::IncidenceChanger(
      mCalendar, this,
      CalendarSupport::KCalPrefs::instance()->defaultCalendarId() ) );

  mChanger->setDestinationPolicy(
    static_cast<CalendarSupport::IncidenceChanger::DestinationPolicy>(
      KOPrefs::instance()->destination() ) );

  mCalendar->registerObserver( this );

  mDateNavigatorContainer->setCalendar( mCalendar );

  mTodoList->setCalendar( mCalendar );

  mEventViewer->setCalendar( mCalendar );
}

void CalendarView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  delete mChanger;
  mChanger = changer;

  emit newIncidenceChanger( mChanger );
  connect( mChanger, SIGNAL(incidenceAddFinished(Akonadi::Item,bool)),
           this, SLOT(incidenceAddFinished(Akonadi::Item,bool)) );

  qRegisterMetaType<Akonadi::Item>( "Akonadi::Item" );
  qRegisterMetaType<CalendarSupport::IncidenceChanger::WhatChanged>(
    "CalendarSupport::IncidenceChanger::WhatChanged" );
  connect( mChanger,
           SIGNAL(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,CalendarSupport::IncidenceChanger::WhatChanged,bool)),
           this,
           SLOT(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,CalendarSupport::IncidenceChanger::WhatChanged,bool)), Qt::QueuedConnection );
  connect( mChanger, SIGNAL(incidenceToBeDeleted(Akonadi::Item)),
           this, SLOT(incidenceToBeDeleted(Akonadi::Item)) );
  connect( mChanger, SIGNAL(incidenceDeleteFinished(Akonadi::Item,bool)),
           this, SLOT(incidenceDeleteFinished(Akonadi::Item,bool)) );

  connect( mChanger, SIGNAL(schedule(KCalCore::iTIPMethod,Akonadi::Item)),
           this, SLOT(schedule(KCalCore::iTIPMethod,Akonadi::Item)) );

  connect( this, SIGNAL(cancelAttendees(Akonadi::Item)),
           mChanger, SLOT(cancelAttendees(Akonadi::Item)) );
}

CalendarSupport::Calendar *CalendarView::calendar() const
{
  return mCalendar;
}

QDate CalendarView::activeDate( bool fallbackToToday )
{
  KOrg::BaseView *curView = mViewManager->currentView();
  if ( curView ) {
    if ( curView->selectionStart().isValid() ) {
      return curView->selectionStart().date();
    }

    // Try the view's selectedIncidenceDates()
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

void CalendarView::createPrinter()
{
  if ( !mCalPrinter ) {
    mCalPrinter = new CalPrinter( this, mCalendar, new KOCoreHelper() );
    connect( this, SIGNAL(configChanged()), mCalPrinter, SLOT(updateConfig()) );
  }
}

bool CalendarView::openCalendar( const QString &filename, bool merge )
{
  if ( filename.isEmpty() ) {
    kDebug() << "Error! Empty filename.";
    return false;
  }

  if ( !QFile::exists( filename ) ) {
    kDebug() << "Error! File '" << filename << "' doesn't exist.";
  }

  bool loadedSuccesfully = true;
  if ( !merge ) {
#ifdef AKONADI_PORT_DISABLED
    mCalendar->close();
#else
    kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
    // otherwise something is majorly wrong
    // openCalendar called without merge and a filename, what should we do?
    return false;
  }

  CalendarSupport::CalendarAdaptor::Ptr adaptor(
    new CalendarSupport::CalendarAdaptor( mCalendar, this, true/*use default collection*/ ) );

  // merge in a file
  adaptor->startBatchAdding();
  KCalCore::FileStorage storage( adaptor );
  storage.setFileName( filename );
  loadedSuccesfully = storage.load();
  adaptor->endBatchAdding();

  if ( loadedSuccesfully ) {
    if ( !merge ) {
      mViewManager->setDocumentId( filename );
      mTodoList->setDocumentId( filename );
    }
    updateCategories();
    updateView();
    return true;
  } else {
#ifdef AKONADI_PORT_DISABLED
    // while failing to load, the calendar object could
    // have become partially populated.  Clear it out.
    if ( !merge ) {
      mCalendar->close();
    }
#else
    kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
    KMessageBox::error( this, i18n( "Could not load calendar '%1'.", filename ) );
    return false;
  }
}

bool CalendarView::saveCalendar( const QString &filename )
{
  // Store back all unsaved data into calendar object
  mViewManager->currentView()->flushView();

  CalendarSupport::CalendarAdaptor::Ptr adaptor(
    new CalendarSupport::CalendarAdaptor( mCalendar, this ) );

  KCalCore::FileStorage storage( adaptor );
  storage.setFileName( filename );
  storage.setSaveFormat( new KCalCore::ICalFormat );

  return storage.save();
}

void CalendarView::closeCalendar()
{
  // child windows no longer valid
  emit closingDown();
  //mCalendar->close();
  updateView();
}

void CalendarView::archiveCalendar()
{
  mDialogManager->showArchiveDialog();
}

void CalendarView::readSettings()
{
  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::self()->config();

  KConfigGroup geometryConfig( config, "KOrganizer Geometry" );

  QList<int> sizes = geometryConfig.readEntry( "Separator1",QList<int>() );
  if ( sizes.count() != 2 || sizes.count() == sizes.count( 0 ) ) {
    sizes << mDateNavigatorContainer->minimumSizeHint().width();
    sizes << 300;
  }
  mPanner->setSizes( sizes );

  sizes = geometryConfig.readEntry( "Separator2", QList<int>() );
  if ( !sizes.isEmpty() && sizes.count() != sizes.count( 0 ) ) {
     mLeftSplitter->setSizes( sizes );
  }

  mViewManager->readSettings( config );
  mTodoList->restoreLayout( config, QString( "Sidebar Todo View" ), true );

  readFilterSettings( config );

  KConfigGroup viewConfig( config, "Views" );
  int dateCount = viewConfig.readEntry( "ShownDatesCount", 7 );
  if ( dateCount == 7 ) {
    mDateNavigator->selectWeek();
  } else {
    mDateNavigator->selectDates( mDateNavigator->selectedDates().first(), dateCount );
  }
}

void CalendarView::writeSettings()
{
  KConfig *config = KOGlobals::self()->config();

  KConfigGroup geometryConfig( config, "KOrganizer Geometry" );

  QList<int> list = mMainSplitterSizes.isEmpty() ? mPanner->sizes() : mMainSplitterSizes;
  // splitter sizes are invalid (all zero) unless we have been shown once
  if ( list.count() != list.count( 0 ) && mSplitterSizesValid ) {
    geometryConfig.writeEntry( "Separator1", list );
  }

  list = mLeftSplitter->sizes();
  if ( list.count() != list.count( 0 ) && mSplitterSizesValid ) {
    geometryConfig.writeEntry( "Separator2", list );
  }

  mViewManager->writeSettings( config );
  mTodoList->saveLayout( config, QString( "Sidebar Todo View" ) );

  KOPrefs::instance()->writeConfig();
  CalendarSupport::KCalPrefs::instance()->writeConfig();

  writeFilterSettings( config );

  KConfigGroup viewConfig( config, "Views" );
  viewConfig.writeEntry( "ShownDatesCount", mDateNavigator->selectedDates().count() );

  config->sync();
}

void CalendarView::readFilterSettings( KConfig *config )
{
  qDeleteAll( mFilters );
  mFilters.clear();

  KConfigGroup generalConfig( config, "General" );
  // FIXME: Move the filter loading and saving to the CalFilter class in libkcal
  QStringList filterList = generalConfig.readEntry( "CalendarFilters", QStringList() );
  QString currentFilter = generalConfig.readEntry( "Current Filter" );

  QStringList::ConstIterator it = filterList.constBegin();
  QStringList::ConstIterator end = filterList.constEnd();
  while ( it != end ) {
    KCalCore::CalFilter *filter = new KCalCore::CalFilter( *it );
    KConfigGroup filterConfig( config, "Filter_" + (*it) );
    filter->setCriteria( filterConfig.readEntry( "Criteria", 0 ) );
    filter->setCategoryList( filterConfig.readEntry( "CategoryList", QStringList() ) );
    if ( filter->criteria() & KCalCore::CalFilter::HideNoMatchingAttendeeTodos ) {
      filter->setEmailList( CalendarSupport::KCalPrefs::instance()->allEmails() );
    }
    filter->setCompletedTimeSpan( filterConfig.readEntry( "HideTodoDays", 0 ) );
    mFilters.append( filter );

    ++it;
  }

  int pos = filterList.indexOf( currentFilter );
  mCurrentFilter = 0;
  if ( pos >= 0 ) {
    mCurrentFilter = mFilters.at( pos );
  }
  updateFilter();
}

void CalendarView::writeFilterSettings( KConfig *config )
{
  QStringList filterList;

  foreach ( KCalCore::CalFilter *filter, mFilters ) {
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
  if ( dynamic_cast<MonthView*>( mViewManager->currentView() ) ) {
    const QDate month = mDateNavigatorContainer->monthOfNavigator( 0 );
    QPair<QDate,QDate> limits = KODayMatrix::matrixLimits( month );
    mDateNavigator->selectNextMonth( month, limits.first, limits.second );
  } else {
    mDateNavigator->selectNext();
  }
}

void CalendarView::goPrevious()
{
  if ( dynamic_cast<MonthView*>( mViewManager->currentView() ) ) {
    const QDate month = mDateNavigatorContainer->monthOfNavigator( 0 );
    QPair<QDate,QDate> limits = KODayMatrix::matrixLimits( month );
    mDateNavigator->selectPreviousMonth( month, limits.first, limits.second );
  } else {
    mDateNavigator->selectPrevious();
  }
}

void CalendarView::updateConfig()
{
  updateConfig( QByteArray( "korganizer" ) );
}

void CalendarView::updateConfig( const QByteArray &receiver )
{
  if ( receiver != "korganizer" ) {
    return;
  }

  if ( mCalPrinter ) {
    mCalPrinter->deleteLater();
    mCalPrinter = 0;
  }

  KOGlobals::self()->setHolidays( new KHolidays::HolidayRegion( KOPrefs::instance()->mHolidays ) );

  // Only set a new time zone if it changed. This prevents the window
  // from being modified on start
  KDateTime::Spec newTimeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  if ( mCalendar->viewTimeSpec() != newTimeSpec ) {

    const QString question( i18n( "The time zone setting was changed. "
                                  "Do you want to keep the absolute time of "
                                  "the items in your calendar, which will show "
                                  "them to be at a different time than "
                                  "before, or move them to be at the old time "
                                  "also in the new time zone?" ) );
    int rc = KMessageBox::questionYesNo( this, question,
                                         i18n( "Keep Absolute Times?" ),
                                         KGuiItem( i18n( "Keep Times" ) ),
                                         KGuiItem( i18n( "Move Times" ) ),
                                         "calendarKeepAbsoluteTimes" );
    if ( rc == KMessageBox::Yes ) {
      // keep the absolute time - note the new viewing time zone in the calendar
      mCalendar->setViewTimeSpec( newTimeSpec );
    } else {
      // only set the new timezone, wihtout shifting events, they will be
      // interpreted as being in the new timezone now
      mCalendar->shiftTimes( mCalendar->viewTimeSpec(), newTimeSpec );
    }
  }

  // config changed lets tell the date navigator the new modes
  // if there weren't changed they are ignored
  updateHighlightModes();

  emit configChanged();

  //switch beetween merged, side by side and tabbed agenda if needed
  mViewManager->updateMultiCalendarDisplay();

  // To make the "fill window" configurations work
  mViewManager->raiseCurrentView();

  mChanger->setDestinationPolicy(
    static_cast<CalendarSupport::IncidenceChanger::DestinationPolicy>(
      KOPrefs::instance()->destination() ) );
}

void CalendarView::incidenceAddFinished( const Akonadi::Item &incidence, bool success )
{
  if ( success ) {
    history()->recordAdd( incidence );
    changeIncidenceDisplay( incidence, CalendarSupport::IncidenceChanger::INCIDENCEADDED );
    updateUnmanagedViews();
    checkForFilteredChange( incidence );
  } else {
    kError() << "Incidence not added, job reported error";
  }
}

void CalendarView::incidenceChangeFinished(
  const Akonadi::Item &oldIncidence_, const Akonadi::Item &newIncidence_,
  CalendarSupport::IncidenceChanger::WhatChanged modified, bool success )
{
  if ( !success ) {
    kError() << "Incidence not chanded, job reported error";
    return;
  }

  if ( !oldIncidence_.isValid() ||
       !newIncidence_.isValid() ) {
    // If the item was deleted while a modify job was active, incidenceChanger
    // wont have a valid item to send here, but also wont return success == false
    // so users don't see an error message
    return;
  }

  KCalCore::Incidence::Ptr oldIncidence = CalendarSupport::incidence( oldIncidence_ );
  KCalCore::Incidence::Ptr newIncidence = CalendarSupport::incidence( newIncidence_ );

  history()->recordEdit( oldIncidence_, newIncidence_ );

  // Record completed todos in journals, if enabled. we should to this here in
  // favor of the todolist. users can mark a task as completed in an editor
  // as well.
  if ( newIncidence->type() == Incidence::TypeTodo &&
       KOPrefs::instance()->recordTodosInJournals() &&
       ( modified == CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED ||
         modified == CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED_WITH_RECURRENCE ) ) {
    KCalCore::Todo::Ptr todo = CalendarSupport::todo( newIncidence_ );
    if ( todo->isCompleted() ||
         modified == CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED_WITH_RECURRENCE ) {
      QString timeStr = KGlobal::locale()->formatTime( QTime::currentTime() );
      QString description = i18n( "Todo completed: %1 (%2)", newIncidence->summary(), timeStr );

      Akonadi::Item::List journals = calendar()->journals( QDate::currentDate() );

      if ( journals.isEmpty() ) {
        Journal::Ptr journal( new Journal );
        journal->setDtStart(
          KDateTime::currentDateTime( CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

        QString dateStr = KGlobal::locale()->formatDate( QDate::currentDate() );
        journal->setSummary( i18n( "Journal of %1", dateStr ) );
        journal->setDescription( description );

        Akonadi::Collection selectedCollection;

        if ( !mChanger->addIncidence( journal, newIncidence_.parentCollection(), this ) ) {
          kError() << "Unable to add Journal";
          return;
        }

      } else { // journal list is not empty
        Akonadi::Item journalItem = journals.first();
        Journal::Ptr journal = CalendarSupport::journal( journalItem );
        Journal::Ptr oldJournal( journal->clone() );
        journal->setDescription( journal->description().append( '\n' + description ) );
        mChanger->changeIncidence( oldJournal, journalItem,
                                   CalendarSupport::IncidenceChanger::DESCRIPTION_MODIFIED, this );

      }
    }
  }

  changeIncidenceDisplay( newIncidence_, CalendarSupport::IncidenceChanger::INCIDENCEEDITED );
  updateUnmanagedViews();
  checkForFilteredChange( newIncidence_ );
}

void CalendarView::incidenceToBeDeleted( const Akonadi::Item &item )
{
  kDebug() << "incidenceToBeDeleted item.id() :" << item.id();
  history()->recordDelete( item );
  //  changeIncidenceDisplay( incidence, CalendarSupport::IncidenceChanger::INCIDENCEDELETED );
  updateUnmanagedViews();
}

void CalendarView::incidenceDeleteFinished( const Akonadi::Item &item, bool success )
{
  if ( success ) {
    changeIncidenceDisplay( item, CalendarSupport::IncidenceChanger::INCIDENCEDELETED );
    updateUnmanagedViews();
  } else {
    kError() << "Incidence not deleted, job reported error";
  }
}

void CalendarView::checkForFilteredChange( const Akonadi::Item &item )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  KCalCore::CalFilter *filter = calendar()->filter();
  if ( filter && !filter->filterIncidence( incidence ) ) {
    // Incidence is filtered and thus not shown in the view, tell the
    // user so that he isn't surprised if his new event doesn't show up
    KMessageBox::information(
      this,
      i18n( "The item \"%1\" is filtered by your current filter rules, "
            "so it will be hidden and not appear in the view.",
            incidence->summary() ),
      i18n( "Filter Applied" ),
      "ChangedIncidenceFiltered" );
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

void CalendarView::changeIncidenceDisplay( const Akonadi::Item &item, int action )
{
  if ( mDateNavigatorContainer->isVisible() ) {
    mDateNavigatorContainer->updateView();
  }

  mDialogManager->updateSearchDialog();

  if ( CalendarSupport::hasIncidence( item ) ) {
    // If there is an event view visible update the display
    mViewManager->currentView()->changeIncidenceDisplay( item, action );
    if ( mTodoList && mTodoList->isVisible() ) {
      mTodoList->changeIncidenceDisplay( item, action );
    }
  } else {
    mViewManager->currentView()->updateView();
    if ( mTodoList && mViewManager->currentView()->identifier() != "DefaultTodoView" ) {
      mTodoList->updateView();
    }
  }
}

void CalendarView::updateView( const QDate &start, const QDate &end,
                               const QDate &preferredMonth,
                               const bool updateTodos )
{
  const bool currentViewIsTodoView = mViewManager->currentView()->identifier() == "DefaultTodoView";

  if ( updateTodos && !currentViewIsTodoView && mTodoList->isVisible() ) {
    // Update the sidepane todoView
    mTodoList->updateView();
  }

  if ( start.isValid() && end.isValid() && !( currentViewIsTodoView && !updateTodos ) ) {
    mViewManager->updateView( start, end, preferredMonth );
  }

  if ( mDateNavigatorContainer->isVisible() ) {
    mDateNavigatorContainer->updateView();
  }
}

void CalendarView::updateView()
{
  const DateList tmpList = mDateNavigator->selectedDates();
  const QDate month = mDateNavigatorContainer->monthOfNavigator();

  // We assume that the navigator only selects consecutive days.
  updateView( tmpList.first(), tmpList.last(), month/**preferredMonth*/ );
}

void CalendarView::updateUnmanagedViews()
{
  if ( mDateNavigatorContainer->isVisible() ) {
    mDateNavigatorContainer->updateDayMatrix();
  }
}

int CalendarView::msgItemDelete( const Akonadi::Item &item )
{
  return KMessageBox::warningContinueCancel(
    this,
    i18nc( "@info",
           "Do you really want to permanently remove the item \"%1\"?",
           CalendarSupport::incidence( item )->summary() ),
    i18nc( "@title:window", "Delete Item?" ),
    KStandardGuiItem::del() );
}

void CalendarView::edit_cut()
{
  const Akonadi::Item item = selectedIncidence();
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence || !mChanger ) {
    KNotification::beep();
    return;
  }

  Akonadi::Item::List items;
  int km = KMessageBox::Yes;

  if ( !mCalendar->findChildren( item ).isEmpty() &&
       incidence->type() == Incidence::TypeTodo ) { // Only todos (yet?)
    km = KMessageBox::questionYesNoCancel(
      this,
      i18n( "The item \"%1\" has sub-to-dos. "
            "Do you want to cut just this item and "
            "make all its sub-to-dos independent, or "
            "cut the to-do with all its sub-to-dos?",
            incidence->summary() ),
      i18n( "KOrganizer Confirmation" ),
      KGuiItem( i18n( "Cut Only This" ) ),
      KGuiItem( i18n( "Cut All" ) ) );
  }

  if ( km == KMessageBox::Yes ) { // only one
    items.append( item );
    makeChildrenIndependent( item );
  } else if ( km == KMessageBox::No ) { // all
    // load incidence + children + grandchildren...
    getIncidenceHierarchy( item, items );
  }

  if ( km != KMessageBox::Cancel ) {
    mChanger->cutIncidences( items, this );
  }

  checkClipboard();
}

void CalendarView::edit_copy()
{
  const Akonadi::Item item = selectedIncidence();

  if ( !item.isValid() ) {
    KNotification::beep();
    return;
  }

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  Akonadi::Item::List items;
  int km = KMessageBox::Yes;

  if ( !mCalendar->findChildren( item ).isEmpty()  &&
       incidence->type() == Incidence::TypeTodo ) { // only todos.
    km = KMessageBox::questionYesNoCancel(
      this,
      i18n( "The item \"%1\" has sub-to-dos. "
            "Do you want to copy just this item or "
            "copy the to-do with all its sub-to-dos?",
            incidence->summary() ),
      i18n( "KOrganizer Confirmation" ),
      KGuiItem( i18n( "Copy Only This" ) ),
      KGuiItem( i18n( "Copy All" ) ) );
   }

  if ( km == KMessageBox::Yes ) { // only one
    items.append( item );
  } else if ( km == KMessageBox::No ) { // all
    // load incidence + children + grandchildren...
    getIncidenceHierarchy( item, items );
  }

  if ( km != KMessageBox::Cancel ) {
    CalendarSupport::CalendarAdaptor::Ptr cal(
      new CalendarSupport::CalendarAdaptor( mCalendar, this ) );

    CalendarSupport::DndFactory factory( cal );

    if ( !factory.copyIncidences( items ) ) {
      KNotification::beep();
    }
  }

  checkClipboard();
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

  KOAgendaView *agendaView = mViewManager->agendaView();
  MonthView *monthView = mViewManager->monthView();

  if ( !curView ) {
    return;
  }

  if ( curView == agendaView && agendaView->selectionStart().isValid() ) {
    date = agendaView->selectionStart().date();
    endDT = agendaView->selectionEnd();
    useEndTime = !agendaView->selectedIsSingleCell();
    if ( !agendaView->selectedIsAllDay() ) {
      time = agendaView->selectionStart().time();
      timeSet = true;
    }
  } else if ( curView == monthView && monthView->selectionStart().isValid() ) {
    date = monthView->selectionStart().date();
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

  CalendarSupport::CalendarAdaptor::Ptr cal(
    new CalendarSupport::CalendarAdaptor( mCalendar, this ) );

  CalendarSupport::DndFactory factory( cal );

  Incidence::List pastedIncidences;
  if ( timeSet && time.isValid() ) {
    pastedIncidences = factory.pasteIncidences( KDateTime( date, time ) );
  } else {
    pastedIncidences = factory.pasteIncidences( KDateTime( date ) );
  }
  Akonadi::Collection col;
  Incidence::List::Iterator it;
  Akonadi::Collection selectedCollection;
  int dialogCode = 0;

  {
    // If only one collection exists, don't bother the user with a prompt
    CalendarSupport::CollectionSelection *selection =
      EventViews::EventView::globalCollectionSelection();

    if ( selection && selection->model()->model()->rowCount() == 1 ) {
      const QModelIndex index = selection->model()->model()->index( 0, 0 );
      if ( index.isValid() ) {
        selectedCollection = CalendarSupport::collectionFromIndex( index );
      }
    }
  }

  for ( it = pastedIncidences.begin(); it != pastedIncidences.end(); ++it ) {
    // FIXME: use a visitor here
    if ( ( *it )->type() == Incidence::TypeEvent ) {
      KCalCore::Event::Ptr pastedEvent = ( *it ).staticCast<Event>();
      // only use selected area if event is of the same type (all-day or non-all-day
      // as the current selection is
      if ( agendaView && endDT.isValid() && useEndTime ) {
        if ( ( pastedEvent->allDay() && agendaView->selectedIsAllDay() ) ||
             ( !pastedEvent->allDay() && !agendaView->selectedIsAllDay() ) ) {
          KDateTime kdt( endDT, CalendarSupport::KCalPrefs::instance()->timeSpec() );
          pastedEvent->setDtEnd( kdt.toTimeSpec( pastedEvent->dtEnd().timeSpec() ) );
        }
      }

      pastedEvent->setRelatedTo( QString() );
      if ( selectedCollection.isValid() ) {
        mChanger->addIncidence( KCalCore::Event::Ptr( pastedEvent->clone() ),
                                selectedCollection, this );
      } else {
        mChanger->addIncidence( KCalCore::Event::Ptr( pastedEvent->clone() ),
                                this, selectedCollection, dialogCode );
      }
    } else if ( ( *it )->type() == Incidence::TypeTodo ) {
      KCalCore::Todo::Ptr pastedTodo = ( *it ).staticCast<Todo>();
      Akonadi::Item _selectedTodoItem = selectedTodo();

      // if we are cutting a hierarchy only the root
      // should be son of _selectedTodo
      KCalCore::Todo::Ptr _selectedTodo = CalendarSupport::todo( _selectedTodoItem );
      if ( _selectedTodo && pastedTodo->relatedTo().isEmpty() ) {
        pastedTodo->setRelatedTo( _selectedTodo->uid() );
      }

      if ( selectedCollection.isValid() ) {
        // When pasting multiple incidences, don't ask which collection to use, for each one
        mChanger->addIncidence( KCalCore::Todo::Ptr( pastedTodo->clone() ),
                                selectedCollection, this );
      } else {
        mChanger->addIncidence( KCalCore::Todo::Ptr( pastedTodo->clone() ),
                                this, selectedCollection, dialogCode );
      }

    } else if ( ( *it )->type() == Incidence::TypeJournal ) {

      if ( selectedCollection.isValid() ) {
        // When pasting multiple incidences, don't ask which collection to use, for each one
        mChanger->addIncidence( KCalCore::Incidence::Ptr( ( *it )->clone() ),
                                selectedCollection, this );
      } else {
        mChanger->addIncidence( KCalCore::Incidence::Ptr( ( *it )->clone() ),
                                this, selectedCollection, dialogCode );
      }
    }
  }
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
}

void CalendarView::dateTimesForNewEvent( QDateTime &startDt, QDateTime &endDt,
                                         bool &allDay )
{
  mViewManager->currentView()->eventDurationHint( startDt, endDt, allDay );

  if ( !startDt.isValid() || !endDt.isValid() ) {
    startDt.setDate( activeDate( true ) );
    startDt.setTime( CalendarSupport::KCalPrefs::instance()->mStartTime.time() );

    int addSecs =
      ( CalendarSupport::KCalPrefs::instance()->mDefaultDuration.time().hour() * 3600 ) +
      ( CalendarSupport::KCalPrefs::instance()->mDefaultDuration.time().minute() * 60 );
    endDt = startDt.addSecs( addSecs );
  }
}

IncidenceEditorNG::IncidenceDialog *CalendarView::newEventEditor(
  const KCalCore::Event::Ptr &event )
{
  Akonadi::Item item;
  item.setPayload( event );

  IncidenceEditorNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
  dialog->load( item );

//  connectIncidenceEditor( dialog );
  mDialogManager->connectTypeAhead(
    dialog, dynamic_cast<KOEventView*>( viewManager()->currentView() ) );

  return dialog;
}

void CalendarView::newEvent()
{
  newEvent( QDateTime(), QDateTime() );
}

void CalendarView::newEvent( const QDate &dt )
{
  QDateTime startDt( dt, CalendarSupport::KCalPrefs::instance()->mStartTime.time() );
  QTime duration = CalendarSupport::KCalPrefs::instance()->defaultDuration().time();
  QTime time = startDt.time();

  time = time.addSecs( duration.hour() * 3600 +
                        duration.minute() * 60 +
                        duration.second() );
  QDateTime endDt( startDt );
  endDt.setTime( time );
  newEvent( startDt, endDt );
}

void CalendarView::newEvent( const QDateTime &startDt )
{
  newEvent( startDt, startDt );
}

void CalendarView::newEvent( const QDateTime &startDtParam, const QDateTime &endDtParam,
                             bool allDay )
{
  // Let the current view change the default start/end datetime
  QDateTime startDt( startDtParam );
  QDateTime endDt( endDtParam );

  // Adjust the start/end date times (i.e. replace invalid values by defaults,
  // and let the view adjust the type.
  dateTimesForNewEvent( startDt, endDt, allDay );

  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
  defaults.setStartDateTime( KDateTime( startDt ) );
  defaults.setEndDateTime( KDateTime( endDt ) );

  KCalCore::Event::Ptr event( new Event );
  defaults.setDefaults( event );
  event->setAllDay( allDay );

  IncidenceEditorNG::IncidenceDialog *eventEditor = newEventEditor( event );
  Q_ASSERT( eventEditor );

  // Fallsback to the default collection defined in config
  eventEditor->selectCollection( defaultCollection( KCalCore::Event::eventMimeType() ) );
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QStringList &attachments, const QStringList &attendees,
                             const QStringList &attachmentMimetypes, bool inlineAttachment )
{
  // Adjust the start/end date times (i.e. replace invalid values by defaults,
  // and let the view adjust the type.
  QDateTime startDt;
  QDateTime endDt;
  bool allDay = false;
  dateTimesForNewEvent( startDt, endDt, allDay );

  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
  defaults.setStartDateTime( KDateTime( startDt ) );
  defaults.setEndDateTime( KDateTime( endDt ) );
  // if attach or attendee list is empty, these methods don't do anything, so
  // it's safe to call them in every case
  defaults.setAttachments( attachments, attachmentMimetypes, QStringList(), inlineAttachment );
  defaults.setAttendees( attendees );

  KCalCore::Event::Ptr event( new Event );
  defaults.setDefaults( event );

  event->setSummary( summary );
  event->setDescription( description );
  event->setAllDay( allDay );
  newEventEditor( event );
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                            const QStringList &attachments, const QStringList &attendees,
                            const QStringList &attachmentMimetypes,
                            bool inlineAttachment )
{
  Akonadi::Collection defaultCol = defaultCollection( Todo::todoMimeType() );

  IncidenceEditorNG::IncidenceDialogFactory::createTodoEditor(
    summary, description, attachments,
    attendees, attachmentMimetypes,
    QStringList()/* attachment labels */,
    inlineAttachment, defaultCol,
    this/* parent */ );
}

void CalendarView::newTodo()
{
  newTodo( Akonadi::Collection() );
}

void CalendarView::newTodo( const Akonadi::Collection &collection )
{
  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();

  bool allDay = true;
  if ( mViewManager->currentView()->isEventView() ) {
    QDateTime startDt;
    QDateTime endDt;
    dateTimesForNewEvent( startDt, endDt, allDay );

    defaults.setStartDateTime( KDateTime( startDt ) );
    defaults.setEndDateTime( KDateTime( endDt ) );
  }

  KCalCore::Todo::Ptr todo( new Todo );
  defaults.setDefaults( todo );
  todo->setAllDay( allDay );

  Akonadi::Item item;
  item.setPayload( todo );

  IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor( item, collection );

  dialog->load( item );
}

void CalendarView::newTodo( const QDate &date )
{
  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
  defaults.setEndDateTime( KDateTime( date, QTime::currentTime() ) );

  KCalCore::Todo::Ptr todo( new Todo );
  defaults.setDefaults( todo );
  todo->setAllDay( true );

  Akonadi::Item item;
  item.setPayload( todo );

  IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor( item );
  dialog->load( item );
}

void CalendarView::newJournal()
{
  newJournal( QString(), activeDate( true ) );
}

void CalendarView::newJournal( const QDate &date )
{
  newJournal( QString(), date.isValid() ? date : activeDate( true ) );
}

void CalendarView::newJournal( const Akonadi::Collection &collection )
{
  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();

  Journal::Ptr journal( new Journal );
  defaults.setDefaults( journal );

  Akonadi::Item item;
  item.setPayload( journal );
  IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor( item, collection );
  dialog->load( item );
}

void CalendarView::newJournal( const QString &text, const QDate &date )
{
  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();

  Journal::Ptr journal( new Journal );
  defaults.setStartDateTime( KDateTime( date ) );
  defaults.setDefaults( journal );

  journal->setSummary( text );

  Akonadi::Item item;
  item.setPayload( journal );

  IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor( item );
  dialog->load( item );
}

KOrg::BaseView *CalendarView::currentView() const
{
  return mViewManager->currentView();
}

void CalendarView::configureCurrentView()
{
  KOrg::BaseView *const view = currentView();
  if ( view && view->hasConfigurationDialog() ) {
    view->showConfigurationDialog( this );
  }
}

void CalendarView::newSubTodo()
{
  const Akonadi::Item item = selectedTodo();
  if ( CalendarSupport::hasTodo( item ) ) {
    newSubTodo( item );
  }
}

void CalendarView::newSubTodo( const Akonadi::Collection &collection )
{
  if ( !CalendarSupport::hasTodo( selectedTodo() ) ) {
    kWarning() << "CalendarSupport::hasTodo() is false";
    return;
  }

  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
  defaults.setRelatedIncidence( CalendarSupport::incidence( selectedTodo() ) );

  KCalCore::Todo::Ptr todo( new Todo );
  defaults.setDefaults( todo );

  Akonadi::Item item;
  item.setPayload( todo );

  IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor( item, collection );
  dialog->load( item );
}

void CalendarView::newSubTodo( const Akonadi::Item &parentTodo )
{
  IncidenceEditorNG::IncidenceDefaults defaults =
    IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults();
  defaults.setRelatedIncidence( CalendarSupport::incidence( parentTodo ) );

  KCalCore::Todo::Ptr todo( new Todo );
  defaults.setDefaults( todo );

  Q_ASSERT( !todo->relatedTo().isEmpty() );

  Akonadi::Item item;
  item.setPayload( todo );

  // Don't use parentTodo.parentCollection() because that can be a search folder.
  Akonadi::Collection collection = mCalendar->collection( parentTodo.storageCollectionId() );
  IncidenceEditorNG::IncidenceDialog *dialog = createIncidenceEditor( item, collection );
  dialog->load( item );
}

void CalendarView::newFloatingEvent()
{
  const QDate date = activeDate();
  newEvent( QDateTime( date, QTime( 12, 0, 0 ) ),
            QDateTime( date, QTime( 12, 0, 0 ) ), true );
}

bool CalendarView::addIncidence( const QString &ical )
{
  KCalCore::ICalFormat format;
  format.setTimeSpec( mCalendar->timeSpec() );
  Incidence::Ptr incidence( format.fromString( ical ) );
  return addIncidence( incidence );
}

bool CalendarView::addIncidence( const Incidence::Ptr &incidence )
{
  Akonadi::Collection col;
  int dialogCode = 0;
  return incidence ? mChanger->addIncidence( incidence, this, col, dialogCode ) : false;
}

void CalendarView::appointment_show()
{
  const Akonadi::Item item  = selectedIncidence();
  if ( CalendarSupport::hasIncidence( item ) ) {
    showIncidence( item );
  } else {
    KNotification::beep();
  }
}

void CalendarView::appointment_edit()
{
  const Akonadi::Item item = selectedIncidence();
  if ( CalendarSupport::hasIncidence( item ) ) {
    editIncidence( item );
  } else {
    KNotification::beep();
  }
}

void CalendarView::appointment_delete()
{
  const Akonadi::Item item = selectedIncidence();
  if ( CalendarSupport::hasIncidence( item ) ) {
    deleteIncidence( item );
  } else {
    KNotification::beep();
  }
}

void CalendarView::todo_unsub()
{
  const Akonadi::Item aTodo = selectedTodo();
  if ( incidence_unsub( aTodo ) ) {
    updateView();
  }
}

bool CalendarView::incidence_unsub( const Akonadi::Item &item )
{
  const Incidence::Ptr inc = CalendarSupport::incidence( item );

  if ( !inc || inc->relatedTo().isEmpty() ) {
    return false;
  }

  Incidence::Ptr oldInc( inc->clone() );
  inc->setRelatedTo( 0 );
  mChanger->changeIncidence( oldInc, item,
                             CalendarSupport::IncidenceChanger::RELATION_MODIFIED, this );

  return true;
}

bool CalendarView::makeSubTodosIndependent( )
{
  bool  status = false;
  const Akonadi::Item aTodo = selectedTodo();

  if( makeChildrenIndependent( aTodo ) ) {
    updateView();
    status = true;
  }
  return status;
}

bool CalendarView::makeChildrenIndependent( const Akonadi::Item &item )
{
  const Incidence::Ptr inc = CalendarSupport::incidence( item );

  Akonadi::Item::List subIncs = mCalendar->findChildren( item );

  if ( !inc || subIncs.isEmpty() ) {
    return false;
  }
  startMultiModify ( i18n( "Make sub-to-dos independent" ) );

  foreach ( const Akonadi::Item &item, subIncs ) {
    incidence_unsub( item );
  }

  endMultiModify();
  return true;
}

bool CalendarView::deleteIncidence( const Akonadi::Item::Id &uid, bool force )
{
  Akonadi::Item item = mCalendar->incidence( uid );
  if ( !CalendarSupport::hasIncidence( item ) ) {
    return false;
  }
  return deleteIncidence( item, force );
}

void CalendarView::toggleAlarm( const Akonadi::Item &item )
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence || !mChanger ) {
    kDebug() << "called without having a clicked item";
    return;
  }
  Incidence::Ptr oldincidence( incidence->clone() );

  Alarm::List alarms = incidence->alarms();
  Alarm::List::ConstIterator it;
  for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
    (*it)->toggleAlarm();
  }
  if ( alarms.isEmpty() ) {
    // Add an alarm if it didn't have one
    Alarm::Ptr alm = incidence->newAlarm();
    alm->setType( Alarm::Display );
    alm->setEnabled( true );
    int duration; // in secs
    switch( CalendarSupport::KCalPrefs::instance()->mReminderTimeUnits ) {
    default:
    case 0: // mins
      duration = CalendarSupport::KCalPrefs::instance()->mReminderTime * 60;
      break;
    case 1: // hours
      duration = CalendarSupport::KCalPrefs::instance()->mReminderTime * 60 * 60;
      break;
    case 2: // days
      duration = CalendarSupport::KCalPrefs::instance()->mReminderTime * 60 * 60 * 24;
      break;
    }
    if ( incidence->type() == Incidence::TypeEvent ) {
      alm->setStartOffset( KCalCore::Duration( -duration ) );
    } else {
      alm->setEndOffset( KCalCore::Duration( -duration ) );
    }
  }
  mChanger->changeIncidence( oldincidence, item,
                             CalendarSupport::IncidenceChanger::ALARM_MODIFIED, this );
}

void CalendarView::toggleTodoCompleted( const Akonadi::Item &todoItem )
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( todoItem );

  if ( !incidence || !mChanger ) {
    kDebug() << "called without having a clicked item";
    return;
  }
  if ( incidence->type() != Incidence::TypeTodo ) {
    kDebug() << "called for a non-Todo incidence";
    return;
  }

  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );
  KCalCore::Todo::Ptr oldtodo( todo->clone() );

  if ( todo->isCompleted() ) {
    todo->setPercentComplete( 0 );
  } else {
    todo->setCompleted( KDateTime::currentDateTime(
                          CalendarSupport::KCalPrefs::instance()->timeSpec() ) );
  }

  mChanger->changeIncidence( oldtodo,
                             todoItem,
                             CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED,
                             this );
}

void CalendarView::copyIncidenceToResource( const Akonadi::Item &item, const QString &resourceId )
{
#ifdef AKONADI_PORT_DISABLED
  if ( !incidence ) {
    kDebug() << "called without having a clicked item";
    return;
  }

  KCalCore::CalendarResources *const resources = KOrg::StdCalendar::self();
  KCalCore::CalendarResourceManager *const manager = resources->resourceManager();

  // Find the resource the incidence should be copied to
  ResourceCalendar *newCal = 0;
  KCalCore::CalendarResourceManager::iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    ResourceCalendar *const resource = *it;
    if ( resource->identifier() == resourceId ) {
      newCal = resource;
      break;
    }
  }
  if ( !newCal ) {
    return;
  }

  // Clone a new Incidence from the selected Incidence and give it a new Uid.
  Incidence::Ptr newInc;
  if ( incidence->type() == Incidence::TypeEvent ) {
    KCalCore::Event::Ptr nEvent( static_cast<KCalCore::Event::Ptr >( incidence )->clone() );
    nEvent->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nEvent;
  } else if ( incidence->type() == Incidence::TypeTodo ) {
    KCalCore::Todo::Ptr nTodo( static_cast<KCalCore::Todo::Ptr >( incidence )->clone() );
    nTodo->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nTodo;
  } else if ( incidence->type() == Incidence::TypeJournal ) {
    KCalCore::Journal::Ptr nJournal( static_cast<KCalCore::Journal::Ptr >( incidence )->clone() );
    nJournal->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nJournal;
  } else {
    kWarning() << "Trying to copy an incidence type that cannot be copied";
    return;
  }

  if ( resources->addIncidence( newInc, newCal ) ) {
    incidenceAddFinished( newInc, true );
    KMessageBox::information(
      this,
      i18nc( "@info",
             "\"%1\" was successfully copied to %2.",
             incidence->summary(),
             newCal->resourceName() ),
      i18nc( "@title:window", "Copying Succeeded" ),
      "CalendarIncidenceCopy" );
  } else {
    KMessageBox::error(
      this,
      i18nc( "@info",
             "Unable to copy the item \"%1\" to %2.",
             incidence->summary(),
             newCal->resourceName() ),
      i18nc( "@title:window", "Copying Failed" ) );
  }
#else
  Q_UNUSED( resourceId );
  Q_UNUSED( item );
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

void CalendarView::moveIncidenceToResource( const Akonadi::Item &item, const QString &resourceId )
{
#ifdef AKONADI_PORT_DISABLED
  if ( !incidence ) {
    kDebug() << "called without having a clicked item";
    return;
  }

  KCalCore::CalendarResources *const resources = KOrg::StdCalendar::self();
  KCalCore::CalendarResourceManager *const manager = resources->resourceManager();

  // Find the resource the incidence should be moved to
  ResourceCalendar *newCal = 0;
  KCalCore::CalendarResourceManager::iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    ResourceCalendar *const resource = *it;
    if ( resource->identifier() == resourceId ) {
      newCal = resource;
      break;
    }
  }
  if ( !newCal ) {
    return;
  }

  // Clone a new Incidence from the selected Incidence and give it a new Uid.
  Incidence *newInc;
  if ( incidence->type() == Incidence::TypeEvent ) {
    KCalCore::Event::Ptr nEvent = static_cast<KCalCore::Event::Ptr >( incidence )->clone();
    nEvent->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nEvent;
  } else if ( incidence->type() == Incidence::TypeTodo ) {
    KCalCore::Todo::Ptr nTodo = static_cast<KCalCore::Todo::Ptr >( incidence )->clone();
    nTodo->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nTodo;
  } else if ( incidence->type() == Incidence::TypeJournal ) {
    KCalCore::Journal::Ptr nJournal = static_cast<KCalCore::Journal::Ptr >( incidence )->clone();
    nJournal->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nJournal;
  } else {
    kWarning() << "Trying to move an incidence type that cannot be moved";
    return;
  }

  if ( resources->addIncidence( newInc, newCal ) ) {
    incidenceAddFinished( newInc, true );
    ResourceCalendar *const oldCal = resources->resource( incidence );
    if ( !oldCal || resources->deleteIncidence( incidence ) ) {
      KMessageBox::error(
        this,
        i18nc( "@info",
               "Unable to remove the item \"%1\" from %2. "
               "However, a copy of this item has been put into %3.",
               incidence->summary(),
               oldCal->resourceName(),
               newCal->resourceName() ),
        i18nc( "@title:window", "Moving Failed" ) );
    } else {
      incidenceDeleteFinished( incidence, true );
      KMessageBox::information(
        this,
        i18nc( "@info",
               "\"%1\" was successfully moved from %2 to %3.",
               incidence->summary(),
               oldCal->resourceName(),
               newCal->resourceName() ),
        i18nc( "@title:window", "Moving Succeeded" ),
        "CalendarIncidenceMove" );
    }
  } else {
    KMessageBox::error(
      this,
      i18nc( "@info",
             "Unable to add the item \"%1\" into %2. "
             "This item has not been moved.",
             incidence->summary(),
             newCal->resourceName() ),
      i18nc( "@title:window", "Moving Failed" ) );
  }
#else
  Q_UNUSED( resourceId );
  Q_UNUSED( item );
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

void CalendarView::dissociateOccurrences( const Akonadi::Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( !incidence || !mChanger ) {
    kError() << "Called without having a clicked item";
    return;
  }

  KDateTime thisDateTime( date, CalendarSupport::KCalPrefs::instance()->timeSpec() );
  bool isFirstOccurrence = !incidence->recurrence()->getPreviousDateTime( thisDateTime ).isValid();

  int answer;
  bool doOnlyThis = false;
  bool doFuture   = false;

  if ( isFirstOccurrence ) {
    answer = KMessageBox::questionYesNo(
      this,
      i18n( "Do you want to dissociate "
            "the occurrence on %1 "
            "from the recurrence?",
            KGlobal::locale()->formatDate( date ) ),
      i18n( "KOrganizer Confirmation" ),
      KGuiItem( i18n( "&Dissociate" ) ),
      KStandardGuiItem::cancel() );

    doOnlyThis = ( answer == KMessageBox::Yes );
  } else {
    answer = KMessageBox::questionYesNoCancel(
      this,
      i18n( "Do you want to dissociate "
            "the occurrence on %1 "
            "from the recurrence or also "
            "dissociate future ones?",
            KGlobal::locale()->formatDate( date ) ),
      i18n( "KOrganizer Confirmation" ),
      KGuiItem( i18n( "&Only Dissociate This One" ) ),
      KGuiItem( i18n( "&Also Dissociate Future Ones" ) ) );

    doOnlyThis = ( answer == KMessageBox::Yes );
    doFuture   = ( answer == KMessageBox::No );
  }

  if ( doOnlyThis ) {
    dissociateOccurrence( item, date );
  } else if ( doFuture ) {
    dissociateFutureOccurrence( item, date );
  }
}
void CalendarView::dissociateOccurrence( const Akonadi::Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( item );

  startMultiModify( i18n( "Dissociate occurrence" ) );
  Incidence::Ptr oldincidence( incidence->clone() );
  Incidence::Ptr newInc(
    mCalendar->dissociateOccurrence(
      item, date, CalendarSupport::KCalPrefs::instance()->timeSpec(), true ) );
  if ( newInc ) {
    mChanger->changeIncidence(
      oldincidence, item, CalendarSupport::IncidenceChanger::NOTHING_MODIFIED, this );
    mChanger->addIncidence( newInc, item.parentCollection(), this );
  } else {
    KMessageBox::sorry(
      this,
      i18n( "Dissociating the occurrence failed." ),
      i18n( "Dissociating Failed" ) );
  }

  endMultiModify();
}

void CalendarView::dissociateFutureOccurrence( const Akonadi::Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( item );

  startMultiModify( i18n( "Dissociate future occurrences" ) );
  Incidence::Ptr oldincidence( incidence->clone() );

  Incidence::Ptr newInc(
    mCalendar->dissociateOccurrence( item, date,
                                     CalendarSupport::KCalPrefs::instance()->timeSpec(), false ) );
  if ( newInc ) {
    mChanger->changeIncidence( oldincidence,
                               item,
                               CalendarSupport::IncidenceChanger::NOTHING_MODIFIED,
                               this );

    mChanger->addIncidence( newInc, item.parentCollection(), this );
  } else {
    KMessageBox::sorry(
      this,
      i18n( "Dissociating the future occurrences failed." ),
      i18n( "Dissociating Failed" ) );
  }
  endMultiModify();
}

void CalendarView::schedule_publish( const Akonadi::Item &item )
{
  Akonadi::Item selectedItem = item;
  if ( !item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    selectedItem = selectedIncidence();
  }

  CalendarSupport::publishItemInformation( selectedItem, mCalendar, this );
}

void CalendarView::schedule_request( const Akonadi::Item &incidence )
{
  schedule( iTIPRequest, incidence );
}

void CalendarView::schedule_refresh( const Akonadi::Item &incidence )
{
  schedule( iTIPRefresh, incidence );
}

void CalendarView::schedule_cancel( const Akonadi::Item &incidence )
{
  schedule( iTIPCancel, incidence );
}

void CalendarView::schedule_add( const Akonadi::Item &incidence )
{
  schedule( iTIPAdd, incidence );
}

void CalendarView::schedule_reply( const Akonadi::Item &incidence )
{
  schedule( iTIPReply, incidence );
}

void CalendarView::schedule_counter( const Akonadi::Item &incidence )
{
  schedule( iTIPCounter, incidence );
}

void CalendarView::schedule_declinecounter( const Akonadi::Item &incidence )
{
  schedule( iTIPDeclineCounter, incidence );
}

void CalendarView::schedule_forward( const Akonadi::Item &item )
{
  Akonadi::Item selectedItem = item;
  if ( !item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    selectedItem = selectedIncidence();
  }

  CalendarSupport::sendAsICalendar( selectedItem, KOCore::self()->identityManager(), this );
}

void CalendarView::mailFreeBusy( int daysToPublish )
{
  CalendarSupport::FreeBusyManager::self()->mailFreeBusy( daysToPublish, this );
}

void CalendarView::uploadFreeBusy()
{
  CalendarSupport::FreeBusyManager::self()->publishFreeBusy( this );
}

void CalendarView::schedule( KCalCore::iTIPMethod method, const Akonadi::Item &item )
{
  Akonadi::Item selectedItem = item;
  if ( !item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    selectedItem = selectedIncidence();
  }

  CalendarSupport::scheduleiTIPMethods( method, selectedItem, mCalendar, this );
}

void CalendarView::openAddressbook()
{
  KRun::runCommand( "kaddressbook", topLevelWidget() );
}

bool CalendarView::isReadOnly() const
{
  return mReadOnly;
}

void CalendarView::setReadOnly( bool readOnly )
{
  if ( mReadOnly != readOnly ) {
    mReadOnly = readOnly;
    emit readOnlyChanged( mReadOnly );
  }
}

void CalendarView::print()
{
  createPrinter();

  KOrg::BaseView *currentView = mViewManager->currentView();

  CalPrinter::PrintType printType = CalPrinter::Month;

  Incidence::List selectedIncidences;
  if ( currentView ) {
    printType = currentView->printType();
    Akonadi::Item::List selectedViewIncidences = currentView->selectedIncidences();
    foreach ( const Akonadi::Item &item, selectedViewIncidences ) {
      if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
        selectedIncidences.append( item.payload<KCalCore::Incidence::Ptr>() );
      }
    }
  }

  DateList tmpDateList = mDateNavigator->selectedDates();
  mCalPrinter->print( printType, tmpDateList.first(), tmpDateList.last(), selectedIncidences );
}

void CalendarView::printPreview()
{
  createPrinter();

  KOrg::BaseView *currentView = mViewManager->currentView();

  CalPrinter::PrintType printType = CalPrinter::Month;

  Incidence::List selectedIncidences;
  if ( currentView ) {
    printType = currentView->printType();
    Akonadi::Item::List selectedViewIncidences = currentView->selectedIncidences();
    foreach ( const Akonadi::Item &item, selectedViewIncidences ) {
      if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
        selectedIncidences.append( item.payload<KCalCore::Incidence::Ptr>() );
      }
    }
  }

  DateList tmpDateList = mDateNavigator->selectedDates();
  mCalPrinter->print( printType, tmpDateList.first(), tmpDateList.last(),
                      selectedIncidences, true );
}

void CalendarView::exportWeb()
{
  // FIXME: Get rid of the settings object. When can I delete it???
  KOrg::HTMLExportSettings *settings = new KOrg::HTMLExportSettings( "KOrganizer" );
  // Manually read in the config, because parameterized kconfigxt objects don't
  // seem to load the config theirselves
  if ( settings ) {
    settings->readConfig();
  }
  ExportWebDialog *dlg = new ExportWebDialog( settings, this );
  connect( dlg, SIGNAL(exportHTML(KOrg::HTMLExportSettings*)),
           this, SIGNAL(exportHTML(KOrg::HTMLExportSettings*)) );
  dlg->show();
}

void CalendarView::exportICalendar()
{
  QString filename =
    KFileDialog::getSaveFileName( KUrl( "icalout.ics" ), i18n( "*.ics|iCalendars" ), this );
  if ( !filename.isEmpty() ) {
    // Force correct extension
    if ( filename.right( 4 ) != ".ics" ) {
      filename += ".ics";
    }

    if ( QFile( filename ).exists() ) {
      if ( KMessageBox::No == KMessageBox::warningYesNo(
             this,
             i18n( "Do you want to overwrite %1?", filename ) ) ) {
        return;
      }
    }
    KCalCore::ICalFormat *format = new KCalCore::ICalFormat;

    CalendarSupport::CalendarAdaptor::Ptr calendar(
      new CalendarSupport::CalendarAdaptor( mCalendar, this ) );

    KCalCore::FileStorage storage( calendar, filename, format );
    if ( !storage.save() ) {
      QString errmess;
      if ( format->exception() ) {
        errmess = KCalUtils::Stringify::errorMessage( *format->exception() );
      } else {
        errmess = i18nc( "save failure cause unknown", "Reason unknown" );
      }
      KMessageBox::error(
        this,
        i18nc( "@info",
               "Cannot write iCalendar file %1. %2",
               filename, errmess ) );
    }
  }
}

void CalendarView::exportVCalendar()
{
  if ( !mCalendar->journals().isEmpty() ) {
    int result = KMessageBox::warningContinueCancel(
      this,
      i18n( "The journal entries cannot be exported to a vCalendar file." ),
      i18n( "Data Loss Warning" ),
      KGuiItem( i18n( "Proceed" ) ),
      KStandardGuiItem::cancel(),
      QString( "dontaskVCalExport" ),
      KMessageBox::Notify );
    if ( result != KMessageBox::Continue ) {
      return;
    }
  }

  QString filename = KFileDialog::getSaveFileName( KUrl( "vcalout.vcs" ),
                                                   i18n( "*.vcs|vCalendars" ), this );
  if ( !filename.isEmpty() ) {
    // Force correct extension
    if ( filename.right( 4 ) != ".vcs" ) {
      filename += ".vcs";
    }
    if ( QFile( filename ).exists() ) {
      if ( KMessageBox::No == KMessageBox::warningYesNo(
             this,
             i18n( "Do you want to overwrite %1?", filename ) ) ) {
        return;
      }
    }
    KCalCore::VCalFormat *format = new KCalCore::VCalFormat;

    CalendarSupport::CalendarAdaptor::Ptr calendar(
      new CalendarSupport::CalendarAdaptor( mCalendar, this ) );

    KCalCore::FileStorage storage( calendar, filename, format );
    if ( !storage.save() ) {
      QString errmess;
      if ( format->exception() ) {
        errmess = KCalUtils::Stringify::errorMessage( *format->exception() );
      } else {
        errmess = i18nc( "save failure cause unknown", "Reason unknown" );
      }
      KMessageBox::error(
        this,
        i18nc( "@info",
               "Cannot write vCalendar file %1. %2",
               filename, errmess ) );
    }
  }
}

void CalendarView::eventUpdated( const Akonadi::Item & )
{
  // Don't call updateView here. The code, which has caused the update of the
  // event is responsible for updating the view.
//  updateView();
}

void CalendarView::adaptNavigationUnits()
{
  if ( mViewManager->currentView()->isEventView() ) {
    int days = mViewManager->currentView()->currentDateCount();
    if ( days == 1 ) {
      emit changeNavStringPrev( i18n( "&Previous Day" ) );
      emit changeNavStringNext( i18n( "&Next Day" ) );
    } else {
      emit changeNavStringPrev( i18n( "&Previous Week" ) );
      emit changeNavStringNext( i18n( "&Next Week" ) );
    }
  }
}

void CalendarView::processMainViewSelection( const Akonadi::Item &item, const QDate &date )
{
  if ( CalendarSupport::hasIncidence( item ) ) {
    mTodoList->clearSelection();
  }
  processIncidenceSelection( item, date );
}

void CalendarView::processTodoListSelection( const Akonadi::Item &item, const QDate &date )
{
  if ( CalendarSupport::hasIncidence( item ) && mViewManager->currentView() ) {
    mViewManager->currentView()->clearSelection();
  }
  processIncidenceSelection( item, date );
}

void CalendarView::processIncidenceSelection( const Akonadi::Item &item, const QDate &date )
{
  Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( item != mSelectedIncidence ) {
    // This signal also must be emitted if incidence is 0
    emit incidenceSelected( item, date );
  }

  if ( !incidence ) {
   mSelectedIncidence = item;
   return;
  }
  if ( item == mSelectedIncidence ) {
    if ( !incidence->recurs() || mSaveDate == date ) {
      return;
    }
  }

  mSelectedIncidence = item;
  mSaveDate = date;

  bool organizerEvents = false;
  bool groupEvents = false;
  bool todo = false;
  bool subtodo = false;

  organizerEvents =
    CalendarSupport::KCalPrefs::instance()->thatIsMe( incidence->organizer()->email() );
  groupEvents = incidence->attendeeByMails( CalendarSupport::KCalPrefs::instance()->allEmails() );

  if ( incidence->type() == Incidence::TypeTodo ) {
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
  if ( KCalUtils::ICalDrag::canDecode( QApplication::clipboard()->mimeData() ) ) {
    emit pasteEnabled( true );
  } else {
    emit pasteEnabled( false );
  }
}

void CalendarView::showDates( const DateList &selectedDates, const QDate &preferredMonth )
{
  mDateNavigatorContainer->selectDates( selectedDates, preferredMonth );
  mNavigatorBar->selectDates( selectedDates );

  if ( mViewManager->currentView() ) {
    updateView( selectedDates.first(), selectedDates.last(), preferredMonth, false );
  } else {
    mViewManager->showAgendaView();
  }
}

void CalendarView::editFilters()
{
  mDialogManager->showFilterEditDialog( &mFilters );
}

void CalendarView::updateFilter()
{
  QStringList filters;

  int pos = mFilters.indexOf( mCurrentFilter );
  if ( pos < 0 ) {
    mCurrentFilter = 0;
  }

  filters << i18n( "No filter" );
  foreach ( KCalCore::CalFilter *filter, mFilters ) {
    if ( filter ) {
      filters << filter->name();
    }
  }

  // account for the additional "No filter" at the beginning! if the
  // filter is not in the list, pos == -1...
  emit filtersUpdated( filters, pos + 1 );

  mCalendar->setFilter( mCurrentFilter );
}

void CalendarView::filterActivated( int filterNo )
{
  KCalCore::CalFilter *newFilter = 0;
  if ( filterNo > 0 && filterNo <= int( mFilters.count() ) ) {
    newFilter = mFilters.at( filterNo - 1 );
  }
  if ( newFilter != mCurrentFilter ) {
    mCurrentFilter = newFilter;
    mCalendar->setFilter( mCurrentFilter );
    mViewManager->addChange( EventViews::EventView::FilterChanged );
    updateView();
  }
  emit filterChanged();
}

bool CalendarView::isFiltered() const
{
  return mCurrentFilter != 0;
}

QString CalendarView::currentFilterName() const
{
  if ( mCurrentFilter ) {
    return mCurrentFilter->name();
  } else {
    return i18n( "No filter" );
  }
}

void CalendarView::takeOverEvent()
{
  const Akonadi::Item item = currentSelection();
  Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( incidence ) {
    return;
  }

  incidence->setOrganizer(
    Person::Ptr( new Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                             CalendarSupport::KCalPrefs::instance()->email() ) ) );
  incidence->recreate();
  incidence->setReadOnly( false );

  //PENDING(AKONADI_PORT) call mChanger?

  updateView();
}

void CalendarView::takeOverCalendar()
{
  const Akonadi::Item::List items = mCalendar->rawIncidences();

  Q_FOREACH ( const Akonadi::Item &item, items ) {
    Incidence::Ptr i = CalendarSupport::incidence( item );
    i->setOrganizer( Person::Ptr( new Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                                              CalendarSupport::KCalPrefs::instance()->email() ) ) );
    i->recreate();
    i->setReadOnly( false );
  }

  //PENDING(AKONADI_PORT) call mChanger?

  updateView();
}

void CalendarView::showIntro()
{
  kDebug() << "To be implemented.";
}

void CalendarView::showDateNavigator( bool show )
{
  if ( show ) {
    mDateNavigatorContainer->show();
    mDateNavigatorContainer->updateView();
  } else {
    mDateNavigatorContainer->hide();
  }
}

void CalendarView::showTodoView( bool show )
{
  if ( show ) {
    mTodoList->show();
    mTodoList->updateView();
  } else {
    mTodoList->hide();
  }
}

void CalendarView::showEventViewer( bool show )
{
  if ( show ) {
    mEventViewerBox->show();
  } else {
    mEventViewerBox->hide();
  }
}

void CalendarView::addView( KOrg::BaseView *view )
{
  mViewManager->addView( view );
}

void CalendarView::showView( KOrg::BaseView *view )
{
  mViewManager->showView( view );
}

void CalendarView::addExtension( CalendarViewExtension::Factory *factory )
{
  CalendarViewExtension *extension = factory->create( mLeftSplitter );

  mExtensions.append( extension );
}

void CalendarView::showLeftFrame( bool show )
{
  if ( show ) {
    mMainSplitterSizes.clear();
    mLeftFrame->show();
    emit calendarViewExpanded( false );
  } else {
    // mPanner splitter sizes are useless if mLeftFrame is hidden, so remember them beforehand.
    if ( mMainSplitterSizes.isEmpty() ) {
      mMainSplitterSizes = mPanner->sizes();
    }

    mLeftFrame->hide();
    emit calendarViewExpanded( true );
  }
}

Akonadi::Item CalendarView::selectedTodo()
{
  const Akonadi::Item item = currentSelection();
  if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo( item ) ) {
    return item;
  }

  Akonadi::Item incidence;

  const Akonadi::Item::List selectedIncidences = mTodoList->selectedIncidences();
  if ( !selectedIncidences.isEmpty() ) {
    incidence = selectedIncidences.first();
  }
  if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo( item ) ) {
    return item;
  }
  return Akonadi::Item();
}

void CalendarView::dialogClosing( const Akonadi::Item & )
{
}

Akonadi::Item CalendarView::currentSelection()
{
  return mViewManager->currentSelection();
}

Akonadi::Item CalendarView::selectedIncidence()
{
  Akonadi::Item item = currentSelection();
  if ( !item.isValid() ) {
    Akonadi::Item::List selectedIncidences = mTodoList->selectedIncidences();
    if ( !selectedIncidences.isEmpty() ) {
      item = selectedIncidences.first();
    }
  }
  return item;
}

void CalendarView::showIncidence()
{
  showIncidence( selectedIncidence() );
}

void CalendarView::editIncidence()
{
  editIncidence( selectedIncidence() );
}

bool CalendarView::editIncidence( Akonadi::Item::Id id )
{
  Akonadi::Item item = mCalendar->incidence( id );
  return editIncidence( item );
}

bool CalendarView::showIncidence( Akonadi::Item::Id id )
{
  Akonadi::Item item = mCalendar->incidence( id );
  if ( !CalendarSupport::hasIncidence( item ) ) {
    return false;
  }
  showIncidence( item );
  return true;
}

bool CalendarView::showIncidenceContext( Akonadi::Item::Id id )
{
  Akonadi::Item item = mCalendar->incidence( id );
  if ( !CalendarSupport::hasIncidence( item ) ) {
    return false;
  }
  showIncidenceContext( item );
  return true;
}

void CalendarView::deleteIncidence()
{
  deleteIncidence( selectedIncidence() );
}

void CalendarView::cutIncidence( const Akonadi::Item &incidence )
{
  Q_UNUSED( incidence );
  edit_cut();
}

void CalendarView::copyIncidence( const Akonadi::Item &incidence )
{
  Q_UNUSED( incidence );
  edit_copy();
}

void CalendarView::pasteIncidence()
{
  edit_paste();
}

void CalendarView::showIncidence( const Akonadi::Item &item )
{
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog( mCalendar, this );
  eventViewer->setIncidence( item, QDate() );
  // Disable the Edit button for read-only Incidences.
  if ( !mCalendar->hasChangeRights( item ) ) {
    eventViewer->enableButton( KDialog::User1, false );
  }

  eventViewer->show();
}

void CalendarView::showIncidenceContext( const Akonadi::Item &item )
{
  Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( CalendarSupport::hasEvent( item ) ) {
    if ( !viewManager()->currentView()->inherits( "KOEventView" ) ) {
      viewManager()->showAgendaView();
    }
    // just select the appropriate date
    mDateNavigator->selectWeek(
      incidence->dtStart().toTimeSpec(
        CalendarSupport::KCalPrefs::instance()->timeSpec() ).date() );
    return;
  } else if ( CalendarSupport::hasJournal( item ) ) {
    if ( !viewManager()->currentView()->inherits( "KOJournalView" ) ) {
      viewManager()->showJournalView();
    }
  } else if ( CalendarSupport::hasTodo( item ) ) {
    if ( !viewManager()->currentView()->inherits( "KOTodoView" ) ) {
      viewManager()->showTodoView();
    }
  }
  Akonadi::Item::List list;
  list.append( item );
  viewManager()->currentView()->showIncidences( list, QDate() );
}

bool CalendarView::editIncidence( const Akonadi::Item &item, bool isCounter )
{
  Q_UNUSED( isCounter );
  Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence ) {
    kDebug() << "Empty Incidence";
    KNotification::beep();
    return false;
  }

  if ( !mChanger ) {
    kDebug() << "Empty Changer";
    KNotification::beep();
    return false;
  }

  /*
    //TODO_NG
  IncidenceEditor *tmp = editorDialog( item );
  if ( tmp ) {
    tmp->reload();
    tmp->raise();
    tmp->show();
    return true;
  }
  */

  if ( !mCalendar->hasChangeRights( item ) ) {
    showIncidence( item );
    return true;
  }

  IncidenceEditorNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );

  // connectIncidenceEditor( dialog );         // TODO: This as well
  dialog->load( item, activeIncidenceDate() ); // Show the dialog as soon as it loads the item.

  return true;
}

void CalendarView::deleteSubTodosIncidence ( const Akonadi::Item &todoItem )
{
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( !todo ) {
    return;
  }
  Akonadi::Item::List subTodos = mCalendar->findChildren( todoItem );
  foreach ( const Akonadi::Item &item, subTodos ) {
    if ( CalendarSupport::hasTodo( item ) ) {
      deleteSubTodosIncidence ( item );
    }
  }

  if ( mChanger->isNotDeleted( todoItem.id() ) ) {
    mChanger->deleteIncidence ( todoItem, 0, this );
  }
}

void CalendarView::deleteTodoIncidence ( const Akonadi::Item &todoItem, bool force )
{
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( !todo ) {
    return ;
  }

  // it a simple todo, ask and delete it.
  if ( mCalendar->findChildren( todoItem ).isEmpty() ) {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( todoItem ) == KMessageBox::Continue );
    }
    if ( doDelete && mChanger->isNotDeleted( todoItem.id() ) ) {
      mChanger->deleteIncidence( todoItem, 0, this );
    }
    return;
  }

  /* Ok, this to-do has sub-to-dos, ask what to do */
  int km = KMessageBox::No;
  if ( !force ) {
    km = KMessageBox::questionYesNoCancel(
      this,
      i18n( "The item \"%1\" has sub-to-dos. "
            "Do you want to delete just this item and "
            "make all its sub-to-dos independent, or "
            "delete the to-do with all its sub-to-dos?",
            todo->summary() ),
      i18n( "KOrganizer Confirmation" ),
      KGuiItem( i18n( "Delete Only This" ) ),
      KGuiItem( i18n( "Delete All" ) ) );
  }
  startMultiModify( i18n( "Deleting sub-to-dos" ) );
  // Delete only the father
  if ( km == KMessageBox::Yes ) {
    makeChildrenIndependent( todoItem );
    if ( mChanger->isNotDeleted( todoItem.id() ) ) {
      mChanger->deleteIncidence( todoItem, 0, this );
    }
  } else if ( km == KMessageBox::No ) {
    // Delete all
    // we have to hide the delete confirmation for each itemDate
    deleteSubTodosIncidence ( todoItem );
  }
  endMultiModify();
}

bool CalendarView::deleteIncidence( const Akonadi::Item &item, bool force )
{
  Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence || !mChanger ) {
    if ( !force ) {
      KNotification::beep();
    }
    return false;
  }

  if ( !mChanger->isNotDeleted( item.id() ) ) {
    // it was deleted already but the etm wasn't notified yet
    return true;
  }

  if ( !mCalendar->hasDeleteRights( item ) ) {
    if ( !force ) {
      KMessageBox::information(
        this,
        i18n( "The item \"%1\" is marked read-only "
              "and cannot be deleted; it probably "
              "belongs to a read-only calendar.",
              incidence->summary() ),
        i18n( "Removing not possible" ),
        "deleteReadOnlyIncidence" );
    }
    return false;
  }

  CanDeleteIncidenceVisitor v( item );

  // Let the visitor do special things for special incidence types.
  // e.g. todos with children cannot be deleted, so act(..) returns false
  IncidenceBase::Ptr ib = incidence.staticCast<IncidenceBase>();
  if ( !v.act( ib, this ) ) {
    return false;
  }
  //If it is a todo, there are specific delete function

  if ( incidence && incidence->type() == Incidence::TypeTodo ) {
    deleteTodoIncidence( item, force );
    return true;
  }

  if ( incidence->recurs() ) {
    QDate itemDate = mViewManager->currentSelectionDate();
    int km = KMessageBox::Ok;
    if ( !force ) {
      if ( !itemDate.isValid() ) {
        kDebug() << "Date Not Valid";
        km = KMessageBox::warningContinueCancel(
          this,
          i18n( "The calendar item \"%1\" recurs over multiple dates; "
                "are you sure you want to delete it "
                "and all its recurrences?", incidence->summary() ),
          i18n( "KOrganizer Confirmation" ),
          KGuiItem( i18n( "Delete All" ) ) );
      } else {
        KDateTime itemDateTime( itemDate, CalendarSupport::KCalPrefs::instance()->timeSpec() );
        bool isFirst = !incidence->recurrence()->getPreviousDateTime( itemDateTime ).isValid();
        bool isLast  = !incidence->recurrence()->getNextDateTime( itemDateTime ).isValid();

        QString message;
        KGuiItem itemFuture( i18n( "Also Delete &Future" ) );

        if ( !isFirst && !isLast ) {
          itemFuture.setEnabled( true );
          message = i18n( "The calendar item \"%1\" recurs over multiple dates. "
                          "Do you want to delete only the current one on %2, also "
                          "future occurrences, or all its occurrences?",
                          incidence->summary(),
                          KGlobal::locale()->formatDate( itemDate ) );
        } else {
          itemFuture.setEnabled( false );
          message = i18n( "The calendar item \"%1\" recurs over multiple dates. "
                          "Do you want to delete only the current one on %2 "
                          "or all its occurrences?",
                          incidence->summary(),
                          KGlobal::locale()->formatDate( itemDate ) );
        }

        if ( !( isFirst && isLast ) ) {
          km = PIMMessageBox::fourBtnMsgBox(
            this,
            QMessageBox::Warning,
            message,
            i18n( "KOrganizer Confirmation" ),
            KGuiItem ( i18n( "Delete C&urrent" ) ),
            itemFuture,
            KGuiItem( i18n( "Delete &All" ) ) );
        } else {
          km = msgItemDelete( item );
        }
      }
    }
    Incidence::Ptr oldIncidence( incidence->clone() );
    switch( km ) {
    case KMessageBox::Ok: // Continue // all
    case KMessageBox::Continue:
      mChanger->deleteIncidence( item, 0, this );
      break;

    case KMessageBox::Yes: // just this one
      incidence->recurrence()->addExDate( itemDate );
      mChanger->changeIncidence(
        oldIncidence, item,
        CalendarSupport::IncidenceChanger::RECURRENCE_MODIFIED_ONE_ONLY, this );

      break;
    case KMessageBox::No: // all future items
      Recurrence *recur = incidence->recurrence();
      recur->setEndDate( itemDate.addDays( -1 ) );
      mChanger->changeIncidence(
        oldIncidence, item,
        CalendarSupport::IncidenceChanger::RECURRENCE_MODIFIED_ALL_FUTURE, this );
      break;
    }
  } else {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( item ) == KMessageBox::Continue );
    }
    if ( doDelete ) {
      mChanger->deleteIncidence( item, 0, this );
      processIncidenceSelection( Akonadi::Item(), QDate() );
    }
  }
  return true;
}

bool CalendarView::purgeCompletedSubTodos( const Akonadi::Item &todoItem, bool &allPurged )
{
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( !todo ) {
    return true;
  }

  bool deleteThisTodo = true;
  Akonadi::Item::List subTodos = mCalendar->findChildren( todoItem );
  foreach ( const Akonadi::Item &item, subTodos ) {
    if ( CalendarSupport::hasTodo( item ) ) {
      deleteThisTodo &= purgeCompletedSubTodos( item, allPurged );
    }
  }

  if ( deleteThisTodo ) {
    if ( todo->isCompleted() ) {
      if ( !mChanger->deleteIncidence( todoItem, 0, this ) ) {
        allPurged = false;
      }
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
    i18n( "Delete all completed to-dos?" ),
    i18n( "Purge To-dos" ),
    KGuiItem( i18n( "Purge" ) ) );

  if ( result == KMessageBox::Continue ) {
    bool allDeleted = true;
    startMultiModify( i18n( "Purging completed to-dos" ) );
    Akonadi::Item::List todos = calendar()->rawTodos();
    Akonadi::Item::List rootTodos;
    Akonadi::Item::List::ConstIterator it;
    for ( it = todos.constBegin(); it != todos.constEnd(); ++it ) {
      KCalCore::Todo::Ptr aTodo = CalendarSupport::todo( *it );
      if ( aTodo && aTodo->relatedTo().isEmpty() ) { // top level todo //REVIEW(AKONADI_PORT)
        rootTodos.append( *it );
      }
    }
    // now that we have a list of all root todos, check them and their children
    for ( it = rootTodos.constBegin(); it != rootTodos.constEnd(); ++it ) {
      purgeCompletedSubTodos( *it, allDeleted );
    }

    endMultiModify();
    if ( !allDeleted ) {
      KMessageBox::information(
        this,
        i18nc( "@info",
               "Unable to purge to-dos with uncompleted children." ),
        i18n( "Delete To-do" ),
        "UncompletedChildrenPurgeTodos" );
    }
  }
}

void CalendarView::warningChangeFailed( const Akonadi::Item &item )
{
  Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( incidence ) {
    KMessageBox::sorry(
      this,
      i18nc( "@info",
             "Unable to edit \"%1\" because it is locked by another process.",
             incidence->summary() ) );
  }
}

void CalendarView::showErrorMessage( const QString &msg )
{
  KMessageBox::error( this, msg );
}

void CalendarView::updateCategories()
{
  QStringList allCats( CalendarSupport::Calendar::categories( calendar() ) );
  allCats.sort();

  CalendarSupport::CategoryConfig cc( KOPrefs::instance() );

  QStringList categories( cc.customCategories() );
  for ( QStringList::ConstIterator si = allCats.constBegin(); si != allCats.constEnd(); ++si ) {
    if ( !categories.contains( *si ) ) {
      categories.append( *si );
    }
  }
  cc.setCustomCategories( categories );
  cc.writeConfig();
  // Make the category editor update the list!
  emit categoriesChanged();
}

void CalendarView::addIncidenceOn( const Akonadi::Item &itemadd, const QDate &dt )
{
  if ( !CalendarSupport::hasIncidence( itemadd ) || !mChanger ) {
    KMessageBox::sorry(
      this,
      i18n( "Unable to copy the item to %1.", dt.toString() ),
      i18n( "Copying Failed" ) );
    return;
  }
  Akonadi::Item item = mCalendar->incidence( itemadd.id() );
  if ( !item.isValid() ) {
    item = itemadd;
  }
  // Create a copy of the incidence, since the incadd doesn't belong to us.
  Incidence::Ptr incidence( CalendarSupport::incidence( item )->clone() );
  incidence->recreate();

  if ( const KCalCore::Event::Ptr event = incidence.dynamicCast<KCalCore::Event>() ) {

    // Adjust date
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } else if ( const KCalCore::Todo::Ptr todo = incidence.dynamicCast<KCalCore::Todo>() ) {
    KDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }

  Akonadi::Collection selectedCollection;
  int dialogCode = 0;
  if ( !mChanger->addIncidence( incidence, this, selectedCollection, dialogCode ) ) {
    if ( dialogCode != QDialog::Rejected ) {
      KOHelper::showSaveIncidenceErrorMsg( this, incidence );
    }
  }
}

void CalendarView::moveIncidenceTo( const Akonadi::Item &itemmove, const QDate &dt )
{
  if ( !CalendarSupport::hasIncidence( itemmove ) || !mChanger ) {
    KMessageBox::sorry(
      this,
      i18n( "Unable to move the item to  %1.", dt.toString() ),
      i18n( "Moving Failed" ) );
    return;
  }
  Akonadi::Item item = mCalendar->incidence( itemmove.id() );
  if ( !item.isValid() ) {
    addIncidenceOn( itemmove, dt );
    return;
  }
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( itemmove );

  KCalCore::Incidence::Ptr oldIncidence( incidence->clone() );

  if ( const KCalCore::Event::Ptr event = incidence.dynamicCast<KCalCore::Event>() ) {
    // Adjust date
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } if ( const KCalCore::Todo::Ptr todo = incidence.dynamicCast<KCalCore::Todo>() ) {
    KDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }
  mChanger->changeIncidence( oldIncidence, itemmove,
                             CalendarSupport::IncidenceChanger::DATE_MODIFIED, this );
}

void CalendarView::resourcesChanged()
{
  mViewManager->addChange( EventViews::EventView::ResourcesChanged );
  mDateNavigatorContainer->setUpdateNeeded();
  updateView();
}

bool CalendarView::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mLeftFrame && event->type() == QEvent::Show ) {
    mSplitterSizesValid = true;
  }
  return KOrg::CalendarViewBase::eventFilter( watched, event );
}

void CalendarView::updateHighlightModes() {

  KOrg::BaseView *view = mViewManager->currentView();
  if ( view ) {
    bool hiEvents;
    bool hiTodos;
    bool hiJournals;

    view->getHighlightMode( hiEvents, hiTodos, hiJournals );
    mDateNavigatorContainer->setHighlightMode( hiEvents,
                                               hiTodos,
                                               hiJournals );
  }
}

void CalendarView::selectWeek( const QDate &date, const QDate &preferredMonth )
{
  if ( KOPrefs::instance()->mWeekNumbersShowWork  &&
       mViewManager->rangeMode() == KOViewManager::WORK_WEEK_RANGE ) {
    mDateNavigator->selectWorkWeek( date );
  } else {
    mDateNavigator->selectWeek( date, preferredMonth );
  }
}

void CalendarView::changeFullView( bool fullView )
{
  if ( mViewManager->currentView() ) {
    if ( mViewManager->currentView()->identifier() == "DefaultTodoView" ) {
      showLeftFrame( !fullView );
    } else if ( mViewManager->currentView()->identifier() == "DefaultMonthView" ) {
      showLeftFrame( !fullView );
      fullView ? mNavigatorBar->show() : mNavigatorBar->hide();
    }
  }
}

void CalendarView::getIncidenceHierarchy( const Akonadi::Item &item,
                                          Akonadi::Item::List &children )
{
  // protecion against looping hierarchies
  if ( item.isValid() && !children.contains( item ) ) {
    Akonadi::Item::List::ConstIterator it;
    Akonadi::Item::List immediateChildren = mCalendar->findChildren( item );

    for ( it = immediateChildren.constBegin();
          it != immediateChildren.constEnd(); ++it ) {
      getIncidenceHierarchy( *it, children );
    }
    children.append( item );
  }
}

Akonadi::Collection CalendarView::defaultCollection( const QLatin1String &mimeType ) const
{
  bool supportsMimeType;

  /**
     If the view's collection is valid and it supports mimeType, return it, otherwise
     if the config's collection is valid and it supports mimeType, return it, otherwise
     return an invalid collection.
  */

  Akonadi::Collection viewCollection =
    mCalendar->collection( mViewManager->currentView()->collectionId() );

  supportsMimeType = viewCollection.contentMimeTypes().contains( mimeType ) || mimeType == "";

  if ( viewCollection.isValid() && supportsMimeType ) {
    return viewCollection;
  } else {
    Akonadi::Collection configCollection =
      mCalendar->collection( CalendarSupport::KCalPrefs::instance()->defaultCalendarId() );
    supportsMimeType = configCollection.contentMimeTypes().contains( mimeType ) || mimeType == "";

    if ( configCollection.isValid() && supportsMimeType ) {
      return configCollection;
    } else {
      if ( EventViews::EventView::globalCollectionSelection() &&
           !EventViews::EventView::globalCollectionSelection()->selectedCollections().isEmpty() ) {
        return EventViews::EventView::globalCollectionSelection()->selectedCollections().first();
      }
    }
  }

  return Akonadi::Collection();
}

IncidenceEditorNG::IncidenceDialog *CalendarView::createIncidenceEditor(
  const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  IncidenceEditorNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  Q_ASSERT( incidence );

  if ( collection.isValid() ) {
    dialog->selectCollection( collection );
  } else {
    dialog->selectCollection( defaultCollection( incidence->mimeType() ) );
  }

  return dialog;
}

#include "calendarview.moc"
