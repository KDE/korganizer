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
#include "categoryconfig.h"
#include "datechecker.h"
#include "datenavigator.h"
#include "datenavigatorcontainer.h"
#include "exportwebdialog.h"
#include "history.h"
#include "kocorehelper.h"
#include "kodialogmanager.h"
#include "koeventviewerdialog.h"
#include "koglobals.h"
#include "koviewmanager.h"
#include "navigatorbar.h"
#include "publishdialog.h"
#include "htmlexportsettings.h"
#include "views/agendaview/koagendaview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/todoview/kotodoview.h"
#include "collectiongeneralpage.h"
#include "kohelper.h"

#include <libkdepim/pimmessagebox.h>

#include <incidenceeditors/journaleditor.h> // TODO: Remove when we have a IncidenceDialog based JournalDialog.
#include <incidenceeditors/incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditors/incidenceeditor-ng/incidencedefaults.h>

#include <akonadi/control.h>
#include <akonadi/collectionpropertiesdialog.h>
#include <akonadi/kcal/utils.h>
#include <akonadi/kcal/calendaradaptor.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/groupware.h>
#include <akonadi/kcal/freebusymanager.h>
#include <akonadi/kcal/mailclient.h>
#include <akonadi/kcal/mailscheduler.h>
#include <akonadi/kcal/dndfactory.h>
#include <akonadi/kcal/incidencechanger.h>
#include <akonadi/kcal/incidenceviewer.h>
#include <akonadi/kcal/kcalprefs.h>

#include <kcalcore/filestorage.h>
#include <kcalcore/calendar.h>
#include <kcalcore/calfilter.h>
#include <kcalcore/freebusy.h>
#include <kcalcore/icalformat.h>
#include <kcalcore/vcalformat.h>

#include <kcalutils/stringify.h>
#include <kcalutils/icaldrag.h>
#include <kcalutils/scheduler.h>

#include <KHolidays/Holidays>

#include <KPIMIdentities/IdentityManager>

#include <mailtransport/transportmanager.h>

#include <KDialog>
#include <KFileDialog>
#include <KNotification>
#include <KRun>
#include <KVBox>

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

using namespace Akonadi;
using namespace KHolidays;
using namespace IncidenceEditors;
using namespace KCalUtils;

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionGeneralPageFactory, CollectionGeneralPage )

IncidenceEditorsNG::IncidenceDefaults minimalIncidenceDefaults()
{
  IncidenceEditorsNG::IncidenceDefaults defaults;
  // Set the full emails manually here, to avoid that we get dependencies on
  // KCalPrefs all over the place.
  defaults.setFullEmails( KCalPrefs::instance()->fullEmails() );
  // NOTE: At some point this should be generalized. That is, we now use the
  //       freebusy url as a hack, but this assumes that the user has only one
  //       groupware account. Which doesn't have to be the case necessarily.
  //       This method should somehow depend on the calendar selected to which
  //       the incidence is added.
  if ( KCalPrefs::instance()->useGroupwareCommunication() )
    defaults.setGroupWareDomain( KUrl( KCalPrefs::instance()->freeBusyRetrieveUrl() ).host() );
  return defaults;
}

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
  mCreatingEnabled = false;

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

  mTodoList = new KOTodoView( mLeftSplitter );
  mTodoList->setObjectName( "todolist" );

  mEventViewerBox = new KVBox( mLeftSplitter );
  mEventViewerBox->setMargin( KDialog::marginHint() );
  mEventViewer = new IncidenceViewer( mEventViewerBox );
  mEventViewer->setObjectName( "EventViewer" );

  KVBox *rightBox = new KVBox( mPanner );
  mNavigatorBar = new NavigatorBar( rightBox );
  mRightFrame = new QStackedWidget( rightBox );
  rightBox->setStretchFactor( mRightFrame, 1 );

  mLeftFrame = mLeftSplitter;
  mLeftFrame->installEventFilter( this );

  CollectionPropertiesDialog::useDefaultPage( false );
  CollectionPropertiesDialog::registerPage( new CollectionGeneralPageFactory() );

  // Signals emitted by mDateNavigator
  connect( mDateNavigator, SIGNAL(datesSelected(const KCalCore::DateList&,const QDate&)),
           SLOT(showDates(const KCalCore::DateList&,const QDate&)) );

  connect( mDateNavigatorContainer, SIGNAL(newEventSignal(const QDate &)),
           SLOT(newEvent(const QDate &)) );
  connect( mDateNavigatorContainer, SIGNAL(newTodoSignal(const QDate &)),
           SLOT(newTodo(const QDate &)) );
  connect( mDateNavigatorContainer, SIGNAL(newJournalSignal(const QDate &)),
           SLOT(newJournal(const QDate &)) );

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
  connect( mDateNavigatorContainer, SIGNAL(weekClicked(const QDate &)),
           SLOT(selectWeek(const QDate &)) );

  connect( mDateNavigatorContainer, SIGNAL(weekClicked(const QDate &) ),
           this, SLOT(selectWeek(const QDate &)) );
  connect( mDateNavigatorContainer, SIGNAL(prevMonthClicked(const QDate &, const QDate &, const QDate &) ),
           mDateNavigator, SLOT( selectPreviousMonth(const QDate &, const QDate &, const QDate &)) );
  connect( mDateNavigatorContainer, SIGNAL(nextMonthClicked(const QDate &, const QDate &, const QDate &) ),
           mDateNavigator, SLOT(selectNextMonth(const QDate &, const QDate &, const QDate &)) );
  connect( mDateNavigatorContainer, SIGNAL( prevYearClicked() ),
           mDateNavigator, SLOT( selectPreviousYear() ) );
  connect( mDateNavigatorContainer, SIGNAL( nextYearClicked() ),
           mDateNavigator, SLOT( selectNextYear() ) );
  connect( mDateNavigatorContainer, SIGNAL( monthSelected(int) ),
           mDateNavigator, SLOT( selectMonth(int) ) );
  connect( mDateNavigatorContainer, SIGNAL(yearSelected(int)),
           mDateNavigator, SLOT(selectYear(int)) );
  connect( mDateNavigatorContainer, SIGNAL(goPrevious()),
           mDateNavigator, SLOT(selectPrevious()) );
  connect( mDateNavigatorContainer, SIGNAL(goNext()),
           mDateNavigator, SLOT(selectNext()) );

  connect( mDateNavigatorContainer, SIGNAL(datesSelected(const KCalCore::DateList &)),
           mDateNavigator, SLOT(selectDates(const KCalCore::DateList &)) );

  connect( mDateNavigatorContainer, SIGNAL(incidenceDropped(Akonadi::Item,QDate)),
           SLOT(addIncidenceOn(Akonadi::Item,QDate)) );
  connect( mDateNavigatorContainer, SIGNAL(incidenceDroppedMove(Akonadi::Item,QDate)),
           SLOT(moveIncidenceTo(Akonadi::Item,QDate)) );

  connect( mDateChecker, SIGNAL(dayPassed(const QDate &)),
           mTodoList, SLOT(dayPassed(const QDate &)) );
  connect( mDateChecker, SIGNAL(dayPassed(const QDate &)),
           SIGNAL(dayPassed(const QDate &)) );
  connect( mDateChecker, SIGNAL(dayPassed(const QDate &)),
           mDateNavigatorContainer, SLOT(updateToday()) );

  connect( this, SIGNAL(configChanged()),
           mDateNavigatorContainer, SLOT(updateConfig()) );

  connect( this, SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)),
           mEventViewer, SLOT(setIncidence(const Akonadi::Item &, const QDate &)) );

  //TODO: do a pretty Summary,
  QString s;
  s = i18n( "<p><em>No Item Selected</em></p>"
            "<p>Select an event, to-do or journal entry to view its details "
            "here.</p>" );

  mEventViewer->setDefaultMessage( s );
  mEventViewer->setWhatsThis(
                   i18n( "View the details of events, journal entries or to-dos "
                         "selected in KOrganizer's main view here." ) );
  mEventViewer->setIncidence( Item(), QDate() );

  mViewManager->connectTodoView( mTodoList );
  mViewManager->connectView( mTodoList );

  KOGlobals::self()->
      setHolidays( new HolidayRegion( KOPrefs::instance()->mHolidays ) );

  connect( QApplication::clipboard(), SIGNAL(dataChanged()),
           SLOT(checkClipboard()) );

  connect( mTodoList, SIGNAL(incidenceSelected(const Akonadi::Item &,const QDate &)),
           this, SLOT(processTodoListSelection(const Akonadi::Item &,const QDate &)) );
  disconnect( mTodoList, SIGNAL(incidenceSelected(const Akonadi::Item &,const QDate &)),
              this, SLOT(processMainViewSelection(const Akonadi::Item &,const QDate &)) );
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

