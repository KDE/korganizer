#ifndef KORG_HOLIDAYS_H
#define KORG_HOLIDAYS_H
// $Id$

#include <qstring.h>

#include <calendar/textdecoration.h>

using namespace KOrg;

class Holidays : public TextDecoration {
  public:
    Holidays();
    ~Holidays();
    
    QString dayShort(const QDate &);
    
    QString info();

    void configure(QWidget *parent);
    
    QString getHoliday(const QDate &);
    
  private:
    QString mHolidayFile;
};

#endif
