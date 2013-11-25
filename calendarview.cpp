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
#include "htmlexportsettings.h"
#include "navigatorbar.h"
#include "views/agendaview/koagendaview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/todoview/kotodoview.h"
#include "kocheckableproxymodel.h"
#include "akonadicollectionview.h"

#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/collectiongeneralpage.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/next/incidenceviewer.h>
#include <calendarsupport/printing/calprinter.h>

#include <incidenceeditor-ng/incidencedefaults.h>
#include <incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditor-ng/incidencedialogfactory.h>

#include <libkdepim/widgets/pimmessagebox.h>
#include <akonadi/calendar/freebusymanager.h>
#include <akonadi/calendar/history.h>
#include <Akonadi/Calendar/IncidenceChanger>
#include <Akonadi/Calendar/CalendarSettings>
#include <Akonadi/Calendar/CalendarClipboard>

#include <pimcommon/acl/collectionaclpage.h>
#include <pimcommon/acl/imapaclattribute.h>

#include <Akonadi/CollectionPropertiesDialog>
#include <Akonadi/Control>
#include <Akonadi/AttributeFactory>
#include <Akonadi/Calendar/TodoPurger>

#include <KCalCore/CalFilter>
#include <KCalCore/FileStorage>
#include <KCalCore/ICalFormat>
#include <KCalCore/VCalFormat>

#include <KCalUtils/ICalDrag>
#include <KCalUtils/Stringify>
#include <KCalUtils/DndFactory>

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

CalendarView::CalendarView( QWidget *parent ) : CalendarViewBase( parent ),
                                                mCheckableProxyModel( 0 ),
                                                mETMCollectionView( 0 )
{
  Akonadi::Control::widgetNeedsAkonadi( this );
  mChanger = new Akonadi::IncidenceChanger( this );
  mChanger->setDefaultCollection( Akonadi::Collection( CalendarSupport::KCalPrefs::instance()->defaultCalendarId() ) );

  mChanger->setDestinationPolicy( static_cast<Akonadi::IncidenceChanger::DestinationPolicy>( KOPrefs::instance()->destination() ) );
  mCalendar = Akonadi::ETMCalendar::Ptr( new Akonadi::ETMCalendar() );
  mCalendar->setObjectName( QLatin1String("KOrg Calendar") );
  mCalendarClipboard = new Akonadi::CalendarClipboard( mCalendar, mChanger, this );
  mITIPHandler = new Akonadi::ITIPHandler( this );
  mITIPHandler->setCalendar( mCalendar );
  connect( mCalendarClipboard, SIGNAL(cutFinished(bool,QString)), SLOT(onCutFinished()) );

  Akonadi::AttributeFactory::registerAttribute<PimCommon::ImapAclAttribute>();

  mViewManager = new KOViewManager( this );
  mDialogManager = new KODialogManager( this );
  mTodoPurger = new Akonadi::TodoPurger( this );
  mTodoPurger->setCalendar( mCalendar );
  mTodoPurger->setIncidenceChager( mChanger );
  connect(mTodoPurger, SIGNAL(todosPurged(bool,int,int)),
          SLOT(onTodosPurged(bool,int,int)));

  mReadOnly = false;
  mSplitterSizesValid = false;

  mCalPrinter = 0;

  mDateNavigator = new DateNavigator( this );
  mDateChecker = new DateChecker( this );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( 0 );

  // create the main layout frames.
  mPanner = new QSplitter( Qt::Horizontal, this );
  mPanner->setObjectName( QLatin1String("CalendarView::Panner") );
  topLayout->addWidget( mPanner );

  mLeftSplitter = new QSplitter( Qt::Vertical, mPanner );
  mLeftSplitter->setObjectName( QLatin1String("CalendarView::LeftFrame") );
  // The GUI checkboxes of "show widget XYZ" are confusing when the QSplitter
  // hides the widget magically. I know I blamed Akonadi for not showing my
  // calendar more than once.
  mLeftSplitter->setChildrenCollapsible( false );

  mDateNavigatorContainer = new DateNavigatorContainer( mLeftSplitter );
  mDateNavigatorContainer->setObjectName( QLatin1String("CalendarView::DateNavigator") );

  mTodoList = new KOTodoView( true/*sidebar*/, mLeftSplitter );
  mTodoList->setObjectName(QLatin1String( "todolist") );

  mEventViewerBox = new KVBox( mLeftSplitter );
  mEventViewerBox->setMargin( 0 );
  mEventViewer = new CalendarSupport::IncidenceViewer( mCalendar.data(), mEventViewerBox );
  mEventViewer->setObjectName( QLatin1String("EventViewer") );

  KVBox *rightBox = new KVBox( mPanner );
  rightBox->layout()->setMargin( 0 );
  mNavigatorBar = new NavigatorBar( rightBox );
  mRightFrame = new QStackedWidget( rightBox );
  mMessageWidget = new CalendarSupport::MessageWidget( rightBox );

  rightBox->setStretchFactor( mRightFrame, 1 );

  mLeftFrame = mLeftSplitter;
  mLeftFrame->installEventFilter( this );

  mChanger->setGroupwareCommunication( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication() );
  connect( mChanger,
           SIGNAL(createFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
           SLOT(slotCreateFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );

  connect( mChanger,
           SIGNAL(deleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)),
           SLOT(slotDeleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)) );

  connect( mChanger,
           SIGNAL(modifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
           SLOT(slotModifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );

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

  KOGlobals::self()->setHolidays( new KHolidays::HolidayRegion( KOPrefs::instance()->mHolidays ) );

  connect( QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(checkClipboard()) );

  connect( mTodoList, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           this, SLOT(processTodoListSelection(Akonadi::Item,QDate)) );
  disconnect( mTodoList, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
              this, SLOT(processMainViewSelection(Akonadi::Item,QDate)) );

  {
    static bool pageRegistered = false;

    if ( !pageRegistered ) {
      Akonadi::CollectionPropertiesDialog::registerPage(
        new CalendarSupport::CollectionGeneralPageFactory );
      Akonadi::CollectionPropertiesDialog::registerPage( new PimCommon::CollectionAclPageFactory );
      pageRegistered = true;
    }
  }

  Akonadi::FreeBusyManager::self()->setCalendar( mCalendar );

  mCalendar->registerObserver( this );
  mDateNavigatorContainer->setCalendar( mCalendar );
  mTodoList->setCalendar( mCalendar );
  mEventViewer->setCalendar( mCalendar.data() );
}

CalendarView::~CalendarView()
{
  mCalendar->unregisterObserver( this );
  mCalendar->setFilter( 0 ); // So calendar doesn't deleted it twice
  qDeleteAll( mFilters );
  qDeleteAll( mExtensions );

  delete mDialogManager;
  delete mViewManager;
  delete mEventViewer;
}

Akonadi::ETMCalendar::Ptr CalendarView::calendar() const
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
    KCalCore::DateList dates = curView->selectedIncidenceDates();
    if ( !dates.isEmpty() ) {
      return dates.first();
    }
  }

  return QDate();
}