void CalendarView::setCalendar( Akonadi::Calendar *cal )
{
  mCalendar = cal;

  delete mHistory;
  mHistory = new History( mCalendar, this );
  connect( mHistory, SIGNAL(undone()), SLOT(updateView()) );
  connect( mHistory, SIGNAL(redone()), SLOT(updateView()) );

  setIncidenceChanger( new IncidenceChanger( mCalendar, this, KCalPrefs::instance()->defaultCalendarId() ) );

  mChanger->setDestinationPolicy( static_cast<IncidenceChanger::DestinationPolicy>( KOPrefs::instance()->destination() ) );

  mCalendar->registerObserver( this );

  mDateNavigatorContainer->setCalendar( mCalendar );

  mTodoList->setCalendar( mCalendar );
}

void CalendarView::setIncidenceChanger( IncidenceChanger *changer )
{
  delete mChanger;
  mChanger = changer;
  mChanger->setGroupware( Groupware::instance() );

  emit newIncidenceChanger( mChanger );
  connect( mChanger, SIGNAL(incidenceAddFinished(Akonadi::Item,bool)),
           this, SLOT(incidenceAddFinished(Akonadi::Item,bool)) );

  qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
  qRegisterMetaType<Akonadi::IncidenceChanger::WhatChanged>("Akonadi::IncidenceChanger::WhatChanged");
  connect( mChanger, SIGNAL(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,Akonadi::IncidenceChanger::WhatChanged,bool)),
           this, SLOT(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,Akonadi::IncidenceChanger::WhatChanged,bool)), Qt::QueuedConnection );
  connect( mChanger, SIGNAL(incidenceToBeDeleted(Akonadi::Item)),
           this, SLOT(incidenceToBeDeleted(Akonadi::Item)) );
  connect( mChanger, SIGNAL(incidenceDeleteFinished(Akonadi::Item,bool)),
           this, SLOT(incidenceDeleteFinished(Akonadi::Item,bool)) );

  connect( mChanger, SIGNAL(schedule(KCalCore::iTIPMethod,Akonadi::Item)),
           this, SLOT(schedule(KCalCore::iTIPMethod,Akonadi::Item)) );

  connect( this, SIGNAL(cancelAttendees(Akonadi::Item)),
           mChanger, SLOT(cancelAttendees(Akonadi::Item)) );
}

Akonadi::Calendar *CalendarView::calendar() const
{
  return mCalendar;
}

IncidenceEditor *CalendarView::editorDialog( const Item &item ) const
{
  return mDialogList.value( item.id() );
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

  CalendarAdaptor::Ptr adaptor( new CalendarAdaptor( mCalendar, this, true /*use default collection*/ ) );
  // merge in a file
  FileStorage storage( adaptor );
  storage.setFileName( filename );
  loadedSuccesfully = storage.load();

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

  CalendarAdaptor::Ptr adaptor( new CalendarAdaptor( mCalendar, this ) );
  FileStorage storage( adaptor );
  storage.setFileName( filename );
  storage.setSaveFormat( new ICalFormat );

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
  KCalPrefs::instance()->writeConfig();

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
    CalFilter *filter;
    filter = new CalFilter( *it );
    KConfigGroup filterConfig( config, "Filter_" + (*it) );
    filter->setCriteria( filterConfig.readEntry( "Criteria", 0 ) );
    filter->setCategoryList( filterConfig.readEntry( "CategoryList", QStringList() ) );
    if ( filter->criteria() & KCalCore::CalFilter::HideNoMatchingAttendeeTodos ) {
      filter->setEmailList( KCalPrefs::instance()->allEmails() );
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

  foreach ( CalFilter *filter, mFilters ) {
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
    mDateNavigator->selectNextMonth();
  } else {
    mDateNavigator->selectNext();
  }
}

void CalendarView::goPrevious()
{
  if ( dynamic_cast<MonthView*>( mViewManager->currentView() ) ) {
    mDateNavigator->selectPreviousMonth();
  } else {
    mDateNavigator->selectPrevious();
  }
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

  KOGlobals::self()->setHolidays( new HolidayRegion( KOPrefs::instance()->mHolidays ) );

  // Only set a new time zone if it changed. This prevents the window
  // from being modified on start
  KDateTime::Spec newTimeSpec = KCalPrefs::instance()->timeSpec();
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

  mChanger->setDestinationPolicy( static_cast<IncidenceChanger::DestinationPolicy>( KOPrefs::instance()->destination() ) );
}

void CalendarView::incidenceAddFinished( const Item &incidence, bool success )
{
  if ( success ) {
    history()->recordAdd( incidence );
    changeIncidenceDisplay( incidence, IncidenceChanger::INCIDENCEADDED );
    updateUnmanagedViews();
    checkForFilteredChange( incidence );
  } else {
    kError() << "Incidence not added, job reported error";
  }
}

void CalendarView::incidenceChangeFinished( const Item &oldIncidence_,
                                            const Item &newIncidence_,
                                            Akonadi::IncidenceChanger::WhatChanged modification,
                                            bool success )
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

  Incidence::Ptr oldIncidence = Akonadi::incidence( oldIncidence_ );
  Incidence::Ptr newIncidence = Akonadi::incidence( newIncidence_ );

  // FIXME: Make use of the what flag, which indicates which parts of the incidence have changed!
  IncidenceEditor *tmp = editorDialog( newIncidence_ );
  if ( tmp ) {
    kDebug() << "Incidence modified and open";
    tmp->modified();
  }

  history()->recordEdit( oldIncidence_, newIncidence_ );

  // Record completed todos in journals, if enabled. we should to this here in
  // favor of the todolist. users can mark a task as completed in an editor
  // as well.
  if ( newIncidence->type() == Incidence::TypeTodo &&
       KOPrefs::instance()->recordTodosInJournals() &&
       ( modification == IncidenceChanger::COMPLETION_MODIFIED ||
         modification == IncidenceChanger::COMPLETION_MODIFIED_WITH_RECURRENCE ) ) {
    Todo::Ptr todo = Akonadi::todo( newIncidence_ );
    if ( todo->isCompleted() ||
         modification == IncidenceChanger::COMPLETION_MODIFIED_WITH_RECURRENCE ) {
      QString timeStr = KGlobal::locale()->formatTime( QTime::currentTime() );
      QString description = i18n( "Todo completed: %1 (%2)", newIncidence->summary(), timeStr );

      Item::List journals = calendar()->journals( QDate::currentDate() );

      if ( journals.isEmpty() ) {
        Journal::Ptr journal( new Journal );
        journal->setDtStart( KDateTime::currentDateTime( KCalPrefs::instance()->timeSpec() ) );

        QString dateStr = KGlobal::locale()->formatDate( QDate::currentDate() );
        journal->setSummary( i18n( "Journal of %1", dateStr ) );
        journal->setDescription( description );

        Akonadi::Collection selectedCollection;

        if ( !mChanger->addIncidence( journal, newIncidence_.parentCollection(), this ) ) {
          kError() << "Unable to add Journal";
          return;
        }

      } else { // journal list is not empty
        Item journalItem = journals.first();
        Journal::Ptr journal = Akonadi::journal( journalItem );
        Journal::Ptr oldJournal( journal->clone() );
        journal->setDescription( journal->description().append( '\n' + description ) );
        mChanger->changeIncidence( oldJournal, journalItem,
                                   IncidenceChanger::DESCRIPTION_MODIFIED, this );

      }
    }
  }

  changeIncidenceDisplay( newIncidence_, IncidenceChanger::INCIDENCEEDITED );
  updateUnmanagedViews();
  checkForFilteredChange( newIncidence_ );
}

