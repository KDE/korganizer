/*
 * CalPrinter is a class for printing CalObjects.  It can print in several
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

#include "calobject.h"

class KDateEdit;
class QButtonGroup;
class CalPrintDialog;

class CalPrinter : public QObject
{
  Q_OBJECT

public:
  enum PrintType { Day, Week, Month, Todolist };
  CalPrinter(QWidget *par, CalObject *cal);
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

protected slots:
  void doPreview(int, QDate, QDate);
  void doPrint(int, QDate, QDate);
  void previewCleanup() { unlink(QFile::encodeName(previewFileName)); };
 
protected:
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

  KProcess    *previewProc;
  KPrinter    *printer;
  QString      previewProg, previewFileName;
  CalObject   *calendar;
  QWidget     *parent;
  int margin, pageWidth, pageHeight, headerHeight, subHeaderHeight;
  bool oldOutputToFile;
  int startHour;
  QString oldFileName;
  CalPrintDialog *cpd;
};

class CalPrintDialog : public QDialog
{
  Q_OBJECT

public:
  CalPrintDialog(KPrinter *p, bool preview, const QDate &fd, 
		 const QDate &td, QWidget *parent=0, const char *name=0);
  virtual ~CalPrintDialog();

  QDate getFrom() const;
  QDate getTo() const;
  const CalPrinter::PrintType getPrintType() const;

public slots:
  void setPrintDay();
  void setPrintWeek();
  void setPrintMonth();
  void setPrintTodo();

protected slots:
  void accept();
  void reject();
 
signals:
  void doneSignal(int, QDate, QDate);

protected:
  QButtonGroup *typeGroup;
  KPrinter *printer;
  KDateEdit *fromDated, *toDated;
  CalPrinter::PrintType pt;
};

#endif // _CALPRINTER_H
