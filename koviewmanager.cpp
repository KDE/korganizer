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
#include "koglobals.h"
#include "koprefs.h"
#include "calendarview.h"
#include "datenavigator.h"
#include "navigatorbar.h"
#include "views/agendaview/koagendaview.h"
#include "views/listview/kolistview.h"
#include "views/journalview/kojournalview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/todoview/kotodoview.h"
#include "views/whatsnextview/kowhatsnextview.h"
#include "views/timelineview/kotimelineview.h"
#include "views/timespentview/kotimespentview.h"

#include <KConfig>
#include <KGlobal>
#include <KTabWidget>

#include <QStackedWidget>

#include "koviewmanager.moc"

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
  mTimeSpentView = 0;
  mMonthView = 0;
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
  QString view = generalConfig.readEntry( "Current View" );

  if ( view == QLatin1String( "WhatsNext" ) ) {
    showWhatsNextView();
  } else if ( view == QLatin1String( "OldMonth" ) ) {
    // the oldmonth view is gone, so we assume the new month view
    showMonthView();
  } else if ( view == QLatin1String( "List" ) ) {
    showListView();
  } else if ( view == QLatin1String( "Journal" ) ) {
    showJournalView();
  } else if ( view == QLatin1String( "Todo" ) ) {
    showTodoView();
  } else if ( view == "Timeline" ) {
    showTimeLineView();
  } else if ( view == "TimeSpent" ) {
    showTimeSpentView();
  } else if ( view == "Month" ) {
    showMonthView();
  } else {
    showAgendaView();
  }
}

void KOViewManager::writeSettings( KConfig *config )
{
  KConfigGroup generalConfig( config, "General" );

  QString view;
  if ( mCurrentView == mWhatsNextView ) {
    view = "WhatsNext";
  } else if ( mCurrentView == mListView ) {
    view = "List";
  } else if ( mCurrentView == mJournalView ) {
    view = "Journal";
  } else if ( mCurrentView == mTodoView ) {
    view = "Todo";
  } else if ( mCurrentView == mTimelineView ) {
    view = "Timeline";
  } else if ( mCurrentView == mTimeSpentView ) {
    view = "TimeSpent";
  } else if ( mCurrentView == mMonthView ) {
    view = "Month";
  } else {
    view = "Agenda";
  }

  generalConfig.writeEntry( "Current View", view );

  if ( mAgendaView ) {
    mAgendaView->writeSettings( config );
  }
  if ( mListView ) {
    mListView->writeSettings( config );
  }
  if ( mTodoView ) {
    mTodoView->saveLayout( config, "Todo View" );
  }
}

void KOViewManager::showView( KOrg::BaseView *view )
{
  if ( view == mCurrentView ) {
    return;
  }

  mCurrentView = view;

  if ( mCurrentView && mCurrentView->isEventView() ) {
    mLastEventView = mCurrentView;
  }

  if ( mAgendaView ) {
    mAgendaView->deleteSelectedDateTime();
  }

  raiseCurrentView();
  mMainView->processIncidenceSelection( 0 );
  mMainView->updateView();
  mMainView->adaptNavigationUnits();
}