void CalendarView::incidenceToBeDeleted( const Item &item )
{
  IncidenceEditor *tmp = editorDialog( item );
  kDebug()<<"incidenceToBeDeleted item.id() :"<<item.id();
  if ( tmp ) {
    kDebug() << "Incidence to be deleted and open in editor";
    tmp->delayedDestruct();
  }
  history()->recordDelete( item );
//  changeIncidenceDisplay( incidence, IncidenceChanger::INCIDENCEDELETED );
  updateUnmanagedViews();
}

void CalendarView::incidenceDeleteFinished( const Item &item, bool success )
{
  if ( success ) {
    changeIncidenceDisplay( item, IncidenceChanger::INCIDENCEDELETED );
    updateUnmanagedViews();
  } else {
    kError() << "Incidence not deleted, job reported error";
  }
}

void CalendarView::checkForFilteredChange( const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
  CalFilter *filter = calendar()->filter();
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

void CalendarView::changeIncidenceDisplay( const Item &item, int action )
{
  mDateNavigatorContainer->updateView();
  mDialogManager->updateSearchDialog();

  if ( Akonadi::hasIncidence( item ) ) {
    // If there is an event view visible update the display
    mViewManager->currentView()->changeIncidenceDisplay( item, action );
    if ( mTodoList ) {
      mTodoList->changeIncidenceDisplay( item, action );
    }
  } else {
    mViewManager->currentView()->updateView();
    if ( mTodoList ) {
      mTodoList->updateView();
    }
  }
}

void CalendarView::updateView( const QDate &start, const QDate &end,
                               const bool updateTodos )
{
  if ( updateTodos ) {
    mTodoList->updateView();
  }

  if ( start.isValid() && end.isValid() ) {
    mViewManager->updateView( start, end );
  }

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

int CalendarView::msgItemDelete( const Akonadi::Item &item )
{
  return KMessageBox::questionYesNo(
    this,
    i18nc( "@info",
           "Do you really want to permanently remove the item \"%1\"?", Akonadi::incidence( item )->summary() ),
    i18nc( "@title:window", "Delete Item?" ),
    KStandardGuiItem::yes(),
    KStandardGuiItem::no(),
    QString(),
    KMessageBox::Notify );
}

void CalendarView::edit_cut()
{
  const Akonadi::Item item = selectedIncidence();
  Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence || !mChanger ) {
    KNotification::beep();
    return;
  }

  Item::List items;
  int km = KMessageBox::Yes;

  if ( !mCalendar->findChildren( item ).isEmpty() &&
       incidence->type() == Incidence::TypeTodo ) { // Only todos (yet?)
    km = KMessageBox::questionYesNoCancel( this,
                                           i18n("The item \"%1\" has sub-to-dos. "
                                                "Do you want to cut just this item and "
                                                "make all its sub-to-dos independent, or "
                                                "cut the to-do with all its sub-to-dos?"
                                             , incidence->summary() ) ,
                                           i18n("KOrganizer Confirmation"),
                                           KGuiItem( i18n("Cut Only This") ),
                                           KGuiItem( i18n("Cut All") ) );
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
  const Item item = selectedIncidence();

  if ( !item.isValid() ) {
    KNotification::beep();
    return;
  }

  Incidence::Ptr incidence = Akonadi::incidence( item );
  Item::List items;
  int km = KMessageBox::Yes;

  if ( !mCalendar->findChildren( item ).isEmpty()  &&
       incidence->type() == Incidence::TypeTodo ) { // only todos.
    km = KMessageBox::questionYesNoCancel( this,
                                           i18n("The item \"%1\" has sub-to-dos. "
                                                "Do you want to copy just this item or "
                                                "copy the to-do with all its sub-to-dos?"
                                           , incidence->summary() ),
                                           i18n("KOrganizer Confirmation"),
                                           KGuiItem( i18n("Copy Only This") ),
                                           KGuiItem( i18n("Copy All") ) );
   }

  if ( km == KMessageBox::Yes ) { // only one
    items.append( item );
  } else if ( km == KMessageBox::No ) { // all
    // load incidence + children + grandchildren...
    getIncidenceHierarchy( item, items );
  }

  if ( km != KMessageBox::Cancel ) {
    CalendarAdaptor::Ptr cal( new CalendarAdaptor( mCalendar, this ) );
    Akonadi::DndFactory factory( cal );
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

  KOAgendaView *aView = mViewManager->agendaView();
  MonthView *mView = mViewManager->monthView();

  if ( !curView ) {
    return;
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

  Akonadi::CalendarAdaptor::Ptr cal( new CalendarAdaptor( mCalendar, this ) );
  Akonadi::DndFactory factory( cal );
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
  for ( it = pastedIncidences.begin(); it != pastedIncidences.end(); ++it ) {
    // FIXME: use a visitor here
    if ( ( *it )->type() == Incidence::TypeEvent ) {
      Event::Ptr pastedEvent = ( *it ).staticCast<Event>();
      // only use selected area if event is of the same type (all-day or non-all-day
      // as the current selection is
      if ( aView && endDT.isValid() && useEndTime ) {
        if ( ( pastedEvent->allDay() && aView->selectedIsAllDay() ) ||
             ( !pastedEvent->allDay() && !aView->selectedIsAllDay() ) ) {
          KDateTime kdt( endDT, KCalPrefs::instance()->timeSpec() );
          pastedEvent->setDtEnd( kdt.toTimeSpec( pastedEvent->dtEnd().timeSpec() ) );
        }
      }

      pastedEvent->setRelatedTo( QString() );
      if ( selectedCollection.isValid() ) {
        mChanger->addIncidence( Event::Ptr( pastedEvent->clone() ), selectedCollection, this );
      } else {
        mChanger->addIncidence( Event::Ptr( pastedEvent->clone() ), this, selectedCollection,
                                dialogCode );
      }
    } else if ( ( *it )->type() == Incidence::TypeTodo ) {
      Todo::Ptr pastedTodo = ( *it ).staticCast<Todo>();
      Akonadi::Item _selectedTodoItem = selectedTodo();

      // if we are cutting a hierarchy only the root
      // should be son of _selectedTodo
      Todo::Ptr _selectedTodo = Akonadi::todo( _selectedTodoItem );
      if ( _selectedTodo && pastedTodo->relatedTo().isEmpty() ) {
        pastedTodo->setRelatedTo( _selectedTodo->uid() );
      }

      if ( selectedCollection.isValid() ) {
        // When pasting multiple incidences, don't ask which collection to use, for each one
        mChanger->addIncidence( Todo::Ptr( pastedTodo->clone() ), selectedCollection, this );
      } else {
        mChanger->addIncidence( Todo::Ptr( pastedTodo->clone() ), this, selectedCollection,
                                dialogCode );
      }

    } else if ( ( *it )->type() == Incidence::TypeJournal ) {

      if ( selectedCollection.isValid() ) {
        // When pasting multiple incidences, don't ask which collection to use, for each one
        mChanger->addIncidence( Incidence::Ptr( ( *it )->clone() ), selectedCollection, this );
      } else {
        mChanger->addIncidence( Incidence::Ptr( ( *it )->clone() ), this, selectedCollection,
                                dialogCode );
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
    startDt.setTime( KCalPrefs::instance()->mStartTime.time() );

    int addSecs =
      ( KCalPrefs::instance()->mDefaultDuration.time().hour() * 3600 ) +
      ( KCalPrefs::instance()->mDefaultDuration.time().minute() * 60 );
    endDt = startDt.addSecs( addSecs );
  }
}

IncidenceEditorsNG::IncidenceDialog *CalendarView::newEventEditor( const Event::Ptr &event )
{
  Akonadi::Item item;
  item.setPayload( event );

  IncidenceEditorsNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
  dialog->load( item );

//  connectIncidenceEditor( dialog );
  mDialogManager->connectTypeAhead(
    dialog, dynamic_cast<KOEventView*>( viewManager()->currentView() ) );

  return dialog;
}

void CalendarView::newEvent()
{
  if ( mCreatingEnabled ) {
    newEvent( Akonadi::Collection::List(), QDateTime(), QDateTime() );
  }
}

void CalendarView::newEvent( const QDate &dt )
{
  if ( mCreatingEnabled ) {
    QDateTime startDt( dt, KCalPrefs::instance()->mStartTime.time() );
    QTime duration = KCalPrefs::instance()->defaultDuration().time();
    QTime time = startDt.time();

    time = time.addSecs( duration.hour()*3600 + duration.minute() * 60 +  duration.second() );
    QDateTime endDt( startDt );
    endDt.setTime( time );
    newEvent( Akonadi::Collection::List(), startDt, endDt );
  }
}

void CalendarView::newEvent( const Akonadi::Collection::List &selectedCollections )
{
  if ( mCreatingEnabled ) {
    newEvent( selectedCollections, QDateTime(), QDateTime() );
  }
}

void CalendarView::newEvent( const Akonadi::Collection::List &selectedCollections, const QDate &dt )
{
  if ( mCreatingEnabled ) {
    QDateTime startDt( dt, KCalPrefs::instance()->mStartTime.time() );
    newEvent( selectedCollections, QDateTime( dt ), QDateTime( dt ) );
  }
}

void CalendarView::newEvent( const Akonadi::Collection::List &selectedCollections, const QDateTime &startDt )
{
  if ( mCreatingEnabled ) {
    newEvent( selectedCollections, startDt, QDateTime( startDt ) );
  }
}

void CalendarView::newEvent(  const Akonadi::Collection::List &selectedCollections,
                              const QDateTime &startDtParam, const QDateTime &endDtParam, bool allDay )
{
  /**
     imho, this parameter should be removed:
     - To-dos and journals don't use this which is inconsistent.
     - Currently an empty List() is being passed.
     - Collections selected in resource view should only influence what the views show.
  */
  Q_UNUSED( selectedCollections );

  if ( mCreatingEnabled ) {
    // Let the current view change the default start/end datetime
    QDateTime startDt( startDtParam );
    QDateTime endDt( endDtParam );

    // Adjust the start/end date times (i.e. replace invalid values by defaults,
    // and let the view adjust the type.
    dateTimesForNewEvent( startDt, endDt, allDay );

    IncidenceEditorsNG::IncidenceDefaults defaults = minimalIncidenceDefaults();
    defaults.setStartDateTime( KDateTime( startDt ) );
    defaults.setEndDateTime( KDateTime( endDt ) );

    Event::Ptr event( new Event );
    defaults.setDefaults( event );
    event->setAllDay( allDay );

    IncidenceEditorsNG::IncidenceDialog *eventEditor = newEventEditor( event );
    Q_ASSERT( eventEditor );

    eventEditor->selectCollection( defaultCollection() );
  }
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QStringList &attachments, const QStringList &attendees,
                             const QStringList &attachmentMimetypes, bool inlineAttachment )
{
  if ( mCreatingEnabled ) {
    // Adjust the start/end date times (i.e. replace invalid values by defaults,
    // and let the view adjust the type.
    QDateTime startDt;
    QDateTime endDt;
    bool allDay = false;
    dateTimesForNewEvent( startDt, endDt, allDay );

    IncidenceEditorsNG::IncidenceDefaults defaults = minimalIncidenceDefaults();
    defaults.setStartDateTime( KDateTime( startDt ) );
    defaults.setEndDateTime( KDateTime( endDt ) );
    // if attach or attendee list is empty, these methods don't do anything, so
    // it's safe to call them in every case
    defaults.setAttachments( attachments, attachmentMimetypes, inlineAttachment );
    defaults.setAttendees( attendees );

    Event::Ptr event( new Event );
    defaults.setDefaults( event );

    event->setSummary( summary );
    event->setDescription( description );
    event->setAllDay( allDay );
    newEventEditor( event );
  }
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                            const QStringList &attachments, const QStringList &attendees,
                            const QStringList &attachmentMimetypes,
                            bool inlineAttachment )
{
  if ( mCreatingEnabled ) {
    IncidenceEditorsNG::IncidenceDefaults defaults = minimalIncidenceDefaults();
    // if attach or attendee list is empty, these methods don't do anything, so
    // it's safe to call them in every case
    defaults.setAttachments( attachments, attachmentMimetypes, inlineAttachment );
    defaults.setAttendees( attendees );

    Todo::Ptr todo( new Todo );
    defaults.setDefaults( todo );

    todo->setSummary( summary );
    todo->setDescription( description );

    Item item;
    item.setPayload( todo );

    IncidenceEditorsNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
    dialog->selectCollection( defaultCollection() );
    dialog->load( item );
  }
}

void CalendarView::newTodo()
{
  newTodo( Akonadi::Collection() );
}

void CalendarView::newTodo( const Akonadi::Collection &collection )
{
  if ( mCreatingEnabled ) {
    IncidenceEditorsNG::IncidenceDefaults defaults = minimalIncidenceDefaults();

    bool allDay = true;
    if ( mViewManager->currentView()->isEventView() ) {
      QDateTime startDt;
      QDateTime endDt;
      dateTimesForNewEvent( startDt, endDt, allDay );

      defaults.setStartDateTime( KDateTime( startDt ) );
      defaults.setEndDateTime( KDateTime( endDt ) );
    }

    Todo::Ptr todo( new Todo );
    defaults.setDefaults( todo );
    todo->setAllDay( allDay );

    Item item;
    item.setPayload( todo );

    IncidenceEditorsNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
//    connectIncidenceEditor( dialog );

    if ( collection.isValid() ) {
      dialog->selectCollection( collection );
    } else {
      dialog->selectCollection( defaultCollection() );
    }

    dialog->load( item );
  }
}

void CalendarView::newTodo( const QDate &date )
{
  if ( mCreatingEnabled ) {
    IncidenceEditorsNG::IncidenceDefaults defaults = minimalIncidenceDefaults();
    defaults.setEndDateTime( KDateTime( date, QTime::currentTime() ) );

    Todo::Ptr todo( new Todo );
    defaults.setDefaults( todo );
    todo->setAllDay( true );

    Item item;
    item.setPayload( todo );

    IncidenceEditorsNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
//    connectIncidenceEditor( dialog );
    dialog->load( item );
  }
}

void CalendarView::newJournal()
{
  if ( mCreatingEnabled ) {
    newJournal( QString(), activeDate( true ) );
  }
}

void CalendarView::newJournal( const QDate &date )
{
  if ( mCreatingEnabled ) {
    newJournal( QString(), date );
  }
}

void CalendarView::newJournal( const Akonadi::Collection &collection )
{
  if ( mCreatingEnabled ) {
    JournalEditor *journalEditor = mDialogManager->getJournalEditor();
    QDate journalDate = activeDate( true );
    connectIncidenceEditor( journalEditor );
    journalEditor->newJournal();
    if ( !journalDate.isValid() ) {
      journalDate = activeDate();
    }
    journalEditor->setDate( journalDate );

    if ( collection.isValid() ) {
      journalEditor->selectCollection( collection );
    } else {
      journalEditor->selectCollection( defaultCollection() );
    }

    journalEditor->show();
  }
}

void CalendarView::newJournal( const QString &text, const QDate &date )
{
  if ( mCreatingEnabled ) {
    JournalEditor *journalEditor = mDialogManager->getJournalEditor();
    QDate journalDate = date;
    connectIncidenceEditor( journalEditor );
    journalEditor->newJournal();

    if ( !journalDate.isValid() ) {
      journalDate = activeDate();
    }

    journalEditor->setDate( journalDate );
    journalEditor->setTexts( text );
    journalEditor->show();
  }
}

KOrg::BaseView *CalendarView::currentView() const
{
  return mViewManager->currentView();
}

void CalendarView::configureCurrentView()
{
  KOrg::BaseView *const view = currentView();
  if ( !view || !view->hasConfigurationDialog() )
    return;
  view->showConfigurationDialog( this );
}

void CalendarView::newSubTodo()
{
  if ( mCreatingEnabled ) {
    const Item item = selectedTodo();
    if ( Akonadi::hasTodo( item ) ) {
      newSubTodo( item );
    }
  }
}

void CalendarView::newSubTodo( const Akonadi::Collection &collection )
{
  if ( mCreatingEnabled ) {
    if ( !Akonadi::hasTodo( selectedTodo() ) ) {
      return;
    }

    IncidenceEditorsNG::IncidenceDefaults defaults = minimalIncidenceDefaults();
    defaults.setRelatedIncidence( Akonadi::incidence( selectedTodo() ) );

    Todo::Ptr todo( new Todo );
    defaults.setDefaults( todo );
    todo->setAllDay( false );

    Item item;
    item.setPayload( todo );

    IncidenceEditorsNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
//    connectIncidenceEditor( dialog );
    if ( collection.isValid() ) {
      dialog->selectCollection( collection );
    } else {
      dialog->selectCollection( defaultCollection() );
    }
    dialog->load( item );
  }
}

void CalendarView::newSubTodo( const Item &parentEvent )
{
  if ( mCreatingEnabled ) {
    IncidenceEditorsNG::IncidenceDefaults defaults = minimalIncidenceDefaults();
    defaults.setRelatedIncidence( Akonadi::incidence( parentEvent ) );

    Todo::Ptr todo( new Todo );
    defaults.setDefaults( todo );
    todo->setAllDay( false );

    Item item;
    item.setPayload( todo );

    IncidenceEditorsNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );
//    connectIncidenceEditor( dialog );
    dialog->load( item );
  }
}

void CalendarView::newFloatingEvent()
{
  if ( mCreatingEnabled ) {
    QDate date = activeDate();
    // TODO_BERTJAN: Find out from where this is called and see if we can get a reasonable default collection
    newEvent( Akonadi::Collection::List(),
              QDateTime( date, QTime( 12, 0, 0 ) ),
              QDateTime( date, QTime( 12, 0, 0 ) ), true );
  }
}

bool CalendarView::addIncidence( const QString &ical )
{
  ICalFormat format;
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
  const Item item  = selectedIncidence();
  if ( Akonadi::hasIncidence( item ) ) {
    showIncidence( item );
  } else {
    KNotification::beep();
  }
}

void CalendarView::appointment_edit()
{
  const Item item = selectedIncidence();
  if ( Akonadi::hasIncidence( item ) ) {
    editIncidence( item );
  } else {
    KNotification::beep();
  }
}

void CalendarView::appointment_delete()
{
  const Item item = selectedIncidence();
  if ( Akonadi::hasIncidence( item ) ) {
    deleteIncidence( item );
  } else {
    KNotification::beep();
  }
}

void CalendarView::todo_unsub()
{
  const Item aTodo = selectedTodo();
  if ( incidence_unsub( aTodo ) ) {
    updateView();
  }
}

bool CalendarView::incidence_unsub( const Item &item )
{
  const Incidence::Ptr inc = Akonadi::incidence( item );

  if ( !inc || inc->relatedTo().isEmpty() ) {
    return false;
  }

  Incidence::Ptr oldInc( inc->clone() );
  inc->setRelatedTo( 0 );
  mChanger->changeIncidence( oldInc, item, IncidenceChanger::RELATION_MODIFIED, this );

  return true;
}

bool CalendarView::makeSubTodosIndependent( )
{
  bool  status = false;
  const Item aTodo = selectedTodo();

  if( makeChildrenIndependent( aTodo ) ) {
    updateView();
    status = true;
  }
  return status;
}

bool CalendarView::makeChildrenIndependent( const Item &item )
{
  const Incidence::Ptr inc = Akonadi::incidence( item );

  Item::List subIncs = mCalendar->findChildren( item );

  if ( !inc || subIncs.isEmpty() ) {
    return false;
  }
  startMultiModify ( i18n( "Make sub-to-dos independent" ) );

  foreach( const Item &item, subIncs ) {
    incidence_unsub( item );
  }

  endMultiModify();
  return true;
}

bool CalendarView::deleteIncidence( const Item::Id &uid, bool force )
{
  Akonadi::Item item = mCalendar->incidence( uid );
  if ( !Akonadi::hasIncidence( item ) )
    return false;
  return deleteIncidence( item, force );
}

void CalendarView::toggleAlarm( const Item &item )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );
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
    switch( KCalPrefs::instance()->mReminderTimeUnits ) {
    default:
    case 0: // mins
      duration = KCalPrefs::instance()->mReminderTime * 60;
      break;
    case 1: // hours
      duration = KCalPrefs::instance()->mReminderTime * 60 * 60;
      break;
    case 2: // days
      duration = KCalPrefs::instance()->mReminderTime * 60 * 60 * 24;
      break;
    }
    if ( incidence->type() == Incidence::TypeEvent ) {
      alm->setStartOffset( KCalCore::Duration( -duration ) );
    } else {
      alm->setEndOffset( KCalCore::Duration( -duration ) );
    }
  }
  mChanger->changeIncidence( oldincidence, item, IncidenceChanger::ALARM_MODIFIED, this );
}

void CalendarView::toggleTodoCompleted( const Item &todoItem )
{
  const Incidence::Ptr incidence = Akonadi::incidence( todoItem );

  if ( !incidence || !mChanger ) {
    kDebug() << "called without having a clicked item";
    return;
  }
  if ( incidence->type() != Incidence::TypeTodo ) {
    kDebug() << "called for a non-Todo incidence";
    return;
  }

  Todo::Ptr todo = Akonadi::todo( todoItem );
  Q_ASSERT( todo );
  Todo::Ptr oldtodo( todo->clone() );

  if ( todo->isCompleted() ) {
    todo->setPercentComplete( 0 );
  } else {
    todo->setCompleted( KDateTime::currentDateTime( KCalPrefs::instance()->timeSpec() ) );
  }

  mChanger->changeIncidence( oldtodo,
                             todoItem,
                             IncidenceChanger::COMPLETION_MODIFIED,
                             this );
}

void CalendarView::copyIncidenceToResource( const Item &item, const QString &resourceId )
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
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

void CalendarView::moveIncidenceToResource( const Item &item, const QString &resourceId )
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
  kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
}

void CalendarView::dissociateOccurrences( const Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );

  if ( !incidence || !mChanger ) {
    kError() << "Called without having a clicked item";
    return;
  }

  KDateTime thisDateTime( date, KCalPrefs::instance()->timeSpec() );
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
      KGuiItem( i18n( "&Cancel" ) ) );

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
void CalendarView::dissociateOccurrence( const Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );

  startMultiModify( i18n( "Dissociate occurrence" ) );
  Incidence::Ptr oldincidence( incidence->clone() );
  Incidence::Ptr newInc(
    mCalendar->dissociateOccurrence( item, date, KCalPrefs::instance()->timeSpec(), true ) );
  if ( newInc ) {
    mChanger->changeIncidence( oldincidence, item, IncidenceChanger::NOTHING_MODIFIED, this );
    mChanger->addIncidence( newInc, item.parentCollection(), this );
  } else {
    KMessageBox::sorry(
      this,
      i18n( "Dissociating the occurrence failed." ),
      i18n( "Dissociating Failed" ) );
  }

  endMultiModify();
}

