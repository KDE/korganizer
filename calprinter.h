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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _CALPRINTER_H
#define _CALPRINTER_H

// #define KORG_NOPRINTER

#ifndef KORG_NOPRINTER

#include <qptrlist.h>

#include <kdialogbase.h>

#include "calprintbase.h"

using namespace KCal;

class QVButtonGroup;
class QWidgetStack;
class KPrinter;
class CalPrintDialog;
class KConfig;
class QComboBox;
class QLabel;


/**
  CalPrinter is a class for printing Calendars.  It can print in several
  different formats (day, week, month).  It also provides a way for setting
  up the printer and remembering these preferences.
*/
class CalPrinter : public QObject
{
    Q_OBJECT
  public:
    CalPrinter(QWidget *par, Calendar *cal);
    virtual ~CalPrinter();

    enum { Day=0, Week, Month, Todolist};
    virtual void init(KPrinter *printer, Calendar *calendar);

    void setupPrinter();

  public slots:
    void updateConfig();
    void setDateRange(const QDate&, const QDate&);

  private slots:
    void doPreview(CalPrintBase*selectedStyle);
    void doPrint(CalPrintBase*selectedStyle);
  signals:
    void setDateRangeSignal(const QDate&, const QDate&);
    void updateConfigSignal();
    void writeConfigSignal();
  public:
    void preview( int type, const QDate &fd, const QDate &td);
    void print( int type, const QDate &fd, const QDate &td);
    void forcePrint( int type, const QDate &fd, const QDate &td, bool preview );

  protected:
    QPtrList<CalPrintBase> mPrintPlugins;

  private:
    KPrinter *mPrinter;
    Calendar *mCalendar;
    QWidget *mParent;
    KConfig *mConfig;

    CalPrintDialog *mPrintDialog;
};

class CalPrintDialog : public KDialogBase
{
    Q_OBJECT
  public:
    CalPrintDialog(QPtrList<CalPrintBase> plugins, KPrinter *p,
        QWidget *parent=0, const char *name=0);
    virtual ~CalPrintDialog();
    CalPrintBase* selectedPlugin();

  public slots:
    void setPrintType(int);
    void setPreview(bool);
  protected slots:
    void slotOk();
    void setupPrinter();
    void setPrinterLabel();

  signals:
    /* sent to make the plugins apply the settings from the config widget */
    void applySettings();
    /* sent to make the plugins applyt the correct settings to the config widget */
    void doSettings();

  private:
    KPrinter *mPrinter;
    QVButtonGroup *mTypeGroup;
    QWidgetStack *mConfigArea;
    QPtrList<CalPrintBase> mPrintPlugins;
    QLabel*mPrinterLabel;
    QString mPreviewText;
    QComboBox*mOrientationSelection;
};
#endif
#endif // _CALPRINTER_H
