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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <kurl.h>
#include <kio/job.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include "koglobals.h"
#include <kurlrequester.h>
#include <kio/netaccess.h>
#include <knotifyclient.h>
#include <ktempfile.h>

#include <libkcal/calendar.h>

#include <libkdepim/kdateedit.h>
#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "kocore.h"

#include "exportwebdialog.h"
#include "exportwebdialog.moc"

ExportWebDialog::ExportWebDialog (Calendar *cal, QWidget *parent,
                                  const char *name) :
  KDialogBase(Tabbed,i18n("Export Calendar as Web Page"),
              Help|Default|User1|Cancel,User1,parent,name,false,false,
              i18n("Export")),
  mCalendar(cal),
  mDataAvailable(false)
{
  mExport = new HtmlExport(cal);

  mConfig = KOGlobals::self()->config();

  setupGeneralPage();
  setupEventPage();
  setupTodoPage();
// Disabled bacause the functionality is not yet implemented.
//  setupAdvancedPage();

  loadSettings();

  QObject::connect( this, SIGNAL( user1Clicked() ), SLOT( exportWebPage() ) );
}

ExportWebDialog::~ExportWebDialog()
{
  delete(mExport);
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
  mToDate->setDate(QDate::currentDate().addMonths(1));

  QButtonGroup *typeGroup = new QVButtonGroup(i18n("View Type"),mGeneralPage);
  topLayout->addWidget(typeGroup);


  // For now we just support the todo view. Other view types will follow
  // shortly.
//  new QRadioButton(i18n("Day"), typeGroup);
//  new QRadioButton(i18n("Week"), typeGroup);
  mCbMonth = new QCheckBox(i18n("Month"), typeGroup);
  mCbEvent = new QCheckBox(i18n("Event list"), typeGroup);
  mCbTodo = new QCheckBox(i18n("To-do list"), typeGroup);

  QGroupBox *destGroup = new QVGroupBox(i18n("Destination"),mGeneralPage);
  topLayout->addWidget(destGroup);

  new QLabel(i18n("Output file:"),destGroup);

  QHBox *outputFileLayout = new QHBox(destGroup);
  mOutputFileEdit = new KURLRequester(KOPrefs::instance()->mHtmlExportFile,
                                  outputFileLayout);
  mOutputFileEdit->setMode( KFile::File );
  mOutputFileEdit->setFilter( "text/html" );
  connect( mOutputFileEdit->lineEdit(), SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotTextChanged( const QString & ) ) );
  slotTextChanged( mOutputFileEdit->lineEdit()->text());
  topLayout->addStretch(1);
}

void ExportWebDialog::slotTextChanged( const QString & _text)
{
    enableButton( User1, !_text.isEmpty() );
}

