#ifndef KORG_DATENUMS_H
#define KORG_DATENUMS_H
// $Id$

#include <qstring.h>

#include <calendar/textdecoration.h>

using namespace KOrg;

class Datenums : public TextDecoration {
  public:
    Datenums() {}
    ~Datenums() {}
    
    QString dayShort(const QDate &);
    QString weekShort(const QDate &);
    
    QString info();
};

#endif
