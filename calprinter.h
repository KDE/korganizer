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

#include <korganizer/baseview.h>

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
class CalPrinter : public QObject, public KOrg::CalPrinterBase
{
    Q_OBJECT

  public:
    enum ePrintOrientation {
      eOrientPlugin=0,
      eOrientPrinter,
      eOrientPortrait,
      eOrientLandscape
    };
  public:
    /**
      \param par parent widget for dialogs
      \param cal calendar to be printed
    */
    CalPrinter( QWidget *par, Calendar *cal );
    virtual ~CalPrinter();

    void init( KPrinter *printer, Calendar *calendar );

    void setupPrinter();

    /**
      Set date range to be printed.
      
      \param start Start date
      \param end   End date
    */
    void setDateRange( const QDate &start, const QDate &end );

  public slots:
    void updateConfig();

  private slots:
    void doPrint( CalPrintBase *selectedStyle, bool preview );

  public:
    void preview( PrintType type, const QDate &fd, const QDate &td );
    void print( PrintType type, const QDate &fd, const QDate &td );

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
    CalPrintDialog( QPtrList<CalPrintBase> plugins, KPrinter *p,
                    QWidget *parent = 0, const char *name = 0 );
    virtual ~CalPrintDialog();
    CalPrintBase *selectedPlugin();
    CalPrinter::ePrintOrientation orientation() { return mOrientation; }

  public slots:
    void setPrintType( int );
    void setPreview( bool );

  protected slots:
    void slotOk();
    void setupPrinter();
    void setPrinterLabel();

  private:
    KPrinter *mPrinter;
    QVButtonGroup *mTypeGroup;
    QWidgetStack *mConfigArea;
    QPtrList<CalPrintBase> mPrintPlugins;
    QLabel *mPrinterLabel;
    QString mPreviewText;
    QComboBox *mOrientationSelection;

    CalPrinter::ePrintOrientation mOrientation;
};

#endif

#endif
