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
#ifndef CALPRINTBASE_H
#define CALPRINTBASE_H
// #define KORG_NOPRINTER

#ifndef KORG_NOPRINTER

#include <qwidget.h>
#include <qdatetime.h>
#include <kprinter.h>
#include <libkcal/event.h>

class PrintCellItem;

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
      Orientation of printout. Default is Portrait. If your plugin wants
      to use some other orientation as default (e.g. depending on some
      config settings), implement this function in your subclass and
      return the desired orientation.
    */
    virtual KPrinter::Orientation orientation() { return KPrinter::Portrait; }

    /**
      Load print format configuration from config file.
    */
    virtual void loadConfig() = 0;
    /**
      Write print format configuration to config file.
    */
    virtual void saveConfig() = 0;

    /**
      Load complete config. This also calls loadConfig() of the derived class.
    */
    void doLoadConfig();
    /**
      Save complete config. This also calls saveConfig() of the derived class.
    */
    void doSaveConfig();
          

  public slots:
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
    int weekdayColumn( int weekday );

    QDate mFromDate;
    QDate mToDate;
    bool mUseColors;

  public:
    /**
      Internal class representing the start of a todo.
    */
    class TodoParentStart;

  protected:
    /** 
      Draw the gray header bar of the printout to the QPainter. 
      It prints the given text and optionally one or two small
      month views, as specified by the two QDate. The printed 
      text can also contain a line feed. 
      If month2 is invalid, only the month that contains month1 
      is printed.
      E.g. the filofax week view draws just the current month, 
      while the month view draws the previous and the next month.
      \param p QPainter of the printout
      \param title The string printed as the title of the page 
                   (e.g. the date, date range or todo list title)
      \param month1 Date specifying the month for the left one of 
                    the small month views in the title bar. If left 
                    empty, only month2 will be printed (or none, 
                    it that is invalid as well).
      \param month2 Date specifying the month for the right one of 
                    the small month views in the title bar. If left 
                    empty, only month1 will be printed (or none, 
                    it that is invalid as well).
      \param x x-coordinate of the upper left coordinate of the title bar
      \param y y-coordinate of the upper left coordinate of the title bar
      \param width width of the title bar
      \param height height of the title bar
    */
    void drawHeader( QPainter &p, QString title,
                     const QDate &month1, const QDate &month2,
                     int x, int y, int width, int height );
    /** 
      Draw a small calendar with the days of a month into the given area.
      Used for example in the title bar of the sheet.
      \param p QPainter of the printout
      \param qd Arbitrary Date within the month to be printed.
      \param x x-coordinate of the upper left coordinate of the small calendar
      \param y y-coordinate of the upper left coordinate of the small calendar
      \param width width of the small calendar
      \param height height of the small calendar
    */
    void drawSmallMonth( QPainter &p, const QDate &qd,
                         int x, int y, int width, int height );

    /** 
      Draw a horizontal bar with the weekday names of the given date range 
      in the given area of the painter. 
      This is used for the weekday-bar on top of the timetable view and the month view.
      \param p QPainter of the printout
      \param fromDate First date of the printed dates
      \param toDate Last date of the printed dates
    */
    void drawDaysOfWeek( QPainter &p,
                         const QDate &fromDate, const QDate &toDate,
                         int x, int y, int width, int height );
    /** 
      Draw a single weekday name in a box inside the given area of the painter. 
      This is called in a loop by drawDaysOfWeek.
      \param p QPainter of the printout
      \param qd Date of the printed day
    */
    void drawDaysOfWeekBox( QPainter &p, const QDate &qd,
                            int x, int y, int width, int height );
    /**
      Draw a (vertical) time scale from time fromTime to toTime inside the given area of the painter.
      Every hour will have a one-pixel line over the whole width, every 
      half-hour the line will only span the left half of the width.
      This is used in the day and timetable print styles
      \param p QPainter of the printout
      \param fromTime Start time of the time range to display
      \param toTime End time of the time range to display
    */
    void drawTimeLine( QPainter &p,
                       const QTime &fromTime, const QTime &toTime,
                       int x, int y, int width, int height );
    /**
      Draw the all-day box for the agenda print view (the box on top which 
      doesn't have a time on the time scale associated). If expandable is set,
      height is the cell height of a single cell, and the returned height will
      be the total height used for the all-day events. If !expandable, only one
      cell will be used, and multiple events are concatenated using ", ".
      \param p QPainter of the printout
      \param eventList The list of all-day events that are supposed to be printed
             inside this box
      \param qd The date of the currently printed day
      \param expandable If true, height is the height of one single cell, the printout
             will use as many cells as events in the list and return the total height
             needed for all of them. If false, height specifies the total height 
             allowed for all events, and the events are displayed in one cell, 
             with their summaries concatenated by ", ".
    */
    void drawAllDayBox( QPainter &p, Event::List &eventList,
                        const QDate &qd, bool expandable,
                        int x, int y, int width, int &height );
    /**
      Draw the agenda box for the day print style (the box showing all events of that day).
      Also draws a grid with half-hour spacing of the grid lines.
      \param p QPainter of the printout
      \param eventList The list of the events that are supposed to be printed
             inside this box
      \param qd The date of the currently printed day
      \param expandable If true, the start and end times are adjusted to include 
             the whole range of all events of that day, not just of the given time range.
             The height of the box will not be affected by this (but the height 
             of one hour will be scaled down so that the whole range fits into 
             the box. fromTime and toTime receive the actual time range printed 
             by this function).
      \param fromTime Start of the time range to be printed. Might be adjusted 
                      to include all events if expandable==true
      \param toTime End of the time range to be printed. Might be adjusted 
                   to include all events if expandable==true          
    */
    void drawAgendaDayBox( QPainter &p, Event::List &eventList,
                           const QDate &qd, bool expandable,
                           QTime &fromTime, QTime &toTime,
                           int x, int y, int width, int height);

    void drawAgendaItem( PrintCellItem *item, QPainter &p, const QDate &,
                         const QDateTime &startPrintDate,
                         const QDateTime &endPrintDate,
                         float minlen, int x, int y, int width );
    /**
      Draw the box containing a list of all events of the given day (with their times, 
      of course). Used in the Filofax and the month print style.
      \param p QPainter of the printout
      \param qd The date of the currently printed day. All events of the calendar 
                that appear on that day will be printed.
      \param fullDate Whether the title bar of the box should contain the full 
                      date string or just a short.
    */
    void drawDayBox( QPainter &p, const QDate &qd,
                     int x, int y, int width, int height,
                     bool fullDate = false );
    /**
      Draw the week (filofax) table of the week containing the date qd. The first 
      three days of the week will be shown in the first column (using drawDayBox), 
      the remaining four in the second column, where the last two days of the week 
      (typically Saturday and Sunday) only get half the height of the other day boxes.
      \param p QPainter of the printout
      \param qd Arbitrary date within the week to be printed.
    */
    void drawWeek( QPainter &p, const QDate &qd,
                   int x, int y, int width, int height );
    /**
      Draw the timetable view of the given time range from fromDate to toDate.
      On the left side the time scale is printed (using drawTimeLine), then each 
      day gets one column (printed using drawAgendaDayBox), 
      and the events are displayed as boxes (like in korganizer's day/week view).
      The first cell of each column contains the all-day events (using 
      drawAllDayBox with expandable=false).
      The given time range cannot be expanded to include all events. 
      \param p QPainter of the printout
      \param fromDate First day to be included in the page
      \param toDate Last day to be included in the page
      \param fromTime Start time of the displayed time range
      \param toTime End time of the displayed time range
    */
    void drawTimeTable( QPainter &p, const QDate &fromDate, const QDate &toDate,
                        QTime &fromTime, QTime &toTime,
                        int x, int y, int width, int height );

    /**
      Draw the month table of the month containing the date qd. Each day gets one 
      box (using drawDayBox) that contains a list of all events on that day. They are arranged 
      in a matrix, with the first column being the first day of the 
      week (so it might display some days of the previous and the next month).
      Above the matrix there is a bar showing the weekdays (drawn using drawDaysOfWeek).
      \param p QPainter of the printout
      \param qd Arbitrary date within the month to be printed.
      \param weeknumbers Whether the week numbers are printed left of each row of the matrix
    */
    void drawMonth( QPainter &p, const QDate &qd, bool weeknumbers,
                    int x, int y, int width, int height );

    /**
      Draws single todo item and its (intented) subitems, optionally connects them by a tree-like line,
      and optionally shows due date, summary, description and priority.
      \param count The number of the currently printed todo (count will be incremented for each drawn item)
      \param item The item to be printed. It's subitems are recursively 
                  drawn, so drawTodo should only be called on the 
                  items of the highest level.
      \param p QPainter of the printout
      \param connectSubTodos Whether subtodos shall be connected with their parent by a line (tree-like).
      \param desc Whether to print the whole description of the item (the summary is always printed).
      \param pospriority x-coordinate where the priority is supposed to be printed. If <0, no priority will be printed.
      \param possummary x-coordinate where the summary of the item is supposed to be printed.
      \param posDueDt x-coordinate where the due date is supposed to the be printed. If <0, no due date will be printed.
      \param level Level of the current item in the todo hierarchy (0 means highest 
                   level of printed items, 1 are their subtodos, etc.)
      \param x x-coordinate of the upper left coordinate of the first item
      \param y y-coordinate of the upper left coordinate of the first item
      \param width width of the whole todo list
      \param pageHeight Total height allowed for the todo list on a page. If an item would be below that 
                   line, a new page is started.
      \param r Internal (used when printing sub items to give information about its parent)
    */
    void drawTodo( int &count, Todo * item, QPainter &p, bool connectSubTodos,
                   bool desc, int pospriority, int possummary, int posDueDt,
                   int level, int x, int &y, int width, int pageHeight, 
                   const Todo::List &todoList, TodoParentStart *r = 0 );

    void drawSplitWeek( QPainter &p, const QDate &fd, const QDate &td );
    void drawSplitHeaderRight( QPainter &p, const QDate &fd, const QDate &td,
                               const QDate &cd, int width, int height );
    void drawSplitDay( QPainter &p, const QDate &qd, int width, int height,
                       int offsetLeft );
    void drawSplitTimes( QPainter &p, int width, int timeWidth, int height );

    /** 
      Determines the column of the given weekday ( 1=Monday, 7=Sunday ), taking the 
      start of the week setting into account as given in kcontrol.
      \param weekday Index of the weekday 
    */
    int weekDayColumn( int weekday );

    KPrinter *mPrinter;
    Calendar *mCalendar;
    KConfig *mConfig;
    QWidget *mConfigWidget;

  protected:
    // TODO_RK: move these to the appropriate subclasses or set them globally.
    static int mSubHeaderHeight;
    static int mHeaderHeight;
    static int mMargin;
};

#endif

#endif