void CalendarView::dissociateFutureOccurrence( const Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );

  startMultiModify( i18n( "Dissociate future occurrences" ) );
  Incidence::Ptr oldincidence( incidence->clone() );

  Incidence::Ptr newInc(
    mCalendar->dissociateOccurrence( item, date,
                                     KCalPrefs::instance()->timeSpec(), false ) );
  if ( newInc ) {
    mChanger->changeIncidence( oldincidence,
                               item,
                               IncidenceChanger::NOTHING_MODIFIED,
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

void CalendarView::schedule_publish( const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );

  if ( !incidence ) {
    const Item item = selectedIncidence();
    incidence = Akonadi::incidence( item );
  }

  if ( !incidence ) {
    KMessageBox::information(
      this,
      i18n( "No item selected." ),
      "PublishNoEventSelected" );
    return;
  }

  QPointer<PublishDialog> publishdlg = new PublishDialog();
  if ( incidence->attendeeCount() > 0 ) {
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
      publishdlg->addAttendee( *it );
    }
  }
  if ( publishdlg->exec() == QDialog::Accepted ) {
    Incidence::Ptr inc( incidence->clone() );
    inc->registerObserver( 0 );
    inc->clearAttendees();

    // Send the mail
    Akonadi::MailScheduler scheduler( mCalendar );
    if ( scheduler.publish( incidence, publishdlg->addresses() ) ) {
      KMessageBox::information(
        this,
        i18n( "The item information was successfully sent." ),
        i18n( "Publishing" ),
        "IncidencePublishSuccess" );
    } else {
      KMessageBox::error(
        this,
        i18n( "Unable to publish the item '%1'", incidence->summary() ) );
    }
  }
  delete publishdlg;
}