QDate CalendarView::startDate()
{
  KCalCore::DateList dates = mDateNavigator->selectedDates();
  return dates.first();
}

QDate CalendarView::endDate()
{
  KCalCore::DateList dates = mDateNavigator->selectedDates();
  return dates.last();
}

void CalendarView::createPrinter()
{
  if ( !mCalPrinter ) {
    mCalPrinter = new CalendarSupport::CalPrinter( this, mCalendar );
    connect( this, SIGNAL(configChanged()), mCalPrinter, SLOT(updateConfig()) );
  }
}

bool CalendarView::saveCalendar( const QString &filename )
{
  // Store back all unsaved data into calendar object
  mViewManager->currentView()->flushView();

  KCalCore::FileStorage storage( mCalendar );
  storage.setFileName( filename );
  storage.setSaveFormat( new KCalCore::ICalFormat );

  return storage.save();
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
  mTodoList->restoreLayout( config, QLatin1String( "Sidebar Todo View" ), true );

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
  mTodoList->saveLayout( config, QLatin1String( "Sidebar Todo View" ) );

  Akonadi::CalendarSettings::self()->writeConfig();
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
    KConfigGroup filterConfig( config, QLatin1String("Filter_") + (*it) );
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
    KConfigGroup filterConfig( config, QLatin1String("Filter_") + filter->name() );
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
                                         QLatin1String("calendarKeepAbsoluteTimes") );
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
    static_cast<Akonadi::IncidenceChanger::DestinationPolicy>(
      KOPrefs::instance()->destination() ) );

  mChanger->setGroupwareCommunication( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication() );
}

void CalendarView::slotCreateFinished( int changeId,
                                       const Akonadi::Item &item,
                                       Akonadi::IncidenceChanger::ResultCode resultCode,
                                       const QString &errorString )
{
  Q_UNUSED( changeId );
  if ( resultCode == Akonadi::IncidenceChanger::ResultCodeSuccess ) {
    changeIncidenceDisplay( item, Akonadi::IncidenceChanger::ChangeTypeCreate );
    updateUnmanagedViews();
    checkForFilteredChange( item );
  } else {
    kError() << "Incidence not added, job reported error: " << errorString;
  }
}

void CalendarView::slotModifyFinished( int changeId,
                                       const Akonadi::Item &item,
                                       Akonadi::IncidenceChanger::ResultCode resultCode,
                                       const QString &errorString )
{
  Q_UNUSED( changeId );
  if ( resultCode != Akonadi::IncidenceChanger::ResultCodeSuccess ) {
    kError() << "Incidence not modified, job reported error: " << errorString;
    return;
  }

  Q_ASSERT( item.isValid() );
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  Q_ASSERT( incidence );
  QSet<KCalCore::IncidenceBase::Field> dirtyFields = incidence->dirtyFields();
  incidence->resetDirtyFields();
  // Record completed todos in journals, if enabled. we should to this here in
  // favor of the todolist. users can mark a task as completed in an editor
  // as well.
  if ( incidence->type() == KCalCore::Incidence::TypeTodo &&
       KOPrefs::instance()->recordTodosInJournals() &&
       ( dirtyFields.contains( KCalCore::Incidence::FieldCompleted ) ) ) {
    KCalCore::Todo::Ptr todo = incidence.dynamicCast<KCalCore::Todo>();
    if ( todo->isCompleted() || todo->recurs() ) {
      QString timeStr = KGlobal::locale()->formatTime( QTime::currentTime() );
      QString description = i18n( "Todo completed: %1 (%2)", incidence->summary(), timeStr );

      KCalCore::Journal::List journals = calendar()->journals( QDate::currentDate() );

      if ( journals.isEmpty() ) {
        KCalCore::Journal::Ptr journal( new KCalCore::Journal );
        journal->setDtStart(
          KDateTime::currentDateTime( CalendarSupport::KCalPrefs::instance()->timeSpec() ) );

        QString dateStr = KGlobal::locale()->formatDate( QDate::currentDate() );
        journal->setSummary( i18n( "Journal of %1", dateStr ) );
        journal->setDescription( description );

        if ( mChanger->createIncidence( journal, item.parentCollection(), this ) == -1 ) {
          kError() << "Unable to add Journal";
          return;
        }

      } else { // journal list is not empty
        Akonadi::Item journalItem = mCalendar->item( journals.first()->uid() );
        KCalCore::Journal::Ptr journal = CalendarSupport::journal( journalItem );
        KCalCore::Journal::Ptr oldJournal( journal->clone() );
        journal->setDescription( journal->description().append( QLatin1Char('\n') + description ) );
        mChanger->modifyIncidence( journalItem, oldJournal, this );

      }
    }
  }

  changeIncidenceDisplay( item, Akonadi::IncidenceChanger::ChangeTypeCreate );
  updateUnmanagedViews();
  checkForFilteredChange( item );
}