void ExportWebDialog::setupTodoPage()
{
  mTodoPage = addPage(i18n("To-do"));

  QVBoxLayout *topLayout = new QVBoxLayout(mTodoPage, 10);

  mCbDueDates = new QCheckBox (i18n("Due dates"),mTodoPage);
  topLayout->addWidget(mCbDueDates);

  mCbCategoriesTodo = new QCheckBox (i18n("Categories"),mTodoPage);
  topLayout->addWidget(mCbCategoriesTodo);

  mCbAttendeesTodo = new QCheckBox (i18n("Attendees"),mTodoPage);
  topLayout->addWidget(mCbAttendeesTodo);

  mCbExcludePrivateTodo = new QCheckBox (i18n("Exclude private"),mTodoPage);
  topLayout->addWidget(mCbExcludePrivateTodo);

  mCbExcludeConfidentialTodo = new QCheckBox (i18n("Exclude confidential"),mTodoPage);
  topLayout->addWidget(mCbExcludeConfidentialTodo);

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

  mCbExcludePrivateEvent = new QCheckBox (i18n("Exclude private"),mEventPage);
  topLayout->addWidget(mCbExcludePrivateEvent);

  mCbExcludeConfidentialEvent = new QCheckBox (i18n("Exclude confidential"),mEventPage);
  topLayout->addWidget(mCbExcludeConfidentialEvent);

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

void ExportWebDialog::loadSettings()
{
  KConfig *cfg = KOGlobals::self()->config();
  cfg->setGroup( "HtmlExport" );

  mCbMonth->setChecked( cfg->readBoolEntry( "Month", false ) );
  mCbEvent->setChecked( cfg->readBoolEntry( "Event", true ) );
  mCbTodo->setChecked( cfg->readBoolEntry( "Todo", true ) );
  mCbCategoriesEvent->setChecked( cfg->readBoolEntry( "CategoriesEvent", false ) );
  mCbAttendeesEvent->setChecked( cfg->readBoolEntry( "AttendeesEvent", false ) );
  mCbExcludePrivateEvent->setChecked( cfg->readBoolEntry( "ExcludePrivateEvent", true ) );
  mCbExcludeConfidentialEvent->setChecked( cfg->readBoolEntry( "ExcludeConfidentialEvent", true ) );
  mCbCategoriesTodo->setChecked( cfg->readBoolEntry( "CategoriesTodo", false ) );
  mCbAttendeesTodo->setChecked( cfg->readBoolEntry( "AttendeesTodo", false ) );
  mCbExcludePrivateTodo->setChecked( cfg->readBoolEntry( "ExcludePrivateTodo", true ) );
  mCbExcludeConfidentialTodo->setChecked( cfg->readBoolEntry( "ExcludeConfidentialTodo", true ) );
  mCbDueDates->setChecked( cfg->readBoolEntry( "DueDates", true ) );
}

void ExportWebDialog::saveSettings()
{
  KConfig *cfg = KOGlobals::self()->config();
  cfg->setGroup( "HtmlExport" );

  cfg->writeEntry( "Month", mCbMonth->isChecked() );
  cfg->writeEntry( "Event", mCbEvent->isChecked() );
  cfg->writeEntry( "Todo", mCbTodo->isChecked() );
  cfg->writeEntry( "CategoriesEvent", mCbCategoriesEvent->isChecked() );
  cfg->writeEntry( "AttendeesEvent", mCbAttendeesEvent->isChecked());
  cfg->writeEntry( "ExcludePrivateEvent", mCbExcludePrivateEvent->isChecked());
  cfg->writeEntry( "ExcludeConfidentialEvent", mCbExcludeConfidentialEvent->isChecked());
  cfg->writeEntry( "CategoriesTodo", mCbCategoriesTodo->isChecked());
  cfg->writeEntry( "AttendeesTodo", mCbAttendeesTodo->isChecked());
  cfg->writeEntry( "ExcludePrivateTodo", mCbExcludePrivateTodo->isChecked());
  cfg->writeEntry( "ExcludeConfidentialTodo", mCbExcludeConfidentialTodo->isChecked());
  cfg->writeEntry( "DueDates", mCbDueDates->isChecked());

  cfg->sync();
}

void ExportWebDialog::exportWebPage(bool synchronous)
{
  saveSettings();

  mExport->setTitle( "KOrganizer Calendar" );
  mExport->setTitleTodo( "KOrganizer To-Do List" );
  mExport->setCredit( "KOrganizer", "http://korganizer.kde.org" );
  mExport->setEmail( KOPrefs::instance()->email() );
  mExport->setFullName( KOPrefs::instance()->fullName() );
  mExport->setMonthViewEnabled(mCbMonth->isChecked());
  mExport->setEventsEnabled(mCbEvent->isChecked());
  mExport->setTodosEnabled(mCbTodo->isChecked());
  mExport->setCategoriesEventEnabled(mCbCategoriesEvent->isChecked());
  mExport->setAttendeesEventEnabled(mCbAttendeesEvent->isChecked());
  mExport->setExcludePrivateEventEnabled(mCbExcludePrivateEvent->isChecked());
  mExport->setExcludeConfidentialEventEnabled(mCbExcludeConfidentialEvent->isChecked());
  mExport->setCategoriesTodoEnabled(mCbCategoriesTodo->isChecked());
  mExport->setAttendeesTodoEnabled(mCbAttendeesTodo->isChecked());
  mExport->setExcludePrivateTodoEnabled(mCbExcludePrivateTodo->isChecked());
  mExport->setExcludeConfidentialTodoEnabled(mCbExcludeConfidentialTodo->isChecked());
  mExport->setDueDateEnabled(mCbDueDates->isChecked());
  mExport->setDateRange(mFromDate->date(),mToDate->date());

  QDate cdate=mFromDate->date();
  while (cdate<=mToDate->date())
  {
    if ( !KOCore::self()->holiday(cdate).isEmpty() )
      mExport->addHoliday( cdate, KOCore::self()->holiday(cdate) );
    cdate = cdate.addDays(1);
  }

  KURL dest(mOutputFileEdit->lineEdit()->text());
  // Remember destination.
  KOPrefs::instance()->mHtmlExportFile = mOutputFileEdit->lineEdit()->text();

  if (synchronous) {
    if (!dest.isLocalFile()) {
      KTempFile tf;
      QString tfile = tf.name();
      tf.close();
      mExport->save(tfile);
      if (!KIO::NetAccess::upload (tfile, dest, this)) {
        KNotifyClient::event (winId(),"Could not upload file.");
      }
      tf.unlink();
    } else {
      mExport->save(dest.path());
    }
  } else {
    mDataAvailable = true;
    KIO::TransferJob *job = KIO::put(dest,-1,true,false);
    connect(job,SIGNAL(dataReq(KIO::Job *,QByteArray &)),
            SLOT(slotDataReq(KIO::Job *,QByteArray &)));
    connect(job,SIGNAL(result(KIO::Job *)),SLOT(slotResult(KIO::Job *)));
  }
}

void ExportWebDialog::slotResult(KIO::Job *job)
{
  kdDebug(5850) << "slotResult" << endl;
  int err = job->error();
  if (err)
  {
    kdDebug(5850) << "  Error " << err << ": " << job->errorString() << endl;
    job->showErrorDialog();
  } else {
    kdDebug(5850) << "  No Error" << endl;
    accept();
  }
  kdDebug(5850) << "slotResult done" << endl;
}

void ExportWebDialog::slotDataReq(KIO::Job *,QByteArray &data)
{
  kdDebug(5850) << "ExportWebDialog::slotDataReq()" << endl;

  if (mDataAvailable) {
    kdDebug(5850) << "  Data availavble" << endl;
    QTextStream ts(data,IO_WriteOnly);
    ts.setEncoding( QTextStream::Latin1 );

    mExport->save(&ts);
    mDataAvailable = false;
  } else
    kdDebug(5850) << "  No Data" << endl;
}
