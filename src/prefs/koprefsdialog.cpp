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

#include "koprefsdialog.h"
#include "widgets/kitemiconcheckcombo.h"
#include "kocore.h"
#include "koglobals.h"
#include "koprefs.h"
#include "ui_kogroupwareprefspage.h"
#include <KTimeComboBox>

#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/CategoryConfig>

#include <IncidenceEditor/IncidenceEditorSettings>

#include <AkonadiCore/AgentFilterProxyModel>
#include <AkonadiCore/AgentInstanceCreateJob>
#include <AkonadiCore/AgentManager>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiWidgets/AgentTypeDialog>
#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiWidgets/ManageAccountWidget>
#include <AkonadiWidgets/TagSelectionComboBox>
#include <akonadi/calendar/calendarsettings.h>  //krazy:exclude=camelcase this is a generated file

#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>

#include <KHolidays/HolidayRegion>

#include <MailTransport/TransportManagementWidget>

#include <KColorButton>
#include <KComboBox>
#include <QHBoxLayout>
#include <QSpinBox>
#include <KMessageBox>
#include <KService>

#include <QTabWidget>
#include <KUrlRequester>
#include "korganizer_debug.h"
#include <QIcon>
#include <QPushButton>

#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QStandardPaths>
#include <QLocale>
#include <QTimeEdit>
#include <KAboutData>
#include <QButtonGroup>

#ifdef WITH_KUSERFEEDBACK
#include <KUserFeedback/FeedbackConfigWidget>
#include "userfeedback/userfeedbackmanager.h"
#endif

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
    personalLayout->addWidget(mEmailControlCenterCheckBox);

    mUserEmailSettings = new QGroupBox(i18nc("@title:group email settings", "Email Settings"),
                                       personalFrame);

    personalLayout->addWidget(mUserEmailSettings);
    QFormLayout *emailSettingsLayout = new QFormLayout(mUserEmailSettings);
    mUserName = new QLineEdit(this);
    emailSettingsLayout->addRow(CalendarSupport::KCalPrefs::instance()->userNameItem()->label(), mUserName);

    mUserEmail = new QLineEdit(this);
    emailSettingsLayout->addRow(CalendarSupport::KCalPrefs::instance()->userEmailItem()->label(), mUserEmail);

//    KPIM::KPrefsWidRadios *defaultEmailAttachMethod
//        = addWidRadios(
//              IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethodItem(),
//              personalFrame);
//    personalLayout->addWidget(defaultEmailAttachMethod->groupBox());
    QGroupBox *defaultEmailAttachMethodGroupBox = new QGroupBox(this);
    QButtonGroup *defaultEmailAttachGroup = new QButtonGroup(this);
    QRadioButton *askRadioButton = new QRadioButton(i18n("Always ask"), this);
    defaultEmailAttachGroup->addButton(askRadioButton, IncidenceEditorNG::IncidenceEditorSettingsBase::EnumDefaultEmailAttachMethod::Ask);
    QVBoxLayout *defaultEmailAttachMethodGroupBoxLayout = new QVBoxLayout(defaultEmailAttachMethodGroupBox);
    defaultEmailAttachMethodGroupBoxLayout->addWidget(askRadioButton);

    personalLayout->addWidget(defaultEmailAttachMethodGroupBox);
    personalLayout->addStretch(1);

    // Save Settings
    QFrame *saveFrame = new QFrame(this);
    tabWidget->addTab(saveFrame, QIcon::fromTheme(QStringLiteral("document-save")),
                      i18nc("@title:tab", "Save"));
    QVBoxLayout *saveLayout = new QVBoxLayout(saveFrame);

    mConfirmCheckBox = new QCheckBox(KOPrefs::instance()->confirmItem()->label(), saveFrame);
    saveLayout->addWidget(mConfirmCheckBox);

    mDestinationCheckBox = new QCheckBox(KOPrefs::instance()->destinationItem()->label(), saveFrame);
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
    mEmailControlCenterCheckBox->setChecked(CalendarSupport::KCalPrefs::instance()->emailControlCenter());
    mUserName->setText(CalendarSupport::KCalPrefs::instance()->userName());
    mUserEmail->setText(CalendarSupport::KCalPrefs::instance()->userEmail());
    mConfirmCheckBox->setChecked(KOPrefs::instance()->confirm());
    mDestinationCheckBox->setChecked(KOPrefs::instance()->destination());
    mShowReminderDaemonCheckBox->setChecked(KOPrefs::instance()->showReminderDaemon());
}

