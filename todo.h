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
};

#endif
