/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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
    : KPrefsModule(KOPrefs::instance(), parent)
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

    Korganizer::KPrefsWidBool *emailControlCenter
        = addWidBool(CalendarSupport::KCalPrefs::instance()->emailControlCenterItem(),
                     personalFrame);
    connect(emailControlCenter->checkBox(), &QAbstractButton::toggled,
            this, &KOPrefsDialogMain::toggleEmailSettings);
    personalLayout->addWidget(emailControlCenter->checkBox());

    mUserEmailSettings = new QGroupBox(i18nc("@title:group email settings", "Email Settings"),
                                       personalFrame);

    personalLayout->addWidget(mUserEmailSettings);
    QFormLayout *emailSettingsLayout = new QFormLayout(mUserEmailSettings);
    Korganizer::KPrefsWidString *s
        = addWidString(CalendarSupport::KCalPrefs::instance()->userNameItem(), mUserEmailSettings);
    emailSettingsLayout->addRow(s->label(), s->lineEdit());

    s = addWidString(CalendarSupport::KCalPrefs::instance()->userEmailItem(), mUserEmailSettings);
    emailSettingsLayout->addRow(s->label(), s->lineEdit());

    Korganizer::KPrefsWidRadios *defaultEmailAttachMethod
        = addWidRadios(
              IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethodItem(),
              personalFrame);
    personalLayout->addWidget(defaultEmailAttachMethod->groupBox());
    personalLayout->addStretch(1);

    // Save Settings
    QFrame *saveFrame = new QFrame(this);
    tabWidget->addTab(saveFrame, QIcon::fromTheme(QStringLiteral("document-save")),
                      i18nc("@title:tab", "Save"));
    QVBoxLayout *saveLayout = new QVBoxLayout(saveFrame);

    Korganizer::KPrefsWidBool *confirmItem
        = addWidBool(KOPrefs::instance()->confirmItem(), saveFrame);
    saveLayout->addWidget(confirmItem->checkBox());
    Korganizer::KPrefsWidRadios *destinationItem
        = addWidRadios(KOPrefs::instance()->destinationItem(), saveFrame);

    saveLayout->addWidget(destinationItem->groupBox());
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

    Korganizer::KPrefsWidBool *showReminderDaemonItem
        = addWidBool(KOPrefs::instance()->showReminderDaemonItem(), systrayGroupBox);
    systrayGroupLayout->addWidget(showReminderDaemonItem->checkBox());
    showReminderDaemonItem->checkBox()->setToolTip(
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

void KOPrefsDialogMain::usrWriteConfig()
{
    Korganizer::KPrefsModule::usrWriteConfig();
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

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigmain(QWidget *parent, const char *)
{
    return new KOPrefsDialogMain(parent);
}
}
