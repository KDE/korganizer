/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _CALPRINTPLUGINS_H
#define _CALPRINTPLUGINS_H


#include <klocale.h>
#include "calprintbase.h"

#ifndef KORG_NOPRINTER
namespace KCal {
class Calendar;
}

using namespace KCal;

class CalPrintDay : public CalPrintBase
{
    Q_OBJECT
  public:
    CalPrintDay(KPrinter *printer, Calendar *cal, KConfig *cfg);
    virtual ~CalPrintDay();
    virtual QString description() { return i18n("Print day"); }
    virtual QString longDescription() { return i18n("Prints all events of a single day on one page"); }
    virtual QWidget *configWidget( QWidget* );

  public slots:
    void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate& from, const QDate& to );

  protected:
    QTime mStartTime, mEndTime;
    bool mIncludeTodos;
    bool mIncludeAllEvents;
};

class CalPrintWeek : public CalPrintBase
{
    Q_OBJECT
  public:
    CalPrintWeek(KPrinter *printer, Calendar *cal, KConfig *cfg);
    virtual ~CalPrintWeek();
    virtual QString description() { return i18n("Print week"); }
    virtual QString longDescription() { return i18n("Prints all events of one week on one page"); }
    virtual QWidget *configWidget(QWidget*);
    virtual KPrinter::Orientation orientation();

  public slots:
    void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate& from, const QDate& to );

  protected:
    enum eWeekPrintType { Filofax=0, Timetable} mWeekPrintType;
    QTime mStartTime, mEndTime;
    bool mIncludeTodos;
};

class CalPrintMonth : public CalPrintBase
{
    Q_OBJECT
  public:
    CalPrintMonth(KPrinter *printer, Calendar *cal, KConfig *cfg);
    virtual ~CalPrintMonth();
    virtual QString description() { return i18n("Print month"); }
    virtual QString longDescription() { return i18n("Prints all events of one month on one page"); }
    virtual QWidget *configWidget(QWidget*);

  public slots:
    void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate& from, const QDate& to );

  protected:
    bool mWeekNumbers;
    bool mIncludeTodos;
};

class CalPrintTodos : public CalPrintBase
{
    Q_OBJECT
  public:
    CalPrintTodos(KPrinter *printer, Calendar *cal, KConfig *cfg);
    virtual ~CalPrintTodos();
    virtual QString description() { return i18n("Print todos"); }
    virtual QString longDescription() { return i18n("Prints all todos in a (tree-like) list"); }
    virtual QWidget *configWidget(QWidget*);

  public slots:
    void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();

  protected:
    QString mPageTitle;

    enum eTodoPrintType {
      TodosAll = 0, TodosUnfinished, TodosSelected, TodosDueRange
    } mTodoPrintType;

    bool mIncludeDescription;
    bool mIncludePriority;
    bool mIncludeDueDate;
    bool mConnectSubTodos;
};

#endif
#endif
