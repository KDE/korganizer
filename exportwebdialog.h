/*
  ExportWebDialog is a class that provides the dialog and functions to export a
  calendar as web page.

  $Id$  
*/

#ifndef _EXPORTWEBDIALOG_H
#define _EXPORTWEBDIALOG_H

#include <kdialogbase.h>
#include <kio/job.h>

class CalObject;
class KDateEdit;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QTextStream;
class KOEvent;
class KConfig;

class ExportWebDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ExportWebDialog(CalObject *cal, QWidget *parent=0, const char *name=0);
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

    void createHtmlEventList (QTextStream *ts);
    void createHtmlTodoList (QTextStream *ts);
    void createHtmlTodo (QTextStream *ts,KOEvent *todo);
    void createHtmlEvent (QTextStream *ts,KOEvent *event,QDate date);

    void formatHtmlCategories (QTextStream *ts,KOEvent *event);
    void formatHtmlAttendees (QTextStream *ts,KOEvent *event);

  private:
    CalObject *mCalendar;

    KConfig *mConfig;
  
    QFrame *mGeneralPage;
    QFrame *mEventPage;
    QFrame *mTodoPage;
    QFrame *mAdvancedPage;
  
    // Widgets containing export parameters
    KDateEdit *mFromDate,*mToDate;
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
