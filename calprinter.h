/*
 * CalPrinter is a class for printing Calendars.  It can print in several
 * different formats (day, week, month).  It also provides a way for setting
 * up the printer and remembering these preferences.
 *
 * $Id$
 */

#ifndef _CALPRINTER_H
#define _CALPRINTER_H

#include <unistd.h>

#include <kprinter.h>
#include <qfile.h>
#include <qdialog.h>

#include <kprocess.h>

#include "calendar.h"

class KDateEdit;
class QButtonGroup;
class CalPrintDialog;
class KTempFile;

using namespace KCal;

class CalPrinter : public QObject
{
    Q_OBJECT
  public:
    enum PrintType { Day, Week, Month, Todolist };
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

  private slots:
    void doPreview(int, QDate, QDate);
    void doPrint(int, QDate, QDate);
    void previewCleanup(KProcess *);
 
  private:
    void drawHeader(QPainter &p, const QDate &fd, const QDate &td,
	            const QDate &cd,
	            int width, int height, PrintType pt = Month);
    void drawDayBox(QPainter &p, const QDate &qd,
                    int x, int y, int width, int height, 
                    bool fullDate = FALSE);

    void drawDay(QPainter &p, const QDate &qd, int width, int height);
    void drawWeek(QPainter &p, const QDate &qd, int width, int height);
    void drawMonth(QPainter &p, const QDate &qd, int width, int height);
    void drawSmallMonth(QPainter &p, const QDate &qd, 
	                int x, int y, int width, int height);
    void drawDaysOfWeekBox(QPainter &p, const QDate &qd,
                           int x, int y, int width, int height);
    void drawDaysOfWeek(QPainter &p, const QDate &qd, int width, int height);
    void drawTodo(int count, Todo *item,QPainter &p,int level=0, QRect *r=0);

    KPrinter *mPrinter;
    KTempFile *mPreviewFile;
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

  private:
    KPrinter *mPrinter;

    QPushButton *mOkButton;
    QButtonGroup *mTypeGroup;
    KDateEdit *mFromDateEdit;
    KDateEdit *mToDateEdit;
    CalPrinter::PrintType mPrintType;
};

#endif // _CALPRINTER_H
