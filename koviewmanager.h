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
*/
#ifndef KOVIEWMANAGER_H
#define KOVIEWMANAGER_H
// $Id$

#include <qobject.h>

class CalendarView;

class KOListView;
class KOAgendaView;
class KOMonthView;
class KOTimeSpanView;
class KOTodoView;
class KOWhatsNextView;
class KOJournalView;

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
    void showView(KOrg::BaseView *);
    
    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);
      
    /** Read which view was shown last from config file */
    void readCurrentView(KConfig *);
    /** Write which view is currently shown to config file */
    void writeCurrentView(KConfig *);

    KOrg::BaseView *currentView();

    void setDocumentId( const QString & );

    void updateView( const QDate &start, const QDate &end );

    void raiseCurrentView();

    void addView(KOrg::BaseView *);

    Incidence *currentSelection();

  public slots:
    
    /** change Agenda view */
    void changeAgendaView( int view );
       
    void showWhatsNextView();
    void showListView();
    void showAgendaView();
    void showDayView();
    void showWorkWeekView();
    void showWeekView();
    void showMonthView();
    void showTodoView();
    void showJournalView();
    void showTimeSpanView();

  private:
    CalendarView *mMainView;

    KOAgendaView    *mAgendaView;
    KOListView      *mListView;
    KOMonthView     *mMonthView; 
    KOTodoView      *mTodoView;
    KOWhatsNextView *mWhatsNextView;
    KOJournalView   *mJournalView;
    KOTimeSpanView  *mTimeSpanView;
  
    KOrg::BaseView     *mCurrentView;  // currently active event view
    
    int mAgendaViewMode;
  
};

#endif
