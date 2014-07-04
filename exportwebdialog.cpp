/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "exportwebdialog.h"
#include "htmlexportsettings.h"

#include <KDateComboBox>
#include <KFileDialog>
#include <KHBox>
#include <KMessageBox>
#include <KUrlRequester>
#include <KVBox>
#include <KLocalizedString>
#include <QDebug>

#include <QBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

// FIXME: The basic structure of this dialog has been copied from KPrefsDialog,
//        because we want custom buttons, a Tabbed dialog, and a different
//        headline... Maybe we should try to achieve the same without code
//        duplication.
ExportWebDialog::ExportWebDialog( KOrg::HTMLExportSettings *settings, QWidget *parent )
  : KPageDialog( parent ), KPIM::KPrefsWidManager( settings ), mSettings( settings )
{
  setAttribute(Qt::WA_DeleteOnClose);
  setFaceType( Tabbed );
  setWindowTitle( i18n( "Export Calendar as Web Page" ) );
  setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults);
  setModal( false );
  mExportButton = new QPushButton(i18n( "Export" ));
  mExportButton->setDefault(true);
  addActionButton(mExportButton);


  setupGeneralPage();
  setupEventPage();
  setupTodoPage();
// Disabled bacause the functionality is not yet implemented.
//  setupJournalPage();
//  setupFreeBusyPage();
//  setupAdvancedPage();

  connect( mExportButton, SIGNAL(clicked()), SLOT(slotOk()) );
  connect( button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(reject()) );
  connect( button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), this, SLOT(slotDefault()) );
  readConfig();
  updateState();
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
  qDebug();

  if ( KMessageBox::warningContinueCancel(
         this,
         i18n( "You are about to set all preferences to default values. "
               "All custom modifications will be lost." ),
         i18n( "Setting Default Preferences" ),
      KGuiItem( i18n( "Reset to Defaults" ) ) ) == KMessageBox::Continue ) {
    setDefaults();
  }
}

void ExportWebDialog::setupGeneralPage()
{
  mGeneralPage = new QFrame( this );
  addPage( mGeneralPage, i18nc( "general settings for html export", "General" ) );
  QVBoxLayout *topLayout = new QVBoxLayout( mGeneralPage );
  topLayout->setSpacing(10);

  mDateRangeGroup = new QGroupBox( i18n( "Date Range" ), mGeneralPage );
  topLayout->addWidget( mDateRangeGroup );

  QHBoxLayout *rangeLayout = new QHBoxLayout( mDateRangeGroup );

  KPIM::KPrefsWidDate *dateStart = addWidDate( mSettings->dateStartItem() );
  rangeLayout->addWidget( dateStart->label() );
  rangeLayout->addWidget( dateStart->dateEdit() );
  KPIM::KPrefsWidDate *dateEnd = addWidDate( mSettings->dateEndItem() );
  rangeLayout->addWidget( dateEnd->label() );
  rangeLayout->addWidget( dateEnd->dateEdit() );

  QGroupBox *typeGroup = new QGroupBox( i18n( "View Type" ), mGeneralPage );
  topLayout->addWidget( typeGroup );

  QBoxLayout *typeLayout = new QVBoxLayout( typeGroup );

  mMonthViewCheckBox = addWidBool( mSettings->monthViewItem() )->checkBox();
  connect( mMonthViewCheckBox, SIGNAL(stateChanged(int)), SLOT(updateState()) );
  typeLayout->addWidget( mMonthViewCheckBox );

  mEventListCheckBox = addWidBool( mSettings->eventViewItem() )->checkBox();
  connect( mEventListCheckBox, SIGNAL(stateChanged(int)), SLOT(updateState()) );
  typeLayout->addWidget( mEventListCheckBox );

  typeLayout->addWidget( addWidBool( mSettings->todoViewItem() )->checkBox() );
  typeLayout->addWidget(
    addWidBool( mSettings->excludePrivateItem() )->checkBox() );
  typeLayout->addWidget(
    addWidBool( mSettings->excludeConfidentialItem() )->checkBox() );

  QGroupBox *destGroup = new QGroupBox( i18n( "Destination" ), mGeneralPage );
  topLayout->addWidget( destGroup );

  QBoxLayout *destLayout = new QHBoxLayout( destGroup );

  KPIM::KPrefsWidPath *pathWid = addWidPath( mSettings->outputFileItem(),
                                             destGroup, QLatin1String("text/html"), KFile::File );
  //QT5 pathWid->urlRequester()->fileDialog()->setOperationMode( KFileDialog::Saving );
  connect( pathWid->urlRequester(), SIGNAL(textChanged(QString)),
           SLOT(slotTextChanged(QString)) );
  destLayout->addWidget( pathWid->label() );
  destLayout->addWidget( pathWid->urlRequester() );

  topLayout->addStretch( 1 );
}

void ExportWebDialog::slotTextChanged( const QString &_text )
{
    mExportButton->setEnabled( !_text.isEmpty() );
}

void ExportWebDialog::setupTodoPage()
{
  mTodoPage = new QFrame( this );
  addPage( mTodoPage, i18n( "To-dos" ) );
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
  addPage( mEventPage, i18n( "Events" ) );
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

void ExportWebDialog::updateState()
{
  const bool exportEvents = mMonthViewCheckBox->isChecked() ||
                            mEventListCheckBox->isChecked();
  mDateRangeGroup->setEnabled( exportEvents );
  mEventPage->setEnabled( exportEvents );
}

