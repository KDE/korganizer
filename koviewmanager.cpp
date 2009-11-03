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
#include "actionmanager.h"
#include "calendarview.h"
#include "datenavigator.h"
#include "navigatorbar.h"
#include "datenavigatorcontainer.h"
#include "korganizer/mainwindow.h"
#include "views/agendaview/koagendaview.h"
#include "views/listview/kolistview.h"
#include "views/journalview/kojournalview.h"
#include "views/monthview/monthview.h"
#include "views/multiagendaview/multiagendaview.h"
#include "views/todoview/kotodoview.h"
#include "views/whatsnextview/kowhatsnextview.h"
#include "views/timelineview/kotimelineview.h"
#include "views/timespentview/kotimespentview.h"

#include <KActionCollection>
#include <KConfig>
#include <KGlobal>
#include <KTabWidget>

#include <QStackedWidget>

#include "koviewmanager.moc"

using namespace Akonadi;

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

  // write out custom view configuration
  Q_FOREACH( KOrg::BaseView* const view, mViews ) {
    KConfigGroup group = KGlobal::config()->group( view->identifier() );
    view->saveConfig( group );
  }
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
  mMainView->processIncidenceSelection( Item(), QDate() );
  mMainView->updateView();
  mMainView->adaptNavigationUnits();
}

void KOViewManager::goMenu( bool enable )
{
  KOrg::MainWindow *w = ActionManager::findInstance( KUrl() );
  if ( w ) {
    KActionCollection *ac = w->getActionCollection();
    if ( ac ) {
      QAction *action;
      action = ac->action( "go_today" );
      if ( action ) {
        action->setEnabled( enable );
      }
      action = ac->action( "go_previous" );
      if ( action ) {
        action->setEnabled( enable );
      }
      action = ac->action( "go_next" );
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
  connect( view, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           mMainView, SLOT(dissociateOccurrences(Akonadi::Item,QDate)) );

  // signals to create new incidences
  connect( view, SIGNAL(newEventSignal()),
           mMainView, SLOT(newEvent()) );
  connect( view, SIGNAL(newEventSignal(QDateTime)),
           mMainView, SLOT(newEvent(QDateTime)) );
  connect( view, SIGNAL(newEventSignal(QDateTime, QDateTime)),
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
    mListView = new KOListView( mMainView->viewStack() );
    mListView->setCalendar( mMainView->calendar() );
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
      connect( mAgendaViewTabs, SIGNAL(currentChanged(QWidget *)),
              this, SLOT(currentAgendaViewTabChanged(QWidget *)) );
      mMainView->viewStack()->addWidget( mAgendaViewTabs );
    }
    parent = mAgendaViewTabs;
  }

  if ( showMerged ) {
    if ( !mAgendaView ) {
      mAgendaView = new KOAgendaView( parent );
      mAgendaView->setCalendar( mMainView->calendar() );
      mAgendaView->setIdentifier( "DefaultAgendaView" );

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
      mAgendaSideBySideView = new MultiAgendaView( parent );
      mAgendaSideBySideView->setCalendar( mMainView->calendar() );
      mAgendaSideBySideView->setIdentifier( "DefaultAgendaSideBySideView" );
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
  const QDate date = mMainView->activeDate();
  mMainView->dateNavigator()->selectDate( date );
}

void KOViewManager::selectWorkWeek()
{
  if ( KOGlobals::self()->getWorkWeekMask() != 0 ) {
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
  QDate date = mMainView->activeDate();
  mMainView->dateNavigator()->selectWeek( date );
}

void KOViewManager::selectNextX()
{
  mMainView->dateNavigator()->selectDates( QDate::currentDate(),
                                           KOPrefs::instance()->mNextXDays );
}

void KOViewManager::showTodoView()
{
  if ( !mTodoView ) {
    mTodoView = new KOTodoView( mMainView->viewStack() );
    mTodoView->setCalendar( mMainView->calendar() );
    mTodoView->setIdentifier( "DefaultTodoView" );
    mTodoView->setCalendar( mMainView->calendar() );
    addView( mTodoView );
    connectTodoView( mTodoView );

    KConfig *config = KOGlobals::self()->config();
    mTodoView->restoreLayout( config, "Todo View", false );
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

Item KOViewManager::currentSelection()
{
  if ( !mCurrentView ) {
    return Item();
  }

  Item::List incidenceList = mCurrentView->selectedIncidences();
  if ( incidenceList.isEmpty() ) {
    return Item();
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
    goMenu( true );
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