void KOViewManager::raiseCurrentView()
{
  if ( ( mMonthView && KOPrefs::instance()->mFullViewMonth && mCurrentView == mMonthView ) ||
       ( mTodoView && KOPrefs::instance()->mFullViewTodo && mCurrentView == mTodoView ) ) {
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

void KOViewManager::updateView( const QDate &start, const QDate &end )
{
  if ( mCurrentView ) {
    mCurrentView->showDates( start, end );
  }

  if ( mTodoView ) {
    mTodoView->updateView();
  }
}

void KOViewManager::connectView( KOrg::BaseView *view )
{
  if ( !view ) {
    return;
  }

  // selecting an incidence
  connect( view, SIGNAL(incidenceSelected(Incidence *)),
           mMainView, SLOT(processMainViewSelection(Incidence *)) );

  // showing/editing/deleting an incidence. The calendar view takes care of the action.
  connect( view, SIGNAL(showIncidenceSignal(Incidence *)),
           mMainView, SLOT(showIncidence(Incidence *)) );
  connect( view, SIGNAL(editIncidenceSignal(Incidence *)),
           mMainView, SLOT(editIncidence(Incidence *)) );
  connect( view, SIGNAL(deleteIncidenceSignal(Incidence *)),
           mMainView, SLOT(deleteIncidence(Incidence *)) );
  connect( view, SIGNAL(copyIncidenceSignal(Incidence *)),
           mMainView, SLOT(copyIncidence(Incidence *)) );
  connect( view, SIGNAL(cutIncidenceSignal(Incidence *)),
           mMainView, SLOT(cutIncidence(Incidence *)) );
  connect( view, SIGNAL(pasteIncidenceSignal()),
           mMainView, SLOT(pasteIncidence()) );
  connect( view, SIGNAL(toggleAlarmSignal(Incidence *)),
           mMainView, SLOT(toggleAlarm(Incidence *)) );
  connect( view, SIGNAL(toggleTodoCompletedSignal(Incidence *)),
           mMainView, SLOT(toggleTodoCompleted(Incidence *)) );
  connect( view, SIGNAL(dissociateOccurrencesSignal(Incidence *,const QDate &)),
           mMainView, SLOT(dissociateOccurrences(Incidence *,const QDate &)) );

  // signals to create new incidences
  connect( view, SIGNAL(newEventSignal()),
           mMainView, SLOT(newEvent()) );
  connect( view, SIGNAL(newEventSignal(const QDateTime &)),
           mMainView, SLOT(newEvent(const QDateTime &)) );
  connect( view, SIGNAL(newEventSignal(const QDateTime &, const QDateTime &)),
           mMainView, SLOT(newEvent(const QDateTime &,const QDateTime &)) );
  connect( view, SIGNAL(newEventSignal(const QDate &)),
           mMainView, SLOT(newEvent(const QDate &)) );
  connect( view, SIGNAL(newTodoSignal(const QDate &)),
           mMainView, SLOT(newTodo(const QDate &)) );
  connect( view, SIGNAL(newSubTodoSignal(Todo *)),
           mMainView, SLOT(newSubTodo(Todo *)) );
  connect( view, SIGNAL(newJournalSignal(const QDate &)),
           mMainView, SLOT(newJournal(const QDate &)) );

  // reload settings
  connect( mMainView, SIGNAL(configChanged()), view, SLOT(updateConfig()) );

  // Notifications about added, changed and deleted incidences
  connect( mMainView, SIGNAL(dayPassed(const QDate &)),
           view, SLOT(dayPassed(const QDate &)) );
  connect( view, SIGNAL(startMultiModify(const QString &)),
           mMainView, SLOT(startMultiModify(const QString &)) );
  connect( view, SIGNAL(endMultiModify()),
           mMainView, SLOT(endMultiModify()) );

  connect( mMainView, SIGNAL(newIncidenceChanger(IncidenceChangerBase *)),
           view, SLOT(setIncidenceChanger(IncidenceChangerBase *)) );
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
           mMainView, SLOT(makeSubTodosIndependents()) );
  connect( mMainView, SIGNAL(categoryConfigChanged()),
           todoView, SLOT(updateCategories()) );
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
  if ( !isTab ) {
    mMainView->viewStack()->addWidget( view );
  }
}

void KOViewManager::showTimeSpentView()
{
  if ( !mTimeSpentView ) {
    mTimeSpentView = new KOTimeSpentView( mMainView->calendar(), mMainView->viewStack() );
    mTimeSpentView->setObjectName( "KOViewManager::TimeSpentView" );
    addView( mTimeSpentView );
  }
  showView( mTimeSpentView );
}

void KOViewManager::showMonthView()
{
  if ( !mMonthView ) {
    mMonthView = new KOrg::MonthView( mMainView->calendar(), mMainView->viewStack() );
    mMonthView->setObjectName( "KOViewManager::MonthView" );
    addView( mMonthView );
  }
  showView( mMonthView );
}

void KOViewManager::showWhatsNextView()
{
  if ( !mWhatsNextView ) {
    mWhatsNextView = new KOWhatsNextView( mMainView->calendar(), mMainView->viewStack() );
    mWhatsNextView->setObjectName( "KOViewManager::WhatsNextView" );
    addView( mWhatsNextView );
  }
  showView( mWhatsNextView );
}

void KOViewManager::showListView()
{
  if ( !mListView ) {
    mListView = new KOListView( mMainView->calendar(), mMainView->viewStack() );
    mListView->setObjectName( "KOViewManager::ListView" );
    addView( mListView );
  }
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
      connect( mAgendaViewTabs, SIGNAL(currentChanged(QWidget *)),
              this, SLOT(currentAgendaViewTabChanged(QWidget *)) );
      mMainView->viewStack()->addWidget( mAgendaViewTabs );
    }
    parent = mAgendaViewTabs;
  }

  if ( showMerged ) {
    if ( !mAgendaView ) {
      mAgendaView = new KOAgendaView( mMainView->calendar(), parent );
      mAgendaView->setObjectName( "KOViewManager::AgendaView" );

      addView( mAgendaView, showBoth );

      connect( mAgendaView,SIGNAL(zoomViewHorizontally(const QDate &,int)),
               mMainView->dateNavigator(), SLOT(selectDates(const QDate &,int)) );
      mAgendaView->readSettings();
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
      mAgendaSideBySideView = new MultiAgendaView( mMainView->calendar(), parent );
      mAgendaSideBySideView->setObjectName( "KOViewManager::AgendaSideBySideView" );
      addView( mAgendaSideBySideView, showBoth );

/*
    connect( mAgendaSideBySideView,SIGNAL(zoomViewHorizontally(const QDate &,int)),
             mMainView->dateNavigator(),SLOT(selectDates(const QDate &,int)) );*/
    }
    if ( showBoth && mAgendaViewTabs->indexOf( mAgendaSideBySideView ) < 0 ) {
      mAgendaViewTabs->addTab( mAgendaSideBySideView, i18n( "Calendars Side by Side" ) );
    } else if ( !showBoth && mMainView->viewStack()->indexOf( mAgendaSideBySideView ) < 0 ) {
      mAgendaSideBySideView->setParent( parent );
      mMainView->viewStack()->addWidget( mAgendaSideBySideView );
    }
  }

  if ( showBoth ) {
    showView( static_cast<KOrg::BaseView*>( mAgendaViewTabs->currentWidget() ) );
  } else if ( showMerged ) {
    showView( mAgendaView );
  } else if ( showSideBySide ) {
    showView( mAgendaSideBySideView );
  }
}

