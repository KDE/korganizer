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
#include <QHBoxLayout>
#include <KMessageBox>
#include <KUrlRequester>
#include <QVBoxLayout>
#include <KLocalizedString>
#include "korganizer_debug.h"

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
ExportWebDialog::ExportWebDialog(KOrg::HTMLExportSettings *settings, QWidget *parent)
    : KPageDialog(parent), KPIM::KPrefsWidManager(settings), mSettings(settings)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFaceType(Tabbed);
    setWindowTitle(i18n("Export Calendar as Web Page"));
    setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    setModal(false);
    mExportButton = new QPushButton(i18n("Export"));
    mExportButton->setDefault(true);
    addActionButton(mExportButton);

    setupGeneralPage();
    setupEventPage();
    setupTodoPage();
// Disabled bacause the functionality is not yet implemented.
//  setupJournalPage();
//  setupFreeBusyPage();
//  setupAdvancedPage();

    connect(mExportButton, &QPushButton::clicked, this, &ExportWebDialog::slotOk);
    connect(button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ExportWebDialog::reject);
    connect(button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &ExportWebDialog::slotDefault);
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
    emit exportHTML(mSettings);
    accept();
}

void ExportWebDialog::slotDefault()
{
    qCDebug(KORGANIZER_LOG);

    if (KMessageBox::warningContinueCancel(
                this,
                i18n("You are about to set all preferences to default values. "
                     "All custom modifications will be lost."),
                i18n("Setting Default Preferences"),
                KGuiItem(i18n("Reset to Defaults"))) == KMessageBox::Continue) {
        setDefaults();
    }
}

void ExportWebDialog::setupGeneralPage()
{
    mGeneralPage = new QFrame(this);
    addPage(mGeneralPage, i18nc("general settings for html export", "General"));
    QVBoxLayout *topLayout = new QVBoxLayout(mGeneralPage);
    topLayout->setSpacing(10);

    mDateRangeGroup = new QGroupBox(i18n("Date Range"), mGeneralPage);
    topLayout->addWidget(mDateRangeGroup);

    QHBoxLayout *rangeLayout = new QHBoxLayout(mDateRangeGroup);

    KPIM::KPrefsWidDate *dateStart = addWidDate(mSettings->dateStartItem());
    rangeLayout->addWidget(dateStart->label());
    rangeLayout->addWidget(dateStart->dateEdit());
    KPIM::KPrefsWidDate *dateEnd = addWidDate(mSettings->dateEndItem());
    rangeLayout->addWidget(dateEnd->label());
    rangeLayout->addWidget(dateEnd->dateEdit());

    QGroupBox *typeGroup = new QGroupBox(i18n("View Type"), mGeneralPage);
    topLayout->addWidget(typeGroup);

    QBoxLayout *typeLayout = new QVBoxLayout(typeGroup);

    mMonthViewCheckBox = addWidBool(mSettings->monthViewItem())->checkBox();
    connect(mMonthViewCheckBox, &QCheckBox::stateChanged, this, &ExportWebDialog::updateState);
    typeLayout->addWidget(mMonthViewCheckBox);

    mEventListCheckBox = addWidBool(mSettings->eventViewItem())->checkBox();
    connect(mEventListCheckBox, &QCheckBox::stateChanged, this, &ExportWebDialog::updateState);
    typeLayout->addWidget(mEventListCheckBox);

    typeLayout->addWidget(addWidBool(mSettings->todoViewItem())->checkBox());
    typeLayout->addWidget(
        addWidBool(mSettings->excludePrivateItem())->checkBox());
    typeLayout->addWidget(
        addWidBool(mSettings->excludeConfidentialItem())->checkBox());

    QGroupBox *destGroup = new QGroupBox(i18n("Destination"), mGeneralPage);
    topLayout->addWidget(destGroup);

    QBoxLayout *destLayout = new QHBoxLayout(destGroup);

    KPIM::KPrefsWidPath *pathWid = addWidPath(mSettings->outputFileItem(),
                                   destGroup, QLatin1String("text/html"), KFile::File);
    connect(pathWid->urlRequester(), SIGNAL(textChanged(QString)),
            SLOT(slotTextChanged(QString)));
    destLayout->addWidget(pathWid->label());
    destLayout->addWidget(pathWid->urlRequester());

    topLayout->addStretch(1);
}

