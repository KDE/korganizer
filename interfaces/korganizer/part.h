#ifndef KORG_PART_H
#define KORG_PART_H
// $Id$

#include <qstring.h>

#include <klibloader.h>
#include <kparts/part.h>

class CalendarView;

namespace KOrg {

class Part : public KParts::Part {
  public:
    typedef QList<Part> List;

    Part(CalendarView *view,QObject *parent, const char *name) :
      KParts::Part(parent,name), mView(view) {};

    virtual ~Part() {};

    virtual QString info() = 0;
  
    CalendarView *view() { return mView; }
  
  private:
    CalendarView *mView;
};

class PartFactory : public KLibFactory {
  public:
    virtual Part *create(CalendarView *,QObject *parent, const char *name) = 0;
};

}

#endif
