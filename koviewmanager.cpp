/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <qwidgetstack.h>

#include <kconfig.h>
#include <kglobal.h>

#include "calendarview.h"
#include "datenavigator.h"
#include "kotodoview.h"
#include "koagendaview.h"
#include "komonthview.h"
#include "kolistview.h"
#include "kowhatsnextview.h"
#include "kojournalview.h"
#include "koprefs.h"
#include "koglobals.h"
#include "navigatorbar.h"

#include "koviewmanager.h"
#include "koviewmanager.moc"

KOViewManager::KOViewManager( CalendarView *mainView ) :
  QObject(), mMainView( mainView )
{
  mCurrentView = 0;

  mLastEventView = 0;

  mWhatsNextView = 0;
  mTodoView = 0;
  mAgendaView = 0;
  mMonthView = 0;
  mListView = 0;
  mJournalView = 0;
}

KOViewManager::~KOViewManager()
{
}


KOrg::BaseView *KOViewManager::currentView()
{
  return mCurrentView;
}

void KOViewManager::readSettings(KConfig *config)
{
  config->setGroup("General");
  QString view = config->readEntry("Current View");

  if (view == "WhatsNext") showWhatsNextView();
  else if (view == "Month") showMonthView();
  else if (view == "List") showListView();
  else if (view == "Journal") showJournalView();
  else if (view == "Todo") showTodoView();
  else showAgendaView();
}

void KOViewManager::writeSettings(KConfig *config)
{
  config->setGroup("General");

  QString view;
  if (mCurrentView == mWhatsNextView) view = "WhatsNext";
  else if (mCurrentView == mMonthView) view = "Month";
  else if (mCurrentView == mListView) view = "List";
  else if (mCurrentView == mJournalView) view = "Journal";
  else if (mCurrentView == mTodoView) view = "Todo";
  else view = "Agenda";

  config->writeEntry("Current View",view);

  if (mAgendaView) {
    mAgendaView->writeSettings(config);
  }
  if (mListView) {
    mListView->writeSettings(config);
  }
  if (mTodoView) {
    mTodoView->saveLayout(config,"Todo View");
  }
}

void KOViewManager::showView(KOrg::BaseView *view)
{
  if( view == mCurrentView ) return;

  mCurrentView = view;

  if ( mCurrentView && mCurrentView->isEventView() ) {
    mLastEventView = mCurrentView;
  }

  if ( mAgendaView ) mAgendaView->deleteSelectedDateTime();

  raiseCurrentView();
  mMainView->processIncidenceSelection( 0 );

  mMainView->updateView();

  mMainView->adaptNavigationUnits();
}

