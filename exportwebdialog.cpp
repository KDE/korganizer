/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include <kmessagebox.h>

#include <libkcal/calendar.h>
#include <libkcal/htmlexportsettings.h>

#include <libkdepim/kdateedit.h>
#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "kocore.h"

#include "exportwebdialog.h"
#include "exportwebdialog.moc"


// FIXME: The basic structure of this dialog has been copied from KPrefsDialog, 
//        because we want custom buttons, a Tabbed dialog, and a different 
//        headline... Maybe we should try to achieve the same without code 
//        duplication.
ExportWebDialog::ExportWebDialog( HTMLExportSettings *settings, QWidget *parent,
                                  const char *name)
  : KDialogBase( Tabbed,i18n("Export Calendar as Web Page"),Help|Default|User1|Cancel, User1, parent, name, false, false, i18n("Export") ),
    KPrefsWidManager( settings ), mSettings( settings )
{
  setupGeneralPage();
  setupEventPage();
  setupTodoPage();
// Disabled bacause the functionality is not yet implemented.
//  setupJournalPage();
//  setupFreeBusyPage();
//  setupAdvancedPage();

  connect( this, SIGNAL( user1Clicked() ), SLOT( slotOk() ) );
  connect( this, SIGNAL( cancelClicked() ), SLOT( reject() ) );
  
  readConfig();
}

ExportWebDialog::~ExportWebDialog()
{
}

void ExportWebDialog::setDefaults()
{
  setWidDefaults();
}

void ExportWebDialog::readConfig()
{
  readWidConfig();
  usrReadConfig();
}

void ExportWebDialog::writeConfig()
{
  writeWidConfig();
  usrWriteConfig();
  readConfig();
}

void ExportWebDialog::slotApply()
{
kdDebug()<<"ExportWebDialog::slotApply"<<endl;
  writeConfig();
  emit configChanged();
}

void ExportWebDialog::slotOk()
{
kdDebug()<<"ExportWebDialog::slotOk"<<endl;
  slotApply();
  emit exportHTML( mSettings );
  accept();
}

void ExportWebDialog::slotDefault()
{
  kdDebug() << "KPrefsDialog::slotDefault()" << endl;

  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      i18n("Reset to Defaults"))
    == KMessageBox::Continue) setDefaults();
}


void ExportWebDialog::setupGeneralPage()
{
  mGeneralPage = addPage( i18n("General") );
  QVBoxLayout *topLayout = new QVBoxLayout(mGeneralPage, 10);

  QGroupBox *rangeGroup = new QHGroupBox( i18n("Date Range"), mGeneralPage );
  topLayout->addWidget( rangeGroup );
  addWidDate( mSettings->dateStartItem(), rangeGroup );
  addWidDate( mSettings->dateEndItem(), rangeGroup );

  QButtonGroup *typeGroup = new QVButtonGroup( i18n("View Type"), mGeneralPage );
  topLayout->addWidget( typeGroup );
//  addWidBool( mSettings->weekViewItem(), typeGroup );
  addWidBool( mSettings->monthViewItem(), typeGroup );
  addWidBool( mSettings->eventViewItem(), typeGroup );
  addWidBool( mSettings->todoViewItem(), typeGroup );
//  addWidBool( mSettings->journalViewItem(), typeGroup );
//  addWidBool( mSettings->freeBusyViewItem(), typeGroup );
  addWidBool( mSettings->excludePrivateItem(), typeGroup );
  addWidBool( mSettings->excludeConfidentialItem(), typeGroup );

  QGroupBox *destGroup = new QVGroupBox(i18n("Destination"), mGeneralPage );
  topLayout->addWidget(destGroup);
  KPrefsWidPath *pathWid = addWidPath( mSettings->outputFileItem(), 
                                       destGroup, "text/html", KFile::File );
  connect( pathWid->urlRequester(), SIGNAL( textChanged( const QString & ) ), 
           SLOT( slotTextChanged( const QString & ) ) );

  topLayout->addStretch( 1 );
}

void ExportWebDialog::slotTextChanged( const QString & _text)
{
    enableButton( User1, !_text.isEmpty() );
}

void ExportWebDialog::setupTodoPage()
{
  mTodoPage = addPage(i18n("To-dos"));
  QVBoxLayout *topLayout = new QVBoxLayout( mTodoPage, 10 );
  
  QHBox *hbox = new QHBox( mTodoPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->todoListTitleItem(), hbox );

  QVBox *vbox = new QVBox( mTodoPage );
  topLayout->addWidget( vbox );
  addWidBool( mSettings->taskDueDateItem(), vbox );
  addWidBool( mSettings->taskCategoriesItem(), vbox );
  addWidBool( mSettings->taskAttendeesItem(), vbox );
//  addWidBool( mSettings->taskExcludePrivateItem(), vbox );
//  addWidBool( mSettings->taskExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}

void ExportWebDialog::setupEventPage()
{
  mEventPage = addPage(i18n("Events"));
  QVBoxLayout *topLayout = new QVBoxLayout( mEventPage, 10 );

  QHBox *hbox = new QHBox( mEventPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->eventTitleItem(), hbox );

  QVBox *vbox = new QVBox( mEventPage );
  topLayout->addWidget( vbox );
  addWidBool( mSettings->eventCategoriesItem(), vbox );
  addWidBool( mSettings->eventAttendeesItem(), vbox );
//  addWidBool( mSettings->eventExcludePrivateItem(), vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}
/*
void ExportWebDialog::setupJournalPage()
{
  mJournalPage = addPage(i18n("Journals"));
  QVBoxLayout *topLayout = new QVBoxLayout( mJournalPage, 10 );

  QHBox *hbox = new QHBox( mJournalPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->journalTitleItem(), hbox );

  QVBox *vbox = new QVBox( mJournalPage );
  topLayout->addWidget( vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}

void ExportWebDialog::setupFreeBusyPage()
{
  mFreeBusyPage = addPage(i18n("Free/Busy"));
  QVBoxLayout *topLayout = new QVBoxLayout( mFreeBusyPage, 10 );

  QHBox *hbox = new QHBox( mFreeBusyPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->journalTitleItem(), hbox );

  QVBox *vbox = new QVBox( mFreeBusyPage );
  topLayout->addWidget( vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}

void ExportWebDialog::setupAdvancedPage()
{
  mAdvancedPage = addPage(i18n("Advanced"));
  QVBoxLayout *topLayout = new QVBoxLayout( mAdvancedPage, 10 );

  QVBox *vbox = new QVBox( mAdvancedPage );
  topLayout->addWidget( vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}
*/
