#ifndef _KOTODOEDITOR_H
#define _KOTODOEDITOR_H
// 	$Id$	

#include <kdialogbase.h>

#include <qdatetime.h>

#include "calendar.h"
#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"

class CategorySelectDialog;

using namespace KCal;

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
    KOTodoEditor(Calendar *calendar);
    virtual ~KOTodoEditor(void);

    /** Clear eventwin for new todo, and preset the dates and times with hint
     */
    void newTodo(QDateTime due,Todo *relatedTodo=0,bool allDay=false);

    /** Edit an existing todo. */
    void editTodo(Todo *, QDate qd=QDate::currentDate());

    /** Set widgets to default values */
    void setDefaults(QDateTime due,Todo *relatedTodo,bool allDay);
    /** Read event object and setup widgets accordingly */
    void readTodo(Todo *);
    /** Write event settings to event object */
    void writeTodo(Todo *);

    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();

  public slots:
    void updateCategoryConfig();

  signals:
    void todoChanged(Todo *);
    void todoAdded(Todo *);
    void todoToBeDeleted(Todo *);
    void todoDeleted();

    void editCategories();

  protected slots:
    void slotDefault();
    void slotApply();
    void slotOk();
    void slotUser1();
  
  protected:
    void setupGeneralTab();
    void setupDetailsTab();

  private:
    Calendar *mCalendar;
  
    Todo *mTodo;
    
    Todo *mRelatedTodo;

    KOEditorGeneralTodo *mGeneral;
    KOEditorDetails     *mDetails;

    CategorySelectDialog *mCategoryDialog;    
};

#endif
