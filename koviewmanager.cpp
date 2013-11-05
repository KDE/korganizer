/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koviewmanager.h"
#include "actionmanager.h"
#include "calendarview.h"
#include "datenavigator.h"
#include "koglobals.h"
#include "koprefs.h"
#include "navigatorbar.h"
#include "korganizer/mainwindow.h"
#include "views/agendaview/koagendaview.h"
#include "views/journalview/kojournalview.h"
#include "views/listview/kolistview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/timelineview/kotimelineview.h"
#include "views/timespentview/kotimespentview.h"
#include "views/todoview/kotodoview.h"
#include "views/whatsnextview/kowhatsnextview.h"

#include <KActionCollection>
#include <KMessageBox>
#include <KTabWidget>

#include <QAction>
#include <QStackedWidget>

KOViewManager::KOViewManager( CalendarView *mainView )
  : QObject(), mMainView( mainView )
{
  mCurrentView = 0;
  mLastEventView = 0;
  mWhatsNextView = 0;
  mTodoView = 0;
  mAgendaView = 0;
  mAgendaSideBySideView = 0;
  mListView = 0;
  mJournalView = 0;
  mTimelineView = 0;
  mAgendaViewTabs = 0;
  mAgendaViewTabIndex = 0;
  mTimeSpentView = 0;
  mMonthView = 0;
  mRangeMode = NO_RANGE;
}

KOViewManager::~KOViewManager()
{
}

KOrg::BaseView *KOViewManager::currentView()
{
  return mCurrentView;
}

void KOViewManager::readSettings( KConfig *config )
{
  KConfigGroup generalConfig( config, "General" );
  const QString view = generalConfig.readEntry( "Current View" );

  if ( view == QLatin1String( "WhatsNext" ) ) {
    showWhatsNextView();
  } else if ( view == QLatin1String( "OldMonth" ) ) {
    // the oldmonth view is gone, so we assume the new month view
    showMonthView();
  } else if ( view == QLatin1String( "List" ) ) {
    showListView();
    mListView->readSettings( config );
  } else if ( view == QLatin1String( "Journal" ) ) {
    showJournalView();
  } else if ( view == QLatin1String( "Todo" ) ) {
    showTodoView();
  } else if ( view == QLatin1String( "Timeline" ) ) {
    showTimeLineView();
  } else if ( view == QLatin1String( "TimeSpent" ) ) {
    showTimeSpentView();
  } else if ( view == QLatin1String( "Month" ) ) {
    showMonthView();
  } else {
    showAgendaView();
  }

  mRangeMode = RangeMode( generalConfig.readEntry( "Range Mode", int( OTHER_RANGE ) ) );

  switch ( mRangeMode ) {
    case WORK_WEEK_RANGE:
      selectWorkWeek();
      break;
    case WEEK_RANGE:
      selectWeek();
      break;
    case NEXTX_RANGE:
      selectNextX();
      break;
    case DAY_RANGE:
      selectDay();
      break;
    case NO_RANGE:
    default:
      // Someone has been playing with the config file.
      mRangeMode = OTHER_RANGE;
  }
}

void KOViewManager::writeSettings( KConfig *config )
{
  KConfigGroup generalConfig( config, "General" );
  QString view;
  if ( mCurrentView == mWhatsNextView ) {
    view = QLatin1String( "WhatsNext" );
  } else if ( mCurrentView == mListView ) {
    view = QLatin1String( "List" );
  } else if ( mCurrentView == mJournalView ) {
    view = QLatin1String( "Journal" );
  } else if ( mCurrentView == mTodoView ) {
    view = QLatin1String( "Todo" );
  } else if ( mCurrentView == mTimelineView ) {
    view = QLatin1String( "Timeline" );
  } else if ( mCurrentView == mTimeSpentView ) {
    view = QLatin1String( "TimeSpent" );
  } else if ( mCurrentView == mMonthView ) {
    view = QLatin1String( "Month" );
  } else {
    view = QLatin1String( "Agenda" );
  }

  generalConfig.writeEntry( "Current View", view );

  if ( mAgendaView ) {
    mAgendaView->writeSettings( config );
  }
  if ( mListView ) {
    mListView->writeSettings( config );
  }
  if ( mTodoView ) {
    mTodoView->saveLayout( config, QLatin1String("Todo View") );
  }

  // write out custom view configuration
  Q_FOREACH ( KOrg::BaseView *const view, mViews ) {
    KConfigGroup group = KGlobal::config()->group( view->identifier() );
    view->saveConfig( group );
  }

  generalConfig.writeEntry( "Range Mode", int( mRangeMode ) );
}

