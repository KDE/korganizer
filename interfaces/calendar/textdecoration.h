#ifndef KORG_TEXTDECORATION_H
#define KORG_TEXTDECORATION_H
// $Id$

#include <qstring.h>
#include <qdatetime.h>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

class TextDecoration : public Plugin {
  public:
    typedef QList<TextDecoration> List;

    TextDecoration() {};
    virtual ~TextDecoration() {};
    
    virtual QString dayShort(const QDate &) { return QString::null; }
    virtual QString dayLong(const QDate &) { return QString::null; }
};

class TextDecorationFactory : public PluginFactory {
  public:
    virtual TextDecoration *create() = 0;
};

}

#endif
