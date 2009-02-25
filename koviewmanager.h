/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef KOVIEWMANAGER_H
#define KOVIEWMANAGER_H

#include <QObject>
#include <QDate>

class KConfig;
class KTabWidget;

class CalendarView;
class KOListView;
class KOAgendaView;
class KOMonthView;
class KOTodoView;
class KOWhatsNextView;
class KOJournalView;
class KOTimelineView;
class KOTimeSpentView;

namespace KOrg {
  class BaseView;
  class MultiAgendaView;
  class MonthView;
}

namespace KCal {
  class Incidence;
}

/**
  This class manages the views of the calendar. It owns the objects and handles
  creation and selection.
*/
class KOViewManager : public QObject
{
  Q_OBJECT
  public:
    explicit KOViewManager( CalendarView * );
    virtual ~KOViewManager();

    /** changes the view to be the currently selected view */
    void showView( KOrg::BaseView * );

    void readSettings( KConfig *config );
    void writeSettings( KConfig *config );

    /** Read which view was shown last from config file */
    void readCurrentView( KConfig *config );
    /** Write which view is currently shown to config file */
    void writeCurrentView( KConfig *config );

    KOrg::BaseView *currentView();

    void setDocumentId( const QString & );

    void updateView();
    void updateView( const QDate &start, const QDate &end );

    void raiseCurrentView();

    void connectView( KOrg::BaseView * );
    void addView( KOrg::BaseView *, bool isTab = false );

    KCal::Incidence *currentSelection();
    QDate currentSelectionDate();

    KOAgendaView *agendaView() const { return mAgendaView; }
    KOTodoView *todoView() const { return mTodoView; }
    KOrg::MonthView *monthView() const { return mMonthView; }

  signals:
    void configChanged();

  public slots:
    void showWhatsNextView();
    void showListView();
    void showAgendaView();
    void showDayView();
    void showWorkWeekView();
    void showWeekView();
    void showNextXView();
    void showTodoView();
    void showTimeLineView();
    void showTimeSpentView();
    void showMonthView();
    void showJournalView();

    void showEventView();

    void connectTodoView( KOTodoView *todoView );

    void zoomInHorizontally();
    void zoomOutHorizontally();
    void zoomInVertically();
    void zoomOutVertically();

    void setUpdateNeeded();

  private slots:
    void currentAgendaViewTabChanged( QWidget * );

  private:
    QWidget *widgetForView( KOrg::BaseView * ) const;
    CalendarView *mMainView;

    KOAgendaView *mAgendaView;
    KOrg::MultiAgendaView *mAgendaSideBySideView;
    KOListView *mListView;
    KOTodoView *mTodoView;
    KOWhatsNextView *mWhatsNextView;
    KOJournalView *mJournalView;
    KOTimelineView *mTimelineView;
    KOTimeSpentView *mTimeSpentView;
    KOrg::MonthView *mMonthView;
    KOrg::BaseView *mCurrentView;

    KOrg::BaseView *mLastEventView;
    KTabWidget *mAgendaViewTabs;
};

#endif
