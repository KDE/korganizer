/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _KOEVENTEDITOR_H
#define _KOEVENTEDITOR_H
// $Id$	

#include <kdialogbase.h>

#include <qdatetime.h>

#include <libkcal/calendar.h>

#include "koeditorgeneralevent.h"
#include "koeditordetails.h"
#include "koeditorrecurrence.h"
#include "koincidenceeditor.h"

class QWidgetStack;

using namespace KCal;

/**
  This class provides a dialog for editing an event.
*/
class KOEventEditor : public KOIncidenceEditor
{
    Q_OBJECT
  public:
    /**
      Construct new event editor.
    */
    KOEventEditor( Calendar *calendar);
    virtual ~KOEventEditor(void);

    /** Clear eventwin for new event, and preset the dates and times with hint */
    void newEvent( QDateTime from, QDateTime to, bool allDay = FALSE );

    /** Edit an existing event. */
    void editEvent( Event * );

    /** Set widgets to default values */
    void setDefaults(QDateTime from,QDateTime to,bool allDay);
    /** Read event object and setup widgets accordingly */
    void readEvent(Event *);
    /** Write event settings to event object */
    void writeEvent(Event *);

  signals:
    void eventAdded(Event *);
    void eventChanged(Event *);
    void eventToBeDeleted(Event *);
    void eventDeleted();

  protected slots:
    void slotDefault();
    void slotUser1();
    void enableRecurrence(bool);

  protected:
    void setupCustomTabs();
  
    void setupRecurrenceTab();
    QWidget *setupGeneralTabWidget(QWidget *);

    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();
    
  private:
    Event *mEvent;

    KOEditorGeneralEvent *mGeneral;
    KOEditorRecurrence   *mRecurrence;

    QWidgetStack *mRecurrenceStack;
    QLabel *mRecurrenceDisabled;
};

#endif