void ExportWebDialog::slotTextChanged(const QString &_text)
{
    mExportButton->setEnabled(!_text.trimmed().isEmpty());
}

void ExportWebDialog::setupTodoPage()
{
    mTodoPage = new QFrame(this);
    addPage(mTodoPage, i18n("To-dos"));
    QVBoxLayout *topLayout = new QVBoxLayout(mTodoPage);
    topLayout->setSpacing(10);

    QWidget *hbox = new QWidget(mTodoPage);
    QHBoxLayout *hboxHBoxLayout = new QHBoxLayout(hbox);
    hboxHBoxLayout->setMargin(0);
    topLayout->addWidget(hbox);
    KPIM::KPrefsWidString *s = addWidString(mSettings->todoListTitleItem(), hbox);
    hboxHBoxLayout->addWidget(s->label());
    hboxHBoxLayout->addWidget(s->lineEdit());

    QWidget *vbox = new QWidget(mTodoPage);
    QVBoxLayout *vboxVBoxLayout = new QVBoxLayout(vbox);
    vboxVBoxLayout->setMargin(0);
    topLayout->addWidget(vbox);
    KPIM::KPrefsWidBool *boolWid = addWidBool(mSettings->taskDueDateItem(), vbox);
    vboxVBoxLayout->addWidget(boolWid->checkBox());
    boolWid = addWidBool(mSettings->taskLocationItem(), vbox);
    vboxVBoxLayout->addWidget(boolWid->checkBox());
    boolWid = addWidBool(mSettings->taskCategoriesItem(), vbox);
    vboxVBoxLayout->addWidget(boolWid->checkBox());
    boolWid = addWidBool(mSettings->taskAttendeesItem(), vbox);
    vboxVBoxLayout->addWidget(boolWid->checkBox());
//  addWidBool( mSettings->taskExcludePrivateItem(), vbox );
//  addWidBool( mSettings->taskExcludeConfidentialItem(), vbox );

    topLayout->addStretch(1);
}

void ExportWebDialog::setupEventPage()
{
    mEventPage = new QFrame(this);
    addPage(mEventPage, i18n("Events"));
    QVBoxLayout *topLayout = new QVBoxLayout(mEventPage);
    topLayout->setSpacing(10);

    QWidget *hbox = new QWidget(mEventPage);
    QHBoxLayout *hboxHBoxLayout = new QHBoxLayout(hbox);
    hboxHBoxLayout->setMargin(0);
    topLayout->addWidget(hbox);
    KPIM::KPrefsWidString *s = addWidString(mSettings->eventTitleItem(), hbox);
    hboxHBoxLayout->addWidget(s->label());
    hboxHBoxLayout->addWidget(s->lineEdit());

    QWidget *vbox = new QWidget(mEventPage);
    QVBoxLayout *vboxVBoxLayout = new QVBoxLayout(vbox);
    vboxVBoxLayout->setMargin(0);
    topLayout->addWidget(vbox);
    KPIM::KPrefsWidBool *boolWid = addWidBool(mSettings->eventLocationItem(), vbox);
    vboxVBoxLayout->addWidget(boolWid->checkBox());
    boolWid = addWidBool(mSettings->eventCategoriesItem(), vbox);
    vboxVBoxLayout->addWidget(boolWid->checkBox());
    boolWid = addWidBool(mSettings->eventAttendeesItem(), vbox);
    vboxVBoxLayout->addWidget(boolWid->checkBox());
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
    mDateRangeGroup->setEnabled(exportEvents);
    mEventPage->setEnabled(exportEvents);
}