void CalendarView::schedule_request( const Item &incidence )
{
  schedule( iTIPRequest, incidence );
}

void CalendarView::schedule_refresh( const Item &incidence )
{
  schedule( iTIPRefresh, incidence );
}

void CalendarView::schedule_cancel( const Item &incidence )
{
  schedule( iTIPCancel, incidence );
}

void CalendarView::schedule_add( const Item &incidence )
{
  schedule( iTIPAdd, incidence );
}

void CalendarView::schedule_reply( const Item &incidence )
{
  schedule( iTIPReply, incidence );
}

void CalendarView::schedule_counter( const Item &incidence )
{
  schedule( iTIPCounter, incidence );
}

void CalendarView::schedule_declinecounter( const Item &incidence )
{
  schedule( iTIPDeclineCounter, incidence );
}

void CalendarView::schedule_forward( const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence ) {
    const Item item = selectedIncidence();
    incidence = Akonadi::incidence( item );
  }

  if ( !incidence ) {
    KMessageBox::information(
      this,
      i18n( "No item selected." ),
      i18n( "Forwarding" ),
      "ForwardNoEventSelected" );
    return;
  }

  QPointer<PublishDialog> publishdlg = new PublishDialog;
  if ( publishdlg->exec() == QDialog::Accepted ) {
    const QString recipients = publishdlg->addresses();
    if ( incidence->organizer()->isEmpty() ) {
      incidence->setOrganizer( Person::Ptr( new Person( KCalPrefs::instance()->fullName(),
                                                        KCalPrefs::instance()->email() ) ) );
    }

    ICalFormat format;
    const QString from = KCalPrefs::instance()->email();
    const bool bccMe = KCalPrefs::instance()->mBcc;
    const QString messageText = format.createScheduleMessage( incidence, iTIPRequest );
    Akonadi::MailClient mailer;
    if ( mailer.mailTo(
           incidence,
           KOCore::self()->identityManager()->identityForAddress( from ),
           from, bccMe, recipients, messageText, MailTransport::TransportManager::self()->defaultTransportName() ) ) {
      KMessageBox::information(
        this,
        i18n( "The item information was successfully sent." ),
        i18n( "Forwarding" ),
        "IncidenceForwardSuccess" );
    } else {
      KMessageBox::error(
        this,
        i18n( "Unable to forward the item '%1'", incidence->summary() ),
        i18n( "Forwarding Error" ) );
    }
  }
  delete publishdlg;
}

