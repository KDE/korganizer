#ifndef _KOEVENTEDITOR_H
#define _KOEVENTEDITOR_H
// 	$Id$	

#include <ktmainwindow.h>
#include <klineedit.h>
#include <kdialogbase.h>

#include <qdatetime.h>

#include "calobject.h"
#include "koeditorgeneralevent.h"
#include "koeditordetails.h"
#include "koeditorrecurrence.h"

class CategoryDialog;
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
    void editEvent( KOEvent *, QDate qd=QDate::currentDate());

    /** Set widgets to default values */
    void setDefaults(QDateTime from,QDateTime to,bool allDay);
    /** Read event object and setup widgets accordingly */
    void readEvent(KOEvent *);
    /** Write event settings to event object */
    void writeEvent(KOEvent *);

  public slots:

  signals:
    void eventAdded(KOEvent *);
    void eventChanged(KOEvent *);
    void eventToBeDeleted(KOEvent *);
    void eventDeleted();

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
    
  private:
    CalObject *mCalendar;
  
    KOEvent *mEvent;

    KOEditorGeneralEvent *mGeneral;
    KOEditorDetails      *mDetails;
    KOEditorRecurrence   *mRecurrence;

    CategoryDialog *mCategoryDialog;
    
    QWidgetStack *mRecurrenceStack;
    QLabel *mRecurrenceDisabled;
};

#endif