void KOPrefsDialogMain::save()
{
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

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigmain(QWidget *parent, const char *)
{
    return new KOPrefsDialogMain(parent);
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class KOPrefsDialogTime : public KCModule
{
public:
    KOPrefsDialogTime(QWidget *parent)
        : KCModule(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QTabWidget *tabWidget = new QTabWidget(this);
        layout->addWidget(tabWidget);

        QFrame *regionalPage = new QFrame(parent);
        tabWidget->addTab(regionalPage, QIcon::fromTheme(QStringLiteral("flag")),
                          i18nc("@title:tab", "Regional"));

        QGridLayout *regionalLayout = new QGridLayout(regionalPage);

        QGroupBox *datetimeGroupBox
            = new QGroupBox(i18nc("@title:group", "General Time and Date"), regionalPage);
        regionalLayout->addWidget(datetimeGroupBox, 0, 0);

        QGridLayout *datetimeLayout = new QGridLayout(datetimeGroupBox);

        mDayBegin = new KTimeComboBox(this);
        datetimeLayout->addWidget(new QLabel(KOPrefs::instance()->dayBeginsItem()->label(), this), 1, 0);
        datetimeLayout->addWidget(mDayBegin, 1, 1);
        connect(mDayBegin, &KTimeComboBox::currentIndexChanged, this, &KOPrefsDialogTime::slotConfigChanged);

        QGroupBox *holidaysGroupBox
            = new QGroupBox(i18nc("@title:group", "Holidays"), regionalPage);
        regionalLayout->addWidget(holidaysGroupBox, 1, 0);

        QGridLayout *holidaysLayout = new QGridLayout(holidaysGroupBox);

        // holiday region selection
        QWidget *holidayRegBox = new QWidget(regionalPage);
        QHBoxLayout *holidayRegBoxHBoxLayout = new QHBoxLayout(holidayRegBox);
        holidayRegBoxHBoxLayout->setContentsMargins(0, 0, 0, 0);
        holidaysLayout->addWidget(holidayRegBox, 1, 0, 1, 2);

        QLabel *holidayLabel = new QLabel(i18nc("@label", "Use holiday region:"), holidayRegBox);
        holidayLabel->setToolTip(KOPrefs::instance()->holidaysItem()->toolTip());
        holidayLabel->setWhatsThis(KOPrefs::instance()->holidaysItem()->whatsThis());

        mHolidayCheckCombo = new KPIM::KCheckComboBox(holidayRegBox);
        connect(mHolidayCheckCombo, &KPIM::KCheckComboBox::activated, this, &KOPrefsDialogTime::slotConfigChanged);
        holidayRegBoxHBoxLayout->addWidget(mHolidayCheckCombo);
        connect(mHolidayCheckCombo, &KPIM::KCheckComboBox::checkedItemsChanged,
                this, &KOPrefsDialogTime::slotConfigChanged);

        mHolidayCheckCombo->setToolTip(KOPrefs::instance()->holidaysItem()->toolTip());
        mHolidayCheckCombo->setWhatsThis(KOPrefs::instance()->holidaysItem()->whatsThis());

        const QStringList regions = KHolidays::HolidayRegion::regionCodes();
        std::vector<std::pair<QString, QString>> regionsMap;
        regionsMap.reserve(regions.size());

        for (const QString &regionCode : regions) {
            const QString name = KHolidays::HolidayRegion::name(regionCode);
            const QLocale locale(KHolidays::HolidayRegion::languageCode(regionCode));
            const QString languageName = QLocale::languageToString(locale.language());
            QString label;
            if (languageName.isEmpty()) {
                label = name;
            } else {
                label = i18nc("@item:inlistbox Holiday region, region language", "%1 (%2)",
                              name, languageName);
            }
            regionsMap.push_back(std::make_pair(label, regionCode));
        }
        std::sort(regionsMap.begin(), regionsMap.end(), [](const auto &lhs, const auto &rhs) { return lhs.first < rhs.first; });

        mHolidayCheckCombo->clear();
        mHolidayCheckCombo->setDefaultText(i18nc("@item:inlistbox", "Select Holiday Regions"));
        for (const auto &entry : regionsMap) {
            mHolidayCheckCombo->addItem(entry.first, entry.second);
        }

        const auto holidays = KOGlobals::self()->holidays();
        for (KHolidays::HolidayRegion *region : holidays) {
            const QString regionStr = region->regionCode();
            mHolidayCheckCombo->setItemCheckState(
                mHolidayCheckCombo->findData(regionStr), Qt::Checked);
        }

        QGroupBox *workingHoursGroupBox = new QGroupBox(i18nc("@title:group", "Working Period"),
                                                        regionalPage);
        regionalLayout->addWidget(workingHoursGroupBox, 2, 0);

        QBoxLayout *workingHoursLayout = new QVBoxLayout(workingHoursGroupBox);

        QBoxLayout *workDaysLayout = new QHBoxLayout;
        workingHoursLayout->addLayout(workDaysLayout);

        // Respect start of week setting
        int weekStart = QLocale().firstDayOfWeek();
        for (int i = 0; i < 7; ++i) {
            QString weekDayName = QLocale().dayName((i + weekStart + 6) % 7 + 1,
                                                    QLocale::ShortFormat);
            int index = (i + weekStart + 6) % 7;
            mWorkDays[ index ] = new QCheckBox(weekDayName);
            mWorkDays[ index ]->setWhatsThis(
                i18nc("@info:whatsthis",
                      "Check this box to make KOrganizer mark the "
                      "working hours for this day of the week. "
                      "If this is a work day for you, check "
                      "this box, or the working hours will not be "
                      "marked with color."));

            connect(mWorkDays[ index ], &QCheckBox::stateChanged,
                    this, &KOPrefsDialogTime::slotConfigChanged);

            workDaysLayout->addWidget(mWorkDays[ index ]);
        }

        mFirstDayCombo = new QComboBox(workingHoursGroupBox);
        connect(mFirstDayCombo, &QComboBox::activated, this, &KOPrefsDialogTime::slotConfigChanged);

        QHBoxLayout *firstDayLayout = new QHBoxLayout;
        workingHoursLayout->addLayout(firstDayLayout);
        QStringList days;
        days << i18nc("@item:inlistbox", "Monday")
             << i18nc("@item:inlistbox", "Tuesday")
             << i18nc("@item:inlistbox", "Wednesday")
             << i18nc("@item:inlistbox", "Thursday")
             << i18nc("@item:inlistbox", "Friday")
             << i18nc("@item:inlistbox", "Saturday")
             << i18nc("@item:inlistbox", "Sunday");
        mFirstDayCombo->addItems(days);

        firstDayLayout->addWidget(new QLabel(KOPrefs::instance()->weekStartDayItem()->label(), this));
        firstDayLayout->addWidget(mFirstDayCombo);

        mWorkStart = new KTimeComboBox(this);
        connect(mWorkStart, &KTimeComboBox::activated, this, &KOPrefsDialogTime::slotConfigChanged);

        QHBoxLayout *workStartLayout = new QHBoxLayout;
        workingHoursLayout->addLayout(workStartLayout);

        workStartLayout->addWidget(new QLabel(KOPrefs::instance()->workingHoursStartItem()->label(), this));
        workStartLayout->addWidget(mWorkStart);

        mWorkEnd = new KTimeComboBox(this);
        connect(mWorkEnd, &KTimeComboBox::activated, this, &KOPrefsDialogTime::slotConfigChanged);

        QHBoxLayout *workEndLayout = new QHBoxLayout;
        workingHoursLayout->addLayout(workEndLayout);

        workEndLayout->addWidget(new QLabel(KOPrefs::instance()->workingHoursEndItem()->label(), this));
        workEndLayout->addWidget(mWorkEnd);

        mExcludeHolidaysCheckbox = new QCheckBox(KOPrefs::instance()->excludeHolidaysItem()->label(), this);
        workingHoursLayout->addWidget(mExcludeHolidaysCheckbox);

        regionalLayout->setRowStretch(4, 1);

        QFrame *defaultPage = new QFrame(parent);
        tabWidget->addTab(defaultPage, QIcon::fromTheme(QStringLiteral("draw-eraser")),
                          i18nc("@title:tab", "Default Values"));
        QGridLayout *defaultLayout = new QGridLayout(defaultPage);

        QGroupBox *timesGroupBox
            = new QGroupBox(i18nc("@title:group", "Appointments"), defaultPage);
        defaultLayout->addWidget(timesGroupBox, 0, 0);

        QGridLayout *timesLayout = new QGridLayout(timesGroupBox);

        mDefaultTime = new KTimeComboBox(this);
        connect(mDefaultTime, &KTimeComboBox::activated, this, &KOPrefsDialogTime::slotConfigChanged);
        timesLayout->addWidget(new QLabel(CalendarSupport::KCalPrefs::instance()->startTimeItem()->label(), this), 0, 0);
        timesLayout->addWidget(mDefaultTime, 0, 1);

        mDefaultDuration = new QTimeEdit(this);
        mDefaultDuration->setDisplayFormat(QStringLiteral("hh:mm"));
        mDefaultDuration->setMinimumTime(QTime(0, 1));     // [1 min]
        mDefaultDuration->setMaximumTime(QTime(24, 0));     // [24 hr]
        connect(mDefaultDuration, &QTimeEdit::dateTimeChanged, this, &KOPrefsDialogTime::slotConfigChanged);
//        KPIM::KPrefsWidDuration *defaultDuration
//            = addWidDuration(CalendarSupport::KCalPrefs::instance()->defaultDurationItem(),
//                             QStringLiteral("hh:mm"), defaultPage);

        timesLayout->addWidget(new QLabel(CalendarSupport::KCalPrefs::instance()->defaultDurationItem()->label(), this), 1, 0);
        timesLayout->addWidget(mDefaultDuration, 1, 1);

        QGroupBox *remindersGroupBox
            = new QGroupBox(i18nc("@title:group", "Reminders"), defaultPage);
        defaultLayout->addWidget(remindersGroupBox, 1, 0);

        QGridLayout *remindersLayout = new QGridLayout(remindersGroupBox);

        QLabel *reminderLabel
            = new QLabel(i18nc("@label", "Default reminder time:"), defaultPage);
        remindersLayout->addWidget(reminderLabel, 0, 0);
        reminderLabel->setWhatsThis(
            CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->whatsThis());
        mReminderTimeSpin = new QSpinBox(defaultPage);
        mReminderTimeSpin->setWhatsThis(
            CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->whatsThis());
        mReminderTimeSpin->setToolTip(
            CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->toolTip());
        connect(mReminderTimeSpin, qOverload<int>(&QSpinBox::valueChanged),
                this, &KOPrefsDialogTime::slotConfigChanged);
        remindersLayout->addWidget(mReminderTimeSpin, 0, 1);

        mReminderUnitsCombo = new KComboBox(defaultPage);
        mReminderUnitsCombo->setToolTip(
            CalendarSupport::KCalPrefs::instance()->reminderTimeUnitsItem()->toolTip());
        mReminderUnitsCombo->setWhatsThis(
            CalendarSupport::KCalPrefs::instance()->reminderTimeUnitsItem()->whatsThis());
        connect(mReminderUnitsCombo, qOverload<int>(&KComboBox::activated),
                this, &KOPrefsDialogTime::slotConfigChanged);
        mReminderUnitsCombo->addItem(
            i18nc("@item:inlistbox reminder units in minutes", "minute(s)"));
        mReminderUnitsCombo->addItem(
            i18nc("@item:inlistbox reminder time units in hours", "hour(s)"));
        mReminderUnitsCombo->addItem(
            i18nc("@item:inlistbox reminder time units in days", "day(s)"));
        remindersLayout->addWidget(mReminderUnitsCombo, 0, 2);

        mDefaultAudioFileRemindersCheckBox = new QCheckBox(CalendarSupport::KCalPrefs::instance()->defaultAudioFileRemindersItem()->label(), this);

        if (CalendarSupport::KCalPrefs::instance()->audioFilePathItem()->value().isEmpty()) {
            const QString defAudioFile
                = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                         QStringLiteral("sound/") + QLatin1String("KDE-Sys-Warning.ogg"));
            CalendarSupport::KCalPrefs::instance()->audioFilePathItem()->setValue(defAudioFile);
        }
        const QString filter = i18n("*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra|"
                              "Audio Files (*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra)");
        mUrlRequester = new KUrlRequester(this);
        mUrlRequester->setFilter(filter);
        mUrlRequester->setEnabled(mDefaultAudioFileRemindersCheckBox->isChecked());

        connect(mDefaultAudioFileRemindersCheckBox, &QCheckBox::toggled, mUrlRequester, &KUrlRequester::setEnabled);

        QVBoxLayout *audioFileRemindersBox = new QVBoxLayout;
        audioFileRemindersBox->addWidget(mDefaultAudioFileRemindersCheckBox);
        audioFileRemindersBox->addWidget(mUrlRequester);

        remindersLayout->addLayout(audioFileRemindersBox, 1, 0);
        mDefaultEventRemindersCheckBox = new QCheckBox(CalendarSupport::KCalPrefs::instance()->defaultEventRemindersItem()->label(), this);
        remindersLayout->addWidget(mDefaultEventRemindersCheckBox, 2, 0);
        mDefaultTodoRemindersCheckBox = new QCheckBox(CalendarSupport::KCalPrefs::instance()->defaultTodoRemindersItem()->label(), this);
        remindersLayout->addWidget(mDefaultTodoRemindersCheckBox, 3, 0);

        defaultLayout->setRowStretch(3, 1);
        load();
    }

    void load() override
    {
        //TODO mFirstDayCombo

        mDefaultAudioFileRemindersCheckBox->setChecked(CalendarSupport::KCalPrefs::instance()->defaultAudioFileReminders());
        mDefaultDuration->setMaximumTime(QTime(24, 0));     // [24 hr]
        mDefaultDuration->setTime(CalendarSupport::KCalPrefs::instance()->defaultDuration().time());
        mDefaultTime->setTime(CalendarSupport::KCalPrefs::instance()->startTime().time());
        mDayBegin->setTime(KOPrefs::instance()->dayBegins().time());
        mWorkStart->setTime(KOPrefs::instance()->workingHoursStart().time());
        mWorkEnd->setTime(KOPrefs::instance()->workingHoursEnd().time());
        mDefaultEventRemindersCheckBox->setChecked(CalendarSupport::KCalPrefs::instance()->defaultEventReminders());
        mDefaultTodoRemindersCheckBox->setChecked(CalendarSupport::KCalPrefs::instance()->defaultTodoReminders());
        mExcludeHolidaysCheckbox->setChecked(KOPrefs::instance()->excludeHolidays());
        mUrlRequester->setText(CalendarSupport::KCalPrefs::instance()->audioFilePath());
        mReminderTimeSpin->setValue(CalendarSupport::KCalPrefs::instance()->mReminderTime);
        mReminderUnitsCombo->setCurrentIndex(
            CalendarSupport::KCalPrefs::instance()->mReminderTimeUnits);
        for (int i = 0; i < 7; ++i) {
            mWorkDays[i]->setChecked((1 << i) & (KOPrefs::instance()->mWorkWeekMask));
        }
    }

    void save() override
    {
        //TODO mFirstDayCombo
        CalendarSupport::KCalPrefs::instance()->setDefaultAudioFileReminders(mDefaultAudioFileRemindersCheckBox->isChecked());
        {
            QDateTime dt(CalendarSupport::KCalPrefs::instance()->defaultDuration());
            dt.setTime(mDefaultDuration->time());
            CalendarSupport::KCalPrefs::instance()->setDefaultDuration(dt);
        }
        {
            QDateTime dt(CalendarSupport::KCalPrefs::instance()->startTime());
            dt.setTime(mDefaultTime->time());
            CalendarSupport::KCalPrefs::instance()->setStartTime(dt);
        }
        {
            QDateTime dt(KOPrefs::instance()->dayBegins());
            dt.setTime(mDayBegin->time());
            KOPrefs::instance()->setDayBegins(dt);
        }
        {
            QDateTime dt(KOPrefs::instance()->workingHoursStart());
            dt.setTime(mWorkStart->time());
            KOPrefs::instance()->setWorkingHoursStart(dt);
        }
        {
            QDateTime dt(KOPrefs::instance()->workingHoursEnd());
            dt.setTime(mWorkEnd->time());
            KOPrefs::instance()->setWorkingHoursEnd(dt);
        }

        CalendarSupport::KCalPrefs::instance()->setAudioFilePath(mUrlRequester->text());
        CalendarSupport::KCalPrefs::instance()->setDefaultEventReminders(mDefaultEventRemindersCheckBox->isChecked());
        CalendarSupport::KCalPrefs::instance()->setDefaultTodoReminders(mDefaultTodoRemindersCheckBox->isChecked());

        KOPrefs::instance()->setExcludeHolidays(mExcludeHolidaysCheckbox->isChecked());
        QStringList HolidayRegions;
        const auto checkedItems = mHolidayCheckCombo->checkedItems();
        for (const QString &str : checkedItems) {
            int index = mHolidayCheckCombo->findText(str);
            if (index >= 0) {
                HolidayRegions.append(mHolidayCheckCombo->itemData(index).toString());
            }
        }
        KOPrefs::instance()->mHolidays = HolidayRegions;

        CalendarSupport::KCalPrefs::instance()->mReminderTime
            = mReminderTimeSpin->value();
        CalendarSupport::KCalPrefs::instance()->mReminderTimeUnits
            = mReminderUnitsCombo->currentIndex();

        int mask = 0;
        for (int i = 0; i < 7; ++i) {
            if (mWorkDays[i]->isChecked()) {
                mask = mask | (1 << i);
            }
        }
        KOPrefs::instance()->mWorkWeekMask = mask;
        KOPrefs::instance()->save();
        CalendarSupport::KCalPrefs::instance()->save();
    }
protected:
    void setCombo(KComboBox *combo, const QString &text, const QStringList *tags = nullptr)
    {
        if (tags) {
            int i = tags->indexOf(text);
            if (i > 0) {
                combo->setCurrentIndex(i);
            }
        } else {
            const int numberOfElements{combo->count()};
            for (int i = 0; i < numberOfElements; ++i) {
                if (combo->itemText(i) == text) {
                    combo->setCurrentIndex(i);
                    break;
                }
            }
        }
    }

private:
    void slotConfigChanged()
    {
        Q_EMIT markAsChanged();
    }
    QStringList tzonenames;
    KPIM::KCheckComboBox *mHolidayCheckCombo = nullptr;
    QSpinBox *mReminderTimeSpin = nullptr;
    KComboBox *mReminderUnitsCombo = nullptr;
    QCheckBox *mWorkDays[7];
    QComboBox *mFirstDayCombo = nullptr;
    QCheckBox *mDefaultEventRemindersCheckBox = nullptr;
    QCheckBox *mDefaultTodoRemindersCheckBox = nullptr;
    KUrlRequester *mUrlRequester = nullptr;
    QCheckBox *mExcludeHolidaysCheckbox = nullptr;
    KTimeComboBox *mDayBegin = nullptr;
    KTimeComboBox *mWorkStart = nullptr;
    KTimeComboBox *mWorkEnd = nullptr;
    KTimeComboBox *mDefaultTime = nullptr;
    QTimeEdit *mDefaultDuration = nullptr;
    QCheckBox *mDefaultAudioFileRemindersCheckBox;
};

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigtime(QWidget *parent, const char *)
{
    return new KOPrefsDialogTime(parent);
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class KOPrefsDialogViews : public KPIM::KPrefsModule
{
public:
    KOPrefsDialogViews(QWidget *parent)
        : KPIM::KPrefsModule(KOPrefs::instance(), parent)
        , mMonthIconComboBox(new KItemIconCheckCombo(KItemIconCheckCombo::MonthType, this))
        , mAgendaIconComboBox(new KItemIconCheckCombo(KItemIconCheckCombo::AgendaType, this))
    {
        QBoxLayout *topTopLayout = new QVBoxLayout(this);
        QTabWidget *tabWidget = new QTabWidget(this);
        topTopLayout->addWidget(tabWidget);

        connect(mMonthIconComboBox, &KPIM::KCheckComboBox::checkedItemsChanged,
                this, &KPIM::KPrefsModule::slotWidChanged);
        connect(mAgendaIconComboBox, &KPIM::KCheckComboBox::checkedItemsChanged,
                this, &KPIM::KPrefsModule::slotWidChanged);

        // Tab: Views->General
        QFrame *generalFrame = new QFrame(this);
        tabWidget->addTab(generalFrame, QIcon::fromTheme(QStringLiteral("view-choose")),
                          i18nc("@title:tab general settings", "General"));

        QBoxLayout *generalLayout = new QVBoxLayout(generalFrame);

        // GroupBox: Views->General->Display Options
        QVBoxLayout *gdisplayLayout = new QVBoxLayout;
        QGroupBox *gdisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));

        QBoxLayout *nextDaysLayout = new QHBoxLayout;
        gdisplayLayout->addLayout(nextDaysLayout);

        KPIM::KPrefsWidInt *nextDays
            = addWidInt(KOPrefs::instance()->nextXDaysItem());
        nextDays->spinBox()->setSuffix(
            i18nc("@label suffix in the N days spin box", " days"));

        nextDaysLayout->addWidget(nextDays->label());
        nextDaysLayout->addWidget(nextDays->spinBox());
        nextDaysLayout->addStretch(1);

        gdisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->enableToolTipsItem())->checkBox());
        gdisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->todosUseCategoryColorsItem())->checkBox());
        gdisplayBox->setLayout(gdisplayLayout);
        generalLayout->addWidget(gdisplayBox);

        // GroupBox: Views->General->Date Navigator
        QVBoxLayout *datenavLayout = new QVBoxLayout;
        QGroupBox *datenavBox = new QGroupBox(i18nc("@title:group", "Date Navigator"));
        datenavLayout->addWidget(
            addWidBool(KOPrefs::instance()->dailyRecurItem())->checkBox());
        datenavLayout->addWidget(
            addWidBool(KOPrefs::instance()->weeklyRecurItem())->checkBox());
        datenavLayout->addWidget(
            addWidBool(KOPrefs::instance()->highlightTodosItem())->checkBox());
        datenavLayout->addWidget(
            addWidBool(KOPrefs::instance()->highlightJournalsItem())->checkBox());
        datenavLayout->addWidget(
            addWidBool(KOPrefs::instance()->weekNumbersShowWorkItem())->checkBox());
        datenavBox->setLayout(datenavLayout);
        generalLayout->addWidget(datenavBox);
        generalLayout->addStretch(1);

        // Tab: Views->Agenda View
        QFrame *agendaFrame = new QFrame(this);
        tabWidget->addTab(agendaFrame, QIcon::fromTheme(QStringLiteral("view-calendar-workweek")),
                          i18nc("@title:tab", "Agenda View"));

        QBoxLayout *agendaLayout = new QVBoxLayout(agendaFrame);

        // GroupBox: Views->Agenda View->Display Options
        QVBoxLayout *adisplayLayout = new QVBoxLayout;
        QGroupBox *adisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));

        QHBoxLayout *hourSizeLayout = new QHBoxLayout;
        adisplayLayout->addLayout(hourSizeLayout);

        KPIM::KPrefsWidInt *hourSize
            = addWidInt(KOPrefs::instance()->hourSizeItem());
        hourSize->spinBox()->setSuffix(
            i18nc("@label suffix in the hour size spin box", " pixels"));

        hourSizeLayout->addWidget(hourSize->label());
        hourSizeLayout->addWidget(hourSize->spinBox());
        hourSizeLayout->addStretch(1);

        adisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->enableAgendaItemIconsItem())->checkBox());
        adisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->showTodosAgendaViewItem())->checkBox());
        KPIM::KPrefsWidBool *marcusBainsEnabled
            = addWidBool(KOPrefs::instance()->marcusBainsEnabledItem());
        adisplayLayout->addWidget(marcusBainsEnabled->checkBox());

        KPIM::KPrefsWidBool *marcusBainsShowSeconds
            = addWidBool(KOPrefs::instance()->marcusBainsShowSecondsItem());
        connect(marcusBainsEnabled->checkBox(), &QAbstractButton::toggled,
                marcusBainsShowSeconds->checkBox(), &QWidget::setEnabled);

        adisplayLayout->addWidget(marcusBainsShowSeconds->checkBox());
        adisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->selectionStartsEditorItem())->checkBox());
        mAgendaIconComboBox->setCheckedIcons(
            KOPrefs::instance()->eventViewsPreferences()->agendaViewIcons());
        adisplayLayout->addWidget(mAgendaIconComboBox);
        adisplayBox->setLayout(adisplayLayout);
        agendaLayout->addWidget(adisplayBox);

        // GroupBox: Views->Agenda View->Color Usage
        agendaLayout->addWidget(
            addWidRadios(KOPrefs::instance()->agendaViewColorsItem())->groupBox());

        agendaLayout->addWidget(
            addWidBool(KOPrefs::instance()->colorBusyDaysEnabledItem())->checkBox());

        // GroupBox: Views->Agenda View->Multiple Calendars
        agendaLayout->addWidget(
            addWidRadios(KOPrefs::instance()->agendaViewCalendarDisplayItem())->groupBox());

        agendaLayout->addStretch(1);

        // Tab: Views->Month View
        QFrame *monthFrame = new QFrame(this);
        tabWidget->addTab(monthFrame, QIcon::fromTheme(QStringLiteral("view-calendar-month")),
                          i18nc("@title:tab", "Month View"));

        QBoxLayout *monthLayout = new QVBoxLayout(monthFrame);

        // GroupBox: Views->Month View->Display Options
        QVBoxLayout *mdisplayLayout = new QVBoxLayout;
        QGroupBox *mdisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));
        /*mdisplayLayout->addWidget(
          addWidBool( KOPrefs::instance()->enableMonthScrollItem() )->checkBox() );*/
        mdisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->showTimeInMonthViewItem())->checkBox());
        mdisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->enableMonthItemIconsItem())->checkBox());
        mdisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->showTodosMonthViewItem())->checkBox());
        mdisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->showJournalsMonthViewItem())->checkBox());
        mdisplayBox->setLayout(mdisplayLayout);

        mMonthIconComboBox->setCheckedIcons(
            KOPrefs::instance()->eventViewsPreferences()->monthViewIcons());
        mdisplayLayout->addWidget(mMonthIconComboBox);

        monthLayout->addWidget(mdisplayBox);

        monthLayout->addWidget(
            addWidBool(KOPrefs::instance()->colorMonthBusyDaysEnabledItem())->checkBox());

        // GroupBox: Views->Month View->Color Usage
        monthLayout->addWidget(
            addWidRadios(KOPrefs::instance()->monthViewColorsItem())->groupBox());
        monthLayout->addStretch(1);

        // Tab: Views->Todo View
        QFrame *todoFrame = new QFrame(this);
        tabWidget->addTab(todoFrame, QIcon::fromTheme(QStringLiteral("view-calendar-tasks")),
                          i18nc("@title:tab", "Todo View"));

        QBoxLayout *todoLayout = new QVBoxLayout(todoFrame);

        // GroupBox: Views->Todo View->Display Options
        QVBoxLayout *tdisplayLayout = new QVBoxLayout;
        QGroupBox *tdisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));
        tdisplayLayout->addWidget(
            addWidBool(KOPrefs::instance()->sortCompletedTodosSeparatelyItem())->checkBox());
        tdisplayBox->setLayout(tdisplayLayout);
        todoLayout->addWidget(tdisplayBox);

        // GroupBox: Views->Todo View->Other
        QVBoxLayout *otherLayout = new QVBoxLayout;
        QGroupBox *otherBox = new QGroupBox(i18nc("@title:group", "Other Options"));
        otherLayout->addWidget(
            addWidBool(KOPrefs::instance()->recordTodosInJournalsItem())->checkBox());
        otherBox->setLayout(otherLayout);
        todoLayout->addWidget(otherBox);
        todoLayout->addStretch(1);

        load();
    }

