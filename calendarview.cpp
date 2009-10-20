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
#include "freebusymanager.h"
#include "history.h"
#include "incidencechanger.h"
#include "kocorehelper.h"
#include "kodialogmanager.h"
#include "koeventviewer.h"
#include "koeventviewerdialog.h"
#include "koglobals.h"
#include "kogroupware.h"
#include "incidenceeditor/koeventeditor.h"
#include "incidenceeditor/koincidenceeditor.h"
#include "incidenceeditor/kojournaleditor.h"
#include "incidenceeditor/kotodoeditor.h"
#include "komailclient.h"
#include "komessagebox.h"
#include "koviewmanager.h"
#include "mailscheduler.h"
#include "navigatorbar.h"
#include "publishdialog.h"
#include "htmlexportsettings.h"
#include "views/agendaview/koagendaview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/todoview/kotodoview.h"

#include <akonadi/kcal/utils.h>
#include <akonadi/kcal/akonadicalendaradaptor.h>

#include <KCal/CalendarResources>
#include <KCal/CalFilter>
#include <KCal/DndFactory>
#include <KCal/FileStorage>
#include <KCal/FreeBusy>
#include <KCal/ICalDrag>
#include <KCal/ICalFormat>
#include <KCal/VCalFormat>

#include <KHolidays/Holidays>
using namespace KHolidays;

#include <KPIMIdentities/IdentityManager>

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

using namespace boost;
using namespace Akonadi;

