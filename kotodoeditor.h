#ifndef _KOTODOEDITOR_H
#define _KOTODOEDITOR_H
// 	$Id$	

#include <kdialogbase.h>

#include <qdatetime.h>

#include "calobject.h"
#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"


/**
  * This is the class to add/edit a new appointment.
  *
  * @short Creates a dialog box to create/edit an appointment
  * @author Preston Brown
  * @version $Revision$
  */
class KOTodoEditor : public KDialogBase
{
    Q_OBJECT
  public:
    /**
     * Constructs a new appointment dialog.
     *
     */  
    KOTodoEditor( CalObject *calendar);
    virtual ~KOTodoEditor(void);

    /** Clear eventwin for new todo, and preset the dates and times with hint
     */
    void newTodo(QDateTime due,KOEvent *relatedTodo=0,bool allDay=false);

    /** Edit an existing todo. */
    void editTodo( KOEvent *, QDate qd=QDate::currentDate());

    /** Set widgets to default values */
    void setDefaults(QDateTime due,KOEvent *relatedTodo,bool allDay);
    /** Read event object and setup widgets accordingly */
    void readTodo(KOEvent *);
    /** Write event settings to event object */
    void writeTodo(KOEvent *);

    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();

  public slots:

  signals:
    void todoChanged(KOEvent *);
    void todoAdded(KOEvent *);
    void todoToBeDeleted(KOEvent *);
    void todoDeleted();

    void categoryConfigChanged();

  protected slots:
    void slotDefault();
    void slotApply();
    void slotOk();
    void slotUser1();
  
  protected:
    void setupGeneralTab();
    void setupDetailsTab();

  private:
    CalObject *mCalendar;
  
    KOEvent *mTodo;
    
    KOEvent *mRelatedTodo;

    KOEditorGeneralTodo *mGeneral;
    KOEditorDetails     *mDetails;

    CategoryDialog *mCategoryDialog;    
};

#endif


