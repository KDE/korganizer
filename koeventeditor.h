#ifndef _KOEVENTEDITOR_H
#define _KOEVENTEDITOR_H
// 	$Id$	

#include <klineedit.h>
#include <kdialogbase.h>

#include <qdatetime.h>

#include "calobject.h"
#include "koeditorgeneralevent.h"
#include "koeditordetails.h"
#include "koeditorrecurrence.h"

class CategorySelectDialog;
class QWidgetStack;

/**
  * This is the class to add/edit a new appointment.
  *
  * @short Creates a dialog box to create/edit an appointment
  * @author Preston Brown
  * @version $Revision$
  */
class KOEventEditor : public KDialogBase
{
    Q_OBJECT
  public:
    /**
     * Constructs a new appointment dialog.
     *
     */
    KOEventEditor( CalObject *calendar);
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

  public slots:
    void updateCategoryConfig();

  signals:
    void eventAdded(Event *);
    void eventChanged(Event *);
    void eventToBeDeleted(Event *);
    void eventDeleted();

    void categoryConfigChanged();
    void editCategories();

  protected slots:
    void slotDefault();
    void slotApply();
    void slotOk();
    void slotUser1();
    void enableRecurrence(bool);

  protected:
    void setupGeneralTab();
    void setupDetailsTab();
    void setupRecurrenceTab();

    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();
    
  private:
    CalObject *mCalendar;
  
    Event *mEvent;

    KOEditorGeneralEvent *mGeneral;
    KOEditorDetails      *mDetails;
    KOEditorRecurrence   *mRecurrence;

    CategorySelectDialog *mCategoryDialog;
    
    QWidgetStack *mRecurrenceStack;
    QLabel *mRecurrenceDisabled;
};

#endif


