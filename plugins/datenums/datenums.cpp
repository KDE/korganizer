// $Id$

#include "datenums.h"

class DatenumsFactory : public TextDecorationFactory {
  public:
    TextDecoration *create() { return new Datenums; }
};

extern "C" {
  void *init_libkorg_datenums()
  {
    return (new DatenumsFactory);
  }
}


QString Datenums::dayShort(const QDate &date)
{
  return QString::number(date.dayOfYear());
}

QString Datenums::weekShort(const QDate &date)
{
  int weekNumber = date.dayOfYear() % 7;

  // TODO: Consider the rule about week counting at the beginning of the year

  return QString::number(weekNumber);
}

QString Datenums::info()
{
  return i18n("This plugin provides numbers of days and weeks.");
}