protected:
    void usrReadConfig() override
    {
        KOPrefs::instance()->eventViewsPreferences()->setAgendaViewIcons(
            mAgendaIconComboBox->checkedIcons());
        KOPrefs::instance()->eventViewsPreferences()->setMonthViewIcons(
            mMonthIconComboBox->checkedIcons());
    }

private:
    KItemIconCheckCombo *mMonthIconComboBox;
    KItemIconCheckCombo *mAgendaIconComboBox;
};

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigviews(QWidget *parent, const char *)
{
    return new KOPrefsDialogViews(parent);
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

KOPrefsDialogColorsAndFonts::KOPrefsDialogColorsAndFonts(QWidget *parent)
    : KPIM::KPrefsModule(KOPrefs::instance(), parent)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);
    topTopLayout->addWidget(tabWidget);

    QWidget *colorFrame = new QWidget(this);
    topTopLayout->addWidget(colorFrame);
    QGridLayout *colorLayout = new QGridLayout(colorFrame);
    tabWidget->addTab(colorFrame, QIcon::fromTheme(QStringLiteral("preferences-desktop-color")),
                      i18nc("@title:tab", "Colors"));

    // Use System color
    KPIM::KPrefsWidBool *useSystemColorBool
        = addWidBool(KOPrefs::instance()->useSystemColorItem(), colorFrame);

    QCheckBox *useSystemColorButton = useSystemColorBool->checkBox();
    QObject::connect(useSystemColorButton, &QCheckBox::toggled,
                     this, &KOPrefsDialogColorsAndFonts::useSystemColorToggle);
    colorLayout->addWidget(useSystemColorBool->checkBox(), 1, 0, 1, 2);

    // agenda view background color
    KPIM::KPrefsWidColor *agendaBgColor
        = addWidColor(KOPrefs::instance()->agendaGridBackgroundColorItem(), colorFrame);
    KColorButton *agendaBgColorButton = agendaBgColor->button();
    mButtonsDisable.push_back(agendaBgColorButton);
    colorLayout->addWidget(agendaBgColor->label(), 2, 0);
    colorLayout->addWidget(agendaBgColorButton, 2, 1);

    KPIM::KPrefsWidColor *viewBgBusyColor
        = addWidColor(KOPrefs::instance()->viewBgBusyColorItem(), colorFrame);
    KColorButton *viewBgBusyColorButton = viewBgBusyColor->button();
    mButtonsDisable.push_back(viewBgBusyColorButton);
    colorLayout->addWidget(viewBgBusyColor->label(), 3, 0);
    colorLayout->addWidget(viewBgBusyColorButton, 3, 1);

    // working hours color
    KPIM::KPrefsWidColor *agendaGridWorkHoursBackgroundColor
        = addWidColor(KOPrefs::instance()->workingHoursColorItem(), colorFrame);
    KColorButton *agendaGridWorkHoursBackgroundColorButton = agendaGridWorkHoursBackgroundColor->button();
    mButtonsDisable.push_back(agendaGridWorkHoursBackgroundColorButton);
    colorLayout->addWidget(agendaGridWorkHoursBackgroundColor->label(), 4, 0);
    colorLayout->addWidget(agendaGridWorkHoursBackgroundColor->button(), 4, 1);

    // agenda view Marcus Bains line color
    KPIM::KPrefsWidColor *mblColor
        = addWidColor(KOPrefs::instance()->agendaMarcusBainsLineLineColorItem(), colorFrame);
    colorLayout->addWidget(mblColor->label(), 5, 0);
    colorLayout->addWidget(mblColor->button(), 5, 1);

    // Holiday Color
    KPIM::KPrefsWidColor *holidayColor
        = addWidColor(KOPrefs::instance()->agendaHolidaysBackgroundColorItem(), colorFrame);
    colorLayout->addWidget(holidayColor->label(), 6, 0);
    colorLayout->addWidget(holidayColor->button(), 6, 1);

    // Todo due today color
    KPIM::KPrefsWidColor *todoDueTodayColor
        = addWidColor(
              KOPrefs::instance()->todoDueTodayColorItem(), colorFrame);
    colorLayout->addWidget(todoDueTodayColor->label(), 7, 0);
    colorLayout->addWidget(todoDueTodayColor->button(), 7, 1);

    // Todo overdue color
    KPIM::KPrefsWidColor *todoOverdueColor
        = addWidColor(
              KOPrefs::instance()->todoOverdueColorItem(), colorFrame);
    colorLayout->addWidget(todoOverdueColor->label(), 8, 0);
    colorLayout->addWidget(todoOverdueColor->button(), 8, 1);

    // categories colors
    QGroupBox *categoryGroup = new QGroupBox(i18nc("@title:group", "Categories"), colorFrame);
    colorLayout->addWidget(categoryGroup, 9, 0, 1, 2);

    QGridLayout *categoryLayout = new QGridLayout;
    categoryGroup->setLayout(categoryLayout);

    KPIM::KPrefsWidColor *unsetCategoryColor
        = addWidColor(
              CalendarSupport::KCalPrefs::instance()->unsetCategoryColorItem(), categoryGroup);
    categoryLayout->addWidget(unsetCategoryColor->label(), 0, 0);
    categoryLayout->addWidget(unsetCategoryColor->button(), 0, 1);
    unsetCategoryColor->label()->setWhatsThis(unsetCategoryColor->button()->whatsThis());
    unsetCategoryColor->label()->setToolTip(unsetCategoryColor->button()->toolTip());

    mCategoryCombo = new Akonadi::TagSelectionComboBox(categoryGroup);
    mCategoryCombo->setWhatsThis(
        i18nc("@info:whatsthis",
              "Select here the event category you want to modify. "
              "You can change the selected category color using "
              "the button below."));
    connect(mCategoryCombo, qOverload<int>(&KComboBox::activated),
            this, &KOPrefsDialogColorsAndFonts::updateCategoryColor);
    categoryLayout->addWidget(mCategoryCombo, 1, 0);

    mCategoryButton = new KColorButton(categoryGroup);
    mCategoryButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose here the color of the event category selected "
              "using the combo box above."));
    connect(mCategoryButton, &KColorButton::changed, this,
            &KOPrefsDialogColorsAndFonts::setCategoryColor);
    categoryLayout->addWidget(mCategoryButton, 1, 1);

    updateCategoryColor();

    // resources colors
    QGroupBox *resourceGroup = new QGroupBox(i18nc("@title:group", "Resources"), colorFrame);
    colorLayout->addWidget(resourceGroup, 10, 0, 1, 2);

    QBoxLayout *resourceLayout = new QHBoxLayout;
    resourceGroup->setLayout(resourceLayout);

    mResourceCombo = new Akonadi::CollectionComboBox(resourceGroup);
    //mResourceCombo->addExcludedSpecialResources(Akonadi::Collection::SearchResource);
    QStringList mimetypes;
    mimetypes << KCalendarCore::Todo::todoMimeType();
    mimetypes << KCalendarCore::Journal::journalMimeType();
    mimetypes << KCalendarCore::Event::eventMimeType();

    mResourceCombo->setMimeTypeFilter(mimetypes);
    mResourceCombo->setWhatsThis(
        i18nc("@info:whatsthis",
              "Select the calendar you want to modify. "
              "You can change the selected calendar color using "
              "the button below."));
    connect(mResourceCombo, qOverload<int>(&Akonadi::CollectionComboBox::activated),
            this, &KOPrefsDialogColorsAndFonts::updateResourceColor);
    resourceLayout->addWidget(mResourceCombo);

    mResourceButton = new KColorButton(resourceGroup);
    mResourceButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose here the color of the calendar selected "
              "using the combo box above."));
    connect(mResourceButton, &KColorButton::changed, this,
            &KOPrefsDialogColorsAndFonts::setResourceColor);
    resourceLayout->addWidget(mResourceButton);

    colorLayout->setRowStretch(11, 1);

    QWidget *fontFrame = new QWidget(this);
    tabWidget->addTab(fontFrame, QIcon::fromTheme(QStringLiteral("preferences-desktop-font")),
                      i18nc("@title:tab", "Fonts"));

    QGridLayout *fontLayout = new QGridLayout(fontFrame);

    KPIM::KPrefsWidFont *timeBarFont
        = addWidFont(KOPrefs::instance()->agendaTimeLabelsFontItem(), fontFrame,
                     QLocale().toString(QTime(12, 34), QLocale::ShortFormat));
    fontLayout->addWidget(timeBarFont->label(), 0, 0);
    fontLayout->addWidget(timeBarFont->preview(), 0, 1);
    fontLayout->addWidget(timeBarFont->button(), 0, 2);

    KPIM::KPrefsWidFont *monthViewFont
        = addWidFont(KOPrefs::instance()->monthViewFontItem(), fontFrame,
                     QLocale().toString(QTime(12, 34), QLocale::ShortFormat) + QLatin1Char(' ')
                     +i18nc("@label", "Event text"));

    fontLayout->addWidget(monthViewFont->label(), 1, 0);
    fontLayout->addWidget(monthViewFont->preview(), 1, 1);
    fontLayout->addWidget(monthViewFont->button(), 1, 2);

    KPIM::KPrefsWidFont *agendaViewFont
        = addWidFont(KOPrefs::instance()->agendaViewFontItem(), fontFrame,
                     i18nc("@label", "Event text"));
    fontLayout->addWidget(agendaViewFont->label(), 2, 0);
    fontLayout->addWidget(agendaViewFont->preview(), 2, 1);
    fontLayout->addWidget(agendaViewFont->button(), 2, 2);

    KPIM::KPrefsWidFont *marcusBainsFont
        = addWidFont(KOPrefs::instance()->agendaMarcusBainsLineFontItem(), fontFrame,
                     QLocale().toString(QTime(12, 34, 23), QLocale::ShortFormat));
    fontLayout->addWidget(marcusBainsFont->label(), 3, 0);
    fontLayout->addWidget(marcusBainsFont->preview(), 3, 1);
    fontLayout->addWidget(marcusBainsFont->button(), 3, 2);

    fontLayout->setColumnStretch(1, 1);
    fontLayout->setRowStretch(4, 1);

    load();
}