void CalendarView::mailFreeBusy( int daysToPublish )
{
  KDateTime start = KDateTime::currentUtcDateTime().toTimeSpec( mCalendar->timeSpec() );
  KDateTime end = start.addDays( daysToPublish );

  Event::List events;
  Akonadi::Item::List items = mCalendar ? mCalendar->rawEvents( start.date(), end.date() ) : Akonadi::Item::List();
  foreach(const Akonadi::Item &item, items) { //FIXME
    events << item.payload<Event::Ptr>();
  }

  FreeBusy::Ptr freebusy( new FreeBusy( events, start, end ) );
  freebusy->setOrganizer( Person::Ptr( new Person( KCalPrefs::instance()->fullName(),
                                                   KCalPrefs::instance()->email() ) ) );

  QPointer<PublishDialog> publishdlg = new PublishDialog();
  if ( publishdlg->exec() == QDialog::Accepted ) {
    // Send the mail
    Akonadi::MailScheduler scheduler( mCalendar );
    if ( scheduler.publish( freebusy, publishdlg->addresses() ) ) {
      KMessageBox::information(
        this,
        i18n( "The free/busy information was successfully sent." ),
        i18n( "Sending Free/Busy" ),
        "FreeBusyPublishSuccess" );
    } else {
      KMessageBox::error(
        this,
        i18n( "Unable to publish the free/busy data." ) );
    }
  }
  delete publishdlg;
}

void CalendarView::uploadFreeBusy()
{
  Akonadi::FreeBusyManager::self()->publishFreeBusy( this );
}