void KOViewManager::showDayView()
{
  QDate date = mMainView->activeDate();
  showAgendaView();
  mMainView->dateNavigator()->selectDate( date );
}

void KOViewManager::showWorkWeekView()
{
  QDate date = mMainView->activeDate();
  showAgendaView();
  mMainView->dateNavigator()->selectWorkWeek( date );
}

void KOViewManager::showWeekView()
{
  QDate date = mMainView->activeDate();
  showAgendaView();
  mMainView->dateNavigator()->selectWeek( date );
}

void KOViewManager::showNextXView()
{
  showAgendaView();
  mMainView->dateNavigator()->selectDates( QDate::currentDate(),
                                           KOPrefs::instance()->mNextXDays );
}

void KOViewManager::showTodoView()
{
  if ( !mTodoView ) {
    mTodoView = new KOTodoView( mMainView->calendar(), mMainView->viewStack() );
    mTodoView->setObjectName( "KOViewManager::TodoView" );
    mTodoView->setCalendar( mMainView->calendar() );
    addView( mTodoView );
    connectTodoView( mTodoView );

    KConfig *config = KOGlobals::self()->config();
    mTodoView->restoreLayout( config, "Todo View" );
  }
  showView( mTodoView );
}

void KOViewManager::showJournalView()
{
  if ( !mJournalView ) {
    mJournalView = new KOJournalView( mMainView->calendar(), mMainView->viewStack() );
    mJournalView->setObjectName( "KOViewManager::JournalView" );
    addView( mJournalView );
  }
  showView( mJournalView );
}

void KOViewManager::showTimeLineView()
{
  if ( !mTimelineView ) {
    mTimelineView = new KOTimelineView( mMainView->calendar(), mMainView->viewStack() );
    mTimelineView->setObjectName( "KOViewManager::TimelineView" );
    addView( mTimelineView );
  }
  showView( mTimelineView );
}

void KOViewManager::showEventView()
{
  if ( mLastEventView ) {
    showView( mLastEventView );
  } else {
    showWeekView();
  }
}

Incidence *KOViewManager::currentSelection()
{
  if ( !mCurrentView ) {
    return 0;
  }

  Incidence::List incidenceList = mCurrentView->selectedIncidences();
  if ( incidenceList.isEmpty() ) {
    return 0;
  }
  return incidenceList.first();
}

QDate KOViewManager::currentSelectionDate()
{
  QDate qd;
  if ( mCurrentView ) {
    DateList qvl = mCurrentView->selectedDates();
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
  if ( widget ) {
    showView( static_cast<KOrg::BaseView*>( widget ) );
  }
}

void KOViewManager::setUpdateNeeded()
{
  if ( mAgendaView ) {
    mAgendaView->setUpdateNeeded();
  }
  if ( mAgendaSideBySideView ) {
    mAgendaSideBySideView->setUpdateNeeded();
  }
}