CalendarView::CalendarView( QWidget *parent )
  : CalendarViewBase( parent ),
    mHistory( 0 ),
    mCalendar( 0 ),
    mChanger( 0 ),
    mSplitterSizesValid( false )
{
  mViewManager = new KOViewManager( this );
  mDialogManager = new KODialogManager( this );

  mModified = false;
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

  mTodoList = new KOTodoView( mLeftSplitter );
  mTodoList->setObjectName( "todolist" );

  mEventViewerBox = new KVBox( mLeftSplitter );
  mEventViewerBox->setMargin( KDialog::marginHint() );
  mEventViewer = new KOEventViewer( mEventViewerBox );
  mEventViewer->setObjectName( "EventViewer" );

  KVBox *rightBox = new KVBox( mPanner );
  mNavigatorBar = new NavigatorBar( rightBox );
  mRightFrame = new QStackedWidget( rightBox );
  rightBox->setStretchFactor( mRightFrame, 1 );

  mLeftFrame = mLeftSplitter;
  mLeftFrame->installEventFilter( this );

  connect( mDateNavigator, SIGNAL(datesSelected(const KCal::DateList &)),
           SLOT(showDates(const KCal::DateList &)) );
  connect( mDateNavigator, SIGNAL(datesSelected(const KCal::DateList &)),
           mDateNavigatorContainer, SLOT(selectDates(const KCal::DateList &)) );

  connect( mDateNavigatorContainer, SIGNAL(newEventSignal(const QDate &)),
           SLOT(newEvent(const QDate &)) );
  connect( mDateNavigatorContainer, SIGNAL(newTodoSignal(const QDate &)),
           SLOT(newTodo(const QDate &)) );
  connect( mDateNavigatorContainer, SIGNAL(newJournalSignal(const QDate &)),
           SLOT(newJournal(const QDate &)) );

  connect( mNavigatorBar, SIGNAL(goPrevYear()),
           mDateNavigator, SLOT(selectPreviousYear()) );
  connect( mNavigatorBar, SIGNAL(goNextYear()),
           mDateNavigator, SLOT(selectNextYear()) );
  connect( mNavigatorBar, SIGNAL(goPrevMonth()),
           mDateNavigator, SLOT(selectPreviousMonth()) );
  connect( mNavigatorBar, SIGNAL(goNextMonth()),
           mDateNavigator, SLOT(selectNextMonth()) );
  connect( mNavigatorBar, SIGNAL(goMonth(int)),
           mDateNavigator, SLOT(selectMonth(int)) );
  connect( mNavigatorBar, SIGNAL(goYear(int)),
           mDateNavigator, SLOT(selectYear(int)) );

  connect( mDateNavigator, SIGNAL(datesSelected(const KCal::DateList &)),
           mNavigatorBar, SLOT(selectDates(const KCal::DateList &)) );

  connect( mDateNavigatorContainer, SIGNAL(weekClicked(const QDate &)),
           mDateNavigator, SLOT(selectWeek(const QDate &)) );

  connect( mDateNavigatorContainer, SIGNAL(goPrevYear()),
           mDateNavigator, SLOT(selectPreviousYear()) );
  connect( mDateNavigatorContainer, SIGNAL(goNextYear()),
           mDateNavigator, SLOT(selectNextYear()) );
  connect( mDateNavigatorContainer, SIGNAL(goPrevMonth()),
           mDateNavigator, SLOT(selectPreviousMonth()) );
  connect( mDateNavigatorContainer, SIGNAL(goNextMonth()),
           mDateNavigator, SLOT(selectNextMonth()) );
  connect( mDateNavigatorContainer, SIGNAL(goMonth(int)),
           mDateNavigator, SLOT(selectMonth(int)) );
  connect( mDateNavigatorContainer, SIGNAL(goYear(int)),
           mDateNavigator, SLOT(selectYear(int)) );

  connect( mDateNavigatorContainer, SIGNAL(goPrevious()),
           mDateNavigator, SLOT(selectPrevious()) );
  connect( mDateNavigatorContainer, SIGNAL(goNext()),
           mDateNavigator, SLOT(selectNext()) );

  connect( mDateNavigatorContainer, SIGNAL(datesSelected(const KCal::DateList &)),
           mDateNavigator, SLOT(selectDates(const KCal::DateList &)) );

  connect( mDateNavigatorContainer, SIGNAL(incidenceDropped(Incidence*, const QDate&)),
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

  mEventViewer->setDefaultText( s );
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

void CalendarView::setCalendar( KOrg::AkonadiCalendar *cal )
{
  mCalendar = cal;

  delete mHistory;
  mHistory = new History( mCalendar );
  connect( mHistory, SIGNAL(undone()), SLOT(updateView()) );
  connect( mHistory, SIGNAL(redone()), SLOT(updateView()) );

  setIncidenceChanger( new IncidenceChanger( mCalendar, this ) );

  mCalendar->registerObserver( this );

  mDateNavigatorContainer->setCalendar( mCalendar );

  mTodoList->setCalendar( mCalendar );
}

void CalendarView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  delete mChanger;
  mChanger = changer;
  emit newIncidenceChanger( mChanger );
  connect( mChanger, SIGNAL(incidenceAdded(Akonadi::Item)),
           this, SLOT(incidenceAdded(Akonadi::Item)) );
  connect( mChanger, SIGNAL(incidenceChanged(Akonadi::Item,Akonadi::Item,int)),
           this, SLOT(incidenceChanged(Akonadi::Item,Akonadi::Item,int)) );
  connect( mChanger, SIGNAL(incidenceChanged(Akonadi::Item,Akonadi::Item)),
           this, SLOT(incidenceChanged(Akonadi::Item,Akonadi::Item)) );
  connect( mChanger, SIGNAL(incidenceToBeDeleted(Akonadi::Item)),
           this, SLOT(incidenceToBeDeleted(Akonadi::Item)) );
  connect( mChanger, SIGNAL(incidenceDeleted(Akonadi::Item)),
           this, SLOT(incidenceDeleted(Akonadi::Item)) );

  connect( mChanger, SIGNAL(schedule(iTIPMethod,Akonadi::Item)),
           this, SLOT(schedule(iTIPMethod,Akonadi::Item)) );

  connect( this, SIGNAL(cancelAttendees(Akonadi::Item)),
           mChanger, SLOT(cancelAttendees(Akonadi::Item)) );
}

KOrg::AkonadiCalendar *CalendarView::calendar()
{
  return mCalendar;
}

KOIncidenceEditor *CalendarView::editorDialog( const Item &item ) const
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

    // Try the view's selectedDates()
    if ( !curView->selectedDates().isEmpty() ) {
      if ( curView->selectedDates().first().isValid() ) {
        return curView->selectedDates().first();
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
#ifdef AKONADI_PORT_DISABLED
  if ( filename.isEmpty() ) {
    kDebug() << "Error! Empty filename.";
    return false;
  }

  if ( !QFile::exists( filename ) ) {
    kDebug() << "Error! File '" << filename << "' doesn't exist.";
  }

  bool loadedSuccesfully = true;
  if ( !merge ) {
    mCalendar->close();
    // otherwise something is majorly wrong
    Q_ASSERT( dynamic_cast<CalendarResources*>( mCalendar ) );
    // openCalendar called without merge and a filename, what should we do?
    return false;
  }

  // merge in a file
  FileStorage storage( mCalendar );
  storage.setFileName( filename );
  loadedSuccesfully = storage.load();

  if ( loadedSuccesfully ) {
    if ( merge ) {
      setModified( true );
    } else {
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
    if ( !merge ) {
      mCalendar->close();
    }
    KMessageBox::error( this, i18n( "Could not load calendar '%1'.", filename ) );
    return false;
  }
#else
  return false;
#endif //AKONADI_PORT_DISABLED
}

bool CalendarView::saveCalendar( const QString &filename )
{
#ifdef AKONADI_PORT_DISABLED
  // Store back all unsaved data into calendar object
  mViewManager->currentView()->flushView();

  FileStorage storage( mCalendar );
  storage.setFileName( filename );
  storage.setSaveFormat( new ICalFormat );

  return storage.save();
#else // AKONADI_PORT_DISABLED
  return false;
#endif // AKONADI_PORT_DISABLED
}

void CalendarView::closeCalendar()
{
  // child windows no longer valid
  emit closingDown();
  //mCalendar->close();
  setModified( false );
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

  mEventViewer->readSettings( config );
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

  mEventViewer->writeSettings( config );
  mViewManager->writeSettings( config );
  mTodoList->saveLayout( config, QString( "Sidebar Todo View" ) );

  KOPrefs::instance()->writeConfig();

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
    if ( filter->criteria() & KCal::CalFilter::HideNoMatchingAttendeeTodos ) {
      filter->setEmailList( KOPrefs::instance()->allEmails() );
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

  KOGlobals::self()->setHolidays( new HolidayRegion( KOPrefs::instance()->mHolidays ) );

  // Only set a new time zone if it changed. This prevents the window
  // from being modified on start
  KDateTime::Spec newTimeSpec = KOPrefs::instance()->timeSpec();
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

  // force reload and handle agenda view type switch
  const bool showMerged =
    KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::CalendarsMerged;
  const bool showSideBySide =
    KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::CalendarsSideBySide;
  KOrg::BaseView *view = mViewManager->currentView();
  mViewManager->showAgendaView();
  if ( view == mViewManager->agendaView() && showSideBySide ) {
    view = mViewManager->multiAgendaView();
  } else if ( view == mViewManager->multiAgendaView() && showMerged ) {
    view = mViewManager->agendaView();
  }
  mViewManager->showView( view );

  // To make the "fill window" configurations work
  mViewManager->raiseCurrentView();
}

void CalendarView::incidenceAdded( const Item &incidence )
{
  setModified( true );
  history()->recordAdd( Akonadi::incidence( incidence ).get() );
  changeIncidenceDisplay( incidence, KOGlobals::INCIDENCEADDED );
  updateUnmanagedViews();
  checkForFilteredChange( incidence );
}

void CalendarView::incidenceChanged( const Item &oldIncidence,
                                     const Item &newIncidence )
{
  incidenceChanged( oldIncidence, newIncidence, KOGlobals::UNKNOWN_MODIFIED );
}

void CalendarView::incidenceChanged( const Item &oldIncidence_,
                                     const Item &newIncidence_, int what )
{
  Incidence* const oldIncidence = Akonadi::incidence( oldIncidence_ ).get();
  Incidence* const newIncidence = Akonadi::incidence( newIncidence_ ).get();

  // FIXME: Make use of the what flag, which indicates which parts of the incidence have changed!
  KOIncidenceEditor *tmp = editorDialog( newIncidence_ );
  if ( tmp ) {
    kDebug() << "Incidence modified and open";
    tmp->modified( what );
  }
  setModified( true );
  history()->recordEdit( oldIncidence, newIncidence );

  // Record completed todos in journals, if enabled. we should to this here in
  // favor of the todolist. users can mark a task as completed in an editor
  // as well.
  if ( newIncidence->type() == "Todo" &&
       KOPrefs::instance()->recordTodosInJournals() &&
       ( what == KOGlobals::COMPLETION_MODIFIED ||
         what == KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE ) ) {
    Todo *todo = static_cast<Todo *>(newIncidence);
    if ( todo->isCompleted() ||
         what == KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE ) {
      QString timeStr = KGlobal::locale()->formatTime( QTime::currentTime() );
      QString description = i18n( "Todo completed: %1 (%2)", newIncidence->summary(), timeStr );

      Item::List journals = calendar()->journals( QDate::currentDate() );

      if ( journals.isEmpty() ) {
        Journal::Ptr journal( new Journal );
        journal->setDtStart( KDateTime::currentDateTime( KOPrefs::instance()->timeSpec() ) );

        QString dateStr = KGlobal::locale()->formatDate( QDate::currentDate() );
        journal->setSummary( i18n( "Journal of %1", dateStr ) );
        journal->setDescription( description );

        if ( !mChanger->addIncidence( journal, this ) ) {
          KODialogManager::errorSaveIncidence( this, journal );
          return;
        }

      } else { // journal list is not empty
        Item journalItem = journals.first();
        Journal::Ptr journal = Akonadi::journal( journalItem );
        Journal::Ptr oldJournal( journal->clone() );
        journal->setDescription( journal->description().append( '\n' + description ) );

        if ( !mChanger->changeIncidence( oldJournal, journalItem ) ) {
          KODialogManager::errorSaveIncidence( this, journal );
          return;
        }
      }
    }
  }

  changeIncidenceDisplay( newIncidence_, KOGlobals::INCIDENCEEDITED );
  updateUnmanagedViews();
  checkForFilteredChange( newIncidence_ );
}

void CalendarView::incidenceToBeDeleted( const Item &item )
{
  Incidence* const incidence = Akonadi::incidence( item ).get();
  KOIncidenceEditor *tmp = editorDialog( item );
  if (tmp) {
    kDebug() << "Incidence to be deleted and open in editor";
    tmp->delayedDestruct();
  }
  setModified( true );
  history()->recordDelete( incidence );
//  changeIncidenceDisplay( incidence, KOGlobals::INCIDENCEDELETED );
  updateUnmanagedViews();
}

void CalendarView::incidenceDeleted( const Item &item )
{
  changeIncidenceDisplay( item, KOGlobals::INCIDENCEDELETED );
  updateUnmanagedViews();
}

void CalendarView::checkForFilteredChange( const Item &item )
{
  Incidence* const incidence = Akonadi::incidence( item ).get();
  CalFilter *filter = calendar()->filter();
  if ( filter && !filter->filterIncidence( incidence ) ) {
    // Incidence is filtered and thus not shown in the view, tell the
    // user so that he isn't surprised if his new event doesn't show up
    KMessageBox::information(
      this,
      i18n( "The item \"%1\" is filtered by your current filter rules, "
            "so it will be hidden and not appear in the view.",
            incidence->summary() ),
      i18n( "Filter Applied" ), "ChangedIncidenceFiltered" );
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
    mEventViewer->changeIncidenceDisplay( item, activeDate( true ), action );
  } else {
    mViewManager->currentView()->updateView();
    if ( mTodoList ) {
      mTodoList->updateView();
    }
  }
}

void CalendarView::updateView( const QDate &start, const QDate &end, const bool updateTodos )
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
  mChanger->cutIncidence( item );

  checkClipboard();
}

void CalendarView::edit_copy()
{
  const Item item = selectedIncidence();

  if ( !item.isValid() ) {
    KNotification::beep();
    return;
  }
#ifdef AKONADI_PORT_DISABLED
  DndFactory factory( mCalendar );
  if ( !factory.copyIncidence( incidence ) ) {
    KNotification::beep();
  }
#endif // AKONADI_PORT_DISABLED
  checkClipboard();
}

void CalendarView::edit_paste()
{
// If in agenda and month view, use the selected time and date from there.
// In all other cases, use the navigator's selected date.

  QDate date;          // null dates are invalid, that's what we want
  QTime time( -1, -1 );// create an invalid time, that's what we want
  QDateTime endDT;     // null datetimes are invalid, that's what we want
  bool useEndTime = false;

  KOrg::BaseView *curView = mViewManager->currentView();
  if ( !curView ) {
    return;
  }

  KOAgendaView *aView = mViewManager->agendaView();
  MonthView *mView = mViewManager->monthView();

  if ( curView == aView && aView->selectionStart().isValid() ) {
    date = aView->selectionStart().date();
    endDT = aView->selectionEnd();
    useEndTime = !aView->selectedIsSingleCell();
    if ( !aView->selectedIsAllDay() ) {
      time = aView->selectionStart().time();
    }
  } else if ( curView == mView && !mView->selectedDates().isEmpty() ) {
    date = mView->selectedDates().first();
  } else {
    // default to the selected date from the navigator
    if ( !mDateNavigator->selectedDates().isEmpty() ) {
      date = mDateNavigator->selectedDates().first();
    }
  }

  if ( !date.isValid() ) {
    KMessageBox::sorry( this,
                        i18n( "Paste failed: unable to determine a valid target date." ) );
    return;
  }
#ifdef AKONADI_PORT_DISABLED
  DndFactory factory( mCalendar );
  Incidence *pastedIncidence;
  if ( time.isValid() ) {
    pastedIncidence = factory.pasteIncidence( date, &time );
  } else {
    pastedIncidence = factory.pasteIncidence( date );
  }
  if ( !pastedIncidence ) {
    return;
  }

  // FIXME: use a visitor here
  if ( pastedIncidence->type() == "Event" ) {
    Event *pastedEvent = static_cast<Event*>( pastedIncidence );
    // only use selected area if event is of the same type (all-day or non-all-day
    // as the current selection is
    if ( aView && endDT.isValid() && useEndTime ) {
      if ( ( pastedEvent->allDay() && aView->selectedIsAllDay() ) ||
           ( !pastedEvent->allDay() && !aView->selectedIsAllDay() ) ) {
        KDateTime kdt( endDT, KOPrefs::instance()->timeSpec() );
        pastedEvent->setDtEnd( kdt.toTimeSpec( pastedIncidence->dtEnd().timeSpec() ) );
      }
    }
    mChanger->addIncidence( pastedEvent, this );

  } else if ( pastedIncidence->type() == "Todo" ) {
    Todo *pastedTodo = static_cast<Todo*>(pastedIncidence);
    Todo *_selectedTodo = selectedTodo();
    if ( _selectedTodo ) {
      pastedTodo->setRelatedTo( _selectedTodo );
    }
    mChanger->addIncidence( pastedTodo, this );
  } else if ( pastedIncidence->type() == "Journal" ) {
    mChanger->addIncidence( pastedIncidence, this );
  }
#endif // AKONADI_PORT_DISABLED
}

void CalendarView::edit_options()
{
  mDialogManager->showOptionsDialog();
}

void CalendarView::dateTimesForNewEvent( QDateTime &startDt, QDateTime &endDt,
                                         bool &allDay )
{
  if ( !startDt.isValid() ) {
    startDt.setDate( activeDate() );
    startDt.setTime( KOPrefs::instance()->mStartTime.time() );
  }
  if ( !endDt.isValid() ) {
    int addSecs =
      ( KOPrefs::instance()->mDefaultDuration.time().hour() * 3600 ) +
      ( KOPrefs::instance()->mDefaultDuration.time().minute() * 60 );
    endDt = startDt.addSecs( addSecs );
  }
  mViewManager->currentView()->eventDurationHint( startDt, endDt, allDay );
}

KOEventEditor *CalendarView::newEventEditor( const QDateTime &startDtParam,
                                             const QDateTime &endDtParam,
                                             bool allDayParam )
{
  // Let the current view change the default start/end datetime
  bool allDay = allDayParam;
  QDateTime startDt( startDtParam ), endDt( endDtParam );

  // Adjust the start/end date times (i.e. replace invalid values by defaults,
  // and let the view adjust the type.
  dateTimesForNewEvent( startDt, endDt, allDay );

  KOEventEditor *eventEditor = mDialogManager->getEventEditor();
  eventEditor->newEvent();
  connectIncidenceEditor( eventEditor );
  eventEditor->setDates( startDt, endDt, allDay );
  mDialogManager->connectTypeAhead(
    eventEditor, dynamic_cast<KOEventView*>( viewManager()->currentView() ) );
  return eventEditor;
}

void CalendarView::newEvent()
{
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

void CalendarView::newEvent( const QDateTime &startDt, const QDateTime &endDt, bool allDay )
{
  KOEventEditor *eventEditor = newEventEditor( startDt, endDt, allDay );
  eventEditor->show();
}

void CalendarView::newEvent( const QString &summary, const QString &description,
                             const QStringList &attachments, const QStringList &attendees,
                             const QStringList &attachmentMimetypes, bool inlineAttachment )
{
  KOEventEditor *eventEditor = newEventEditor();
  eventEditor->setTexts( summary, description );

  // if attach or attendee list is empty, these methods don't do anything, so
  // it's safe to call them in every case
  eventEditor->addAttachments( attachments, attachmentMimetypes, inlineAttachment );
  eventEditor->addAttendees( attendees );
  eventEditor->show();
}

void CalendarView::newTodo( const QString &summary, const QString &description,
                            const QStringList &attachments, const QStringList &attendees,
                            const QStringList &attachmentMimetypes,
                            bool inlineAttachment, bool isTask )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  todoEditor->setTexts( summary, description );
  todoEditor->addAttachments( attachments, attachmentMimetypes, inlineAttachment );
  todoEditor->addAttendees( attendees );
  todoEditor->selectCreateTask( isTask );
  todoEditor->show();
}

void CalendarView::newTodo()
{
  QDateTime dtDue;
  bool allday = true;
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  if ( mViewManager->currentView()->isEventView() ) {
    dtDue.setDate( activeDate() );
    QDateTime dtDummy = QDateTime::currentDateTime();
    mViewManager->currentView()->eventDurationHint( dtDue, dtDummy, allday );
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
  newJournal( QString(), activeDate( true ) );
}

void CalendarView::newJournal( const QDate &date )
{
  newJournal( QString(), date );
}

void CalendarView::newJournal( const QString &text, const QDate &date )
{
  KOJournalEditor *journalEditor = mDialogManager->getJournalEditor();
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

void CalendarView::newSubTodo()
{
  const Item item = selectedTodo();
  if ( Akonadi::hasTodo( item ) ) {
    newSubTodo( item );
  }
}

void CalendarView::newSubTodo( const Item &parentEvent )
{
  KOTodoEditor *todoEditor = mDialogManager->getTodoEditor();
  connectIncidenceEditor( todoEditor );
  todoEditor->newTodo();
  todoEditor->setDates( QDateTime(), false, parentEvent );
  todoEditor->show();
}

void CalendarView::newFloatingEvent()
{
  QDate date = activeDate();
  newEvent( QDateTime( date, QTime( 12, 0, 0 ) ),
            QDateTime( date, QTime( 12, 0, 0 ) ), true );
}

bool CalendarView::addIncidence( const QString &ical )
{
  ICalFormat format;
  format.setTimeSpec( mCalendar->timeSpec() );
  Incidence::Ptr incidence( format.fromString( ical ) );
  return addIncidence(incidence);
}

bool CalendarView::addIncidence( const Incidence::Ptr &incidence )
{
  return incidence ? mChanger->addIncidence( incidence, this ) : false;
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
  const Item anTodo = selectedTodo();
  if ( todo_unsub( anTodo ) ) {
    updateView();
  }
}

bool CalendarView::todo_unsub( const Item &todoItem )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  bool status= false;
  if ( !todo || !todo->relatedTo() ) {
    return false;
  }

  if ( mChanger->beginChange( todoItem ) ) {
      Todo::Ptr oldTodo( todo->clone() );
      todo->setRelatedTo(0);
      mChanger->changeIncidence( oldTodo, todoItem, KOGlobals::RELATION_MODIFIED );
      mChanger->endChange( todoItem );
      setModified(true);
      status = true;
  }
  if ( !status ) {
    KMessageBox::sorry( this, i18n( "Unable to turn sub-to-do into a top-level "
                                    "to-do, because it cannot be locked." ) );
  }

  return status;
}

bool CalendarView::makeSubTodosIndependents ( )
{
  bool  status = false;
  const Item anTodo = selectedTodo();

  if( makeSubTodosIndependents( anTodo ) ) {
    updateView();
    status = true;
  }
  return status;
}

bool CalendarView::makeSubTodosIndependents ( const Item &todoItem )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );

  if ( !todo || todo->relations().isEmpty() ) {
    return false;
  }
#ifdef AKONADI_PORT_DISABLED
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
#else
  return false;
#endif
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
  if ( !mChanger->beginChange( item ) ) {
    kDebug() << "Unable to lock incidence";
    return;
  }

  Alarm::List alarms = incidence->alarms();
  Alarm::List::ConstIterator it;
  for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
    (*it)->toggleAlarm();
  }
  if ( alarms.isEmpty() ) {
    // Add an alarm if it didn't have one
    Alarm *alm = incidence->newAlarm();
    alm->setType( Alarm::Display );
    alm->setEnabled( true );
  }
  mChanger->changeIncidence( oldincidence, item, KOGlobals::ALARM_MODIFIED );
  mChanger->endChange( item );
}

void CalendarView::toggleTodoCompleted( const Item &todoItem )
{
  const Incidence::Ptr incidence = Akonadi::incidence( todoItem );

  if ( !incidence || !mChanger ) {
    kDebug() << "called without having a clicked item";
    return;
  }
  if ( incidence->type() != "Todo" ) {
    kDebug() << "called for a non-Todo incidence";
    return;
  }

  Todo::Ptr todo = Akonadi::todo( todoItem );
  Q_ASSERT( todo );
  Todo::Ptr oldtodo( todo->clone() );
  if ( !mChanger->beginChange( todoItem ) ) {
    kDebug() << "Unable to lock todo";
    return;
  }

  if ( todo->isCompleted() ) {
    todo->setPercentComplete( 0 );
  } else {
    todo->setCompleted( KDateTime::currentDateTime( KOPrefs::instance()->timeSpec() ) );
  }

  mChanger->changeIncidence( oldtodo, todoItem, KOGlobals::COMPLETION_MODIFIED );
  mChanger->endChange( todoItem );
}

void CalendarView::dissociateOccurrences( const Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );

  if ( !incidence || !mChanger ) {
    kError() << "Called without having a clicked item";
    return;
  }

  KDateTime thisDateTime( date, KOPrefs::instance()->timeSpec() );
  bool isFirstOccurrence = !incidence->recurrence()->getPreviousDateTime( thisDateTime ).isValid();

  int answer;
  bool doOnlyThis = false;
  bool doFuture   = false;

  if ( isFirstOccurrence ) {
    answer = KMessageBox::questionYesNo( this,
                                         i18n( "Do you want to dissociate "
                                               "the occurrence at %1 "
                                               "from the recurrence?",
                                                KGlobal::locale()->formatDate( date ) ),
                                                i18n( "KOrganizer Confirmation" ),
                                                KGuiItem( i18n( "&Dissociate" ) ),
                                                KGuiItem( i18n( "&Cancel" ) ) );

    doOnlyThis = ( answer == KMessageBox::Yes );
  } else {
    answer = KMessageBox::questionYesNoCancel( this,
                                               i18n( "Do you want to dissociate "
                                                     "the occurrence at %1 "
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

  if ( !mChanger->beginChange( item ) ) {
    kDebug() << "Unable to lock incidence";
    return;
  }
  startMultiModify( i18n( "Dissociate occurrence" ) );
  Incidence::Ptr oldincidence( incidence->clone() );

  Incidence::Ptr newInc(
    mCalendar->dissociateOccurrence( item, date, KOPrefs::instance()->timeSpec(), true ) );

  if ( newInc ) {
    // TODO: Use the same resource instead of asking again!
    mChanger->changeIncidence( oldincidence, item );
    mChanger->addIncidence( newInc, this );
  } else {
    KMessageBox::sorry( this, i18n( "Dissociating the occurrence failed." ),
                        i18n( "Dissociating Failed" ) );
  }
  mChanger->endChange( item );
  endMultiModify();
}

void CalendarView::dissociateFutureOccurrence( const Item &item, const QDate &date )
{
  const Incidence::Ptr incidence = Akonadi::incidence( item );

  if ( !mChanger->beginChange( item ) ) {
    kDebug() << "Unable to lock incidence";
    return;
  }
  startMultiModify( i18n( "Dissociate future occurrences" ) );
  Incidence::Ptr oldincidence( incidence->clone() );

  Incidence::Ptr newInc(
    mCalendar->dissociateOccurrence( item, date,
                                     KOPrefs::instance()->timeSpec(), false ) );
  if ( newInc ) {
    // TODO: Use the same resource instead of asking again!
    mChanger->changeIncidence( oldincidence, item );
    mChanger->addIncidence( newInc, this );
  } else {
    KMessageBox::sorry(
      this,
      i18n( "Dissociating the future occurrences failed." ),
      i18n( "Dissociating Failed" ) );
  }
  endMultiModify();
  mChanger->endChange( item );
}

void CalendarView::schedule_publish( const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );

  if ( !incidence ) {
    const Item item = selectedIncidence();
    incidence = Akonadi::incidence( item );
  }

  if ( !incidence ) {
    KMessageBox::information( this, i18n( "No item selected." ), "PublishNoEventSelected" );
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
    Incidence* const inc( incidence->clone() );
    inc->registerObserver( 0 );
    inc->clearAttendees();

    // Send the mail
    MailScheduler scheduler( mCalendar );
    if ( scheduler.publish( incidence.get(), publishdlg->addresses() ) ) {
      KMessageBox::information( this, i18n( "The item information was successfully sent." ),
                                i18n( "Publishing" ), "IncidencePublishSuccess" );
    } else {
      KMessageBox::error( this, i18n( "Unable to publish the item '%1'", incidence->summary() ) );
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
    KMessageBox::information( this, i18n( "No item selected." ), "ForwardNoEventSelected" );
    return;
  }

  QPointer<PublishDialog> publishdlg = new PublishDialog;
  if ( publishdlg->exec() == QDialog::Accepted ) {
    QString recipients = publishdlg->addresses();
    if ( incidence->organizer().isEmpty() ) {
      incidence->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                       KOPrefs::instance()->email() ) );
    }

    ICalFormat format;
    QString from = KOPrefs::instance()->email();
    bool bccMe = KOPrefs::instance()->mBcc;
    bool useSendmail = ( KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail );
    QString messageText = format.createScheduleMessage( incidence.get(), iTIPRequest );
    KOMailClient mailer;
    if ( mailer.mailTo(
           incidence.get(),
           KOCore::self()->identityManager()->identityForAddress( from ),
           from, bccMe, recipients, messageText, useSendmail ) ) {
      KMessageBox::information(
        this,
        i18n( "The item information was successfully sent." ),
        i18n( "Forwarding" ), "IncidenceForwardSuccess" );
    } else {
      KMessageBox::error(
        this,
        i18n( "Unable to forward the item '%1'", incidence->summary() ) );
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
    events << item.payload<Event::Ptr>().get();
  }

  FreeBusy *freebusy = new FreeBusy( events, start, end );
  freebusy->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                  KOPrefs::instance()->email() ) );

  QPointer<PublishDialog> publishdlg = new PublishDialog();
  if ( publishdlg->exec() == QDialog::Accepted ) {
    // Send the mail
    MailScheduler scheduler( mCalendar );
    if ( scheduler.publish( freebusy, publishdlg->addresses() ) ) {
      KMessageBox::information(
        this,
        i18n( "The free/busy information was successfully sent." ),
        i18n( "Sending Free/Busy" ),
        "FreeBusyPublishSuccess" );
    } else {
      KMessageBox::error( this, i18n( "Unable to publish the free/busy data." ) );
    }
  }
  delete freebusy;
  delete publishdlg;
}

