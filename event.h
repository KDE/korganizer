#ifndef EVENT_H
#define EVENT_H
// $Id$
//
// Event component, representing a VEVENT object
//

#include "incidence.h"

class Event : public Incidence
{
  public:
    Event();
    ~Event();
};

#endif
