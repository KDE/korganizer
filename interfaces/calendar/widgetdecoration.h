#ifndef KORG_WIDGETDECORATION_H
#define KORG_WIDGETDECORATION_H
// $Id$

#include <qstring.h>
#include <qdatetime.h>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

class WidgetDecoration : public Plugin {
  public:
    typedef QList<WidgetDecoration> List;

    WidgetDecoration() {};
    virtual ~WidgetDecoration() {};
    
    virtual QWidget *daySmall(QWidget *,const QDate &) { return 0; }
};

class WidgetDecorationFactory : public PluginFactory {
  public:
    virtual WidgetDecoration *create() = 0;
};

}

#endif