void CalendarView::uploadFreeBusy()
{
  KOGroupware::instance()->freeBusyManager()->publishFreeBusy();
}

void CalendarView::schedule( iTIPMethod method, const Item &item )
{
  Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence ) {
    const Item item = selectedIncidence();
    incidence = Akonadi::incidence( item );
  }

  if ( !incidence ) {
    KMessageBox::sorry( this, i18n( "No item selected." ), "ScheduleNoEventSelected" );
    return;
  }

  if ( incidence->attendeeCount() == 0 && method != iTIPPublish ) {
    KMessageBox::information( this, i18n( "The item has no attendees." ), "ScheduleNoIncidences" );
    return;
  }

  Incidence *inc = incidence->clone();
  inc->registerObserver( 0 );
  inc->clearAttendees();

  // Send the mail
  MailScheduler scheduler( mCalendar );
  if ( scheduler.performTransaction( incidence.get(), method ) ) {
    KMessageBox::information( this,
                              i18n( "The groupware message for item '%1' "
                                    "was successfully sent.\nMethod: %2",
                                    incidence->summary(),
                                    Scheduler::methodName( method ) ),
                              i18n( "Sending Free/Busy" ),
                              "FreeBusyPublishSuccess" );
  } else {
    KMessageBox::error( this,
                        i18nc( "Groupware message sending failed. "
                               "%2 is request/reply/add/cancel/counter/etc.",
                               "Unable to send the item '%1'.\nMethod: %2",
                               incidence->summary(),
                               Scheduler::methodName( method ) ) );
  }
}

