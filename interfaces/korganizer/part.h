#ifndef KORG_PART_H
#define KORG_PART_H
// $Id$

#include <qstring.h>

#include <klibloader.h>
#include <kparts/part.h>

#include <korganizer/mainwindow.h>

namespace KOrg {

class Part : public KParts::Part {
  public:
    typedef QPtrList<Part> List;

    Part(MainWindow *parent, const char *name) :
      KParts::Part(parent,name), mMainWindow(parent) {};

    virtual ~Part() {};

    virtual QString info() = 0;
  
    MainWindow *mainWindow() { return mMainWindow; }
  
  private:
    MainWindow *mMainWindow;
};

class PartFactory : public KLibFactory {
  public:
    virtual Part *create(MainWindow *parent, const char *name=0) = 0;

  protected:
    virtual QObject* createObject(QObject*, const char*,const char*,
                                  const QStringList &)
    {
      return 0;
    } 
};

}

#endif
