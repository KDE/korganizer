// $Id$
//
// ExportWebDialog.cpp - Implementation of web page export dialog.
//

#include "exportwebdialog.h"
#include "exportwebdialog.moc"

#include <qlayout.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kio/job.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kglobal.h>

#include "calendar.h"
#include "kdateedit.h"
#include "koprefs.h"
#include "htmlexport.h"

ExportWebDialog::ExportWebDialog (Calendar *cal, QWidget *parent,
                                  const char *name) :
  KDialogBase(Tabbed,i18n("Export calendar as web page"),
              Help|Default|User1|Cancel,User1,parent,name,false,false,
              i18n("Export")),
  mCalendar(cal)
{
  mExport = new HtmlExport(cal);

  setupGeneralPage();
  setupEventPage();
  setupTodoPage();
// Disabled bacause the functionality is not yet implemented.
//  setupAdvancedPage();
  
  QObject::connect(this,SIGNAL(user1Clicked()),this,SLOT(exportWebPage()));
}

ExportWebDialog::~ExportWebDialog()
{
  delete mConfig;
}

void ExportWebDialog::setupGeneralPage()
{
  mGeneralPage = addPage(i18n("General"));

  QVBoxLayout *topLayout = new QVBoxLayout(mGeneralPage, 10);
  
  QGroupBox *rangeGroup = new QHGroupBox(i18n("Date Range"),mGeneralPage);
  topLayout->addWidget(rangeGroup);
  
  mFromDate = new KDateEdit(rangeGroup);
  mFromDate->setDate(QDate::currentDate());
  
  mToDate = new KDateEdit(rangeGroup);
  mToDate->setDate(QDate::currentDate());

  QButtonGroup *typeGroup = new QVButtonGroup(i18n("View Type"),mGeneralPage);
  topLayout->addWidget(typeGroup);
  
  
  // For now we just support the todo view. Other view types will follow
  // shortly.
//  new QRadioButton(i18n("Day"), typeGroup);
//  new QRadioButton(i18n("Week"), typeGroup);
  mCbMonth = new QCheckBox(i18n("Month"), typeGroup);
  mCbEvent = new QCheckBox(i18n("Event List"), typeGroup);
  mCbTodo = new QCheckBox(i18n("To-Do List"), typeGroup);
  mCbTodo->setChecked(true);

  QGroupBox *destGroup = new QVGroupBox(i18n("Destination"),mGeneralPage);
  topLayout->addWidget(destGroup);

  new QLabel(i18n("Output File:"),destGroup);

  QHBox *outputFileLayout = new QHBox(destGroup);
  mOutputFileEdit = new QLineEdit(KOPrefs::instance()->mHtmlExportFile,
                                  outputFileLayout);
  QPushButton *browseButton = new QPushButton(i18n("Browse"),outputFileLayout);
  QObject::connect(browseButton, SIGNAL(clicked()),
                   this, SLOT(browseOutputFile()));
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupTodoPage()
{
  mTodoPage = addPage(i18n("To-Do"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mTodoPage, 10);
  
  mCbDueDates = new QCheckBox (i18n("Due Dates"),mTodoPage);
  topLayout->addWidget(mCbDueDates);
  
  mCbCategoriesTodo = new QCheckBox (i18n("Categories"),mTodoPage);
  topLayout->addWidget(mCbCategoriesTodo);
  
  mCbAttendeesTodo = new QCheckBox (i18n("Attendees"),mTodoPage);
  topLayout->addWidget(mCbAttendeesTodo);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupEventPage()
{
  mEventPage = addPage(i18n("Event"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mEventPage, 10);
  
  mCbCategoriesEvent = new QCheckBox (i18n("Categories"),mEventPage);
  topLayout->addWidget(mCbCategoriesEvent);
  
  mCbAttendeesEvent = new QCheckBox (i18n("Attendees"),mEventPage);
  topLayout->addWidget(mCbAttendeesEvent);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupAdvancedPage()
{
  mAdvancedPage = addPage(i18n("Advanced"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mAdvancedPage, 10);
  
  mCbHtmlFragment = new QCheckBox (i18n("Only generate HTML fragment"),
                                   mAdvancedPage);
  topLayout->addWidget(mCbHtmlFragment);  
  
  QPushButton *colorsButton = new QPushButton(i18n("Colors"),mAdvancedPage);
  topLayout->addWidget(colorsButton);
  
  // Implement the functionality to enable this buttons.
  mCbHtmlFragment->setEnabled(false);
  colorsButton->setEnabled(false);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::browseOutputFile()
{
//  kdDebug() << "ExportWebDialog::browseOutputFile()" << endl;

  KURL u = KFileDialog::getSaveURL();
  if(!u.isEmpty()) mOutputFileEdit->setText(u.prettyURL());
}

void ExportWebDialog::exportWebPage()
{
  mExport->setMonthViewEnabled(mCbMonth->isChecked());
  mExport->setEventsEnabled(mCbEvent->isChecked());
  mExport->setTodosEnabled(mCbTodo->isChecked());
  mExport->setCategoriesEventEnabled(mCbCategoriesEvent->isChecked());
  mExport->setAttendeesEventEnabled(mCbAttendeesEvent->isChecked());
  mExport->setCategoriesTodoEnabled(mCbCategoriesTodo->isChecked());
  mExport->setAttendeesTodoEnabled(mCbAttendeesTodo->isChecked());
  mExport->setDueDateEnabled(mCbDueDates->isChecked());
  mExport->setDateRange(mFromDate->getDate(),mToDate->getDate());

  // Create temporary file
  KTempFile tmpFile;

  // Save to temporary file
  mExport->save(tmpFile.file());

  // Copy temporary file to final destination
  KURL src;
  src.setPath(tmpFile.name());

  KURL dest(mOutputFileEdit->text());
  // Remember destination.
  KOPrefs::instance()->mHtmlExportFile = mOutputFileEdit->text();

  KIO::Job *job = KIO::move(src,dest);
  connect(job,SIGNAL(result(KIO::Job *)),SLOT(slotResult(KIO::Job *)));
}

void ExportWebDialog::slotResult(KIO::Job *job)
{
//  kdDebug() << "slotResult" << endl;
  int err = job->error();
  if (err)
  {
//    kdDebug() << "Error " << err << ": " << job->errorString() << endl;
    job->showErrorDialog();
  } else {
//    kdDebug() << "No Error" << endl;
    accept();
  }
//  kdDebug() << "slotResult done" << endl;
}
