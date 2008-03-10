/*
    This file is part of the KOrganizer interfaces.

    Copyright (c) 1999,2001,2003 Cornelius Schumacher <schumacher@kde.org>
		Copyright (C) 2004           Reinhold Kainhofer   <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KORG_BASEVIEW_H
#define KORG_BASEVIEW_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qvaluelist.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kdepimmacros.h>
#include "korganizer/incidencechangerbase.h"

#include "printplugin.h"

#include <libkcal/event.h>

using namespace KCal;

namespace KCal { class Calendar; }

namespace KOrg {


/**
  This class provides an interface for all views being displayed within the main
  calendar view. It has functions to update the view, to specify date range and
  other display parameter and to return selected objects. An important class,
  which inherits KOBaseView is KOEventView, which provides the interface for all
  views of event data like the agenda or the month view.

  @short Base class for calendar views
  @author Preston Brown, Cornelius Schumacher
  @see KOTodoView, KOEventView, KOListView, KOAgendaView, KOMonthView
*/
class KDE_EXPORT BaseView : public QWidget
{
    Q_OBJECT
  public:
    /**
      Constructs a view.

      @param cal    Pointer to the calendar object from which events
                    will be retrieved for display.
      @param parent parent widget.
      @param name   name of this widget.
    */
    BaseView( Calendar *cal, QWidget *parent = 0,
              const char *name = 0 )
      : QWidget( parent, name ), mCalendar( cal ), mChanger( 0 ) {}

    /**
      Destructor.  Views will do view-specific cleanups here.
    */
    virtual ~BaseView() {}

    virtual void setCalendar( Calendar *cal ) { mCalendar = cal; }
    /**
      Return calendar object of this view.
    */
    virtual Calendar *calendar() { return mCalendar; }

    /**
      @return a list of selected events.  Most views can probably only
      select a single event at a time, but some may be able to select
      more than one.
    */
    virtual Incidence::List selectedIncidences() = 0;

    /**
      @return a list of the dates of selected events.  Most views can probably only
      select a single event at a time, but some may be able to select
      more than one.
    */
    virtual DateList selectedDates() = 0;

    virtual CalPrinterBase::PrintType printType()
    {
      return CalPrinterBase::Month;
    }

    /**
      Return number of currently shown dates. A return value of 0 means no idea.
    */
    virtual int currentDateCount() = 0;

    /** Return if this view is a view for displaying events. */
    virtual bool isEventView() { return false; }

  public slots:
    /**
      Show incidences for the given date range. The date range actually shown may be
      different from the requested range, depending on the particular requirements
      of the view.

      @param start Start of date range.
      @param end   End of date range.
    */
    virtual void showDates( const QDate &start, const QDate &end ) = 0;

    /**
      Show given incidences. Depending on the actual view it might not be possible to
      show all given events.

      @param incidenceList a list of incidences to show.
    */
    virtual void showIncidences( const Incidence::List &incidenceList ) = 0;

    /**
      Updates the current display to reflect changes that may have happened
      in the calendar since the last display refresh.
    */
    virtual void updateView() = 0;
    virtual void dayPassed( const QDate & ) { updateView(); }

    /**
      Assign a new incidence change helper object.
     */
    virtual void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }

    /**
      Write all unsaved data back to calendar store.
    */
    virtual void flushView() {}

    /**
      Updates the current display to reflect the changes to one particular incidence.
    */
    virtual void changeIncidenceDisplay( Incidence *, int ) = 0;

    /**
      Re-reads the KOrganizer configuration and picks up relevant
      changes which are applicable to the view.
    */
    virtual void updateConfig() {}

    /**
      Clear selection. The incidenceSelected signal is not emitted.
    */
    virtual void clearSelection() {}

    /**
      Set the default start/end date/time for new events. Return true if anything was changed
    */
    virtual bool eventDurationHint(QDateTime &/*startDt*/, QDateTime &/*endDt*/, bool &/*allDay*/) { return false; }

  signals:
    void incidenceSelected( Incidence * );

    /**
     * instructs the receiver to show the incidence in read-only mode.
     */
    void showIncidenceSignal(Incidence *);

    /**
     * instructs the receiver to begin editing the incidence specified in
     * some manner.  Doesn't make sense to connect to more than one
     * receiver.
     */
    void editIncidenceSignal(Incidence *);

    /**
     * instructs the receiver to delete the Incidence in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteIncidenceSignal(Incidence *);

    /**
    * instructs the receiver to cut the Incidence
    */
    void cutIncidenceSignal(Incidence *);

    /**
    * instructs the receiver to copy the incidence
    */
    void copyIncidenceSignal(Incidence *);

    /**
    * instructs the receiver to paste the incidence
    */
    void pasteIncidenceSignal();

    /**
     * instructs the receiver to toggle the alarms of the Incidence.
     */
    void toggleAlarmSignal(Incidence *);
    /** Dissociate from a recurring incidence the occurrence on the given
        date to a new incidence */
    void dissociateOccurrenceSignal( Incidence *, const QDate & );
    /** Dissociate from a recurring incidence all occurrences after the given
        date to a new incidence */
    void dissociateFutureOccurrenceSignal( Incidence *, const QDate & );

    void startMultiModify( const QString & );
    void endMultiModify();

    /**
     * instructs the receiver to create a new event.  Doesn't make
     * sense to connect to more than one receiver.
     */
    void newEventSignal();
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const QDate & );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const QDateTime & );
    /**
     * instructs the receiver to create a new event, with the specified
     * beginning end ending times.  Doesn't make sense to connect to more
     * than one receiver.
     */
    void newEventSignal( const QDateTime &, const QDateTime & );

    void newTodoSignal( const QDate & );
    void newSubTodoSignal( Todo * );

    void newJournalSignal( const QDate & );

  private:
    Calendar *mCalendar;
  protected:
    IncidenceChangerBase *mChanger;
};

}

#endif
