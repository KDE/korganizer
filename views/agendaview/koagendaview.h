/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef KOAGENDAVIEW_H
#define KOAGENDAVIEW_H

#include "../../koeventview.h"
#include "../../printing/calprinter.h"

#include <KCalCore/Todo>

class QSplitter;

namespace EventViews {
  class AgendaView;
}

/**
  KOAgendaView is the agenda-like view that displays events in a single
  or multi-day view.
*/
class KOAgendaView : public KOEventView
{
  Q_OBJECT
  public:
    explicit KOAgendaView( QWidget *parent = 0, bool isSideBySide = false );
    virtual ~KOAgendaView();

    /** Returns maximum number of days supported by the koagendaview */
    virtual int maxDatesHint() const;

    /** Returns number of currently shown dates. */
    virtual int currentDateCount() const;

    /** returns the currently selected events */
    virtual Akonadi::Item::List selectedIncidences();

    /** returns the currently selected incidence's dates */
    virtual KCalCore::DateList selectedIncidenceDates();

    /** return the default start/end date/time for new events   */
    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay );

    /** Remove all events from view */
    void clearView();

    CalPrinter::PrintType printType() const;

    /** start-datetime of selection */
    virtual QDateTime selectionStart();

    /** end-datetime of selection */
    virtual QDateTime selectionEnd();

    /** returns true if selection is for whole day */
    bool selectedIsAllDay();
    /** make selected start/end invalid */
    void deleteSelectedDateTime();
    /** returns if only a single cell is selected, or a range of cells */
    bool selectedIsSingleCell();

    /* reimp from BaseView */
    virtual void setCalendar( CalendarSupport::Calendar *cal );

    /** Show only incidences from the given collection selection. */
//    void setCollectionSelection( CollectionSelection* selection );
    void setCollection( Akonadi::Collection::Id id );
    Akonadi::Collection::Id collection() const;

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );

    void changeIncidenceDisplayAdded( const Akonadi::Item &incidence );
    void changeIncidenceDisplay( const Akonadi::Item &incidence, int mode );

    void clearSelection();

    void readSettings();
    void readSettings( KConfig * );
    void writeSettings( KConfig * );

    /** reschedule the todo  to the given x- and y- coordinates.
        Third parameter determines all-day (no time specified) */
    void slotTodosDropped( const Todo::List  &todos, const QPoint &, bool );
    void slotTodosDropped( const QList<KUrl> &todos, const QPoint &, bool );

    void enableAgendaUpdate( bool enable );
    void setIncidenceChanger( CalendarSupport::IncidenceChanger *changer );

    void zoomInHorizontally( const QDate &date=QDate() );
    void zoomOutHorizontally( const QDate &date=QDate() );

    void zoomInVertically( );
    void zoomOutVertically( );

    void zoomView( const int delta, const QPoint &pos, const Qt::Orientation orient=Qt::Horizontal );

  signals:
    void zoomViewHorizontally( const QDate &, int count );
    void timeSpanSelectionChanged();

  private:
    class Private;
    Private *const d;
};

#endif