void CalendarView::slotDeleteFinished( int changeId,
                                       const QVector<Akonadi::Item::Id> &itemIdList,
                                       Akonadi::IncidenceChanger::ResultCode resultCode,
                                       const QString &errorString )
{
  Q_UNUSED( changeId );
  if ( resultCode == Akonadi::IncidenceChanger::ResultCodeSuccess ) {
    foreach( Akonadi::Item::Id id, itemIdList ) {
      Akonadi::Item item = mCalendar->item( id );
      if ( item.isValid() )
        changeIncidenceDisplay( item, Akonadi::IncidenceChanger::ChangeTypeDelete );
    }
    updateUnmanagedViews();
  } else {
    kError() << "Incidence not deleted, job reported error: " << errorString;
  }
}

void CalendarView::checkForFilteredChange( const Akonadi::Item &item )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  KCalCore::CalFilter *filter = calendar()->filter();
  if ( filter && !filter->filterIncidence( incidence ) ) {
    // Incidence is filtered and thus not shown in the view, tell the
    // user so that he isn't surprised if his new event doesn't show up
    mMessageWidget->setText( i18n( "The item \"%1\" is filtered by your current filter rules, "
                                   "so it will be hidden and not appear in the view.",
                                   incidence->summary() ) );
    mMessageWidget->show();
  }
}

void CalendarView::startMultiModify( const QString &text )
{
  mChanger->startAtomicOperation( text );
}

void CalendarView::endMultiModify()
{
  mChanger->endAtomicOperation();
}

