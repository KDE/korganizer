#ifndef _KOEVENTEDITOR_H
#define _KOEVENTEDITOR_H
// 	$Id$	

#include <klineedit.h>
#include <kdialogbase.h>

#include <qdatetime.h>

#include "calendar.h"
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
    void editEvent( Event *, QDate qd=QDate::currentDate());

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
