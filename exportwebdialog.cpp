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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>


#include <QPushButton>
#include <q3filedialog.h>
#include <QTextStream>
#include <QLabel>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGroupBox>

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
#include <ktempfile.h>
#include <kmessagebox.h>

#include <kcal/calendar.h>
#include <kcal/htmlexportsettings.h>

#include <libkdepim/kdateedit.h>
#include <libkdepim/kdateedit.h>
#include <kvbox.h>

#include "koprefs.h"
#include "kocore.h"

#include "exportwebdialog.h"
#include "exportwebdialog.moc"


// FIXME: The basic structure of this dialog has been copied from KPrefsDialog,
//        because we want custom buttons, a Tabbed dialog, and a different
//        headline... Maybe we should try to achieve the same without code
//        duplication.
ExportWebDialog::ExportWebDialog( HTMLExportSettings *settings, QWidget *parent)
// TODO_QT4: Use constructor without *name=0 param
  : KPageDialog( parent ),
    KPrefsWidManager( settings ), mSettings( settings )
{
  setFaceType( Tabbed );
  setCaption( i18n("Export Calendar as Web Page") );
  setButtons( Help|Default|User1|Cancel );
  setDefaultButton( User1 );
  setModal( false );
  showButtonSeparator( false );
  setButtonText( User1, i18n("Export") );

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
  writeConfig();
  emit configChanged();
}

void ExportWebDialog::slotOk()
{
  slotApply();
  emit exportHTML( mSettings );
  accept();
}

void ExportWebDialog::slotDefault()
{
  kDebug(5850) << "KPrefsDialog::slotDefault()" << endl;

  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      KGuiItem(i18n("Reset to Defaults")))
    == KMessageBox::Continue) setDefaults();
}


void ExportWebDialog::setupGeneralPage()
{
  mGeneralPage = new QFrame( this );
  addPage( mGeneralPage, i18n("General") );
  QVBoxLayout *topLayout = new QVBoxLayout(mGeneralPage);
  topLayout->setSpacing(10);

  QGroupBox *rangeGroup = new QGroupBox( i18n("Date Range"), mGeneralPage );
  topLayout->addWidget( rangeGroup );
  addWidDate( mSettings->dateStartItem(), rangeGroup );
  addWidDate( mSettings->dateEndItem(), rangeGroup );

  QGroupBox *typeGroup = new QGroupBox( i18n("View Type"), mGeneralPage );
  topLayout->addWidget( typeGroup );
//  addWidBool( mSettings->weekViewItem(), typeGroup );
  addWidBool( mSettings->monthViewItem(), typeGroup );
  addWidBool( mSettings->eventViewItem(), typeGroup );
  addWidBool( mSettings->todoViewItem(), typeGroup );
//  addWidBool( mSettings->journalViewItem(), typeGroup );
//  addWidBool( mSettings->freeBusyViewItem(), typeGroup );
  addWidBool( mSettings->excludePrivateItem(), typeGroup );
  addWidBool( mSettings->excludeConfidentialItem(), typeGroup );

  QGroupBox *destGroup = new QGroupBox(i18n("Destination"), mGeneralPage );
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
  mTodoPage = new QFrame( this );
  addPage( mTodoPage, i18n("To-dos") );
  QVBoxLayout *topLayout = new QVBoxLayout( mTodoPage );
  topLayout->setSpacing( 10 );

  KHBox *hbox = new KHBox( mTodoPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->todoListTitleItem(), hbox );

  KVBox *vbox = new KVBox( mTodoPage );
  topLayout->addWidget( vbox );
  addWidBool( mSettings->taskDueDateItem(), vbox );
  addWidBool( mSettings->taskLocationItem(), vbox );
  addWidBool( mSettings->taskCategoriesItem(), vbox );
  addWidBool( mSettings->taskAttendeesItem(), vbox );
//  addWidBool( mSettings->taskExcludePrivateItem(), vbox );
//  addWidBool( mSettings->taskExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}

void ExportWebDialog::setupEventPage()
{
  mEventPage = new QFrame( this );
  addPage( mEventPage, i18n("Events") );
  QVBoxLayout *topLayout = new QVBoxLayout( mEventPage );
  topLayout->setSpacing( 10 );

  KHBox *hbox = new KHBox( mEventPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->eventTitleItem(), hbox );

  KVBox *vbox = new KVBox( mEventPage );
  topLayout->addWidget( vbox );
  addWidBool( mSettings->eventLocationItem(), vbox );
  addWidBool( mSettings->eventCategoriesItem(), vbox );
  addWidBool( mSettings->eventAttendeesItem(), vbox );
//  addWidBool( mSettings->eventExcludePrivateItem(), vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}
/*
void ExportWebDialog::setupJournalPage()
{
  mJournalPage = addPage(i18n("Journal"));
  QVBoxLayout *topLayout = new QVBoxLayout( mJournalPage );
  topLayout->setSpacing( 10 );

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
  QVBoxLayout *topLayout = new QVBoxLayout( mFreeBusyPage );
  topLayout->setSpacing( 10 );

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
  QVBoxLayout *topLayout = new QVBoxLayout( mAdvancedPage );
  topLayout->setSpacing( 10 );

  QVBox *vbox = new QVBox( mAdvancedPage );
  topLayout->addWidget( vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}
*/