void CalendarView::changeIncidenceDisplay( const Akonadi::Item &item,
                                           Akonadi::IncidenceChanger::ChangeType changeType )
{
  if ( mDateNavigatorContainer->isVisible() ) {
    mDateNavigatorContainer->updateView();
  }

  mDialogManager->updateSearchDialog();

  if ( CalendarSupport::hasIncidence( item ) ) {
    // If there is an event view visible update the display
    mViewManager->currentView()->changeIncidenceDisplay( item, changeType );
  } else {
    mViewManager->currentView()->updateView();
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
  const KCalCore::DateList tmpList = mDateNavigator->selectedDates();
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
  if ( !incidence ) {
    kError() << "Null incidence";
    return;
  }

  mCalendarClipboard->cutIncidence( incidence, Akonadi::CalendarClipboard::AskMode );
}

void CalendarView::edit_copy()
{
  const Akonadi::Item item = selectedIncidence();

  if ( !item.isValid() ) {
    KNotification::beep();
    kError() << "Invalid item";
    return;
  }

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  Q_ASSERT( incidence );
  if ( !mCalendarClipboard->copyIncidence( incidence, Akonadi::CalendarClipboard::AskMode ) )
    kError() << "Error copying incidence";

  checkClipboard();
}

void CalendarView::edit_paste()
{
// If in agenda and month view, use the selected time and date from there.
// In all other cases, use the navigator's selected date.

  QDateTime endDT;
  KDateTime finalDateTime;
  bool useEndTime = false;
  KCalUtils::DndFactory::PasteFlags pasteFlags = 0;

  KOrg::BaseView *curView = mViewManager->currentView();
  KOAgendaView *agendaView = mViewManager->agendaView();
  MonthView *monthView = mViewManager->monthView();

  if ( !curView ) {
    kWarning() << "No view is selected, can't paste";
    return;
  }

  if ( curView == agendaView && agendaView->selectionStart().isValid() ) {
    const QDate date = agendaView->selectionStart().date();
    endDT = agendaView->selectionEnd();
    useEndTime = !agendaView->selectedIsSingleCell();
    if ( agendaView->selectedIsAllDay() ) {
      finalDateTime = KDateTime( date );
    } else {
      finalDateTime = KDateTime( date, agendaView->selectionStart().time() );
    }
  } else if ( curView == monthView && monthView->selectionStart().isValid() ) {
    finalDateTime = KDateTime( monthView->selectionStart().date() );
    pasteFlags = KCalUtils::DndFactory::FlagPasteAtOriginalTime;
  } else if ( !mDateNavigator->selectedDates().isEmpty() &&
              curView->supportsDateNavigation() ) {
    // default to the selected date from the navigator
    finalDateTime = KDateTime( mDateNavigator->selectedDates().first() );
    pasteFlags = KCalUtils::DndFactory::FlagPasteAtOriginalTime;
  }

  if ( !finalDateTime.isValid() && curView->supportsDateNavigation() ) {
    KMessageBox::sorry(
      this,
      i18n( "Paste failed: unable to determine a valid target date." ) );
    return;
  }

  KCalUtils::DndFactory factory( mCalendar );

  KCalCore::Incidence::List pastedIncidences = factory.pasteIncidences( finalDateTime, pasteFlags );
  KCalCore::Incidence::List::Iterator it;

  for ( it = pastedIncidences.begin(); it != pastedIncidences.end(); ++it ) {
    // FIXME: use a visitor here
    if ( ( *it )->type() == KCalCore::Incidence::TypeEvent ) {
      KCalCore::Event::Ptr pastedEvent = ( *it ).staticCast<KCalCore::Event>();
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
      mChanger->createIncidence( KCalCore::Event::Ptr( pastedEvent->clone() ),
                                 Akonadi::Collection(), this );
    } else if ( ( *it )->type() == KCalCore::Incidence::TypeTodo ) {
      KCalCore::Todo::Ptr pastedTodo = ( *it ).staticCast<KCalCore::Todo>();
      Akonadi::Item _selectedTodoItem = selectedTodo();

      // if we are cutting a hierarchy only the root
      // should be son of _selectedTodo
      KCalCore::Todo::Ptr _selectedTodo = CalendarSupport::todo( _selectedTodoItem );
      if ( _selectedTodo && pastedTodo->relatedTo().isEmpty() ) {
        pastedTodo->setRelatedTo( _selectedTodo->uid() );
      }

      // When pasting multiple incidences, don't ask which collection to use, for each one
      mChanger->createIncidence( KCalCore::Todo::Ptr( pastedTodo->clone() ),
                                 Akonadi::Collection(), this );
    } else if ( ( *it )->type() == KCalCore::Incidence::TypeJournal ) {
      // When pasting multiple incidences, don't ask which collection to use, for each one
      mChanger->createIncidence( KCalCore::Incidence::Ptr( ( *it )->clone() ),
                                 Akonadi::Collection(), this );
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

IncidenceEditorNG::IncidenceDialog *CalendarView::incidenceDialog(const Akonadi::Item &item)
{
    IncidenceEditorNG::IncidenceDialog *dialog = mDialogManager->createDialog(item);
    connect(dialog, SIGNAL(incidenceCreated(Akonadi::Item)), SLOT(handleIncidenceCreated(Akonadi::Item)));
    return dialog;
}

IncidenceEditorNG::IncidenceDialog *CalendarView::newEventEditor( const KCalCore::Event::Ptr &event )
{
  Akonadi::Item item;
  item.setPayload( event );
  IncidenceEditorNG::IncidenceDialog *dialog = incidenceDialog( item );

  dialog->load( item );

  mDialogManager->connectTypeAhead(
    dialog, qobject_cast<KOEventView*>( viewManager()->currentView() ) );

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

  KCalCore::Event::Ptr event( new KCalCore::Event );
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

  KCalCore::Event::Ptr event( new KCalCore::Event );
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
  Akonadi::Collection defaultCol = defaultCollection( KCalCore::Todo::todoMimeType() );

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

  KCalCore::Todo::Ptr todo( new KCalCore::Todo );
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

  KCalCore::Todo::Ptr todo( new KCalCore::Todo );
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

  if ( mViewManager->currentView()->isEventView() ) {
    QDateTime startDt;
    QDateTime endDt;
    bool allDay = true;
    dateTimesForNewEvent( startDt, endDt, allDay );

    defaults.setStartDateTime( KDateTime( startDt ) );
    defaults.setEndDateTime( KDateTime( endDt ) );
  }

  KCalCore::Journal::Ptr journal( new KCalCore::Journal );
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

  KCalCore::Journal::Ptr journal( new KCalCore::Journal );
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

  KCalCore::Todo::Ptr todo( new KCalCore::Todo );
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

  KCalCore::Todo::Ptr todo( new KCalCore::Todo );
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
  KCalCore::Incidence::Ptr incidence( format.fromString( ical ) );
  return addIncidence( incidence );
}

bool CalendarView::addIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  return incidence ? mChanger->createIncidence( incidence, Akonadi::Collection(), this ) != -1 : false;
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
  const KCalCore::Incidence::Ptr inc = CalendarSupport::incidence( item );

  if ( !inc || inc->relatedTo().isEmpty() ) {
    kDebug() << "Refusing to unparent this to-do" << inc;
    return false;
  }

  KCalCore::Incidence::Ptr oldInc( inc->clone() );
  inc->setRelatedTo( QString() );
  mChanger->modifyIncidence( item, oldInc, this );

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
  const KCalCore::Incidence::Ptr inc = CalendarSupport::incidence( item );

  Akonadi::Item::List subIncs = mCalendar->childItems( item.id() );

  if ( !inc || subIncs.isEmpty() ) {
    kDebug() << "Refusing to  make children independent" << inc;
    return false;
  }
  startMultiModify ( i18n( "Make sub-to-dos independent" ) );

  foreach ( const Akonadi::Item &item, subIncs ) {
    incidence_unsub( item );
  }

  endMultiModify();
  return true;
}

bool CalendarView::deleteIncidence( Akonadi::Item::Id id, bool force )
{
  Akonadi::Item item = mCalendar->item( id );
  if ( !CalendarSupport::hasIncidence( item ) ) {
    kError() << "CalendarView::deleteIncidence(): Item does not contain incidence.";
    return false;
  }
  return deleteIncidence( item, force );
}

void CalendarView::toggleAlarm( const Akonadi::Item &item )
{
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence ) {
    kError() << "Null incidence";
    return;
  }
  KCalCore::Incidence::Ptr oldincidence( incidence->clone() );

  KCalCore::Alarm::List alarms = incidence->alarms();
  KCalCore::Alarm::List::ConstIterator it;
  for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
    (*it)->toggleAlarm();
  }
  if ( alarms.isEmpty() ) {
    // Add an alarm if it didn't have one
    KCalCore::Alarm::Ptr alm = incidence->newAlarm();
    alm->setType( KCalCore::Alarm::Display );
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
    if ( incidence->type() == KCalCore::Incidence::TypeEvent ) {
      alm->setStartOffset( KCalCore::Duration( -duration ) );
    } else {
      alm->setEndOffset( KCalCore::Duration( -duration ) );
    }
  }
  mChanger->startAtomicOperation( i18n( "Toggle Reminder" ) );
  mChanger->modifyIncidence( item, oldincidence, this );
  mChanger->endAtomicOperation();
}

void CalendarView::toggleTodoCompleted( const Akonadi::Item &todoItem )
{
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( todoItem );

  if ( !incidence ) {
    kError() << "Null incidence";
    return;
  }
  if ( incidence->type() != KCalCore::Incidence::TypeTodo ) {
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

  mChanger->startAtomicOperation( i18n( "Toggle To-do Completed") );
  mChanger->modifyIncidence( todoItem, oldtodo, this );
  mChanger->endAtomicOperation();
}

void CalendarView::copyIncidenceToResource( const Akonadi::Item &item, const QString &resourceId )
{
#ifdef AKONADI_PORT_DISABLED
  if ( !incidence ) {
    kError() << "Null incidence";
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
  KCalCore::Incidence::Ptr newInc;
  if ( incidence->type() == KCalCore::Incidence::TypeEvent ) {
    KCalCore::Event::Ptr nEvent( static_cast<KCalCore::Event::Ptr >( incidence )->clone() );
    nEvent->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nEvent;
  } else if ( incidence->type() == KCalCore::Incidence::TypeTodo ) {
    KCalCore::Todo::Ptr nTodo( static_cast<KCalCore::Todo::Ptr >( incidence )->clone() );
    nTodo->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nTodo;
  } else if ( incidence->type() == KCalCore::Incidence::TypeJournal ) {
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
    kError() << "Null incidence";
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
  KCalCore::Incidence *newInc;
  if ( incidence->type() == KCalCore::Incidence::TypeEvent ) {
    KCalCore::Event::Ptr nEvent = static_cast<KCalCore::Event::Ptr >( incidence )->clone();
    nEvent->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nEvent;
  } else if ( incidence->type() == KCalCore::Incidence::TypeTodo ) {
    KCalCore::Todo::Ptr nTodo = static_cast<KCalCore::Todo::Ptr >( incidence )->clone();
    nTodo->setUid( KCalCore::CalFormat::createUniqueId() );
    newInc = nTodo;
  } else if ( incidence->type() == KCalCore::Incidence::TypeJournal ) {
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
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( !incidence ) {
    kError() << "Null incidence";
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
    dissociateOccurrence( item, date, false );
  } else if ( doFuture ) {
    dissociateOccurrence( item, date, true );
  }
}
void CalendarView::dissociateOccurrence( const Akonadi::Item &item, const QDate &date, bool thisAndFuture )
{
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( thisAndFuture ) {
    startMultiModify( i18n( "Dissociate future occurrences" ) );
  } else {
    startMultiModify( i18n( "Dissociate occurrence" ) );
  }
  KDateTime occurrenceDate( incidence->dtStart() );
  occurrenceDate.setDate(date);
  kDebug() << "create exception: " << occurrenceDate;
  KCalCore::Incidence::Ptr newInc( KCalCore::Calendar::createException(
      incidence, occurrenceDate, thisAndFuture) );
  if ( newInc ) {
    mChanger->createIncidence( newInc, item.parentCollection(), this );
  } else {
    if ( thisAndFuture ) {
      KMessageBox::sorry(
        this,
        i18n( "Dissociating the future occurrences failed." ),
        i18n( "Dissociating Failed" ) );
    } else {
      KMessageBox::sorry(
        this,
        i18n( "Dissociating the occurrence failed." ),
        i18n( "Dissociating Failed" ) );
    }
  }
  endMultiModify();
}

void CalendarView::schedule_publish( const Akonadi::Item &item )
{
  Akonadi::Item selectedItem = item;
  if ( !item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    selectedItem = selectedIncidence();
  }

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( selectedItem );
  if ( incidence )
    mITIPHandler->publishInformation( incidence, this );
}

void CalendarView::schedule_request( const Akonadi::Item &incidence )
{
  schedule( KCalCore::iTIPRequest, incidence );
}

void CalendarView::schedule_refresh( const Akonadi::Item &incidence )
{
  schedule( KCalCore::iTIPRefresh, incidence );
}

void CalendarView::schedule_cancel( const Akonadi::Item &incidence )
{
  schedule( KCalCore::iTIPCancel, incidence );
}

void CalendarView::schedule_add( const Akonadi::Item &incidence )
{
  schedule( KCalCore::iTIPAdd, incidence );
}

void CalendarView::schedule_reply( const Akonadi::Item &incidence )
{
  schedule( KCalCore::iTIPReply, incidence );
}

void CalendarView::schedule_counter( const Akonadi::Item &incidence )
{
  schedule( KCalCore::iTIPCounter, incidence );
}

void CalendarView::schedule_declinecounter( const Akonadi::Item &incidence )
{
  schedule( KCalCore::iTIPDeclineCounter, incidence );
}

void CalendarView::schedule_forward( const Akonadi::Item &item )
{
  Akonadi::Item selectedItem = item;
  if ( !item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    selectedItem = selectedIncidence();
  }

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( selectedItem );

  if ( incidence )
    mITIPHandler->sendAsICalendar( incidence, this );
}

void CalendarView::mailFreeBusy( int daysToPublish )
{
  Akonadi::FreeBusyManager::self()->mailFreeBusy( daysToPublish, this );
}

void CalendarView::uploadFreeBusy()
{
  Akonadi::FreeBusyManager::self()->publishFreeBusy( this );
}

void CalendarView::schedule( KCalCore::iTIPMethod method, const Akonadi::Item &item )
{
  Akonadi::Item selectedItem = item;
  if ( !item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    selectedItem = selectedIncidence();
  }

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( selectedItem );

  if ( incidence )
    mITIPHandler->sendiTIPMessage( method, incidence, this );
}

void CalendarView::openAddressbook()
{
  KRun::runCommand( QLatin1String("kaddressbook"), topLevelWidget() );
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

  CalendarSupport::CalPrinter::PrintType printType = CalendarSupport::CalPrinter::Month;

  KCalCore::Incidence::List selectedIncidences;
  if ( currentView ) {
    printType = currentView->printType();
    Akonadi::Item::List selectedViewIncidences = currentView->selectedIncidences();
    foreach ( const Akonadi::Item &item, selectedViewIncidences ) {
      if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
        selectedIncidences.append( item.payload<KCalCore::Incidence::Ptr>() );
      }
    }
  }

  KCalCore::DateList tmpDateList = mDateNavigator->selectedDates();
  mCalPrinter->print( printType, tmpDateList.first(), tmpDateList.last(), selectedIncidences );
}

void CalendarView::printPreview()
{
  createPrinter();

  KOrg::BaseView *currentView = mViewManager->currentView();

  CalendarSupport::CalPrinter::PrintType printType = CalendarSupport::CalPrinter::Month;

  KCalCore::Incidence::List selectedIncidences;
  if ( currentView ) {
    printType = currentView->printType();
    Akonadi::Item::List selectedViewIncidences = currentView->selectedIncidences();
    foreach ( const Akonadi::Item &item, selectedViewIncidences ) {
      if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
        selectedIncidences.append( item.payload<KCalCore::Incidence::Ptr>() );
      }
    }
  }

  KCalCore::DateList tmpDateList = mDateNavigator->selectedDates();
  mCalPrinter->print( printType, tmpDateList.first(), tmpDateList.last(),
                      selectedIncidences, true );
}

void CalendarView::exportWeb()
{
  KOrg::HTMLExportSettings *settings = new KOrg::HTMLExportSettings( QLatin1String("KOrganizer") );
  Q_ASSERT(settings);
  // Manually read in the config, because parameterized kconfigxt objects don't
  // seem to load the config theirselves
  settings->readConfig();
  ExportWebDialog *dlg = new ExportWebDialog( settings, this );
  connect( dlg, SIGNAL(exportHTML(KOrg::HTMLExportSettings*)),
           this, SIGNAL(exportHTML(KOrg::HTMLExportSettings*)) );
  dlg->show();
}

void CalendarView::exportICalendar()
{
  QString filename =
    KFileDialog::getSaveFileName( KUrl( QLatin1String("icalout.ics") ), i18n( "*.ics|iCalendars" ), this );
  if ( !filename.isEmpty() ) {
    // Force correct extension
    if ( filename.right( 4 ) != QLatin1String(".ics") ) {
      filename += QLatin1String(".ics");
    }

    if ( QFile( filename ).exists() ) {
      if ( KMessageBox::No == KMessageBox::warningYesNo(
             this,
             i18n( "Do you want to overwrite %1?", filename ) ) ) {
        return;
      }
    }
    KCalCore::ICalFormat *format = new KCalCore::ICalFormat;

    KCalCore::FileStorage storage( mCalendar, filename, format );
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
      QLatin1String( "dontaskVCalExport" ),
      KMessageBox::Notify );
    if ( result != KMessageBox::Continue ) {
      return;
    }
  }

  QString filename = KFileDialog::getSaveFileName( KUrl( QLatin1String("vcalout.vcs") ),
                                                   i18n( "*.vcs|vCalendars" ), this );
  if ( !filename.isEmpty() ) {
    // Force correct extension
    if ( filename.right( 4 ) != QLatin1String(".vcs") ) {
      filename += QLatin1String(".vcs");
    }
    if ( QFile( filename ).exists() ) {
      if ( KMessageBox::No == KMessageBox::warningYesNo(
             this,
             i18n( "Do you want to overwrite %1?", filename ) ) ) {
        return;
      }
    }
    KCalCore::VCalFormat *format = new KCalCore::VCalFormat;

    KCalCore::FileStorage storage( mCalendar, filename, format );
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
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
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

  bool todo = false;
  bool subtodo = false;

  const bool organizerEvents =
    CalendarSupport::KCalPrefs::instance()->thatIsMe( incidence->organizer()->email() );
  const bool groupEvents = incidence->attendeeByMails( CalendarSupport::KCalPrefs::instance()->allEmails() );

  if ( incidence->type() == KCalCore::Incidence::TypeTodo ) {
    todo = true;
    subtodo = ( incidence->relatedTo() != QString() );
  }
  emit todoSelected( todo );
  emit subtodoSelected( subtodo );
  emit organizerEventsSelected( organizerEvents );
  emit groupEventsSelected( groupEvents );
}

void CalendarView::checkClipboard()
{
  emit pasteEnabled( mCalendarClipboard->pasteAvailable() );
}

void CalendarView::showDates( const KCalCore::DateList &selectedDates, const QDate &preferredMonth )
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
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( incidence ) {
    return;
  }

  incidence->setOrganizer(
    KCalCore::Person::Ptr( new KCalCore::Person(
                             CalendarSupport::KCalPrefs::instance()->fullName(),
                             CalendarSupport::KCalPrefs::instance()->email() ) ) );
  incidence->recreate();
  incidence->setReadOnly( false );

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
  if ( !mETMCollectionView ) {
    mETMCollectionView = qobject_cast<AkonadiCollectionView*>( extension );
  }
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
  Akonadi::Item item = mCalendar->item( id );
  return editIncidence( item );
}

bool CalendarView::showIncidence( Akonadi::Item::Id id )
{
  Akonadi::Item item = mCalendar->item( id );
  if ( !CalendarSupport::hasIncidence( item ) ) {
    return false;
  }
  showIncidence( item );
  return true;
}

bool CalendarView::showIncidenceContext( Akonadi::Item::Id id )
{
  Akonadi::Item item = mCalendar->item( id );
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
  KOEventViewerDialog *eventViewer = new KOEventViewerDialog( mCalendar.data(), this );
  eventViewer->setIncidence( item, QDate() );
  // Disable the Edit button for read-only Incidences.
  if ( !mCalendar->hasRight( item, Akonadi::Collection::CanChangeItem ) ) {
    eventViewer->enableButton( KDialog::User1, false );
  }

  eventViewer->show();
}

void CalendarView::showIncidenceContext( const Akonadi::Item &item )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
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
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence ) {
    kError() << "Null incidence";
    KNotification::beep();
    return false;
  }

  if ( !mCalendar->hasRight( item, Akonadi::Collection::CanChangeItem ) ) {
    showIncidence( item );
    return true;
  }


  IncidenceEditorNG::IncidenceDialog *dialog = incidenceDialog( item );

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
  Akonadi::Item::List subTodos = mCalendar->childItems( todoItem.id() );
  foreach ( const Akonadi::Item &item, subTodos ) {
    if ( CalendarSupport::hasTodo( item ) ) {
      deleteSubTodosIncidence ( item );
    }
  }

  if ( !mChanger->deletedRecently( todoItem.id() ) ) {
    mChanger->deleteIncidence( todoItem, this );
  }
}

