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
#include "kotimespanview.h"
#include "koprefs.h"
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
  mTimeSpanView = 0;
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
  else if (view == "TimeSpan") showTimeSpanView();
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
  else if (mCurrentView == mTimeSpanView) view = "TimeSpan";
  else if (mCurrentView == mTodoView) view = "Todo";
  else view = "Agenda";

  config->writeEntry("Current View",view);

  if (mAgendaView) {
    mAgendaView->writeSettings(config);
  }
  if (mTimeSpanView) {
    mTimeSpanView->writeSettings(config);
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

  if ( mCurrentView && mCurrentView->isEventView() ) {
    mLastEventView = mCurrentView;
  }

  if ( mAgendaView ) mAgendaView->deleteSelectedDateTime();
  mCurrentView = view;

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

    connect(mListView, SIGNAL(showIncidenceSignal(Incidence *)),
            mMainView, SLOT(showIncidence(Incidence *)));
    connect(mListView, SIGNAL(editIncidenceSignal(Incidence *)),
            mMainView, SLOT(editIncidence(Incidence *)));
    connect(mListView, SIGNAL(deleteIncidenceSignal(Incidence *)),
            mMainView, SLOT(deleteIncidence(Incidence *)));

    connect( mListView, SIGNAL( incidenceSelected( Incidence * ) ),
             mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

    connect(mMainView, SIGNAL(configChanged()), mListView, SLOT(updateConfig()));
  }

  showView(mListView);
}