void KOPrefsDialogColorsAndFonts::usrWriteConfig()
{
    QHash<QString, QColor>::const_iterator i = mCategoryDict.constBegin();
    while (i != mCategoryDict.constEnd()) {
        CalendarSupport::KCalPrefs::instance()->setCategoryColor(i.key(), i.value());
        ++i;
    }

    i = mResourceDict.constBegin();
    while (i != mResourceDict.constEnd()) {
        KOPrefs::instance()->setResourceColor(i.key(), i.value());
        ++i;
    }

    //mCalendarViewsPrefs->writeConfig();
}

void KOPrefsDialogColorsAndFonts::usrReadConfig()
{
    updateCategories();
    updateResources();
    //mCalendarViewsPrefs->readConfig();
}

void KOPrefsDialogColorsAndFonts::useSystemColorToggle(bool useSystemColor)
{
    for (KColorButton *colorButton : qAsConst(mButtonsDisable)) {
        if (useSystemColor) {
            colorButton->setEnabled(false);
        } else {
            colorButton->setEnabled(true);
        }
    }
}

void KOPrefsDialogColorsAndFonts::updateCategories()
{
    updateCategoryColor();
}

void KOPrefsDialogColorsAndFonts::setCategoryColor()
{
    mCategoryDict.insert(mCategoryCombo->currentText(), mCategoryButton->color());
    slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateCategoryColor()
{
    const QString cat = mCategoryCombo->currentText();
    QColor color = mCategoryDict.value(cat);
    if (!color.isValid()) {
        //TODO get this from the tag
        color = CalendarSupport::KCalPrefs::instance()->categoryColor(cat);
    }
    if (color.isValid()) {
        mCategoryButton->setColor(color);
    }
}

void KOPrefsDialogColorsAndFonts::updateResources()
{
    updateResourceColor();
}

void KOPrefsDialogColorsAndFonts::setResourceColor()
{
    bool ok;
    const QString id
        = QString::number(mResourceCombo->itemData(
                              mResourceCombo->currentIndex(),
                              Akonadi::EntityTreeModel::CollectionIdRole).toLongLong(&ok));
    if (!ok) {
        return;
    }
    mResourceDict.insert(id, mResourceButton->color());
    slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateResourceColor()
{
    bool ok;
    const QString id
        = QString::number(mResourceCombo->itemData(
                              mResourceCombo->currentIndex(),
                              Akonadi::EntityTreeModel::CollectionIdRole).toLongLong(&ok));
    if (!ok) {
        return;
    }
    qCDebug(KORGANIZER_LOG) << id << mResourceCombo->itemText(mResourceCombo->currentIndex());

    QColor color = mResourceDict.value(id);
    if (!color.isValid()) {
        color = KOPrefs::instance()->resourceColor(id);
    }
    mResourceButton->setColor(color);
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigcolorsandfonts(QWidget *parent, const char *)
{
    return new KOPrefsDialogColorsAndFonts(parent);
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling(QWidget *parent)
    : KCModule(parent)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);

    QWidget *topFrame = new QWidget(this);
    topTopLayout->addWidget(topFrame);

    QGridLayout *topLayout = new QGridLayout(topFrame);
    topLayout->setContentsMargins(0, 0, 0, 0);

    mUseGroupwareCommunicationCheckBox = new QCheckBox(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunicationItem()->label(), this);
    topLayout->addWidget(mUseGroupwareCommunicationCheckBox, 0, 0, 1, 2);
    connect(mUseGroupwareCommunicationCheckBox, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupScheduling::slotConfigChanged);

    mBccBox = new QCheckBox(Akonadi::CalendarSettings::self()->bccItem()->label(), this);
    topLayout->addWidget(mBccBox, 1, 0, 1, 2);
    connect(mBccBox, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupScheduling::slotConfigChanged);

    QLabel *aTransportLabel = new QLabel(
        i18nc("@label", "Mail transport:"), topFrame);
    topLayout->addWidget(aTransportLabel, 2, 0, 1, 2);

    MailTransport::TransportManagementWidget *tmw
        = new MailTransport::TransportManagementWidget(topFrame);
    tmw->layout()->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(tmw, 3, 0, 1, 2);

    load();
}

void KOPrefsDialogGroupScheduling::slotConfigChanged()
{
    Q_EMIT markAsChanged();
}

void KOPrefsDialogGroupScheduling::save()
{
    CalendarSupport::KCalPrefs::instance()->setUseGroupwareCommunication(mUseGroupwareCommunicationCheckBox->isChecked());
    Akonadi::CalendarSettings::self()->setBcc(mBccBox->isChecked());
}

void KOPrefsDialogGroupScheduling::load()
{
    mUseGroupwareCommunicationCheckBox->setChecked(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication());
    mBccBox->setChecked(Akonadi::CalendarSettings::self()->bcc());
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfiggroupscheduling(QWidget *parent, const char *)
{
    return new KOPrefsDialogGroupScheduling(parent);
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

KOPrefsDialogGroupwareScheduling::KOPrefsDialogGroupwareScheduling(QWidget *parent)
    : KCModule(parent)
{
    mGroupwarePage = new Ui::KOGroupwarePrefsPage();
    QWidget *widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("KOGrouparePrefsPage"));

    mGroupwarePage->setupUi(widget);

    mGroupwarePage->groupwareTab->setTabIcon(0, QIcon::fromTheme(QStringLiteral("go-up")));
    mGroupwarePage->groupwareTab->setTabIcon(1, QIcon::fromTheme(QStringLiteral("go-down")));

    // signals and slots connections

    connect(mGroupwarePage->publishDays, qOverload<int>(&QSpinBox::valueChanged),
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishUrl, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishUser, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishPassword, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishSavePassword, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveEnable, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveUser, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrievePassword, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveSavePassword, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveUrl, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishDelay, qOverload<int>(&QSpinBox::valueChanged),
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->fullDomainRetrieval, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishEnable, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);

    (new QVBoxLayout(this))->addWidget(widget);

    load();
}

KOPrefsDialogGroupwareScheduling::~KOPrefsDialogGroupwareScheduling()
{
    delete mGroupwarePage;
}

void KOPrefsDialogGroupwareScheduling::slotConfigChanged()
{
    Q_EMIT changed(true);
}

void KOPrefsDialogGroupwareScheduling::load()
{
    mGroupwarePage->publishEnable->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyPublishAuto());
    mGroupwarePage->publishDelay->setValue(
        Akonadi::CalendarSettings::self()->freeBusyPublishDelay());
    mGroupwarePage->publishDays->setValue(
        Akonadi::CalendarSettings::self()->freeBusyPublishDays());
    mGroupwarePage->publishUrl->setText(
        Akonadi::CalendarSettings::self()->freeBusyPublishUrl());
    mGroupwarePage->publishUser->setText(
        Akonadi::CalendarSettings::self()->freeBusyPublishUser());
    mGroupwarePage->publishPassword->setText(
        Akonadi::CalendarSettings::self()->freeBusyPublishPassword());
    mGroupwarePage->publishSavePassword->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyPublishSavePassword());

    mGroupwarePage->retrieveEnable->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveAuto());
    mGroupwarePage->fullDomainRetrieval->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyFullDomainRetrieval());
    mGroupwarePage->retrieveUrl->setText(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl());
    mGroupwarePage->retrieveUser->setText(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveUser());
    mGroupwarePage->retrievePassword->setText(
        Akonadi::CalendarSettings::self()->freeBusyRetrievePassword());
    mGroupwarePage->retrieveSavePassword->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveSavePassword());
}