void CalendarView::deleteTodoIncidence ( const Akonadi::Item &todoItem, bool force )
{
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( !todo ) {
    return ;
  }

  // it a simple todo, ask and delete it.
  if ( mCalendar->childItems( todoItem.id() ).isEmpty() ) {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( todoItem ) == KMessageBox::Continue );
    }
    if ( doDelete && !mChanger->deletedRecently( todoItem.id() ) ) {
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
  // Delete only the father
  if ( km == KMessageBox::Yes ) {
    startMultiModify( i18n( "Delete parent to-do" ) );
    makeChildrenIndependent( todoItem );
    if ( !mChanger->deletedRecently( todoItem.id() ) ) {
      mChanger->deleteIncidence( todoItem, this );
    }
  } else if ( km == KMessageBox::No ) {
    startMultiModify( i18n( "Delete parent to-do and sub-to-dos" ) );
    // Delete all
    // we have to hide the delete confirmation for each itemDate
    deleteSubTodosIncidence ( todoItem );
  }
  endMultiModify();
}

bool CalendarView::deleteIncidence( const Akonadi::Item &item, bool force )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence ) {
    if ( !force ) {
      kError() << "Null incidence";
      KNotification::beep();
    }
    kError() << "CalendarView::deleteIncidence(): Unable do delete, incidence is null.";
    return false;
  }

  if ( mChanger->deletedRecently( item.id() ) ) {
    // it was deleted already but the etm wasn't notified yet
    kWarning() << "CalendarView::deleteIncidence(): item with id" << item.id()
               << "was deleted recently, skipping";
    return true;
  }

  if ( !mCalendar->hasRight( item, Akonadi::Collection::CanDeleteItem ) ) {
    if ( !force ) {
      KMessageBox::information(
        this,
        i18n( "The item \"%1\" is marked read-only "
              "and cannot be deleted; it probably "
              "belongs to a read-only calendar.",
              incidence->summary() ),
        i18n( "Removing not possible" ),
        QLatin1String("deleteReadOnlyIncidence") );
    }
    kWarning() << "CalendarView::deleteIncidence(): No rights to delete item";
    return false;
  }

  //If it is a todo, there are specific delete function

  if ( incidence && incidence->type() == KCalCore::Incidence::TypeTodo ) {
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
    KCalCore::Incidence::Ptr oldIncidence( incidence->clone() );
    switch( km ) {
    case KMessageBox::Ok: // Continue // all
    case KMessageBox::Continue:
      mChanger->deleteIncidence( item, this );
      break;

    case KMessageBox::Yes: // just this one
      incidence->recurrence()->addExDate( itemDate );
      mChanger->modifyIncidence( item, oldIncidence, this );

      break;
    case KMessageBox::No: // all future items
      KCalCore::Recurrence *recur = incidence->recurrence();
      recur->setEndDate( itemDate.addDays( -1 ) );
      mChanger->modifyIncidence( item, oldIncidence, this );
      break;
    }
  } else {
    bool doDelete = true;
    if ( !force && KOPrefs::instance()->mConfirm ) {
      doDelete = ( msgItemDelete( item ) == KMessageBox::Continue );
    }
    if ( doDelete ) {
      mChanger->deleteIncidence( item, this );
      processIncidenceSelection( Akonadi::Item(), QDate() );
    }
  }
  return true;
}