void KOViewManager::showView( KOrg::BaseView *view )
{
  if ( view == mCurrentView ) {
    return;
  }

  mCurrentView = view;
  mMainView->updateHighlightModes();

  if ( mCurrentView && mCurrentView->isEventView() ) {
    mLastEventView = mCurrentView;
  }

  if ( mAgendaView ) {
    mAgendaView->deleteSelectedDateTime();
  }

  raiseCurrentView();
  mMainView->processIncidenceSelection( Akonadi::Item(), QDate() );
  mMainView->updateView();
  mMainView->adaptNavigationUnits();
  KOrg::MainWindow *w = ActionManager::findInstance( KUrl() );

  if ( w ) {
    KActionCollection *ac = w->getActionCollection();
    if ( ac ) {
      if ( QAction *action = ac->action( QLatin1String("configure_view") ) ) {
        action->setEnabled( view->hasConfigurationDialog() );
      }

      QStringList zoomActions;
      QStringList rangeActions;

      zoomActions << QLatin1String("zoom_in_horizontally")
                  << QLatin1String("zoom_out_horizontally")
                  << QLatin1String("zoom_in_vertically")
                  << QLatin1String("zoom_out_vertically");
      rangeActions << QLatin1String("select_workweek")
                   << QLatin1String("select_week")
                   << QLatin1String("select_day")
                   << QLatin1String("select_nextx");

      for ( int i = 0; i < zoomActions.size(); ++i ) {
        if ( QAction *action = ac->action( zoomActions[i] ) ) {
          action->setEnabled( view->supportsZoom() );
        }
      }

      for ( int i = 0; i < rangeActions.size(); ++i ) {
        if ( QAction *action = ac->action( rangeActions[i] ) ) {
          action->setEnabled( view->supportsDateRangeSelection() );
        }
      }
    }
  }
}

void KOViewManager::goMenu( bool enable )
{
  KOrg::MainWindow *w = ActionManager::findInstance( KUrl() );
  if ( w ) {
    KActionCollection *ac = w->getActionCollection();
    if ( ac ) {
      QAction *action;
      action = ac->action( QLatin1String("go_today") );
      if ( action ) {
        action->setEnabled( enable );
      }
      action = ac->action( QLatin1String("go_previous") );
      if ( action ) {
        action->setEnabled( enable );
      }
      action = ac->action( QLatin1String("go_next") );
      if ( action ) {
        action->setEnabled( enable );
      }
    }
  }
}

void KOViewManager::raiseCurrentView()
{
  if ( mCurrentView && mCurrentView->usesFullWindow() ) {
    mMainView->showLeftFrame( false );
    if ( mCurrentView == mTodoView ) {
      mMainView->navigatorBar()->hide();
    } else {
      mMainView->navigatorBar()->show();
    }
  } else {
    mMainView->showLeftFrame( true );
    mMainView->navigatorBar()->hide();
  }
  mMainView->viewStack()->setCurrentWidget( widgetForView( mCurrentView ) );
}

void KOViewManager::updateView()
{
  if ( mCurrentView ) {
    mCurrentView->updateView();
  }
}

void KOViewManager::updateView( const QDate &start, const QDate &end, const QDate &preferredMonth )
{
  if ( mCurrentView && mCurrentView != mTodoView ) {
    mCurrentView->setDateRange( KDateTime( start ), KDateTime( end ), preferredMonth );
  } else if ( mTodoView ) {
    mTodoView->updateView();
  }
}

