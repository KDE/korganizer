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
#ifndef _CALPRINTBASE_H
#define _CALPRINTBASE_H
// #define KORG_NOPRINTER

#ifndef KORG_NOPRINTER

#include <qwidget.h>
#include <qdatetime.h>
#include <kprinter.h>
#include <libkcal/event.h>

namespace KCal {
class Calendar;
class Todo;
}

using namespace KCal;

/**
  Base class for KOrganizer printing classes. Each sub class represents one
  calendar print format.
*/
class CalPrintBase : public QObject
{
    Q_OBJECT
  public:
    /**
      Constructor
      
      \param pr KPrinter object used to print.
      \param cal Calendar to be printed.
      \param cfg KConfig object for reading/writing printing configuration
    */
    CalPrintBase( KPrinter *pr, Calendar *cal, KConfig *cfg );
    virtual ~CalPrintBase();

    /**
      Returns short description of print format.
    */
    virtual QString description() = 0;
    /**
      Returns long description of print format.
    */
    virtual QString longDescription() = 0;

    /**
      Returns widget for configuring the print format.
    */
    virtual QWidget *configWidget( QWidget * );

    /**
      Actually do the printing.
      
      \param p QPainter the print result is painted to
      \param width Width of printable area
      \param height Height of printable area
    */
    virtual void print( QPainter &p, int width, int height ) = 0;
    /**
      Start printing.
    */
    virtual void doPrint();

    /**
      Orientation of printout.
    */
    virtual KPrinter::Orientation orientation() { return mOrientation; }

  public slots:
    /**
      Load print format configuration from config file.
    */
    virtual void loadConfig();
    /**
      Write print format configuration to config file.
    */
    virtual void saveConfig();
    /**
      Read settings from configuration widget and apply them to current object.
    */
    virtual void readSettingsWidget() {}
    /**
      Set configuration widget to reflect settings of current object.
    */
    virtual void setSettingsWidget() {}

    /**
      Set date range which should be printed.
    */
    virtual void setDateRange( const QDate &from, const QDate &to )
    {
      mFromDate = from;
      mToDate = to;
    }

  protected:
    QDate mFromDate, mToDate;
    bool mUseColors;

  public:
    /**
      Internal class representing the start of a todo.
    */
    class TodoParentStart;

  protected:
    // Printing help functions
    void drawHeader( QPainter &p, QString title,
                     const QDate &month1, const QDate &month2,
                     int x, int y, int width, int height );
    void drawSmallMonth( QPainter &p, const QDate &qd,
                         int x, int y, int width, int height );

    void drawDaysOfWeek( QPainter &p,
                         const QDate &fromDate, const QDate &toDate,
                         int x, int y, int width, int height );
    void drawDaysOfWeekBox( QPainter &p, const QDate &qd,
                            int x, int y, int width, int height );
    void drawTimeLine( QPainter &p,
                       const QTime &fromTime, const QTime &toTime,
                       int x, int y, int width, int height );
    void drawAllDayBox( QPainter &p, Event::List &eventList,
                        const QDate &qd, bool expandable,
                        int x, int y, int width, int &height );
    void drawAgendaDayBox( QPainter &p, Event::List &eventList,
                           const QDate &qd, bool expandable,
                           QTime &fromTime, QTime &toTime,
                           int x, int y, int width, int height);
    void drawDayBox( QPainter &p, const QDate &qd,
                     int x, int y, int width, int height,
                     bool fullDate = false );

    void drawWeek( QPainter &p, const QDate &qd,
                   int x, int y, int width, int height );
    void drawTimeTable( QPainter &p, const QDate &fromDate, const QDate &toDate,
                        QTime &fromTime, QTime &toTime,
                        int x, int y, int width, int height );

    void drawMonth( QPainter &p, const QDate &qd, bool weeknumbers,
                    int x, int y, int width, int height );

    void drawTodo( int &count, Todo * item, QPainter &p, bool connectSubTodos,
                   bool desc, int pospriority, int possummary, int posDueDt,
                   int level, int x, int &y, int width, int &height,
                   int pageHeight, TodoParentStart *r = 0 );

    KPrinter *mPrinter;
    Calendar *mCalendar;
    KConfig *mConfig;
    QWidget *mConfigWidget;

    KPrinter::Orientation mOrientation;

  protected:
    // TODO_RK: move these to the appropriate subclasses or set them globally.
    static int mSubHeaderHeight;
    static int mHeaderHeight;
    static int mMargin;
};

#endif

#endif