void CalendarView::openAddressbook()
{
  KRun::runCommand( "kcontactmanager", topLevelWidget() );
}

void CalendarView::setModified( bool modified )
{
  if ( mModified != modified ) {
    mModified = modified;
    emit modifiedChanged( mModified );
  }
}

bool CalendarView::isReadOnly()
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

bool CalendarView::isModified()
{
  return mModified;
}

void CalendarView::print()
{
  createPrinter();

  KOrg::BaseView *currentView = mViewManager->currentView();

  CalPrinter::PrintType printType = CalPrinter::Month;

  if ( currentView ) {
    printType = currentView->printType();
  }

  DateList tmpDateList = mDateNavigator->selectedDates();
  mCalPrinter->print( printType, tmpDateList.first(), tmpDateList.last() );
}

void CalendarView::printPreview()
{
  createPrinter();

  KOrg::BaseView *currentView = mViewManager->currentView();

  CalPrinter::PrintType printType = CalPrinter::Month;

  if ( currentView ) {
    printType = currentView->printType();
  }

  DateList tmpDateList = mDateNavigator->selectedDates();
  mCalPrinter->print( printType, tmpDateList.first(), tmpDateList.last(),
                      Incidence::List(), true );
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
    AkonadiCalendarAdaptor calendar(mCalendar);
    FileStorage storage( &calendar, filename, format );
    if ( !storage.save() ) {
      QString errmess;
      if ( format->exception() ) {
        errmess = format->exception()->message();
      } else {
        errmess = i18nc( "save failure cause unknown", "Reason unknown" );
      }
      KMessageBox::error( this,
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
    AkonadiCalendarAdaptor calendar(mCalendar);
    FileStorage storage( &calendar, filename, format );
    if ( !storage.save() ) {
      QString errmess;
      if ( format->exception() ) {
        errmess = format->exception()->message();
      } else {
        errmess = i18nc( "save failure cause unknown", "Reason unknown" );
      }
      KMessageBox::error( this,
                          i18nc( "@info",
                                 "Cannot write vCalendar file %1. %2",
                                 filename, errmess ) );
    }
  }
}

void CalendarView::eventUpdated( const Item & )
{
  setModified();
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
  Incidence* const incidence = Akonadi::incidence( item ).get();
  if ( !incidence || item == mSelectedIncidence ) {
    return;
  }

  mSelectedIncidence = item;
  emit incidenceSelected( mSelectedIncidence, date );

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
  if ( ICalDrag::canDecode( QApplication::clipboard()->mimeData() ) ) {
    emit pasteEnabled( true );
  } else {
    emit pasteEnabled( false );
  }
#endif
}

void CalendarView::showDates( const DateList &selectedDates )
{
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

  emit newFilterListSignal( filters );
  // account for the additional "No filter" at the beginning! if the
  // filter is not in the list, pos == -1...
  emit selectFilterSignal( pos + 1 );
  mCalendar->setFilter( mCurrentFilter );
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

  incidence->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                   KOPrefs::instance()->email() ) );
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
    i->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                 KOPrefs::instance()->email() ) );
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

