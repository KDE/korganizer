/*
  ExportWebDialog is a class that provides the dialog and functions to export a
  calendar as web page.

  $Id$  
*/

#ifndef _EXPORTWEBDIALOG_H
#define _EXPORTWEBDIALOG_H

#include <kdialogbase.h>
#include <kio/job.h>

#include <calendar.h>

using namespace KCal;

class KDateEdit;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QTextStream;
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
};

#endif // _EXPORTWEBDIALOG_H