void CalendarView::schedule( KCalCore::iTIPMethod method, const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence ) {
    const Item item = selectedIncidence();
    incidence = Akonadi::incidence( item );
  }

  if ( !incidence ) {
    KMessageBox::sorry(
      this,
      i18n( "No item selected." ),
      "ScheduleNoEventSelected" );
    return;
  }

  if ( incidence->attendeeCount() == 0 && method != iTIPPublish ) {
    KMessageBox::information(
      this,
      i18n( "The item has no attendees." ),
      "ScheduleNoIncidences" );
    return;
  }

  Incidence *inc = incidence->clone();
  inc->registerObserver( 0 );
  inc->clearAttendees();

  // Send the mail
  Akonadi::MailScheduler scheduler( mCalendar );
  if ( scheduler.performTransaction( incidence, method ) ) {
    KMessageBox::information(
      this,
      i18n( "The groupware message for item '%1' "
            "was successfully sent.\nMethod: %2",
            incidence->summary(),
            ScheduleMessage::methodName( method ) ),
      i18n( "Sending Free/Busy" ),
      "FreeBusyPublishSuccess" );
  } else {
    KMessageBox::error(
      this,
      i18nc( "Groupware message sending failed. "
             "%2 is request/reply/add/cancel/counter/etc.",
             "Unable to send the item '%1'.\nMethod: %2",
             incidence->summary(),
             ScheduleMessage::methodName( method ) ) );
  }
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
    foreach (const Akonadi::Item &item, selectedViewIncidences ) {
      if (item.hasPayload<KCalCore::Incidence::Ptr>() ) {
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
    foreach (const Akonadi::Item &item, selectedViewIncidences ) {
      if (item.hasPayload<KCalCore::Incidence::Ptr>() ) {
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
  QString filename = KFileDialog::getSaveFileName( KUrl( "icalout.ics" ), i18n( "*.ics|iCalendars" ), this );
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
    ICalFormat *format = new ICalFormat;
    CalendarAdaptor::Ptr calendar( new CalendarAdaptor( mCalendar, this ) );
    FileStorage storage( calendar, filename, format );
    if ( !storage.save() ) {
      QString errmess;
      if ( format->exception() ) {
        errmess = Stringify::errorMessage( *format->exception() );
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
      i18n( "The journal entries can not be exported to a vCalendar file." ),
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
    VCalFormat *format = new VCalFormat;
    CalendarAdaptor::Ptr calendar( new CalendarAdaptor( mCalendar, this ) );
    FileStorage storage( calendar, filename, format );
    if ( !storage.save() ) {
      QString errmess;
      if ( format->exception() ) {
        errmess = Stringify::errorMessage( *format->exception() );
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

void CalendarView::eventUpdated( const Item & )
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

void CalendarView::processMainViewSelection( const Item &item, const QDate &date )
{
  if ( Akonadi::hasIncidence( item ) ) {
    mTodoList->clearSelection();
  }
  processIncidenceSelection( item, date );
}

void CalendarView::processTodoListSelection( const Item &item, const QDate &date )
{
  if (  Akonadi::hasIncidence( item ) && mViewManager->currentView() ) {
    mViewManager->currentView()->clearSelection();
  }
  processIncidenceSelection( item, date );
}

void CalendarView::processIncidenceSelection( const Item &item, const QDate &date )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
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

  organizerEvents = KCalPrefs::instance()->thatIsMe( incidence->organizer()->email() );
  groupEvents = incidence->attendeeByMails( KCalPrefs::instance()->allEmails() );

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
#ifndef KORG_NODND
  if ( ICalDrag::canDecode( QApplication::clipboard()->mimeData() ) ) {
    emit pasteEnabled( true );
  } else {
    emit pasteEnabled( false );
  }
#endif
}

void CalendarView::showDates( const DateList &selectedDates, const QDate &preferredMonth )
{
  mDateNavigatorContainer->selectDates( selectedDates, preferredMonth );
  mNavigatorBar->selectDates( selectedDates );

  if ( mViewManager->currentView() ) {
    updateView( selectedDates.first(), selectedDates.last(), false );
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
  foreach ( CalFilter *filter, mFilters ) {
    if ( filter ) {
      filters << filter->name();
    }
  }

  // account for the additional "No filter" at the beginning! if the
  // filter is not in the list, pos == -1...
  emit filtersUpdated( filters, pos + 1 );

  mCalendar->setFilter( mCurrentFilter );
  mViewManager->setFilter( mCurrentFilter );
  updateView();
}

void CalendarView::filterActivated( int filterNo )
{
  CalFilter *newFilter = 0;
  if ( filterNo > 0 && filterNo <= int( mFilters.count() ) ) {
    newFilter = mFilters.at( filterNo - 1 );
  }
  if ( newFilter != mCurrentFilter ) {
    mCurrentFilter = newFilter;
    mCalendar->setFilter( mCurrentFilter );
    mViewManager->setFilter( mCurrentFilter );
    mViewManager->setUpdateNeeded();
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
  const Item item = currentSelection();
  Incidence::Ptr incidence = Akonadi::incidence( item );

  if ( incidence ) {
    return;
  }

  incidence->setOrganizer( Person::Ptr( new Person( KCalPrefs::instance()->fullName(),
                                                    KCalPrefs::instance()->email() ) ) );
  incidence->recreate();
  incidence->setReadOnly( false );

  //PENDING(AKONADI_PORT) call mChanger?

  updateView();
}

void CalendarView::takeOverCalendar()
{
  const Item::List items = mCalendar->rawIncidences();

  Q_FOREACH( const Item& item, items ) {
    Incidence::Ptr i = Akonadi::incidence( item );
    i->setOrganizer( Person::Ptr( new Person( KCalPrefs::instance()->fullName(),
                                              KCalPrefs::instance()->email() ) ) );
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
  } else {
    mDateNavigatorContainer->hide();
  }
}

void CalendarView::showTodoView( bool show )
{
  if ( show ) {
    mTodoList->show();
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

Item CalendarView::selectedTodo()
{
  const Item item = currentSelection();
  if ( const Todo::Ptr t = Akonadi::todo( item ) ) {
    return item;
  }

  Item incidence;

  const Item::List selectedIncidences = mTodoList->selectedIncidences();
  if ( !selectedIncidences.isEmpty() ) {
    incidence = selectedIncidences.first();
  }
  if ( const Todo::Ptr t = Akonadi::todo( item ) )
    return item;
  return Item();
}

void CalendarView::dialogClosing( const Item &item )
{
  mDialogList.remove( item.id() );
}

Item CalendarView::currentSelection()
{
  return mViewManager->currentSelection();
}

Item CalendarView::selectedIncidence()
{
  Item item = currentSelection();
  if ( !item.isValid() ) {
    Item::List selectedIncidences = mTodoList->selectedIncidences();
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

bool CalendarView::editIncidence( const Item::Id &uid )
{
  Akonadi::Item item = mCalendar->incidence( uid );
  return editIncidence( item );
}

bool CalendarView::showIncidence( const Item::Id &uid )
{
  Akonadi::Item item = mCalendar->incidence( uid );
  if ( !Akonadi::hasIncidence( item ) ) {
    return false;
  }
  showIncidence( item );
  return true;
}

bool CalendarView::showIncidenceContext( const Item::Id &uid )
{
  Akonadi::Item item = mCalendar->incidence( uid );
  if ( !Akonadi::hasIncidence( item ) ) {
    return false;
  }
  showIncidenceContext( item );
  return true;
}

void CalendarView::deleteIncidence()
{
  deleteIncidence( selectedIncidence() );
}

void CalendarView::cutIncidence( const Item &incidence )
{
  Q_UNUSED( incidence );
  edit_cut();
}

void CalendarView::copyIncidence( const Item &incidence )
{
  Q_UNUSED( incidence );
  edit_copy();
}

void CalendarView::pasteIncidence()
{
  edit_paste();
}

void CalendarView::showIncidence( const Item &item )
{
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog( this );
  eventViewer->setIncidence( item, QDate() );
  // Disable the Edit button for read-only Incidences.
  if ( !mCalendar->hasChangeRights( item ) ) {
    eventViewer->enableButton( KDialog::User1, false );
  }

  eventViewer->show();
}

void CalendarView::showIncidenceContext( const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( Akonadi::hasEvent( item ) ) {
    if ( !viewManager()->currentView()->inherits( "KOEventView" ) ) {
      viewManager()->showAgendaView();
    }
    // just select the appropriate date
    mDateNavigator->selectWeek(
      incidence->dtStart().toTimeSpec( KCalPrefs::instance()->timeSpec() ).date() );
    return;
  } else if ( Akonadi::hasJournal( item ) ) {
    if ( !viewManager()->currentView()->inherits( "KOJournalView" ) ) {
      viewManager()->showJournalView();
    }
  } else if ( Akonadi::hasTodo( item ) ) {
    if ( !viewManager()->currentView()->inherits( "KOTodoView" ) ) {
      viewManager()->showTodoView();
    }
  }
  Item::List list;
  list.append( item );
  viewManager()->currentView()->showIncidences( list, QDate() );
}

bool CalendarView::editIncidence( const Item &item, bool isCounter )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
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

  IncidenceEditor *tmp = editorDialog( item );
  if ( tmp ) {
    tmp->reload();
    tmp->raise();
    tmp->show();
    return true;
  }

  if ( !mCalendar->hasChangeRights( item ) ) {
    showIncidence( item );
    return true;
  }

  if ( IncidenceEditor *incidenceEditor = mDialogManager->getEditor( item ) ) {
    connectIncidenceEditor( incidenceEditor );

    mDialogList.insert( item.id(), incidenceEditor );
    incidenceEditor->editIncidence( item, activeIncidenceDate() );
    incidenceEditor->show();
  } else {
    IncidenceEditorsNG::IncidenceDialog *dialog = mDialogManager->createDialog( item );

    // mDialogList.insert( item.id(), incidenceEditor ); // TODO: Need to investigate this.
    // connectIncidenceEditor( dialog );                 // TODO: This as well
    dialog->load( item, activeIncidenceDate() ); // Will show the dialog as soon as it has loaded the item.
  }

  return true;
}

void CalendarView::deleteSubTodosIncidence ( const Item &todoItem )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return;
  }
  Item::List subTodos = mCalendar->findChildren( todoItem );
  foreach( const Item &item,  subTodos ) {
    if ( Akonadi::hasTodo( item ) ) {
      deleteSubTodosIncidence ( item );
    }
  }

  if ( mChanger->isNotDeleted( todoItem.id() ) ) {
    mChanger->deleteIncidence ( todoItem, this );
  }
}

void CalendarView::deleteTodoIncidence ( const Item& todoItem, bool force )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return ;
  }

  // it a simple todo, ask and delete it.
  if ( todo->relatedTo().isEmpty() ) {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( todoItem ) == KMessageBox::Yes );
    }
    if ( doDelete && mChanger->isNotDeleted( todoItem.id() ) ) {
      mChanger->deleteIncidence( todoItem, this );
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
      mChanger->deleteIncidence( todoItem, this );
    }
  } else if ( km == KMessageBox::No ) {
    // Delete all
    // we have to hide the delete confirmation for each itemDate
    deleteSubTodosIncidence ( todoItem );
  }
  endMultiModify();
}

bool CalendarView::deleteIncidence( const Item &item, bool force )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
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
        KDateTime itemDateTime( itemDate, KCalPrefs::instance()->timeSpec() );
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
          km = ( msgItemDelete( item ) == KMessageBox::Yes ?
                 KMessageBox::Continue : KMessageBox::Cancel );
        }
      }
    }
    Incidence::Ptr oldIncidence( incidence->clone() );
    switch( km ) {
    case KMessageBox::Ok: // Continue // all
    case KMessageBox::Continue:
      mChanger->deleteIncidence( item, this );
      break;

    case KMessageBox::Yes: // just this one
      incidence->recurrence()->addExDate( itemDate );
      mChanger->changeIncidence( oldIncidence, item,
                                 IncidenceChanger::RECURRENCE_MODIFIED_ONE_ONLY, this );

      break;
    case KMessageBox::No: // all future items
      Recurrence *recur = incidence->recurrence();
      recur->setEndDate( itemDate.addDays( -1 ) );
      mChanger->changeIncidence( oldIncidence, item,
                                 IncidenceChanger::RECURRENCE_MODIFIED_ALL_FUTURE, this );
      break;
    }
  } else {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( item ) == KMessageBox::Yes );
    }
    if ( doDelete ) {
      mChanger->deleteIncidence( item, this );
      processIncidenceSelection( Item(), QDate() );
    }
  }
  return true;
}

