/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdialogmain.h"

#include "koprefs.h"
#include <KPluginFactory>
#include <QFormLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include <CalendarSupport/KCalPrefs>

#include <Akonadi/ManageAccountWidget>
#include <IncidenceEditor/IncidenceEditorSettings>
#include <akonadi/calendarsettings.h>

#include <KLocalizedString>
#include <QCheckBox>
#include <QLabel>

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogMain, "korganizer_configmain.json")

KOPrefsDialogMain::KOPrefsDialogMain(QObject *parent, const KPluginMetaData &data)
    : Korganizer::KPrefsModule(KOPrefs::instance(), parent, data)
{
    auto topTopLayout = new QVBoxLayout(widget());
    topTopLayout->setContentsMargins({});
    auto tabWidget = new QTabWidget(widget());
    tabWidget->setDocumentMode(true);
    topTopLayout->addWidget(tabWidget);

    // Personal Settings
    auto personalFrame = new QWidget(widget());
    auto personalLayout = new QVBoxLayout(personalFrame);
    tabWidget->addTab(personalFrame, QIcon::fromTheme(QStringLiteral("preferences-desktop-personal")), i18nc("@title:tab personal settings", "Personal"));

    Korganizer::KPrefsWidBool *emailControlCenter = addWidBool(CalendarSupport::KCalPrefs::instance()->emailControlCenterItem(), personalFrame);
    connect(emailControlCenter->checkBox(), &QAbstractButton::toggled, this, &KOPrefsDialogMain::toggleEmailSettings);
    personalLayout->addWidget(emailControlCenter->checkBox());

    mUserEmailSettings = new QGroupBox(i18nc("@title:group email settings", "Email Settings"), personalFrame);

    personalLayout->addWidget(mUserEmailSettings);
    auto emailSettingsLayout = new QFormLayout(mUserEmailSettings);
    Korganizer::KPrefsWidString *s = addWidString(CalendarSupport::KCalPrefs::instance()->userNameItem(), mUserEmailSettings);
    emailSettingsLayout->addRow(s->label(), s->lineEdit());

    s = addWidString(CalendarSupport::KCalPrefs::instance()->userEmailItem(), mUserEmailSettings);
    emailSettingsLayout->addRow(s->label(), s->lineEdit());

    Korganizer::KPrefsWidRadios *defaultEmailAttachMethod =
        addWidRadios(IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethodItem(), personalFrame);
    personalLayout->addWidget(defaultEmailAttachMethod->groupBox());

    auto remindersGroup = new QGroupBox(i18nc("@title:group", "Reminders"), personalFrame);
    auto remindersLayout = new QFormLayout(remindersGroup);
    Korganizer::KPrefsWidBool *onlyMyReminders = addWidBool(Akonadi::CalendarSettings::self()->onlyShowRemindersForMyEventsItem(), remindersGroup);
    remindersLayout->addWidget(onlyMyReminders->checkBox());
    personalLayout->addWidget(remindersGroup);
    personalLayout->addStretch(1);

    // Save Settings
    auto saveFrame = new QFrame(widget());
    tabWidget->addTab(saveFrame, QIcon::fromTheme(QStringLiteral("document-save")), i18nc("@title:tab", "Save"));
    auto saveLayout = new QVBoxLayout(saveFrame);

    Korganizer::KPrefsWidBool *confirmItem = addWidBool(KOPrefs::instance()->confirmItem(), saveFrame);
    saveLayout->addWidget(confirmItem->checkBox());
    Korganizer::KPrefsWidRadios *destinationItem = addWidRadios(KOPrefs::instance()->destinationItem(), saveFrame);

    saveLayout->addWidget(destinationItem->groupBox());
    saveLayout->addStretch(1);

    // Calendar Account
    auto calendarFrame = new QFrame(widget());
    tabWidget->addTab(calendarFrame, QIcon::fromTheme(QStringLiteral("office-calendar")), i18nc("@title:tab calendar account settings", "Calendars"));
    auto calendarFrameLayout = new QHBoxLayout;
    calendarFrame->setLayout(calendarFrameLayout);
    auto manageAccountWidget = new Akonadi::ManageAccountWidget(widget());
    manageAccountWidget->setDescriptionLabelText(i18nc("@title", "Calendar Accounts"));
    calendarFrameLayout->addWidget(manageAccountWidget);

    manageAccountWidget->setMimeTypeFilter(QStringList() << QStringLiteral("text/calendar"));
    // show only resources, no agents
    manageAccountWidget->setCapabilityFilter(QStringList() << QStringLiteral("Resource"));

#if HAVE_ACTIVITY_SUPPORT
    auto activitiesFrame = new QFrame(widget());
    tabWidget->addTab(activitiesFrame, QIcon::fromTheme(QStringLiteral("activities")), i18nc("@title:tab activities settings", "Activities"));

    auto activitiesFrameLayout = new QVBoxLayout;
    activitiesFrame->setLayout(activitiesFrameLayout);

    mActivities = new QCheckBox(i18n("Enabled"), widget());
    activitiesFrameLayout->addWidget(mActivities);
    activitiesFrameLayout->addStretch(1);
    mActivities->setChecked(KOPrefs::instance()->enabledActivities());
    connect(mActivities, &QCheckBox::clicked, this, &KOPrefsDialogMain::slotWidChanged);
#endif
    load();
}

void KOPrefsDialogMain::usrWriteConfig()
{
    Korganizer::KPrefsModule::usrWriteConfig();
    IncidenceEditorNG::IncidenceEditorSettings::self()->save();
#if HAVE_ACTIVITY_SUPPORT
    KOPrefs::instance()->setEnabledActivities(mActivities->isChecked());
#endif
    setNeedsSave(false);
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

#include "koprefsdialogmain.moc"

#include "moc_koprefsdialogmain.cpp"