void KOViewManager::showAgendaView()
{
  if (!mAgendaView) {
    mAgendaView = new KOAgendaView(mMainView->calendar(), mMainView->viewStack(), "KOViewManager::AgendaView");

    addView(mAgendaView);

    connect( mAgendaView, SIGNAL( incidenceChanged( Incidence*, Incidence* ) ),
             mMainView, SLOT( updateUnmanagedViews() ) );
    connect( mAgendaView, SIGNAL( incidenceChanged( Incidence*, Incidence* ) ),
             mMainView, SLOT( incidenceChanged( Incidence*, Incidence* ) ) );

    // SIGNALS/SLOTS FOR DAY/WEEK VIEW
    connect( mAgendaView, SIGNAL( newEventSignal() ),
             mMainView, SLOT( newEvent() ) );
    connect( mAgendaView, SIGNAL( newEventSignal( QDateTime ) ),
             mMainView, SLOT( newEvent( QDateTime ) ) );
    connect( mAgendaView, SIGNAL( newEventSignal( QDateTime, QDateTime ) ),
             mMainView, SLOT( newEvent( QDateTime, QDateTime ) ) );
    connect( mAgendaView, SIGNAL( newEventSignal( QDate ) ),
             mMainView, SLOT( newEvent( QDate ) ) );

    connect(mAgendaView, SIGNAL(editIncidenceSignal(Incidence *)),
            mMainView, SLOT(editIncidence(Incidence *)));
    connect(mAgendaView, SIGNAL(showIncidenceSignal(Incidence *)),
            mMainView, SLOT(showIncidence(Incidence *)));
    connect(mAgendaView, SIGNAL(deleteIncidenceSignal(Incidence *)),
            mMainView, SLOT(deleteIncidence(Incidence *)));

    connect( mAgendaView, SIGNAL( incidenceSelected( Incidence * ) ),
             mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

    connect(mAgendaView, SIGNAL( toggleExpand() ),
            mMainView, SLOT( toggleExpand() ) );
    connect(mMainView, SIGNAL( calendarViewExpanded( bool ) ),
            mAgendaView, SLOT( setExpandedButton( bool ) ) );

    connect(mAgendaView, SIGNAL( todoChanged( Todo *, Todo* ) ),
            mMainView, SLOT( todoChanged( Todo *, Todo * ) ) );
    connect(mAgendaView, SIGNAL( todoDropped( Todo* ) ),
            mMainView, SLOT( todoAdded( Todo* ) ) );

    connect(mMainView, SIGNAL(configChanged()), mAgendaView, SLOT(updateConfig()));

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

    // SIGNALS/SLOTS FOR MONTH VIEW
    connect(mMonthView, SIGNAL(newEventSignal(QDateTime)),
            mMainView, SLOT(newEvent(QDateTime)));
    connect(mMonthView, SIGNAL(newEventSignal(QDate)),
            mMainView, SLOT(newEvent(QDate)));
    connect(mMonthView, SIGNAL(newEventSignal()),
            mMainView, SLOT(newEvent()));

    connect(mMonthView, SIGNAL(showIncidenceSignal(Incidence *)),
            mMainView, SLOT(showIncidence(Incidence *)));
    connect(mMonthView, SIGNAL(editIncidenceSignal(Incidence *)),
            mMainView, SLOT(editIncidence(Incidence *)));
    connect(mMonthView, SIGNAL(deleteIncidenceSignal(Incidence *)),
            mMainView, SLOT(deleteIncidence(Incidence *)));

    connect( mMonthView, SIGNAL( incidenceSelected( Incidence * ) ),
             mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

    connect(mMainView, SIGNAL(configChanged()), mMonthView, SLOT(updateConfig()));
  }

  showView(mMonthView);
}

void KOViewManager::connectTodoView( KOTodoView* todoView )
{
  if (!todoView) return;

  // SIGNALS/SLOTS FOR TODO VIEW
  connect( todoView, SIGNAL( newTodoSignal() ),
           mMainView, SLOT( newTodo() ) );
  connect( todoView, SIGNAL( newSubTodoSignal( Todo * ) ),
           mMainView, SLOT( newSubTodo( Todo *) ) );
  connect( todoView, SIGNAL( showTodoSignal( Todo *) ),
           mMainView, SLOT( showTodo( Todo * ) ) );
  connect( todoView, SIGNAL( editTodoSignal( Todo * ) ),
           mMainView, SLOT( editTodo( Todo * ) ) );
  connect( todoView, SIGNAL( deleteTodoSignal( Todo * ) ),
           mMainView, SLOT( deleteTodo( Todo * ) ) );
  connect( todoView, SIGNAL( purgeCompletedSignal() ),
           mMainView, SLOT( purgeCompleted() ) );
  connect( todoView, SIGNAL( unSubTodoSignal() ),
           mMainView, SLOT( todo_unsub() ) );
  connect( todoView, SIGNAL( todoChanged( Todo*, Todo* ) ),
           mMainView, SLOT( todoChanged( Todo*, Todo* ) ) );
  connect( todoView, SIGNAL( todoAdded( Todo*) ),
           mMainView, SLOT( todoAdded( Todo* ) ) );
  connect( todoView, SIGNAL( todoModifiedSignal( Todo *, Todo *, int ) ),
           mMainView, SLOT( todoModified( Todo *, Todo *, int ) ) );

  connect( mMainView, SIGNAL( configChanged() ),
           todoView, SLOT( updateConfig() ) );
}

void KOViewManager::showTodoView()
{
  if ( !mTodoView ) {
    mTodoView = new KOTodoView( mMainView->calendar(), mMainView->viewStack(),
                                "KOViewManager::TodoView" );
    mTodoView->setCalendar( mMainView->calendar() );
    addView( mTodoView );
    connectTodoView( mTodoView );

    connect( mTodoView, SIGNAL( incidenceSelected( Incidence * ) ),
             mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

    KConfig *config = KOGlobals::config();
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

void KOViewManager::showTimeSpanView()
{
  if (!mTimeSpanView) {
    mTimeSpanView = new KOTimeSpanView(mMainView->calendar(),mMainView->viewStack(),
                                       "KOViewManager::TimeSpanView");
    addView(mTimeSpanView);

    mTimeSpanView->readSettings();
  }

  showView(mTimeSpanView);
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

void KOViewManager::addView(KOrg::BaseView *view)
{
#if QT_VERSION >= 300
  mMainView->viewStack()->addWidget( view );
#else
  mMainView->viewStack()->addWidget( view, 1 );
#endif
}

void KOViewManager::setDocumentId( const QString &id )
{
  if (mTodoView) mTodoView->setDocumentId( id );
}