void CalendarView::calendarModified( bool modified, KOrg::CalendarBase *calendar )
{
  Q_UNUSED( calendar );
  setModified( modified );
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
  // FIXME: this doesn't work, because if it's a new incidence, it's not locked!
  mChanger->endChange( item );
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
  const Incidence::Ptr incidence = Akonadi::incidence( item );
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog( this );
  eventViewer->setIncidence( item, QDate() );

  // Disable the Edit button for read-only Incidences.
  if ( incidence->isReadOnly() ) {
    eventViewer->enableButton( KDialog::User1, false );
  }

  eventViewer->show();
}

void CalendarView::showIncidenceContext( const Item &item )
{
  Incidence* const incidence = Akonadi::incidence( item ).get();
  if ( Akonadi::hasEvent( item ) ) {
    if ( !viewManager()->currentView()->inherits( "KOEventView" ) ) {
      viewManager()->showAgendaView();
    }
    // just select the appropriate date
    mDateNavigator->selectWeek(
      incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).date() );
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

  KOIncidenceEditor *tmp = editorDialog( item );
  if ( tmp ) {
    tmp->reload();
    tmp->raise();
    tmp->show();
    return true;
  }

  if ( incidence->isReadOnly() ) {
    showIncidence( item );
    return true;
  }

  if ( !isCounter && !mChanger->beginChange( item ) ) {
    warningChangeFailed( item );
    showIncidence( item );
    return false;
  }

  KOIncidenceEditor *incidenceEditor = mDialogManager->getEditor( item );
  connectIncidenceEditor( incidenceEditor );

  mDialogList.insert( item.id(), incidenceEditor );
  incidenceEditor->editIncidence( item );
  incidenceEditor->show();
  return true;
}