void KOViewManager::connectView( KOrg::BaseView *view )
{
  if ( !view ) {
    return;
  }

  if ( view->isEventView() ) {
    connect( view, SIGNAL(datesSelected(KCalCore::DateList)),
                   SIGNAL(datesSelected(KCalCore::DateList)) );
  }

  // selecting an incidence
  connect( view, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           mMainView, SLOT(processMainViewSelection(Akonadi::Item,QDate)) );

  // showing/editing/deleting an incidence. The calendar view takes care of the action.
  connect( view, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           mMainView, SLOT(showIncidence(Akonadi::Item)) );
  connect( view, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           mMainView, SLOT(editIncidence(Akonadi::Item)) );
  connect( view, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           mMainView, SLOT(deleteIncidence(Akonadi::Item)) );
  connect( view, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           mMainView, SLOT(copyIncidence(Akonadi::Item)) );
  connect( view, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           mMainView, SLOT(cutIncidence(Akonadi::Item)) );
  connect( view, SIGNAL(pasteIncidenceSignal()),
           mMainView, SLOT(pasteIncidence()) );
  connect( view, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           mMainView, SLOT(toggleAlarm(Akonadi::Item)) );
  connect( view, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           mMainView, SLOT(toggleTodoCompleted(Akonadi::Item)) );
  connect( view, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)),
           mMainView, SLOT(copyIncidenceToResource(Akonadi::Item,QString)) );
  connect( view, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)),
           mMainView, SLOT(moveIncidenceToResource(Akonadi::Item,QString)) );
  connect( view, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           mMainView, SLOT(dissociateOccurrences(Akonadi::Item,QDate)) );

  // signals to create new incidences
  connect( view, SIGNAL(newEventSignal()),
           mMainView, SLOT(newEvent()) );
  connect( view, SIGNAL(newEventSignal(QDateTime)),
           mMainView, SLOT(newEvent(QDateTime)) );
  connect( view, SIGNAL(newEventSignal(QDateTime,QDateTime)),
           mMainView, SLOT(newEvent(QDateTime,QDateTime)) );
  connect( view, SIGNAL(newEventSignal(QDate)),
           mMainView, SLOT(newEvent(QDate)) );

  connect( view, SIGNAL(newTodoSignal(QDate)),
           mMainView, SLOT(newTodo(QDate)) );
  connect( view, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           mMainView, SLOT(newSubTodo(Akonadi::Item)) );
  connect( view, SIGNAL(newJournalSignal(QDate)),
           mMainView, SLOT(newJournal(QDate)) );

  // reload settings
  connect( mMainView, SIGNAL(configChanged()), view, SLOT(updateConfig()) );

  // Notifications about added, changed and deleted incidences
  connect( mMainView, SIGNAL(dayPassed(QDate)),
           view, SLOT(dayPassed(QDate)) );
  connect( view, SIGNAL(startMultiModify(QString)),
           mMainView, SLOT(startMultiModify(QString)) );
  connect( view, SIGNAL(endMultiModify()),
           mMainView, SLOT(endMultiModify()) );

  connect( mMainView, SIGNAL(newIncidenceChanger(Akonadi::IncidenceChanger*)),
           view, SLOT(setIncidenceChanger(Akonadi::IncidenceChanger*)) );

  view->setIncidenceChanger( mMainView->incidenceChanger() );
}

void KOViewManager::connectTodoView( KOTodoView *todoView )
{
  if ( !todoView ) {
    return;
  }

  // SIGNALS/SLOTS FOR TODO VIEW
  connect( todoView, SIGNAL(purgeCompletedSignal()),
           mMainView, SLOT(purgeCompleted()) );
  connect( todoView, SIGNAL(unSubTodoSignal()),
           mMainView, SLOT(todo_unsub()) );
  connect( todoView, SIGNAL(unAllSubTodoSignal()),
           mMainView, SLOT(makeSubTodosIndependent()) );
  connect( mMainView, SIGNAL(categoryConfigChanged()),
           todoView, SLOT(updateCategories()) );

  connect( todoView, SIGNAL(fullViewChanged(bool)),
           mMainView, SLOT(changeFullView(bool)) );
}

void KOViewManager::zoomInHorizontally()
{
  if ( mAgendaView == mCurrentView ) {
    mAgendaView->zoomInHorizontally();
  }
}

void KOViewManager::zoomOutHorizontally()
{
  if ( mAgendaView == mCurrentView ) {
    mAgendaView->zoomOutHorizontally();
  }
}

void KOViewManager::zoomInVertically()
{
  if ( mAgendaView == mCurrentView ) {
    mAgendaView->zoomInVertically();
  }
}

void KOViewManager::zoomOutVertically()
{
  if ( mAgendaView == mCurrentView ) {
    mAgendaView->zoomOutVertically();
  }
}

void KOViewManager::addView( KOrg::BaseView *view, bool isTab )
{
  connectView( view );
  mViews.append( view );
  const KConfigGroup group = KGlobal::config()->group( view->identifier() );
  view->restoreConfig( group );
  if ( !isTab ) {
    mMainView->viewStack()->addWidget( view );
  }
}

void KOViewManager::showTimeSpentView()
{
  if ( !mTimeSpentView ) {
    mTimeSpentView = new KOTimeSpentView( mMainView->viewStack() );
    mTimeSpentView->setCalendar( mMainView->calendar() );
    mTimeSpentView->setIdentifier( "DefaultTimeSpentView" );
    addView( mTimeSpentView );
  }
  goMenu( true );
  showView( mTimeSpentView );
}

