#ifndef TODO_H
#define TODO_H
// $Id$
//
// Todo component, representing a VTODO object
//

#include "incidence.h"

class Todo : public Incidence
{
  public:
    Todo();
    ~Todo();

    bool accept(IncidenceVisitor &v) { return v.visit(this); }

    /** for setting the todo's due date/time with a QDateTime. */
    void setDtDue(const QDateTime &dtDue);
    /** returns an event's Due date/time as a QDateTime. */
    const QDateTime &dtDue() const;
    /** returns an event's due time as a string formatted according to the
     users locale settings */
    QString dtDueTimeStr() const;
    /** returns an event's due date as a string formatted according to the
     users locale settings */
    QString dtDueDateStr(bool shortfmt=true) const;
    /** returns an event's due date and time as a string formatted according
     to the users locale settings */
    QString dtDueStr() const;

    /** returns TRUE or FALSE depending on whether the todo has a due date */
    bool hasDueDate() const;
    /** sets the event's hasDueDate value. */
    void setHasDueDate(bool f);

    /** returns TRUE or FALSE depending on whether the todo has a start date */
    bool hasStartDate() const;
    /** sets the event's hasStartDate value. */
    void setHasStartDate(bool f);

    /** sets the event's status to the string specified.  The string
     * must be a recognized value for the status field, i.e. a string
     * equivalent of the possible status enumerations previously described. */
    void setStatus(const QString &statStr);
    /** sets the event's status to the value specified.  See the enumeration
     * above for possible values. */
    void setStatus(int);
    /** return the event's status. */
    int status() const;
    /** return the event's status in string format. */
    QString statusStr() const;

    void setTodoStatus(bool stat) { mIsTodo = stat; emit eventUpdated(this); };
    bool todoStatus() const { return mIsTodo; };

  private:

    QDateTime mDtDue;                     // due date of todo

    bool mHasDueDate;                    // if todo has associated due date
    bool mHasStartDate;                  // if todo has associated start date

    int  mStatus;                         // confirmed/delegated/tentative/etc

    bool mIsTodo;                         // true if this is a "todo"
};

#endif
