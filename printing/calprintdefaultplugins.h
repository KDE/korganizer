/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2008 Ron Goodheart <rong.dev@gmail.com>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef CALPRINTDEFAULTPLUGINS_H
#define CALPRINTDEFAULTPLUGINS_H

#include "calprintpluginbase.h"

#include "ui_calprintincidenceconfig_base.h"
#include "ui_calprintdayconfig_base.h"
#include "ui_calprintweekconfig_base.h"
#include "ui_calprintmonthconfig_base.h"
#include "ui_calprinttodoconfig_base.h"

#include <KLocale>

using namespace KCalCore;
using namespace KOrg;

class CalPrintIncidence : public CalPrintPluginBase
{
  public:
    CalPrintIncidence();
    virtual ~CalPrintIncidence();
    virtual QString groupName()
    {
        return QString::fromLatin1("Print incidence");
    }
    virtual QString description()
    {
      return i18n( "Print &incidence" );
    }
    virtual QString info() const
    {
      return i18n( "Prints an incidence on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Incidence;
    }

  // Enable the Print Incidence option only if there are selected incidences.
    virtual bool enabled()
    {
      if ( mSelectedIncidences.count() > 0 ) {
        return true;
      } else {
        return false;
      }
    }
    virtual QWidget *createConfigWidget( QWidget * );
    virtual QPrinter::Orientation defaultOrientation()
    {
      return QPrinter::Portrait;
    }

  public:
    void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();

  protected:
    int printCaptionAndText( QPainter &p, const QRect &box, const QString &caption,
                             const QString &text, QFont captionFont, QFont textFont );

    bool mShowOptions;
    bool mShowSubitemsNotes;
    bool mShowAttendees;
    bool mShowAttachments;
    bool mShowNoteLines;
};

class CalPrintDay : public CalPrintPluginBase
{
  public:
    CalPrintDay();
    virtual ~CalPrintDay();
    virtual QString groupName()
    {
      return QString::fromLatin1( "Print day" );
    }
    virtual QString description()
    {
      return i18n( "Print da&y" );
    }
    virtual QString info() const
    {
      return i18n( "Prints all events of a single day on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Day;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual QWidget *createConfigWidget( QWidget * );

  public:
    void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate &from, const QDate &to );

  protected:
    enum eDayPrintType {
      Filofax=0,
      Timetable,
      SingleTimetable
    } mDayPrintType;
    QTime mStartTime, mEndTime;
    bool mIncludeDescription;
    bool mSingleLineLimit;
    bool mIncludeTodos;
    bool mIncludeAllEvents;
    bool mExcludeTime;
};

class CalPrintWeek : public CalPrintPluginBase
{
  public:
    CalPrintWeek();
    virtual ~CalPrintWeek();

    virtual QString groupName()
    {
      return QString::fromLatin1( "Print week" );
    }
    virtual QString description()
    {
      return i18n( "Print &week" );
    }
    virtual QString info() const
    {
      return i18n( "Prints all events of one week on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Week;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual QWidget *createConfigWidget( QWidget * );

    /**
      Returns the default orientation for the eWeekPrintType.
    */
    virtual QPrinter::Orientation defaultOrientation();

  public:
    void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate &from, const QDate &to );

  protected:
    enum eWeekPrintType {
      Filofax=0,
      Timetable,
      SplitWeek
    } mWeekPrintType;
    QTime mStartTime, mEndTime;
    bool mSingleLineLimit;
    bool mIncludeTodos;
    bool mIncludeDescription;
    bool mExcludeTime;
};

class CalPrintMonth : public CalPrintPluginBase
{
  public:
    CalPrintMonth();
    virtual ~CalPrintMonth();
    virtual QString groupName()
    {
        return QString::fromLatin1("Print month");
    }
    virtual QString description()
    {
      return i18n( "Print mont&h" );
    }
    virtual QString info() const
    {
      return i18n( "Prints all events of one month on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Month;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual QWidget *createConfigWidget( QWidget * );
    virtual QPrinter::Orientation defaultOrientation()
    {
      return QPrinter::Landscape;
    }

  public:
    void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate &from, const QDate &to );

  protected:
    bool mWeekNumbers;
    bool mRecurDaily;
    bool mRecurWeekly;
    bool mIncludeTodos;
    bool mSingleLineLimit;
    bool mIncludeDescription;
};

class CalPrintTodos : public CalPrintPluginBase
{
  public:
    CalPrintTodos();
    virtual ~CalPrintTodos();

    virtual QString groupName()
    {
      return QString::fromLatin1( "Print to-dos" );
    }
    virtual QString description()
    {
      return i18n( "Print to-&dos" );
    }
    virtual QString info() const
    {
      return i18n( "Prints all to-dos in a (tree-like) list" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Todolist;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual QWidget *createConfigWidget( QWidget * );

  public:
    void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();

  protected:
    QString mPageTitle;

    enum eTodoPrintType {
      TodosAll = 0,
      TodosUnfinished,
      TodosDueRange
    } mTodoPrintType;

    enum eTodoSortField {
      TodoFieldSummary = 0,
      TodoFieldStartDate,
      TodoFieldDueDate,
      TodoFieldPriority,
      TodoFieldPercentComplete,
      TodoFieldUnset
    } mTodoSortField;

    enum eTodoSortDirection {
      TodoDirectionAscending = 0,
      TodoDirectionDescending,
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

class CalPrintIncidenceConfig : public QWidget, public Ui::CalPrintIncidenceConfig_Base
{
  public:
    explicit CalPrintIncidenceConfig( QWidget *parent ) : QWidget( parent )
    {
      setupUi( this );
    }
};

class CalPrintDayConfig : public QWidget, public Ui::CalPrintDayConfig_Base
{
  public:
    explicit CalPrintDayConfig( QWidget *parent ) : QWidget( parent )
    {
      setupUi( this );
    }
};

class CalPrintWeekConfig : public QWidget, public Ui::CalPrintWeekConfig_Base
{
  public:
    explicit CalPrintWeekConfig( QWidget *parent ) : QWidget( parent )
    {
      setupUi( this );
    }
};

class CalPrintMonthConfig : public QWidget, public Ui::CalPrintMonthConfig_Base
{
  public:
    explicit CalPrintMonthConfig( QWidget *parent ) : QWidget( parent )
    {
      setupUi( this );
    }
};

class CalPrintTodoConfig : public QWidget, public Ui::CalPrintTodoConfig_Base
{
  public:
    explicit CalPrintTodoConfig( QWidget *parent ) : QWidget( parent )
    {
      setupUi( this );
    }
};

#endif
