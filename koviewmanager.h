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

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KOVIEWMANAGER_H
#define KOVIEWMANAGER_H

#include <qobject.h>

class CalendarView;

class KOListView;
class KOAgendaView;
class KOMonthView;
class KOTodoView;
class KOWhatsNextView;
class KOJournalView;

namespace KOrg { class BaseView; }
using namespace KCal;

/**
  This class manages the views of the calendar. It owns the objects and handles
  creation and selection.
*/
class KOViewManager : public QObject
{
    Q_OBJECT
  public:
    KOViewManager( CalendarView * );
    virtual ~KOViewManager();

    /** changes the view to be the currently selected view */
    void showView( KOrg::BaseView * );

    void readSettings( KConfig *config );
    void writeSettings( KConfig *config );

    /** Read which view was shown last from config file */
    void readCurrentView( KConfig * );
    /** Write which view is currently shown to config file */
    void writeCurrentView( KConfig * );

    KOrg::BaseView *currentView();

    void setDocumentId( const QString & );

    void updateView();
    void updateView( const QDate &start, const QDate &end );

    void raiseCurrentView();

    void connectView( KOrg::BaseView * );
    void addView( KOrg::BaseView * );

    Incidence *currentSelection();
    QDate currentSelectionDate();

    KOAgendaView *agendaView() const { return mAgendaView; }
    KOTodoView   *todoView() const { return mTodoView; }

  public slots:
    void showWhatsNextView();
    void showListView();
    void showAgendaView();
    void showDayView();
    void showWorkWeekView();
    void showWeekView();
    void showNextXView();
    void showMonthView();
    void showTodoView();
    void showJournalView();

    void showEventView();

    void connectTodoView( KOTodoView *todoView );

    void zoomInHorizontally();
    void zoomOutHorizontally();
    void zoomInVertically();
    void zoomOutVertically();

  private:
    CalendarView *mMainView;

    KOAgendaView    *mAgendaView;
    KOListView      *mListView;
    KOMonthView     *mMonthView;
    KOTodoView      *mTodoView;
    KOWhatsNextView *mWhatsNextView;
    KOJournalView   *mJournalView;

    KOrg::BaseView *mCurrentView;

    KOrg::BaseView *mLastEventView;

    int mAgendaViewMode;
};

#endif