void CalendarView::purgeCompleted()
{
  if ( checkedCollections().isEmpty() ) {
    showMessage( i18n( "All calendars are unchecked in the Calendar Manager. No to-do was purged." ), KMessageWidget::Warning );
    return;
  }

  if ( mCalendar->rawTodos().isEmpty() ) {
    showMessage( i18n( "There are no completed to-dos to purge." ), KMessageWidget::Information );
    return;
  }

  int result = KMessageBox::warningContinueCancel(
    this,
    i18n( "Delete all completed to-dos from checked calendars?" ),
    i18n( "Purge To-dos" ),
    KGuiItem( i18n( "Purge" ) ) );

  if ( result == KMessageBox::Continue ) {
      mTodoPurger->purgeCompletedTodos();
  }
}

void CalendarView::warningChangeFailed( const Akonadi::Item &item )
{
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
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
  QStringList allCats( CalendarSupport::categories( mCalendar->rawIncidences() ) );
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
  if ( !CalendarSupport::hasIncidence( itemadd ) ) {
    KMessageBox::sorry(
      this,
      i18n( "Unable to copy the item to %1.", dt.toString() ),
      i18n( "Copying Failed" ) );
    return;
  }
  Akonadi::Item item = mCalendar->item( itemadd.id() );
  if ( !item.isValid() ) {
    item = itemadd;
  }
  // Create a copy of the incidence, since the incadd doesn't belong to us.
  KCalCore::Incidence::Ptr incidence( CalendarSupport::incidence( item )->clone() );
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
  }

  mChanger->createIncidence( incidence, Akonadi::Collection(), this );
}