void CalendarView::deleteSubTodosIncidence ( const Item &todoItem )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return;
  }
#ifdef AKONADI_PORT_DISABLED
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
#endif // AKONADI_PORT_DISABLED
  mChanger->deleteIncidence ( todoItem );
}

void CalendarView::deleteTodoIncidence ( const Item& todoItem, bool force )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return ;
  }


  // it a simple todo, ask and delete it.
  if ( todo->relations().isEmpty() ) {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( todoItem ) == KMessageBox::Yes );
    }
    if ( doDelete ) {
      mChanger->deleteIncidence( todoItem );
    }
    return;
  }

  /* Ok, this to-do has sub-to-dos, ask what to do */
  int km = KMessageBox::No;
  if ( !force ) {
    km=KMessageBox::questionYesNoCancel(
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
    makeSubTodosIndependents ( todoItem );
    mChanger->deleteIncidence( todoItem );
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
  if ( incidence->isReadOnly() ) {
    if ( !force ) {
      KMessageBox::information( this,
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
  if ( !v.act( incidence.get(), this ) ) {
    return false;
  }
  //If it is a todo, there are specific delete function

  if ( incidence && incidence->type() == "Todo" ) {
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
        KDateTime itemDateTime( itemDate, KOPrefs::instance()->timeSpec() );
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
          km = KOMessageBox::fourBtnMsgBox(
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
    switch( km ) {
    case KMessageBox::Ok: // Continue // all
    case KMessageBox::Continue:
      mChanger->deleteIncidence( item );
      break;

    case KMessageBox::Yes: // just this one
      if ( mChanger->beginChange( item ) ) {
        Incidence::Ptr oldIncidence( incidence->clone() );
        incidence->recurrence()->addExDate( itemDate );
        mChanger->changeIncidence( oldIncidence, item );
        mChanger->endChange( item );
      }
      break;
    case KMessageBox::No: // all future items
      if ( mChanger->beginChange( item ) ) {
        Incidence::Ptr oldIncidence( incidence->clone() );
        Recurrence *recur = incidence->recurrence();
        recur->setEndDate( itemDate.addDays(-1) );
        mChanger->changeIncidence( oldIncidence, item );
        mChanger->endChange( item );
      }
      break;
    }
  } else {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( item ) == KMessageBox::Yes );
    }
    if ( doDelete ) {
      mChanger->deleteIncidence( item );
      processIncidenceSelection( Item(), QDate() );
    }
  }
  return true;
}

void CalendarView::connectIncidenceEditor( KOIncidenceEditor *editor )
{
  connect( this, SIGNAL(newIncidenceChanger(IncidenceChangerBase *)),
           editor, SLOT(setIncidenceChanger(IncidenceChangerBase *)) );
  editor->setIncidenceChanger( mChanger );
}

bool CalendarView::purgeCompletedSubTodos( const Item &todoItem, bool &allPurged )
{
  const Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( !todo ) {
    return true;
  }
#ifdef AKONADI_PORT_DISABLED
  bool deleteThisTodo = true;
  Incidence::List subTodos( todo->relations() );
  Incidence *aIncidence;
  Todo *aTodo;
  Incidence::List::Iterator it;
  for ( it = subTodos.begin(); it != subTodos.end(); ++it ) {
    aIncidence = *it;
    if ( aIncidence && aIncidence->type() == "Todo" ) {
      aTodo = static_cast<Todo*>( aIncidence );
      deleteThisTodo &= purgeCompletedSubTodos( aTodo, allPurged );
    }
  }

  if ( deleteThisTodo ) {
    if ( todo->isCompleted() ) {
      if ( !mChanger->deleteIncidence( todoItem ) ) {
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
#else
  return false;
#endif // AKONADI_PORT_DISABLED
}

void CalendarView::purgeCompleted()
{
  int result = KMessageBox::warningContinueCancel( this,
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
      if ( aTodo && !aTodo->relatedTo() ) { // top level todo //REVIEW(AKONADI_PORT)
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
  Incidence* const incidence = Akonadi::incidence( item ).get();
  if ( incidence ) {
    KMessageBox::sorry(
      this,
      i18nc( "@info",
             "Unable to edit \"%1\" because it is locked by another process.",
             incidence->summary() ) );
  }
}

void CalendarView::editCanceled( const Item &item )
{
  mCalendar->endChange( item );
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
    if ( !categories.contains( *si ) ) {
      categories.append( *si );
    }
  }
  KOPrefs::instance()->mCustomCategories = categories;
  KOPrefs::instance()->writeConfig();
  // Make the category editor update the list!
  emit categoriesChanged();
}

void CalendarView::addIncidenceOn( const Item &itemadd, const QDate &dt )
{
  if ( !Akonadi::hasIncidence( itemadd ) || !mChanger ) {
    KMessageBox::sorry( this,
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

  if ( const Event::Ptr event = dynamic_pointer_cast<Event>( incidence ) ) {

    // Adjust date
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } else if ( const Todo::Ptr todo = dynamic_pointer_cast<Todo>( incidence ) ) {
    KDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }

  if ( !mChanger->addIncidence( incidence, this ) ) {
    KODialogManager::errorSaveIncidence( this, incidence );
  }
}

void CalendarView::moveIncidenceTo( const Item &itemmove, const QDate &dt )
{
  if ( !Akonadi::hasIncidence( itemmove ) || !mChanger ) {
    KMessageBox::sorry( this,
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
  if ( !mChanger->beginChange( itemmove ) ) {
    return;
  }

  if ( const Event::Ptr event = dynamic_pointer_cast<Event>( incidence ) ) {
    // Adjust date
    KDateTime start = event->dtStart();
    KDateTime end = event->dtEnd();

    int duration = start.daysTo( end );
    start.setDate( dt );
    end.setDate( dt.addDays( duration ) );

    event->setDtStart( start );
    event->setDtEnd( end );

  } if ( const Todo::Ptr todo = dynamic_pointer_cast<Todo>( incidence ) ) {
    KDateTime due = todo->dtDue();
    due.setDate( dt );

    todo->setDtDue( due );
    todo->setHasDueDate( true );
  }
  mChanger->changeIncidence( oldIncidence, itemmove );
  mChanger->endChange( itemmove );
}

void CalendarView::resourcesChanged()
{
  mViewManager->setUpdateNeeded();
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
#include "calendarview.moc"
