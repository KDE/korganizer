/*
  This file is part of KOrganizer.

  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koprefsdialogmain.h"
#include "koprefs.h"
#include <QButtonGroup>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include <CalendarSupport/KCalPrefs>

#include <AkonadiWidgets/ManageAccountWidget>
#include <IncidenceEditor/IncidenceEditorSettings>

#include <KLocalizedString>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>

KOPrefsDialogMain::KOPrefsDialogMain(QWidget *parent)
    : KCModule(parent)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);
    topTopLayout->addWidget(tabWidget);

    // Personal Settings
    QWidget *personalFrame = new QWidget(this);
    QVBoxLayout *personalLayout = new QVBoxLayout(personalFrame);
    tabWidget->addTab(personalFrame, QIcon::fromTheme(QStringLiteral(
                                                          "preferences-desktop-personal")),
                      i18nc("@title:tab personal settings", "Personal"));

    mEmailControlCenterCheckBox = new QCheckBox(CalendarSupport::KCalPrefs::instance()->emailControlCenterItem()->label(), this);
    connect(mEmailControlCenterCheckBox, &QAbstractButton::toggled,
            this, &KOPrefsDialogMain::toggleEmailSettings);
    connect(mEmailControlCenterCheckBox, &QCheckBox::clicked, this, &KOPrefsDialogMain::slotConfigChanged);
    personalLayout->addWidget(mEmailControlCenterCheckBox);

    mUserEmailSettings = new QGroupBox(i18nc("@title:group email settings", "Email Settings"),
                                       personalFrame);

    personalLayout->addWidget(mUserEmailSettings);
    QFormLayout *emailSettingsLayout = new QFormLayout(mUserEmailSettings);
    mUserName = new QLineEdit(this);
    emailSettingsLayout->addRow(CalendarSupport::KCalPrefs::instance()->userNameItem()->label(), mUserName);

    mUserEmail = new QLineEdit(this);
    emailSettingsLayout->addRow(CalendarSupport::KCalPrefs::instance()->userEmailItem()->label(), mUserEmail);

    //FIXME
//    KPIM::KPrefsWidRadios *defaultEmailAttachMethod
//        = addWidRadios(
//              IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethodItem(),
//              personalFrame);
//    personalLayout->addWidget(defaultEmailAttachMethod->groupBox());
    QGroupBox *defaultEmailAttachMethodGroupBox = new QGroupBox(this);
    QButtonGroup *defaultEmailAttachGroup = new QButtonGroup(this);
    QRadioButton *askRadioButton = new QRadioButton(i18n("Always ask"), this);
    defaultEmailAttachGroup->addButton(askRadioButton, IncidenceEditorNG::IncidenceEditorSettingsBase::EnumDefaultEmailAttachMethod::Ask);

    QRadioButton *linkRadioButton = new QRadioButton(i18n("Only attach link to message"), this);
    defaultEmailAttachGroup->addButton(linkRadioButton, IncidenceEditorNG::IncidenceEditorSettingsBase::EnumDefaultEmailAttachMethod::Link);

    QRadioButton *inlineFullRadioButton = new QRadioButton(i18n("Attach complete message"), this);
    defaultEmailAttachGroup->addButton(inlineFullRadioButton, IncidenceEditorNG::IncidenceEditorSettingsBase::EnumDefaultEmailAttachMethod::InlineFull);

    QRadioButton *inlineBodyRadioButton = new QRadioButton(i18n("Attach message without attachments"), this);
    defaultEmailAttachGroup->addButton(inlineBodyRadioButton, IncidenceEditorNG::IncidenceEditorSettingsBase::EnumDefaultEmailAttachMethod::InlineBody);

    QVBoxLayout *defaultEmailAttachMethodGroupBoxLayout = new QVBoxLayout(defaultEmailAttachMethodGroupBox);
    defaultEmailAttachMethodGroupBoxLayout->addWidget(askRadioButton);
    defaultEmailAttachMethodGroupBoxLayout->addWidget(linkRadioButton);
    defaultEmailAttachMethodGroupBoxLayout->addWidget(inlineFullRadioButton);
    defaultEmailAttachMethodGroupBoxLayout->addWidget(inlineBodyRadioButton);

    personalLayout->addWidget(defaultEmailAttachMethodGroupBox);
    personalLayout->addStretch(1);

    // Save Settings
    QFrame *saveFrame = new QFrame(this);
    tabWidget->addTab(saveFrame, QIcon::fromTheme(QStringLiteral("document-save")),
                      i18nc("@title:tab", "Save"));
    QVBoxLayout *saveLayout = new QVBoxLayout(saveFrame);

    mConfirmCheckBox = new QCheckBox(KOPrefs::instance()->confirmItem()->label(), saveFrame);
    connect(mConfirmCheckBox, &QCheckBox::clicked, this, &KOPrefsDialogMain::slotConfigChanged);
    saveLayout->addWidget(mConfirmCheckBox);

    mDestinationCheckBox = new QCheckBox(KOPrefs::instance()->destinationItem()->label(), saveFrame);
    connect(mDestinationCheckBox, &QCheckBox::clicked, this, &KOPrefsDialogMain::slotConfigChanged);
    saveLayout->addWidget(mDestinationCheckBox);
    saveLayout->addStretch(1);

    // System Tray Settings
    QFrame *systrayFrame = new QFrame(this);
    QVBoxLayout *systrayLayout = new QVBoxLayout(systrayFrame);
    tabWidget->addTab(systrayFrame, QIcon::fromTheme(QStringLiteral("preferences-other")),
                      i18nc("@title:tab systray settings", "System Tray"));

    QGroupBox *systrayGroupBox
        = new QGroupBox(i18nc("@title:group", "Show/Hide Options"), systrayFrame);
    systrayLayout->addWidget(systrayGroupBox);
    QVBoxLayout *systrayGroupLayout = new QVBoxLayout;
    systrayGroupBox->setLayout(systrayGroupLayout);

    mShowReminderDaemonCheckBox = new QCheckBox(KOPrefs::instance()->showReminderDaemonItem()->label(), systrayGroupBox);
    connect(mShowReminderDaemonCheckBox, &QCheckBox::clicked, this, &KOPrefsDialogMain::slotConfigChanged);
    systrayGroupLayout->addWidget(mShowReminderDaemonCheckBox);
    mShowReminderDaemonCheckBox->setToolTip(
        i18nc("@info:tooltip", "Enable this setting to show the KOrganizer "
                               "reminder daemon in your system tray (recommended)."));

    QLabel *note = new QLabel(
        xi18nc("@info",
               "<note>The daemon will continue running even if it is not shown "
               "in the system tray.</note>"));
    systrayGroupLayout->addWidget(note);

    systrayLayout->addStretch(1);

    //Calendar Account
    QFrame *calendarFrame = new QFrame(this);
    tabWidget->addTab(calendarFrame, QIcon::fromTheme(QStringLiteral("office-calendar")),
                      i18nc("@title:tab calendar account settings", "Calendars"));
    QHBoxLayout *calendarFrameLayout = new QHBoxLayout;
    calendarFrame->setLayout(calendarFrameLayout);
    Akonadi::ManageAccountWidget *manageAccountWidget = new Akonadi::ManageAccountWidget(this);
    manageAccountWidget->setDescriptionLabelText(i18nc("@title", "Calendar Accounts"));
    calendarFrameLayout->addWidget(manageAccountWidget);

    manageAccountWidget->setMimeTypeFilter(QStringList() << QStringLiteral("text/calendar"));
    // show only resources, no agents
    manageAccountWidget->setCapabilityFilter(QStringList() << QStringLiteral("Resource"));

    load();
}

void KOPrefsDialogMain::load()
{
    //TODO Load DefaultEmailAttachMethod
    mEmailControlCenterCheckBox->setChecked(CalendarSupport::KCalPrefs::instance()->emailControlCenter());
    mUserName->setText(CalendarSupport::KCalPrefs::instance()->userName());
    mUserEmail->setText(CalendarSupport::KCalPrefs::instance()->userEmail());
    mConfirmCheckBox->setChecked(KOPrefs::instance()->confirm());
    mDestinationCheckBox->setChecked(KOPrefs::instance()->destination());
    mShowReminderDaemonCheckBox->setChecked(KOPrefs::instance()->showReminderDaemon());
}

void KOPrefsDialogMain::save()
{
    //TODO Save DefaultEmailAttachMethod
    CalendarSupport::KCalPrefs::instance()->setEmailControlCenter(mEmailControlCenterCheckBox->isChecked());
    CalendarSupport::KCalPrefs::instance()->setUserName(mUserName->text());
    CalendarSupport::KCalPrefs::instance()->setUserEmail(mUserEmail->text());
    KOPrefs::instance()->setConfirm(mConfirmCheckBox->isChecked());
    KOPrefs::instance()->setDestination(mDestinationCheckBox->isChecked());
    KOPrefs::instance()->setShowReminderDaemon(mShowReminderDaemonCheckBox->isChecked());
    IncidenceEditorNG::IncidenceEditorSettings::self()->save();
}

void KOPrefsDialogMain::toggleEmailSettings(bool on)
{
    mUserEmailSettings->setEnabled(!on);
    /*  if (on) {
        KEMailSettings settings;
        mNameEdit->setText( settings.getSetting(KEMailSettings::RealName) );
        mEmailEdit->setText( settings.getSetting(KEMailSettings::EmailAddress) );
      } else {
        mNameEdit->setText( CalendarSupport::KCalPrefs::instance()->mName );
        mEmailEdit->setText( CalendarSupport::KCalPrefs::instance()->mEmail );
      }*/
}

void KOPrefsDialogMain::slotConfigChanged()
{
    Q_EMIT markAsChanged();
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigmain(QWidget *parent, const char *)
{
    return new KOPrefsDialogMain(parent);
}
}