void KOViewManager::showMonthView()
{
  if ( !mMonthView ) {
    mMonthView = new KOrg::MonthView( mMainView->viewStack() );
    mMonthView->setCalendar( mMainView->calendar() );
    mMonthView->setIdentifier( "DefaultMonthView" );
    addView( mMonthView );
    connect( mMonthView, SIGNAL(fullViewChanged(bool)),
             mMainView, SLOT(changeFullView(bool)) );
  }
  goMenu( true );
  showView( mMonthView );
}

void KOViewManager::showWhatsNextView()
{
  if ( !mWhatsNextView ) {
    mWhatsNextView = new KOWhatsNextView( mMainView->viewStack() );
    mWhatsNextView->setCalendar( mMainView->calendar() );
    mWhatsNextView->setIdentifier( "DefaultWhatsNextView" );
    addView( mWhatsNextView );
  }
  goMenu( true );
  showView( mWhatsNextView );
}

void KOViewManager::showListView()
{
  if ( !mListView ) {
    mListView = new KOListView( mMainView->calendar(), mMainView->viewStack() );
    mListView->setIdentifier( "DefaultListView" );
    addView( mListView );
  }
  goMenu( true );
  showView( mListView );
}

void KOViewManager::showAgendaView()
{
  const bool showBoth =
    KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::AllCalendarViews;
  const bool showMerged =
    showBoth || KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::CalendarsMerged;
  const bool showSideBySide =
    showBoth || KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::CalendarsSideBySide;

  QWidget *parent = mMainView->viewStack();
  if ( showBoth ) {
    if ( !mAgendaViewTabs && showBoth ) {
      mAgendaViewTabs = new KTabWidget( mMainView->viewStack() );
      connect( mAgendaViewTabs, SIGNAL(currentChanged(QWidget*)),
              this, SLOT(currentAgendaViewTabChanged(QWidget*)) );
      mMainView->viewStack()->addWidget( mAgendaViewTabs );

      KConfig *config = KOGlobals::self()->config();
      KConfigGroup viewConfig( config, "Views" );
      mAgendaViewTabIndex = viewConfig.readEntry( "Agenda View Tab Index", 0 );
    }
    parent = mAgendaViewTabs;
  }

  if ( showMerged ) {
    if ( !mAgendaView ) {
      mAgendaView = new KOAgendaView( parent );
      mAgendaView->setCalendar( mMainView->calendar() );
      mAgendaView->setIdentifier( "DefaultAgendaView" );

      addView( mAgendaView, showBoth );

      connect( mAgendaView,SIGNAL(zoomViewHorizontally(QDate,int)),
               mMainView->dateNavigator(), SLOT(selectDates(QDate,int)) );
      mAgendaView->readSettings( KOGlobals::self()->config() );
    }
    if ( showBoth && mAgendaViewTabs->indexOf( mAgendaView ) < 0 ) {
      mAgendaViewTabs->addTab( mAgendaView, i18n( "Merged calendar" ) );
    } else if ( !showBoth && mMainView->viewStack()->indexOf( mAgendaView ) < 0 ) {
      mAgendaView->setParent( parent );
      mMainView->viewStack()->addWidget( mAgendaView );
    }
  }

  if ( showSideBySide ) {
    if ( !mAgendaSideBySideView ) {
      mAgendaSideBySideView = new MultiAgendaView( parent );
      mAgendaSideBySideView->setIdentifier( "DefaultAgendaSideBySideView" );
      mAgendaSideBySideView->setCalendar( mMainView->calendar() );
      addView( mAgendaSideBySideView, showBoth );

/*
    connect( mAgendaSideBySideView,SIGNAL(zoomViewHorizontally(QDate,int)),
             mMainView->dateNavigator(),SLOT(selectDates(QDate,int)) );*/
    }
    if ( showBoth && mAgendaViewTabs->indexOf( mAgendaSideBySideView ) < 0 ) {
      mAgendaViewTabs->addTab( mAgendaSideBySideView, i18n( "Calendars Side by Side" ) );
      mAgendaViewTabs->setCurrentIndex( mAgendaViewTabIndex );
    } else if ( !showBoth && mMainView->viewStack()->indexOf( mAgendaSideBySideView ) < 0 ) {
      mAgendaSideBySideView->setParent( parent );
      mMainView->viewStack()->addWidget( mAgendaSideBySideView );
    }
  }

  goMenu( true );
  if ( showBoth ) {
    showView( static_cast<KOrg::BaseView*>( mAgendaViewTabs->currentWidget() ) );
  } else if ( showMerged ) {
    showView( mAgendaView );
  } else if ( showSideBySide ) {
    showView( mAgendaSideBySideView );
  }
}

void KOViewManager::selectDay()
{
  mRangeMode = DAY_RANGE;
  const QDate date = mMainView->activeDate();
  mMainView->dateNavigator()->selectDate( date );
}

