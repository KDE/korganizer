#ifndef KORG_PART_H
#define KORG_PART_H
// $Id$

#include <qstring.h>

#include <klibloader.h>
#include <kparts/part.h>

#include <calendar.h>

namespace KOrg {

class Part : public KParts::Part {
  public:
    typedef QList<Part> List;

    Part(KCal::Calendar *calendar,QObject *parent, const char *name) :
      KParts::Part(parent,name), mCalendar(calendar) {};

    virtual ~Part() {};

    virtual QString info() = 0;
  
    KCal::Calendar *calendar() { return mCalendar; }
  
  private:
    KCal::Calendar *mCalendar;
};

class PartFactory : public KLibFactory {
  public:
    virtual Part *create(KCal::Calendar *,QObject *parent, const char *name) = 0;
};

}

#endif
