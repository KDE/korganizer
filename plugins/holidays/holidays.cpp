// $Id$

#include <qfile.h>

#include <kapp.h>
#include <kconfig.h>
#include <kstddirs.h>

#include "configdialog.h"

#include "holidays.h"

class HolidaysFactory : public TextDecorationFactory {
  public:
    TextDecoration *create() { return new Holidays; }
};

extern "C" {
  void *init_libkorg_holidays()
  {
    return (new HolidaysFactory);
  }
}


extern "C" {
  char *parse_holidays(const char *, int year, short force);
  /** \internal */
  struct holiday {
    char            *string;        /* name of holiday, 0=not a holiday */
    unsigned short  dup;            /* reference count */
  };
  extern struct holiday holiday[366];
};


Holidays::Holidays()
{
  kapp->config()->setGroup("Calendar/Holiday Plugin");
  QString holiday = kapp->config()->readEntry("Holidays");

  mHolidayFile = locate("appdata","holiday_" + holiday);
}

Holidays::~Holidays()
{
}

QString Holidays::dayShort(const QDate &date)
{
  return getHoliday(date);
}

QString Holidays::info()
{
  return i18n("This plugin provides holidays.");
}

void Holidays::configure(QWidget *parent)
{
  ConfigDialog *dlg = new ConfigDialog(parent);
  dlg->exec();
}

QString Holidays::getHoliday(const QDate &qd)
{
  static int lastYear = 0;

  if (mHolidayFile.isEmpty()) return QString::null;

  if ((lastYear == 0) || (qd.year() != lastYear)) {
      lastYear = qd.year() - 1900; // silly parse_year takes 2 digit year...
      parse_holidays(QFile::encodeName(mHolidayFile), lastYear, 0);
  }

  if (holiday[qd.dayOfYear()-1].string) {
    QString holidayname = QString::fromLocal8Bit(holiday[qd.dayOfYear()-1].string);
    return holidayname;
  } else {
    return QString::null;
  }
}