void KOPrefsDialogGroupwareScheduling::save()
{
    Akonadi::CalendarSettings::self()->setFreeBusyPublishAuto(
        mGroupwarePage->publishEnable->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishDelay(mGroupwarePage->publishDelay->value());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishDays(
        mGroupwarePage->publishDays->value());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishUrl(
        mGroupwarePage->publishUrl->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishUser(
        mGroupwarePage->publishUser->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishPassword(
        mGroupwarePage->publishPassword->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishSavePassword(
        mGroupwarePage->publishSavePassword->isChecked());

    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveAuto(
        mGroupwarePage->retrieveEnable->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyFullDomainRetrieval(
        mGroupwarePage->fullDomainRetrieval->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUrl(
        mGroupwarePage->retrieveUrl->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUser(
        mGroupwarePage->retrieveUser->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrievePassword(
        mGroupwarePage->retrievePassword->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveSavePassword(
        mGroupwarePage->retrieveSavePassword->isChecked());

    // clear the url cache for our user
    const QString configFile
        = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String(
              "/korganizer/freebusyurls");
    KConfig cfg(configFile);
    cfg.deleteGroup(CalendarSupport::KCalPrefs::instance()->email());

    Akonadi::CalendarSettings::self()->save();
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigfreebusy(QWidget *parent, const char *)
{
    return new KOPrefsDialogGroupwareScheduling(parent);
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class PluginItem : public QTreeWidgetItem
{
public:
    PluginItem(QTreeWidget *parent, const KService::Ptr &service)
        : QTreeWidgetItem(parent, QStringList(service->name()))
        , mService(service)
    {
    }

    PluginItem(QTreeWidgetItem *parent, const KService::Ptr &service)
        : QTreeWidgetItem(parent, QStringList(service->name()))
        , mService(service)
    {
    }

    KService::Ptr service()
    {
        return mService;
    }

private:
    KService::Ptr mService;
};

/**
  Dialog for selecting and configuring KOrganizer plugins
*/
KOPrefsDialogPlugins::KOPrefsDialogPlugins(QWidget *parent)
    : KCModule(parent)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);
    mTreeWidget = new QTreeWidget(this);
    mTreeWidget->setColumnCount(1);
    mTreeWidget->setHeaderLabel(i18nc("@title:column plugin name", "Name"));
    topTopLayout->addWidget(mTreeWidget);

    mDescription = new QLabel(this);
    mDescription->setAlignment(Qt::AlignVCenter);
    mDescription->setWordWrap(true);
    mDescription->setFrameShape(QLabel::Panel);
    mDescription->setFrameShadow(QLabel::Sunken);
    mDescription->setMinimumSize(QSize(0, 55));
    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(0);
    policy.setHeightForWidth(mDescription->sizePolicy().hasHeightForWidth());
    mDescription->setSizePolicy(policy);
    topTopLayout->addWidget(mDescription);

    QWidget *buttonRow = new QWidget(this);
    QBoxLayout *buttonRowLayout = new QHBoxLayout(buttonRow);
    buttonRowLayout->setContentsMargins(0, 0, 0, 0);
    mConfigureButton = new QPushButton(buttonRow);
    KGuiItem::assign(mConfigureButton, KGuiItem(i18nc("@action:button", "Configure &Plugin..."),
                                                QStringLiteral("configure"), QString(),
                                                i18nc("@info:whatsthis",
                                                      "This button allows you to configure"
                                                      " the plugin that you have selected in the list above")));
    buttonRowLayout->addWidget(mConfigureButton);
    buttonRowLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    topTopLayout->addWidget(buttonRow);

    mPositioningGroupBox = new QGroupBox(i18nc("@title:group", "Position"), this);
    //mPositionMonthTop = new QCheckBox(
    //i18nc( "@option:check", "Show in the month view" ), mPositioningGroupBox );
    mPositionAgendaTop = new QRadioButton(
        i18nc("@option:check", "Show at the top of the agenda views"), mPositioningGroupBox);
    mPositionAgendaBottom = new QRadioButton(
        i18nc("@option:check", "Show at the bottom of the agenda views"), mPositioningGroupBox);
    QVBoxLayout *positioningLayout = new QVBoxLayout(mPositioningGroupBox);
    //positioningLayout->addWidget( mPositionMonthTop );
    positioningLayout->addWidget(mPositionAgendaTop);
    positioningLayout->addWidget(mPositionAgendaBottom);
    positioningLayout->addStretch(1);
    topTopLayout->addWidget(mPositioningGroupBox);

    connect(mConfigureButton, &QPushButton::clicked, this, &KOPrefsDialogPlugins::configure);

    connect(mPositionAgendaTop, &QRadioButton::clicked, this,
            &KOPrefsDialogPlugins::positioningChanged);
    connect(mPositionAgendaBottom, &QRadioButton::clicked, this,
            &KOPrefsDialogPlugins::positioningChanged);

    connect(mTreeWidget, &QTreeWidget::itemSelectionChanged, this,
            &KOPrefsDialogPlugins::selectionChanged);
    connect(mTreeWidget, &QTreeWidget::itemChanged, this, &KOPrefsDialogPlugins::selectionChanged);
    connect(mTreeWidget, &QTreeWidget::itemClicked, this, &KOPrefsDialogPlugins::slotConfigChanged);

    load();

    selectionChanged();
}

KOPrefsDialogPlugins::~KOPrefsDialogPlugins()
{
    delete mDecorations;
    delete mOthers;
}

void KOPrefsDialogPlugins::slotConfigChanged()
{
    Q_EMIT markAsChanged();
}

void KOPrefsDialogPlugins::load()
{
    mTreeWidget->clear();
    const KService::List plugins = KOCore::self()->availablePlugins() + KOCore::self()->availableParts();

    EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();

    QStringList selectedPlugins = viewPrefs->selectedPlugins();

    mDecorations = new QTreeWidgetItem(mTreeWidget,
                                       QStringList(i18nc("@title:group", "Calendar Decorations")));
    mOthers = new QTreeWidgetItem(mTreeWidget,
                                  QStringList(i18nc("@title:group", "Other Plugins")));

    KService::List::ConstIterator it;
    KService::List::ConstIterator end(plugins.constEnd());

    for (it = plugins.constBegin(); it != end; ++it) {
        QTreeWidgetItem *item = nullptr;
        if ((*it)->hasServiceType(EventViews::CalendarDecoration::Decoration::serviceType())) {
            item = new PluginItem(mDecorations, *it);
        } else {
            continue;
        }
        if (selectedPlugins.contains((*it)->desktopEntryName())) {
            item->setCheckState(0, Qt::Checked);
        } else {
            item->setCheckState(0, Qt::Unchecked);
        }
    }

    mDecorations->setExpanded(true);
    mOthers->setExpanded(true);

    //Disable for the moment it crashs. Not understand why
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    mDecorationsAtMonthViewTop = KOPrefs::instance()->decorationsAtMonthViewTop().toSet();
    mDecorationsAtAgendaViewTop = viewPrefs->decorationsAtAgendaViewTop().toSet();
    mDecorationsAtAgendaViewBottom = viewPrefs->decorationsAtAgendaViewBottom().toSet();
#else
    const auto monthViewTop = KOPrefs::instance()->decorationsAtMonthViewTop();
    mDecorationsAtMonthViewTop = QSet<QString>(monthViewTop.begin(), monthViewTop.end());
    const auto agendaViewTop = viewPrefs->decorationsAtAgendaViewTop();
    mDecorationsAtAgendaViewTop = QSet<QString>(agendaViewTop.begin(), agendaViewTop.end());
    const auto agendaViewBottom = viewPrefs->decorationsAtAgendaViewBottom();
    mDecorationsAtAgendaViewBottom = QSet<QString>(agendaViewBottom.begin(), agendaViewBottom.end());
#endif
}

void KOPrefsDialogPlugins::save()
{
    QStringList selectedPlugins;

    for (int i = 0; i < mTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *serviceTypeGroup = mTreeWidget->topLevelItem(i);
        for (int j = 0; j < serviceTypeGroup->childCount(); ++j) {
            PluginItem *item = static_cast<PluginItem *>(serviceTypeGroup->child(j));
            if (item->checkState(0) == Qt::Checked) {
                selectedPlugins.append(item->service()->desktopEntryName());
            }
        }
    }
    EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();
    viewPrefs->setSelectedPlugins(selectedPlugins);

    KOPrefs::instance()->setDecorationsAtMonthViewTop(mDecorationsAtMonthViewTop.values());
    viewPrefs->setDecorationsAtAgendaViewTop(mDecorationsAtAgendaViewTop.values());
    viewPrefs->setDecorationsAtAgendaViewBottom(mDecorationsAtAgendaViewBottom.values());
}

void KOPrefsDialogPlugins::configure()
{
    if (mTreeWidget->selectedItems().count() != 1) {
        return;
    }

    PluginItem *item = static_cast<PluginItem *>(mTreeWidget->selectedItems().last());
    if (!item) {
        return;
    }

    CalendarSupport::Plugin *plugin = KOCore::self()->loadPlugin(item->service());

    if (plugin) {
        plugin->configure(this);
        delete plugin;

        slotConfigChanged();
    } else {
        KMessageBox::sorry(this,
                           i18nc("@info", "Unable to configure this plugin"),
                           QStringLiteral("PluginConfigUnable"));
    }
}

void KOPrefsDialogPlugins::positioningChanged()
{
    if (mTreeWidget->selectedItems().count() != 1) {
        return;
    }

    PluginItem *item = dynamic_cast<PluginItem *>(mTreeWidget->selectedItems().last());
    if (!item) {
        return;
    }

    const QString decoration = item->service()->desktopEntryName();

    /*if ( mPositionMonthTop->checkState() == Qt::Checked ) {
      if ( !mDecorationsAtMonthViewTop.contains( decoration ) ) {
        mDecorationsAtMonthViewTop.insert( decoration );
      }
    } else {
      mDecorationsAtMonthViewTop.remove( decoration );
    }*/

    if (mPositionAgendaTop->isChecked()) {
        if (!mDecorationsAtAgendaViewTop.contains(decoration)) {
            mDecorationsAtAgendaViewTop.insert(decoration);
        }
    } else {
        mDecorationsAtAgendaViewTop.remove(decoration);
    }

    if (mPositionAgendaBottom->isChecked()) {
        if (!mDecorationsAtAgendaViewBottom.contains(decoration)) {
            mDecorationsAtAgendaViewBottom.insert(decoration);
        }
    } else {
        mDecorationsAtAgendaViewBottom.remove(decoration);
    }

    slotConfigChanged();
}

void KOPrefsDialogPlugins::selectionChanged()
{
    mPositioningGroupBox->hide();
    //mPositionMonthTop->setChecked( false );
    mPositionAgendaTop->setChecked(false);
    mPositionAgendaBottom->setChecked(false);

    if (mTreeWidget->selectedItems().count() != 1) {
        mConfigureButton->setEnabled(false);
        mDescription->setText(QString());
        return;
    }

    PluginItem *item = dynamic_cast<PluginItem *>(mTreeWidget->selectedItems().last());
    if (!item) {
        mConfigureButton->setEnabled(false);
        mConfigureButton->hide();
        mDescription->setText(QString());
        return;
    }

    QVariant variant = item->service()->property(QStringLiteral("X-KDE-KOrganizer-HasSettings"));

    bool hasSettings = false;
    if (variant.isValid()) {
        hasSettings = variant.toBool();
    }

    mDescription->setText(item->service()->comment());
    if (!hasSettings) {
        mConfigureButton->hide();
    } else {
        mConfigureButton->show();
        mConfigureButton->setEnabled(item->checkState(0) == Qt::Checked);
    }

    if (item->service()->hasServiceType(
            EventViews::CalendarDecoration::Decoration::serviceType())) {
        bool hasPosition = false;
        QString decoration = item->service()->desktopEntryName();
        /*if ( mDecorationsAtMonthViewTop.contains( decoration ) ) {
          mPositionMonthTop->setChecked( true );
          hasPosition = true;
        }*/
        if (mDecorationsAtAgendaViewTop.contains(decoration)) {
            mPositionAgendaTop->setChecked(true);
            hasPosition = true;
        }
        if (mDecorationsAtAgendaViewBottom.contains(decoration)) {
            mPositionAgendaBottom->setChecked(true);
            hasPosition = true;
        }

        if (!hasPosition) {
            // no position has been selected, so default to Agenda Top
            mDecorationsAtAgendaViewTop << decoration;
            mPositionAgendaTop->setChecked(true);
        }

        mPositioningGroupBox->setEnabled(item->checkState(0) == Qt::Checked);
        mPositioningGroupBox->show();
    }

    slotConfigChanged();
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigplugins(QWidget *parent, const char *)
{
    return new KOPrefsDialogPlugins(parent);
}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
Q_DECL_EXPORT KCModule *create_korgdesignerfields(QWidget *parent, const char *)
{
    return new KOPrefsDesignerFields(parent);
}
}

KOPrefsDesignerFields::KOPrefsDesignerFields(QWidget *parent)
    : KCMDesignerFields(parent)
{
}

QString KOPrefsDesignerFields::localUiDir()
{
    const QString dir
        = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + uiPath();
    return dir;
}

QString KOPrefsDesignerFields::uiPath()
{
    return QStringLiteral("/korganizer/designer/event/");
}

void KOPrefsDesignerFields::writeActivePages(const QStringList &activePages)
{
    CalendarSupport::KCalPrefs::instance()->setActiveDesignerFields(activePages);
    CalendarSupport::KCalPrefs::instance()->save();
}

QStringList KOPrefsDesignerFields::readActivePages()
{
    return CalendarSupport::KCalPrefs::instance()->activeDesignerFields();
}

QString KOPrefsDesignerFields::applicationName()
{
    return QStringLiteral("KORGANIZER");
}

#ifdef WITH_KUSERFEEDBACK

extern "C"
{
Q_DECL_EXPORT KCModule *create_korguserfeeback(QWidget *parent, const char *)
{
    return new KOPrefsUserFeedBack(parent);
}
}

KOPrefsUserFeedBack::KOPrefsUserFeedBack(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    KAboutData *about = new KAboutData(QStringLiteral("KCMUserFeedBack"),
                                       i18n("KCMUserFeedBack"),
                                       QString(),
                                       i18n("KOrganizer Configure User FeedBack"),
                                       KAboutLicense::LGPL,
                                       i18n("(c) 2020 Laurent Montel"));
    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));
    setAboutData(about);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    mUserFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(this);
    connect(mUserFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &KOPrefsUserFeedBack::markAsChanged);

    layout->addWidget(mUserFeedbackWidget);
    mUserFeedbackWidget->setFeedbackProvider(UserFeedBackManager::self()->userFeedbackProvider());
}

void KOPrefsUserFeedBack::load()
{
    mUserFeedbackWidget->setFeedbackProvider(UserFeedBackManager::self()->userFeedbackProvider());
}

void KOPrefsUserFeedBack::save()
{
    UserFeedBackManager::self()->userFeedbackProvider()->setTelemetryMode(mUserFeedbackWidget->telemetryMode());
    UserFeedBackManager::self()->userFeedbackProvider()->setSurveyInterval(mUserFeedbackWidget->surveyInterval());
}

#endif
