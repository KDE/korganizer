/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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
using namespace KOrg;

class CalPrintIncidence : public CalPrintPluginBase
{
  public:
    CalPrintIncidence();
    virtual ~CalPrintIncidence();
    virtual QString description() { return i18n("Print &incidence"); }
    virtual QString info() { return i18n("Prints an incidence on one page"); }
    virtual int sortID() { return CalPrinterBase::Incidence; }
    // Enable the Print Incidence option only if there are selected incidences.
    virtual bool enabled()
      {
        if ( mSelectedIncidences.count() > 0 ) {
          return true;
        } else {
          return false;
        }
      }
    virtual QWidget *createConfigWidget(QWidget*);
    virtual KPrinter::Orientation defaultOrientation()
      { return KPrinter::Portrait; }

  public:
    void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
  protected:
    int printCaptionAndText( QPainter &p, const QRect &box, const QString &caption, 
           const QString &text, QFont captionFont, QFont textFont );
  

  protected:
    bool mShowOptions;
    bool mShowSubitemsNotes;
    bool mShowAttendees;
    bool mShowAttachments;
};


class CalPrintDay : public CalPrintPluginBase
{
  public:
    CalPrintDay();
    virtual ~CalPrintDay();
    virtual QString description() { return i18n("Print da&y"); }
    virtual QString info() { return i18n("Prints all events of a single day on one page"); }
    virtual int sortID() { return CalPrinterBase::Day; }
    virtual bool enabled() { return true; }
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
    virtual QString description() { return i18n("Print &week"); }
    virtual QString info() { return i18n("Prints all events of one week on one page"); }
    virtual int sortID() { return CalPrinterBase::Week; }
    virtual bool enabled() { return true; }
    virtual QWidget *createConfigWidget(QWidget*);
    /**
      Returns the default orientation for the eWeekPrintType.
    */
    virtual KPrinter::Orientation defaultOrientation();

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
    virtual QString description() { return i18n("Print mont&h"); }
    virtual QString info() { return i18n("Prints all events of one month on one page"); }
    virtual int sortID() { return CalPrinterBase::Month; }
    virtual bool enabled() { return true; }
    virtual QWidget *createConfigWidget(QWidget*);
    virtual KPrinter::Orientation defaultOrientation() { return KPrinter::Landscape; }

  public:
    void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate& from, const QDate& to );

  protected:
    bool mWeekNumbers;
    bool mRecurDaily;
    bool mRecurWeekly;
    bool mIncludeTodos;
};

class CalPrintTodos : public CalPrintPluginBase
{
  public:
    CalPrintTodos();
    virtual ~CalPrintTodos();
    virtual QString description() { return i18n("Print to-&dos"); }
    virtual QString info() { return i18n("Prints all to-dos in a (tree-like) list"); }
    virtual int sortID() { return CalPrinterBase::Todolist; }
    virtual bool enabled() { return true; }
    virtual QWidget *createConfigWidget(QWidget*);

  public:
    void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();

  protected:
    QString mPageTitle;

    enum eTodoPrintType {
      TodosAll = 0, TodosUnfinished, TodosDueRange
    } mTodoPrintType;

    enum eTodoSortField {
      TodoFieldSummary=0,
      TodoFieldStartDate, TodoFieldDueDate,
      TodoFieldPriority, TodoFieldPercentComplete,
      TodoFieldUnset
    } mTodoSortField;

    enum eTodoSortDirection {
      TodoDirectionAscending=0, TodoDirectionDescending,
      TodoDirectionUnset
    } mTodoSortDirection;

    bool mIncludeDescription;
    bool mIncludePriority;
    bool mIncludeDueDate;
    bool mIncludePercentComplete;
    bool mConnectSubTodos;
    bool mStrikeOutCompleted;
    bool mSortField;
    bool mSortDirection;
};

#endif
#endif