void KOViewManager::raiseCurrentView()
{
  if ((mMonthView && KOPrefs::instance()->mFullViewMonth && mCurrentView == mMonthView) ||
      (mTodoView && KOPrefs::instance()->mFullViewTodo && mCurrentView == mTodoView)) {
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
  mMainView->viewStack()->raiseWidget(mCurrentView);
}

void KOViewManager::updateView()
{
  if ( mCurrentView ) mCurrentView->updateView();
}

void KOViewManager::updateView(const QDate &start, const QDate &end)
{
//  kdDebug(5850) << "KOViewManager::updateView()" << endl;

  if (mCurrentView) mCurrentView->showDates(start, end);

  if (mTodoView) mTodoView->updateView();
}

void KOViewManager::connectView(KOrg::BaseView *view)
{
  if (!view) return;

  // selecting an incidence
  connect( view, SIGNAL( incidenceSelected( Incidence * ) ),
           mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

  // showing/editing/deleting an incidence. The calendar view takes care of the action.
  connect(view, SIGNAL(showIncidenceSignal(Incidence *)),
          mMainView, SLOT(showIncidence(Incidence *)));
  connect(view, SIGNAL(editIncidenceSignal(Incidence *)),
          mMainView, SLOT(editIncidence(Incidence *)));
  connect(view, SIGNAL(deleteIncidenceSignal(Incidence *)),
          mMainView, SLOT(deleteIncidence(Incidence *)));
  connect(view, SIGNAL(toggleAlarmSignal(Incidence *)),
          mMainView, SLOT(toggleAlarm(Incidence *)));
  connect(view,SIGNAL(dissociateOccurrenceSignal( Incidence *, const QDate & )),
          mMainView, SLOT(dissociateOccurrence( Incidence *, const QDate & )));
  connect(view,SIGNAL(dissociateFutureOccurrenceSignal( Incidence *, const QDate & )),
          mMainView, SLOT(dissociateFutureOccurrence( Incidence *, const QDate & )));

  // signals to create new incidences
  connect( view, SIGNAL( newEventSignal() ),
           mMainView, SLOT( newEvent() ) );
  connect( view, SIGNAL( newEventSignal( QDateTime ) ),
           mMainView, SLOT( newEvent( QDateTime ) ) );
  connect( view, SIGNAL( newEventSignal( QDateTime, QDateTime ) ),
           mMainView, SLOT( newEvent( QDateTime, QDateTime ) ) );
  connect( view, SIGNAL( newEventSignal( QDate ) ),
           mMainView, SLOT( newEvent( QDate ) ) );
  connect( view, SIGNAL( newTodoSignal( QDate ) ),
           mMainView, SLOT( newTodo( QDate ) ) );
  connect( view, SIGNAL( newSubTodoSignal( Todo * ) ),
           mMainView, SLOT( newSubTodo( Todo *) ) );

  // reload settings
  connect(mMainView, SIGNAL(configChanged()), view, SLOT(updateConfig()));

  // Notifications about added, changed and deleted incidences
  connect( mMainView, SIGNAL( dayPassed( QDate ) ),
           view, SLOT( dayPassed( QDate ) ) );
  connect( view, SIGNAL( startMultiModify( const QString & ) ),
           mMainView, SLOT( startMultiModify( const QString & ) ) );
  connect( view, SIGNAL( endMultiModify() ),
           mMainView, SLOT( endMultiModify() ) );
           
  connect( mMainView, SIGNAL( newIncidenceChanger( IncidenceChangerBase* ) ),
           view, SLOT( setIncidenceChanger( IncidenceChangerBase * ) ) );
  view->setIncidenceChanger( mMainView->incidenceChanger() );
}

void KOViewManager::connectTodoView( KOTodoView* todoView )
{
  if (!todoView) return;

  // SIGNALS/SLOTS FOR TODO VIEW
  connect( todoView, SIGNAL( purgeCompletedSignal() ),
           mMainView, SLOT( purgeCompleted() ) );
  connect( todoView, SIGNAL( unSubTodoSignal() ),
           mMainView, SLOT( todo_unsub() ) );
  connect( todoView, SIGNAL( unAllSubTodoSignal() ),
           mMainView, SLOT( makeSubTodosIndependents() ) );
}

void KOViewManager::zoomInHorizontally() 
{
  if( mAgendaView == mCurrentView ) mAgendaView->zoomInHorizontally();
}
void KOViewManager::zoomOutHorizontally()
{
  if( mAgendaView== mCurrentView ) mAgendaView->zoomOutHorizontally();
}
void KOViewManager::zoomInVertically()
{
  if( mAgendaView== mCurrentView ) mAgendaView->zoomInVertically();
}
void KOViewManager::zoomOutVertically()
{
  if( mAgendaView== mCurrentView ) mAgendaView->zoomOutVertically();
}

void KOViewManager::addView(KOrg::BaseView *view)
{
  connectView( view );
#if QT_VERSION >= 300
  mMainView->viewStack()->addWidget( view );
#else
  mMainView->viewStack()->addWidget( view, 1 );
#endif
}

void KOViewManager::showWhatsNextView()
{
  if (!mWhatsNextView) {
    mWhatsNextView = new KOWhatsNextView(mMainView->calendar(),mMainView->viewStack(),
                                         "KOViewManager::WhatsNextView");
    addView(mWhatsNextView);
  }
  showView(mWhatsNextView);
}

void KOViewManager::showListView()
{
  if (!mListView) {
    mListView = new KOListView(mMainView->calendar(), mMainView->viewStack(), "KOViewManager::ListView");
    addView(mListView);
  }
  showView(mListView);
}

void KOViewManager::showAgendaView()
{
  if (!mAgendaView) {
    mAgendaView = new KOAgendaView(mMainView->calendar(), mMainView->viewStack(), "KOViewManager::AgendaView");

    addView(mAgendaView);

    connect(mAgendaView, SIGNAL( toggleExpand() ),
            mMainView, SLOT( toggleExpand() ) );
    connect(mMainView, SIGNAL( calendarViewExpanded( bool ) ),
            mAgendaView, SLOT( setExpandedButton( bool ) ) );

    connect( mAgendaView,SIGNAL( zoomViewHorizontally(const QDate &, int )),
      mMainView->dateNavigator(),SLOT( selectDates( const QDate &, int ) ) );
    mAgendaView->readSettings();
  }

  showView(mAgendaView);
}

void KOViewManager::showDayView()
{
  showAgendaView();
  mMainView->dateNavigator()->selectDates( 1 );
}

void KOViewManager::showWorkWeekView()
{
  showAgendaView();
  mMainView->dateNavigator()->selectWorkWeek();
}

void KOViewManager::showWeekView()
{
  showAgendaView();
  mMainView->dateNavigator()->selectWeek();
}

void KOViewManager::showNextXView()
{
  showAgendaView();
  mMainView->dateNavigator()->selectDates( QDate::currentDate(),
                                           KOPrefs::instance()->mNextXDays );
}

void KOViewManager::showMonthView()
{
  if (!mMonthView) {
    mMonthView = new KOMonthView(mMainView->calendar(), mMainView->viewStack(), "KOViewManager::MonthView");
    addView(mMonthView);
  }

  showView(mMonthView);
}

void KOViewManager::showTodoView()
{
  if ( !mTodoView ) {
    mTodoView = new KOTodoView( mMainView->calendar(), mMainView->viewStack(),
                                "KOViewManager::TodoView" );
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
  if (!mJournalView) {
    mJournalView = new KOJournalView(mMainView->calendar(),mMainView->viewStack(),
                                     "KOViewManager::JournalView");
    addView(mJournalView);
  }

  showView(mJournalView);
}

void KOViewManager::showEventView()
{
  if ( mLastEventView ) showView( mLastEventView );
  else showWeekView();
}

Incidence *KOViewManager::currentSelection()
{
  if ( !mCurrentView ) return 0;
  Incidence::List incidenceList = mCurrentView->selectedIncidences();
  if ( incidenceList.isEmpty() ) return 0;

  return incidenceList.first();
}

QDate KOViewManager::currentSelectionDate()
{
  QDate qd;
  if (mCurrentView) {
    DateList qvl = mCurrentView->selectedDates();
    if (!qvl.isEmpty()) qd = qvl.first();
  }
  return qd;
}

void KOViewManager::setDocumentId( const QString &id )
{
  if (mTodoView) mTodoView->setDocumentId( id );
}
