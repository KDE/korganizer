/*
    This file is part of KOrganizer.

    Copyright (c) 2001
    Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <qwidgetstack.h>

#include <kconfig.h>

#include "calendarview.h"
#include "kotodoview.h"
#include "koagendaview.h"
#include "komonthview.h"
#include "kolistview.h"
#include "kowhatsnextview.h"
#include "kojournalview.h"
#include "kotimespanview.h"
#include "koprefs.h"

#include "koviewmanager.h"
#include "koviewmanager.moc"

KOViewManager::KOViewManager( CalendarView *mainView ) :
  QObject(), mMainView( mainView )
{
  mCurrentView = 0;

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
}

void KOViewManager::changeAgendaView( int newView )
{
  if (!mAgendaView) return;

  if (newView == mAgendaView->currentView()) return;

  switch( newView ) {
  case KOAgendaView::DAY: {
#if 0
    DateList tmpList = mDateNavigator->selectedDates();
    if (mSaveSingleDate != tmpList.first()) {
      mDateNavigator->selectDates(mSaveSingleDate);
      updateView();
    }
#endif
    break;
  }
  // if its a workweek view, calculate the dates and emit
  case KOAgendaView::WORKWEEK:
    break;
    // if its a week view, calculate the dates and emit
  case KOAgendaView::WEEK:
    break;
    // if its a list view, update the list properties.
  case KOAgendaView::LIST:
    // we want to make sure that this is up to date.
    break;
  }
  mAgendaView->slotViewChange( newView );

  mMainView->adaptNavigationUnits();
}


void KOViewManager::showView(KOrg::BaseView *view)
{
  if(view == mCurrentView) return;

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
    mMainView->leftFrame()->hide();
  } else {
    mMainView->leftFrame()->show();
  }
  mMainView->viewStack()->raiseWidget(mCurrentView);
}

void KOViewManager::updateView(const QDate &start, const QDate &end)
{
//  kdDebug() << "KOViewManager::updateView()" << endl;

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

    connect(mListView, SIGNAL(datesSelected(const DateList &)),
            mMainView->dateNavigator(), SLOT(selectDates(const DateList &)));

    connect(mListView, SIGNAL(showEventSignal(Event *)),
            mMainView, SLOT(showEvent(Event *)));
    connect(mListView, SIGNAL(editEventSignal(Event *)),
            mMainView, SLOT(editEvent(Event *)));
    connect(mListView, SIGNAL(deleteEventSignal(Event *)),
            mMainView, SLOT(deleteEvent(Event *)));

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

    connect(mAgendaView, SIGNAL(datesSelected(const DateList &)),
            mMainView->dateNavigator(), SLOT(selectDates(const DateList &)));

//ET
    kdDebug() << "KOViewManager::showAgendaView() slot shiftedEvents" << endl;

    connect(mAgendaView, SIGNAL(shiftedEvent(const QDate&, const QDate&)),
            mMainView->dateNavigator(), SLOT(shiftEvent(const QDate&, const QDate&)));
//ET

    // SIGNALS/SLOTS FOR DAY/WEEK VIEW
    connect(mAgendaView,SIGNAL(newEventSignal(QDateTime)),
            mMainView, SLOT(newEvent(QDateTime)));
    connect(mAgendaView,SIGNAL(newEventSignal(QDateTime,QDateTime)),
            mMainView, SLOT(newEvent(QDateTime,QDateTime)));
    connect(mAgendaView,SIGNAL(newEventSignal(QDate)),
            mMainView, SLOT(newEvent(QDate)));
//  connect(mAgendaView,SIGNAL(newEventSignal()),
//		mMainView, SLOT(newEvent()));
    connect(mAgendaView, SIGNAL(editEventSignal(Event *)),
	    mMainView, SLOT(editEvent(Event *)));
    connect(mAgendaView, SIGNAL(showEventSignal(Event *)),
            mMainView, SLOT(showEvent(Event *)));
    connect(mAgendaView, SIGNAL(deleteEventSignal(Event *)),
            mMainView, SLOT(deleteEvent(Event *)));

    connect( mAgendaView, SIGNAL( incidenceSelected( Incidence * ) ),
             mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

    connect(mAgendaView, SIGNAL( toggleExpand() ),
            mMainView, SLOT( toggleExpand() ) );
    connect(mMainView, SIGNAL( calendarViewExpanded( bool ) ),
            mAgendaView, SLOT( setExpandedButton( bool ) ) );

    connect(mMainView, SIGNAL(configChanged()), mAgendaView, SLOT(updateConfig()));

    mAgendaView->readSettings();
  }
  
  showView(mAgendaView);
}

void KOViewManager::showDayView()
{
  showAgendaView();
  changeAgendaView(KOAgendaView::DAY);
}

void KOViewManager::showWorkWeekView()
{
  showAgendaView();
  changeAgendaView(KOAgendaView::WORKWEEK);
}

void KOViewManager::showWeekView()
{
  showAgendaView();
  changeAgendaView(KOAgendaView::WEEK);
}

void KOViewManager::showMonthView()
{
  if (!mMonthView) {
    mMonthView = new KOMonthView(mMainView->calendar(), mMainView->viewStack(), "KOViewManager::MonthView");
    addView(mMonthView);

    connect(mMonthView, SIGNAL(datesSelected(const DateList &)),
            mMainView->dateNavigator(), SLOT(selectDates(const DateList &)));

    // SIGNALS/SLOTS FOR MONTH VIEW
    connect(mMonthView, SIGNAL(showEventSignal(Event *)),
            mMainView, SLOT(showEvent(Event *)));
    connect(mMonthView, SIGNAL(newEventSignal(QDate)),
            mMainView, SLOT(newEvent(QDate)));
    connect(mMonthView, SIGNAL(newEventSignal(QDateTime)),
            mMainView, SLOT(newEvent(QDateTime)));
    connect(mMonthView, SIGNAL(editEventSignal(Event *)),
            mMainView, SLOT(editEvent(Event *)));
    connect(mMonthView, SIGNAL(deleteEventSignal(Event *)),
            mMainView, SLOT(deleteEvent(Event *)));

    connect( mMonthView, SIGNAL( incidenceSelected( Incidence * ) ),
             mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

    connect(mMainView, SIGNAL(configChanged()), mMonthView, SLOT(updateConfig()));
  }

  showView(mMonthView);
}

void KOViewManager::showTodoView()
{
  if ( !mTodoView ) {
    mTodoView = new KOTodoView( mMainView->calendar(), mMainView->viewStack(),
                                "KOViewManager::TodoView" );
    addView( mTodoView );

    // SIGNALS/SLOTS FOR TODO VIEW
    connect( mTodoView, SIGNAL( newTodoSignal() ),
             mMainView, SLOT( newTodo() ) );
    connect( mTodoView, SIGNAL( newSubTodoSignal( Todo * ) ),
             mMainView, SLOT( newSubTodo( Todo *) ) );
    connect( mTodoView, SIGNAL( showTodoSignal( Todo *) ),
             mMainView, SLOT( showTodo( Todo * ) ) );
    connect( mTodoView, SIGNAL( editTodoSignal( Todo * ) ),
             mMainView, SLOT( editTodo( Todo * ) ) );
    connect( mTodoView, SIGNAL( deleteTodoSignal( Todo * ) ),
             mMainView, SLOT( deleteTodo( Todo * ) ) );
    connect( mTodoView, SIGNAL( purgeCompletedSignal() ),
             mMainView, SLOT( purgeCompleted() ) );

    connect( mTodoView, SIGNAL( incidenceSelected( Incidence * ) ),
             mMainView, SLOT( processMainViewSelection( Incidence * ) ) );

    connect( mMainView, SIGNAL( configChanged() ), mTodoView,
             SLOT( updateConfig() ) );
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

Incidence *KOViewManager::currentSelection()
{
  if (!mCurrentView) return 0;
  
  return mCurrentView->selectedIncidences().first();
}

void KOViewManager::addView(KOrg::BaseView *view)
{
  mMainView->viewStack()->addWidget(view,1);
}

void KOViewManager::setDocumentId( const QString &id )
{
  if (mTodoView) mTodoView->setDocumentId( id );
}