void CalendarView::moveIncidenceTo( const Akonadi::Item &itemmove, const QDate &dt )
{
  if ( !CalendarSupport::hasIncidence( itemmove ) ) {
    KMessageBox::sorry(
      this,
      i18n( "Unable to move the item to  %1.", dt.toString() ),
      i18n( "Moving Failed" ) );
    return;
  }
  Akonadi::Item item = mCalendar->item( itemmove.id() );
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
  }
  mChanger->modifyIncidence( itemmove, oldIncidence, this );
}

void CalendarView::resourcesChanged()
{
  mViewManager->addChange( EventViews::EventView::ResourcesChanged );
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

Akonadi::Collection CalendarView::defaultCollection( const QLatin1String &mimeType ) const
{
  // 1. Try the view collection ( used in multi-agenda view )
  Akonadi::Collection collection = mCalendar->collection( mViewManager->currentView()->collectionId() );
  bool supportsMimeType = collection.contentMimeTypes().contains( mimeType ) || mimeType == "";
  bool hasRights = collection.rights() & Akonadi::Collection::CanCreateItem;
  if ( collection.isValid() && supportsMimeType && hasRights )
    return collection;

  // 2. Try the selected collection
  collection = selectedCollection();
  supportsMimeType = collection.contentMimeTypes().contains( mimeType ) || mimeType == "";
  hasRights = collection.rights() & Akonadi::Collection::CanCreateItem;
  if ( collection.isValid() && supportsMimeType && hasRights )
    return collection;

  // 3. Try the checked collections
  Akonadi::Collection::List collections = checkedCollections();
  foreach( const Akonadi::Collection &checkedCollection, collections ) {
    supportsMimeType = checkedCollection.contentMimeTypes().contains( mimeType ) || mimeType == "";
    hasRights = checkedCollection.rights() & Akonadi::Collection::CanCreateItem;
    if ( checkedCollection.isValid() && supportsMimeType && hasRights )
      return checkedCollection;
  }

  // 4. Try the configured default collection
  collection = mCalendar->collection( CalendarSupport::KCalPrefs::instance()->defaultCalendarId() );
  supportsMimeType = collection.contentMimeTypes().contains( mimeType ) || mimeType == "";
  hasRights = collection.rights() & Akonadi::Collection::CanCreateItem;
  if ( collection.isValid() && supportsMimeType && hasRights )
    return collection;


  // 5. Return a invalid collection, the editor will use the first one in the combo
  return Akonadi::Collection();
}

IncidenceEditorNG::IncidenceDialog *CalendarView::createIncidenceEditor(
  const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  IncidenceEditorNG::IncidenceDialog *dialog = incidenceDialog( item );
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  Q_ASSERT( incidence );

  if ( collection.isValid() ) {
    dialog->selectCollection( collection );
  } else {
    dialog->selectCollection( defaultCollection( incidence->mimeType() ) );
  }

  return dialog;
}

Akonadi::History *CalendarView::history() const
{
  return mChanger->history();
}

void CalendarView::onCutFinished()
{
  checkClipboard();
}

void CalendarView::setCheckableProxyModel( KOCheckableProxyModel *model )
{
  if ( mCheckableProxyModel )
    mCheckableProxyModel->disconnect( this );

  mCheckableProxyModel = model;
  connect( model, SIGNAL(aboutToToggle(bool)), SLOT(onCheckableProxyAboutToToggle(bool)) );
  connect( model, SIGNAL(toggled(bool)), SLOT(onCheckableProxyToggled(bool)) );
}

void CalendarView::onCheckableProxyAboutToToggle( bool newState )
{
  // Someone unchecked a collection, save the view state now.
  if ( !newState ) {
    mTodoList->saveViewState();
    KOTodoView *todoView = mViewManager->todoView();
    if ( todoView )
      todoView->saveViewState();
  }
}

void CalendarView::onCheckableProxyToggled( bool newState )
{
  // Someone checked a collection, restore the view state now.
  if ( newState ) {
    mTodoList->restoreViewState();
    KOTodoView *todoView = mViewManager->todoView();
    if ( todoView )
      todoView->restoreViewState();
  }
}

void CalendarView::onTodosPurged(bool success, int numDeleted, int numIgnored)
{
    QString message;
    KMessageWidget::MessageType type = KMessageWidget::Information;
    if (success) {
        if (numDeleted == 0 && numIgnored > 0) {
            type = KMessageWidget::Warning;
            message = i18n("0 completed to-dos were purged.") + QLatin1Char('\n') +
                    i18np("%1 to-do was ignored because it has uncompleted or read-only children.",
                          "%1 to-dos were ignored because they have uncompleted or read-only children.", numIgnored);
        } else if (numDeleted > 0 && numIgnored == 0) {
            message = i18np("%1 completed to-do was purged.", "%1 completed to-dos were purged.", numDeleted);
        } else if (numDeleted == 0 && numIgnored == 0) {
            message = i18n("There are no completed to-dos to purge.");
        } else {
            type = KMessageWidget::Warning;
            message = i18np("%1 completed to-do was purged.", "%1 completed to-dos were purged.", numDeleted) + QLatin1Char('\n') +
                    i18np("%1 to-do was ignored because it has uncompleted or read-only children.",
                          "%1 to-dos were ignored because they have uncompleted or read-only children.", numIgnored);
        }
    } else {
        message = i18n("An error occurred while purging completed to-dos: %1", mTodoPurger->lastError());
        type = KMessageWidget::Error;
    }

    showMessage(message, type);
}

void CalendarView::showMessage(const QString &message, KMessageWidget::MessageType type)
{
    mMessageWidget->setText(message);
    mMessageWidget->setMessageType(type);
    mMessageWidget->show();
}

Akonadi::Collection CalendarView::selectedCollection() const
{
  return mETMCollectionView ? mETMCollectionView->selectedCollection() : Akonadi::Collection();
}

Akonadi::Collection::List CalendarView::checkedCollections() const
{
  Akonadi::Collection::List collections;
  if ( mETMCollectionView )
    collections = mETMCollectionView->checkedCollections();

  // If the default calendar is here, it should be first.
  int count = collections.count();
  Akonadi::Collection::Id id = CalendarSupport::KCalPrefs::instance()->defaultCalendarId();
  for( int i=0; i<count; ++i ) {
    if ( id == collections[i].id() ) {
      collections.move( i, 0 );
      break;
    }
  }

  return collections;
}

void CalendarView::handleIncidenceCreated(const Akonadi::Item &item)
{
    Akonadi::Collection collection = item.parentCollection();
    if (!collection.isValid()) {
        kWarning() << "Item was creating in an invalid collection !? item id=" << item.id();
        return;
    }

    const bool collectionIsChecked = mETMCollectionView->isChecked(collection);

    if (!collectionIsChecked) {
        QString message;
        if (mETMCollectionView->isVisible()) {
            message = i18n("You created an incidence in a calendar that is currently filtered out.\n"
                           "On the left sidebar, enable it in the calendar manager to see the incidence.");
        } else {
            message = i18n("You created an incidence in a calendar that is currently filtered out.\n"
                           "You can enable it through the calendar manager (Settings->Sidebar->Show Calendar Manager)");
        }

        mMessageWidget->setText(message);
        mMessageWidget->setMessageType(KMessageWidget::Information);
        mMessageWidget->show();
    }
}