void CalendarView::connectIncidenceEditor( IncidenceEditor *editor )
{
  // TODO_BERTJAN: Remove, should be set already at this point.
  if ( currentView()->collectionSelection()->hasSelection() )
    editor->selectCollection( currentView()->collectionSelection()->selectedCollections().first() );

  connect( this, SIGNAL(newIncidenceChanger(Akonadi::IncidenceChanger *)),
           editor, SLOT(setIncidenceChanger(Akonadi::IncidenceChanger *)) );
  editor->setIncidenceChanger( mChanger );
}

bool CalendarView::purgeCompletedSubTodos( const Item &todoItem, bool &allPurged )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return true;
  }

  bool deleteThisTodo = true;
  Item::List subTodos = mCalendar->findChildren( todoItem );
  foreach( const Item &item,  subTodos ) {
    if ( Akonadi::hasTodo( item ) ) {
      deleteThisTodo &= purgeCompletedSubTodos( item, allPurged );
    }
  }

  if ( deleteThisTodo ) {
    if ( todo->isCompleted() ) {
      if ( !mChanger->deleteIncidence( todoItem, this ) ) {
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
    Item::List todos = calendar()->rawTodos();
    Item::List rootTodos;
    Item::List::ConstIterator it;
    for ( it = todos.constBegin(); it != todos.constEnd(); ++it ) {
      Todo::Ptr aTodo = Akonadi::todo( *it );
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

void CalendarView::warningChangeFailed( const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
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
  QStringList allCats( Akonadi::Calendar::categories( calendar() ) );
  allCats.sort();

  IncidenceEditors::CategoryConfig cc( KOPrefs::instance() );

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

void CalendarView::addIncidenceOn( const Item &itemadd, const QDate &dt )
{
  if ( !Akonadi::hasIncidence( itemadd ) || !mChanger ) {
    KMessageBox::sorry(
      this,
      i18n( "Unable to copy the item to %1.", dt.toString() ),
      i18n( "Copying Failed" ) );
    return;
  }
  Item item = mCalendar->incidence( itemadd.id() );
  if ( !item.isValid() ) {
    item = itemadd;
  }
  // Create a copy of the incidence, since the incadd doesn't belong to us.
  Incidence::Ptr incidence( Akonadi::incidence( item )->clone() );
  incidence->recreate();

  if ( const Event::Ptr event = incidence.dynamicCast<Event>() ) {

    // Adjust date
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } else if ( const Todo::Ptr todo = incidence.dynamicCast<Todo>()  ) {
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

void CalendarView::moveIncidenceTo( const Item &itemmove, const QDate &dt )
{
  if ( !Akonadi::hasIncidence( itemmove ) || !mChanger ) {
    KMessageBox::sorry(
      this,
      i18n( "Unable to move the item to  %1.", dt.toString() ),
      i18n( "Moving Failed" ) );
    return;
  }
  Item item = mCalendar->incidence( itemmove.id() );
  if ( !item.isValid() ) {
    addIncidenceOn( itemmove, dt );
    return;
  }
  Incidence::Ptr incidence = Akonadi::incidence( itemmove );

  Incidence::Ptr oldIncidence( incidence->clone() );

  if ( const Event::Ptr event = incidence.dynamicCast<Event>() ) {
    // Adjust date
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } if ( const Todo::Ptr todo = incidence.dynamicCast<Todo>()  ) {
    KDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }
  mChanger->changeIncidence( oldIncidence, itemmove, IncidenceChanger::DATE_MODIFIED, this );
}

void CalendarView::resourcesChanged()
{
  mViewManager->setUpdateNeeded();
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

void CalendarView::selectWeek( const QDate &date )
{
  if ( KOPrefs::instance()->mWeekNumbersShowWork  &&
       mViewManager->rangeMode() == KOViewManager::WORK_WEEK_RANGE ) {
    mDateNavigator->selectWorkWeek( date );
  } else {
    mDateNavigator->selectWeek( date );
  }
}

void CalendarView::getIncidenceHierarchy( const Item &item,
                                          Item::List &children )
{
  // protecion against looping hierarchies
  if ( item.isValid() && !children.contains( item ) ) {
    Item::List::ConstIterator it;
    Item::List immediateChildren = mCalendar->findChildren( item );

    for ( it = immediateChildren.constBegin();
          it != immediateChildren.constEnd(); ++it ) {
      getIncidenceHierarchy( *it, children );
    }
    children.append( item );
  }
}

void CalendarView::setCreatingEnabled( bool enabled )
{
  mCreatingEnabled = enabled;
}

Collection CalendarView::defaultCollection() const
{
  const Collection::Id id = KCalPrefs::instance()->defaultCalendarId();
  return mCalendar->collection( id );
}


#include "calendarview.moc"
