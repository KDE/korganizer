#ifndef KORG_PLUGIN_H
#define KORG_PLUGIN_H
// $Id$

#include <klocale.h>
#include <klibloader.h>

namespace KOrg {

class Plugin {
  public:
    Plugin() {};
    virtual ~Plugin() {};
    
    virtual QString info() = 0;

    virtual void configure(QWidget *) {};
};

class PluginFactory : KLibFactory {
  public:
    virtual Plugin *create() = 0;

  protected:
    virtual QObject* createObject(QObject*, const char*,const char*,
                                  const QStringList &)
    {
      return 0;
    }
};

}

#endif
