// $Id$
// EVENT CLASS

#ifndef _KOEVENT_H
#define _KOEVENT_H

#include <qobject.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qbitarray.h>

#include "qdatelist.h"
#include "event.h"

class CalObject;
class KOEvent;
class VCalFormat;

/** This is a class which contains all the information necessary about a single
 * event, or an event that occurs multiple times through recurrence
 * information. Methods provide access to the data, which is as fully
 * encapsulated as possible.
 *
 * @short a class which provides an abstract view of an event or appointment.
 * @author Preston Brown
 * @version $Revision$
 */
class KOEvent : public Event {
public:
  /** constructs a new event with variables initialized to "sane" values. */
  KOEvent();
  ~KOEvent();

};

#endif
