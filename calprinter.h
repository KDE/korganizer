/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _CALPRINTER_H
#define _CALPRINTER_H
/*
 * CalPrinter is a class for printing Calendars.  It can print in several
 * different formats (day, week, month).  It also provides a way for setting
 * up the printer and remembering these preferences.
 *
 * $Id$
 */

#include <unistd.h>

#include <qfile.h>
#include <qdialog.h>

#include <kprinter.h>
#include <kprocess.h>

#include <libkcal/calendar.h>

class KDateEdit;
class QButtonGroup;
class CalPrintDialog;
class KTempFile;

using namespace KCal;

class CalPrinter : public QObject
{
    Q_OBJECT
  public:
    enum PrintType { Day, Week, Month, Todolist, TimeTable };
    CalPrinter(QWidget *par, Calendar *cal);
    virtual ~CalPrinter();

    void setupPrinter();
    void preview(PrintType pt, const QDate &fd, const QDate &td);
    void print(PrintType pt, const QDate &fd, const QDate &td);

  public slots:
    void updateConfig();
    void printDay(const QDate &fd, const QDate &td);
    void printWeek(const QDate &fd, const QDate &td);
    void printMonth(const QDate &fd, const QDate &td);
    void printTodo(const QDate &fd, const QDate &td);
    void printTimeTable(const QDate &fd, const QDate &td);

  private slots:
    void doPreview(int, QDate, QDate);
    void doPrint(int, QDate, QDate);

  private:
    void drawHeader(QPainter &p, const QDate &fd, const QDate &td,
	            const QDate &cd,
	            int width, int height, PrintType pt = Month);
    void drawDayBox(QPainter &p, const QDate &qd,
                    int x, int y, int width, int height,
                    bool fullDate = FALSE);
    void drawTTDayBox(QPainter &p, const QDate &qd,
                    int x, int y, int width, int height,
                    bool fullDate = FALSE);
    void drawDay(QPainter &p, const QDate &qd, int width, int height);
    void drawWeek(QPainter &p, const QDate &qd, int width, int height);
    void drawTimeTable(QPainter &p, const QDate &qd, int width, int height);
    void drawMonth(QPainter &p, const QDate &qd, int width, int height);
    void drawSmallMonth(QPainter &p, const QDate &qd,
	                int x, int y, int width, int height);
    void drawDaysOfWeekBox(QPainter &p, const QDate &qd,
                           int x, int y, int width, int height);
    void drawDaysOfWeek(QPainter &p, const QDate &qd, int width, int height);
    void drawTodo(int count, Todo *item,QPainter &p,int level=0, QRect *r=0);

    KPrinter *mPrinter;
    Calendar *mCalendar;
    QWidget *mParent;
    int mHeaderHeight;
    int mSubHeaderHeight;
    int mStartHour;
    int mCurrentLinePos;
    CalPrintDialog *mPrintDialog;
};

class CalPrintDialog : public QDialog
{
    Q_OBJECT
  public:
    CalPrintDialog(KPrinter *p, QWidget *parent=0, const char *name=0);
    virtual ~CalPrintDialog();

    QDate fromDate() const;
    QDate toDate() const;
    CalPrinter::PrintType printType() const;

    void setRange(const QDate &from, const QDate &to);
    void setPreview(bool);

  public slots:
    void setPrintDay();
    void setPrintWeek();
    void setPrintMonth();
    void setPrintTodo();
    void setPrintTimeTable();

  private:
    KPrinter *mPrinter;

    QPushButton *mOkButton;
    QButtonGroup *mTypeGroup;
    KDateEdit *mFromDateEdit;
    KDateEdit *mToDateEdit;
    CalPrinter::PrintType mPrintType;
};

#endif // _CALPRINTER_H
