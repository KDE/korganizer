/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
*/
#ifndef _EXPORTWEBDIALOG_H
#define _EXPORTWEBDIALOG_H
/*
  ExportWebDialog is a class that provides the dialog and functions to export a
  calendar as web page.

  $Id$  
*/

#include <kdialogbase.h>
#include <kio/job.h>

#include <libkcal/calendar.h>

using namespace KCal;

class KDateEdit;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QTextStream;
class QFrame;
class KConfig;
class HtmlExport;

class ExportWebDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ExportWebDialog(Calendar *cal, QWidget *parent=0, const char *name=0);
    virtual ~ExportWebDialog();

  public slots:
    void exportWebPage();

    void browseOutputFile();

    void slotResult(KIO::Job *);
    void slotDataReq(KIO::Job *,QByteArray &data);

  protected slots:
 
  signals:

  protected:
    void setupGeneralPage();
    void setupEventPage();
    void setupTodoPage();
    void setupAdvancedPage();

  private:
    Calendar *mCalendar;

    HtmlExport *mExport;

    KConfig *mConfig;
  
    QFrame *mGeneralPage;
    QFrame *mEventPage;
    QFrame *mTodoPage;
    QFrame *mAdvancedPage;
  
    // Widgets containing export parameters
    KDateEdit *mFromDate,*mToDate;
    QCheckBox *mCbMonth;
    QCheckBox *mCbEvent;
    QCheckBox *mCbTodo;
    QCheckBox *mCbDueDates;
    QCheckBox *mCbCategoriesTodo;
    QCheckBox *mCbCategoriesEvent;
    QCheckBox *mCbAttendeesTodo;
    QCheckBox *mCbAttendeesEvent;
    QCheckBox *mCbHtmlFragment;
    QLineEdit *mOutputFileEdit;

    bool mDataAvailable;
};

#endif // _EXPORTWEBDIALOG_H
