#ifndef KORG_PART_H
#define KORG_PART_H
// $Id$

#include <qstring.h>

#include <klibloader.h>
#include <kparts/part.h>

#include <korganizer.h>

namespace KOrg {

class Part : public KParts::Part {
  public:
    typedef QList<Part> List;

    Part(KOrganizer *parent, const char *name) :
      KParts::Part(parent,name), mMainWindow(parent) {};

    virtual ~Part() {};

    virtual QString info() = 0;
  
    KOrganizer *mainWindow() { return mMainWindow; }
  
  private:
    KOrganizer *mMainWindow;
};

class PartFactory : public KLibFactory {
  public:
    virtual Part *create(KOrganizer *parent, const char *name=0) = 0;
};

}

#endif
