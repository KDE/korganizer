#ifndef _KOTODOEDITOR_H
#define _KOTODOEDITOR_H
// 	$Id$	

#include <qdatetime.h>

#include "calendar.h"
#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"
#include "koincidenceeditor.h"

/**
  This class provides a dialog for editing a Todo.
*/
class KOTodoEditor : public KOIncidenceEditor
{
    Q_OBJECT
  public:
    /**
      Constructs a new todo editor.
    */  
    KOTodoEditor(Calendar *calendar);
    virtual ~KOTodoEditor(void);

    /**
      Clear editor for new todo, and preset the dates and times with hint.
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

  signals:
    void todoChanged(Todo *);
    void todoAdded(Todo *);
    void todoToBeDeleted(Todo *);
    void todoDeleted();

  protected slots:
    void slotDefault();
    void slotUser1();
  
  protected:
    QWidget *setupGeneralTabWidget(QWidget *);

  private:  
    Todo *mTodo;
    
    Todo *mRelatedTodo;

    KOEditorGeneralTodo *mGeneral;
};

#endif
