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
#ifndef CALPRINTDEFAULTPLUGINS_H
#define CALPRINTDEFAULTPLUGINS_H


#include <klocale.h>
#include "calprintpluginbase.h"

#ifndef KORG_NOPRINTER
namespace KCal {
class Calendar;
}

using namespace KCal;

class CalPrintDay : public CalPrintPluginBase
{
  public:
    CalPrintDay();
    virtual ~CalPrintDay();
    virtual QString description() { return i18n("Print day"); }
    virtual QString info() { return i18n("Prints all events of a single day on one page"); }
    virtual QWidget *createConfigWidget( QWidget* );

  public:
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

class CalPrintWeek : public CalPrintPluginBase
{
  public:
    CalPrintWeek();
    virtual ~CalPrintWeek();
    virtual QString description() { return i18n("Print week"); }
    virtual QString info() { return i18n("Prints all events of one week on one page"); }
    virtual QWidget *createConfigWidget(QWidget*);
    virtual KPrinter::Orientation orientation();

  public:
    void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate& from, const QDate& to );

  protected:
    enum eWeekPrintType { Filofax=0, Timetable, SplitWeek } mWeekPrintType;
    QTime mStartTime, mEndTime;
    bool mIncludeTodos;
};

class CalPrintMonth : public CalPrintPluginBase
{
  public:
    CalPrintMonth();
    virtual ~CalPrintMonth();
    virtual QString description() { return i18n("Print month"); }
    virtual QString info() { return i18n("Prints all events of one month on one page"); }
    virtual QWidget *createConfigWidget(QWidget*);
    virtual KPrinter::Orientation orientation() { return KPrinter::Landscape; }

  public:
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

class CalPrintTodos : public CalPrintPluginBase
{
  public:
    CalPrintTodos();
    virtual ~CalPrintTodos();
    virtual QString description() { return i18n("Print todos"); }
    virtual QString info() { return i18n("Prints all todos in a (tree-like) list"); }
    virtual QWidget *createConfigWidget(QWidget*);

  public:
    void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();

  protected:
    QString mPageTitle;

    enum eTodoPrintType {
      TodosAll = 0, TodosUnfinished, TodosDueRange
    } mTodoPrintType;

    bool mIncludeDescription;
    bool mIncludePriority;
    bool mIncludeDueDate;
    bool mConnectSubTodos;
};

#endif
#endif
