#ifndef INCIDENCEVISITOR_H
#define INCIDENCEVISITOR_H
// $Id$
//
// IncidenceVisitor
//

class Event;
class Todo;
class Journal;

/**
  This class provides the interface for a visitor of calendar components. It
  serves as base class for concrete visitors, which implement certain actions on
  calendar components. It allows to add functions, which operate on the concrete
  types of calendar components, without changing the calendar component classes.
*/
class IncidenceVisitor
{
  public:
    virtual ~IncidenceVisitor() {}

    virtual bool visit(Event *) { return false; }
    virtual bool visit(Todo *) { return false; }
    virtual bool visit(Journal *) { return false; }
    
  protected:
    IncidenceVisitor() {}
};

#endif