void KOViewManager::selectWorkWeek()
{
  if ( KOGlobals::self()->getWorkWeekMask() != 0 ) {
    mRangeMode = WORK_WEEK_RANGE;
    QDate date = mMainView->activeDate();
    mMainView->dateNavigator()->selectWorkWeek( date );
  } else {
    KMessageBox::sorry(
      mMainView,
      i18n( "Unable to display the work week since there are no work days configured. "
            "Please properly configure at least 1 work day in the Time and Date preferences." ) );
  }
}

void KOViewManager::selectWeek()
{
  mRangeMode = WEEK_RANGE;
  QDate date = mMainView->activeDate();
  mMainView->dateNavigator()->selectWeek( date );
}

void KOViewManager::selectNextX()
{
  mRangeMode = NEXTX_RANGE;
  mMainView->dateNavigator()->selectDates( QDate::currentDate(),
                                           KOPrefs::instance()->mNextXDays );
}

void KOViewManager::showTodoView()
{
  if ( !mTodoView ) {
    mTodoView = new KOTodoView( false/*not sidebar*/, mMainView->viewStack() );
    mTodoView->setCalendar( mMainView->calendar() );
    mTodoView->setIdentifier( "DefaultTodoView" );
    mTodoView->setCalendar( mMainView->calendar() );
    addView( mTodoView );
    connectTodoView( mTodoView );

    KConfig *config = KOGlobals::self()->config();
    mTodoView->restoreLayout( config, QLatin1String("Todo View"), false );
  }
  goMenu( false );
  showView( mTodoView );
}

void KOViewManager::showJournalView()
{
  if ( !mJournalView ) {
    mJournalView = new KOJournalView( mMainView->viewStack() );
    mJournalView->setCalendar( mMainView->calendar() );
    mJournalView->setIdentifier( "DefaultJournalView" );
    addView( mJournalView );
  }
  goMenu( true );
  showView( mJournalView );
}

void KOViewManager::showTimeLineView()
{
  if ( !mTimelineView ) {
    mTimelineView = new KOTimelineView( mMainView->viewStack() );
    mTimelineView->setCalendar( mMainView->calendar() );
    mTimelineView->setIdentifier( "DefaultTimelineView" );
    addView( mTimelineView );
  }
  goMenu( true );
  showView( mTimelineView );
}

void KOViewManager::showEventView()
{
  if ( mLastEventView ) {
    goMenu( true );
    showView( mLastEventView );
  } else {
    showAgendaView();
    selectWeek();
  }
}

Akonadi::Item KOViewManager::currentSelection()
{
  if ( !mCurrentView ) {
    return Akonadi::Item();
  }

  Akonadi::Item::List incidenceList = mCurrentView->selectedIncidences();
  if ( incidenceList.isEmpty() ) {
    return Akonadi::Item();
  }
  return incidenceList.first();
}

QDate KOViewManager::currentSelectionDate()
{
  QDate qd;
  if ( mCurrentView ) {
    DateList qvl = mCurrentView->selectedIncidenceDates();
    if ( !qvl.isEmpty() ) {
      qd = qvl.first();
    }
  }
  return qd;
}

void KOViewManager::setDocumentId( const QString &id )
{
  if ( mTodoView ) {
    mTodoView->setDocumentId( id );
  }
}

QWidget *KOViewManager::widgetForView( KOrg::BaseView *view ) const
{
  if ( mAgendaViewTabs && mAgendaViewTabs->indexOf( view ) >= 0 ) {
    return mAgendaViewTabs;
  }
  return view;
}

void KOViewManager::currentAgendaViewTabChanged( QWidget *widget )
{
  KConfig *config = KOGlobals::self()->config();
  KConfigGroup viewConfig( config, "Views" );
  viewConfig.writeEntry( "Agenda View Tab Index", mAgendaViewTabs->currentIndex() );

  if ( widget ) {
    goMenu( true );
    showView( static_cast<KOrg::BaseView*>( widget ) );
  }
}

void KOViewManager::addChange( EventViews::EventView::Change change )
{
  foreach ( BaseView *view, mViews ) {
    if ( view ) {
      view->setChanges( view->changes() | change );
    }
  }
}

void KOViewManager::updateMultiCalendarDisplay()
{
  if ( agendaIsSelected() ) {
    showAgendaView();
  } else {
    updateView();
  }
}

bool KOViewManager::agendaIsSelected() const
{
  return mCurrentView == mAgendaView            ||
         mCurrentView == mAgendaSideBySideView  ||
        ( mAgendaViewTabs && mCurrentView == mAgendaViewTabs->currentWidget() );
}


