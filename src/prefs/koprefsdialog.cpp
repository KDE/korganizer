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
#include <QFontDialog>
#include <QCheckBox>

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
        connect(mDayBegin, qOverload<int>(&KTimeComboBox::currentIndexChanged), this, &KOPrefsDialogTime::slotConfigChanged);

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

class KOPrefsDialogViews : public KCModule
{
public:
    KOPrefsDialogViews(QWidget *parent)
        : KCModule(parent)
        , mMonthIconComboBox(new KItemIconCheckCombo(KItemIconCheckCombo::MonthType, this))
        , mAgendaIconComboBox(new KItemIconCheckCombo(KItemIconCheckCombo::AgendaType, this))
    {
        QBoxLayout *topTopLayout = new QVBoxLayout(this);
        QTabWidget *tabWidget = new QTabWidget(this);
        topTopLayout->addWidget(tabWidget);

        connect(mMonthIconComboBox, &KPIM::KCheckComboBox::checkedItemsChanged,
                this, &KOPrefsDialogViews::slotConfigChanged);
        connect(mAgendaIconComboBox, &KPIM::KCheckComboBox::checkedItemsChanged,
                this, &KOPrefsDialogViews::slotConfigChanged);

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

        mNextDay = new QSpinBox(this);
        mNextDay->setSuffix(
                    i18nc("@label suffix in the N days spin box", " days"));

        nextDaysLayout->addWidget(new QLabel(KOPrefs::instance()->nextXDaysItem()->label(), this));
        nextDaysLayout->addWidget(mNextDay);
        nextDaysLayout->addStretch(1);


        mEnableToolTipsCheckBox = new QCheckBox(KOPrefs::instance()->enableToolTipsItem()->label(), this);
        mTodosUseCategoryColorsCheckBox = new QCheckBox(KOPrefs::instance()->todosUseCategoryColorsItem()->label(), this);
        gdisplayLayout->addWidget(mEnableToolTipsCheckBox);
        gdisplayLayout->addWidget(mTodosUseCategoryColorsCheckBox);
        gdisplayBox->setLayout(gdisplayLayout);
        generalLayout->addWidget(gdisplayBox);

        // GroupBox: Views->General->Date Navigator
        QVBoxLayout *datenavLayout = new QVBoxLayout;
        QGroupBox *datenavBox = new QGroupBox(i18nc("@title:group", "Date Navigator"));
        mDailyRecurCheckbox = new QCheckBox(KOPrefs::instance()->dailyRecurItem()->label(), this);
        mWeeklyRecurCheckbox = new QCheckBox(KOPrefs::instance()->weeklyRecurItem()->label(), this);
        mHighlightTodosCheckbox = new QCheckBox(KOPrefs::instance()->highlightTodosItem()->label(), this);
        mHighlightJournalsCheckbox = new QCheckBox(KOPrefs::instance()->highlightJournalsItem()->label(), this);
        mWeekNumbersShowWorkCheckbox = new QCheckBox(KOPrefs::instance()->weekNumbersShowWorkItem()->label(), this);
        datenavLayout->addWidget(mDailyRecurCheckbox);
        datenavLayout->addWidget(mWeeklyRecurCheckbox);
        datenavLayout->addWidget(mHighlightTodosCheckbox);
        datenavLayout->addWidget(mHighlightJournalsCheckbox);
        datenavLayout->addWidget(mWeekNumbersShowWorkCheckbox);
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

        mHourSize = new QSpinBox(this);
        mHourSize->setSuffix(
            i18nc("@label suffix in the hour size spin box", " pixels"));

        hourSizeLayout->addWidget(new QLabel(KOPrefs::instance()->hourSizeItem()->label(), this));
        hourSizeLayout->addWidget(mHourSize);
        hourSizeLayout->addStretch(1);

        mEnableAgendaItemIconsCheckbox = new QCheckBox(KOPrefs::instance()->enableAgendaItemIconsItem()->label(), this);
        adisplayLayout->addWidget(mEnableAgendaItemIconsCheckbox);
        mShowTodosAgendaViewCheckbox = new QCheckBox(KOPrefs::instance()->showTodosAgendaViewItem()->label(), this);
        adisplayLayout->addWidget(mShowTodosAgendaViewCheckbox);
        mMarcusBainsEnabledCheckbox = new QCheckBox(KOPrefs::instance()->marcusBainsEnabledItem()->label(), this);
        adisplayLayout->addWidget(mMarcusBainsEnabledCheckbox);
        mMarcusBainsShowSecondsCheckbox = new QCheckBox(KOPrefs::instance()->marcusBainsShowSecondsItem()->label(), this);
        adisplayLayout->addWidget(mMarcusBainsShowSecondsCheckbox);
        connect(mMarcusBainsEnabledCheckbox, &QAbstractButton::toggled,
                mMarcusBainsShowSecondsCheckbox, &QWidget::setEnabled);
        mSelectionStartsEditorCheckbox = new QCheckBox(KOPrefs::instance()->selectionStartsEditorItem()->label(), this);
        adisplayLayout->addWidget(mSelectionStartsEditorCheckbox);

        mAgendaIconComboBox->setCheckedIcons(
            KOPrefs::instance()->eventViewsPreferences()->agendaViewIcons());
        adisplayLayout->addWidget(mAgendaIconComboBox);
        adisplayBox->setLayout(adisplayLayout);
        agendaLayout->addWidget(adisplayBox);

        // GroupBox: Views->Agenda View->Color Usage
        //FIXME
//        agendaLayout->addWidget(
//            addWidRadios(KOPrefs::instance()->agendaViewColorsItem())->groupBox());

        mColorBusyDaysEnabledCheckBox = new QCheckBox(KOPrefs::instance()->colorBusyDaysEnabledItem()->label(), this);
        agendaLayout->addWidget(mColorBusyDaysEnabledCheckBox);

        // GroupBox: Views->Agenda View->Multiple Calendars
        //FIXME
//        agendaLayout->addWidget(
//            addWidRadios(KOPrefs::instance()->agendaViewCalendarDisplayItem())->groupBox());

        agendaLayout->addStretch(1);

        // Tab: Views->Month View
        QFrame *monthFrame = new QFrame(this);
        tabWidget->addTab(monthFrame, QIcon::fromTheme(QStringLiteral("view-calendar-month")),
                          i18nc("@title:tab", "Month View"));

        QBoxLayout *monthLayout = new QVBoxLayout(monthFrame);

        // GroupBox: Views->Month View->Display Options
        QVBoxLayout *mdisplayLayout = new QVBoxLayout;
        QGroupBox *mdisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));
        mShowTimeInMonthViewCheckBox = new QCheckBox(KOPrefs::instance()->showTimeInMonthViewItem()->label(), this);
        mdisplayLayout->addWidget(mShowTimeInMonthViewCheckBox);
        mEnableMonthItemIconsCheckBox = new QCheckBox(KOPrefs::instance()->enableMonthItemIconsItem()->label(), this);
        mdisplayLayout->addWidget(mEnableMonthItemIconsCheckBox);
        mShowTodosMonthViewCheckBox = new QCheckBox(KOPrefs::instance()->showTodosMonthViewItem()->label(), this);
        mdisplayLayout->addWidget(mShowTodosMonthViewCheckBox);
        mShowJournalsMonthViewCheckBox = new QCheckBox(KOPrefs::instance()->showJournalsMonthViewItem()->label(), this);
        mdisplayLayout->addWidget(mShowJournalsMonthViewCheckBox);
        mdisplayBox->setLayout(mdisplayLayout);

        mMonthIconComboBox->setCheckedIcons(
            KOPrefs::instance()->eventViewsPreferences()->monthViewIcons());
        mdisplayLayout->addWidget(mMonthIconComboBox);

        monthLayout->addWidget(mdisplayBox);

        mColorMonthBusyDaysEnabledCheckBox = new QCheckBox(KOPrefs::instance()->colorMonthBusyDaysEnabledItem()->label(), this);
        monthLayout->addWidget(mColorMonthBusyDaysEnabledCheckBox);

        // GroupBox: Views->Month View->Color Usage
        //FIXME
//        monthLayout->addWidget(
//            addWidRadios(KOPrefs::instance()->monthViewColorsItem())->groupBox());
        monthLayout->addStretch(1);

        // Tab: Views->Todo View
        QFrame *todoFrame = new QFrame(this);
        tabWidget->addTab(todoFrame, QIcon::fromTheme(QStringLiteral("view-calendar-tasks")),
                          i18nc("@title:tab", "Todo View"));

        QBoxLayout *todoLayout = new QVBoxLayout(todoFrame);

        // GroupBox: Views->Todo View->Display Options
        QVBoxLayout *tdisplayLayout = new QVBoxLayout;
        QGroupBox *tdisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));
        mSortCompletedTodosSeparatelyCheckBox = new QCheckBox(KOPrefs::instance()->sortCompletedTodosSeparatelyItem()->label(), this);
        tdisplayLayout->addWidget(mSortCompletedTodosSeparatelyCheckBox);
        tdisplayBox->setLayout(tdisplayLayout);
        todoLayout->addWidget(tdisplayBox);

        // GroupBox: Views->Todo View->Other
        QVBoxLayout *otherLayout = new QVBoxLayout;
        QGroupBox *otherBox = new QGroupBox(i18nc("@title:group", "Other Options"));
        mRecordTodosInJournalsCheckBox = new QCheckBox(KOPrefs::instance()->recordTodosInJournalsItem()->label(), this);
        otherLayout->addWidget(mRecordTodosInJournalsCheckBox);
        otherBox->setLayout(otherLayout);
        todoLayout->addWidget(otherBox);
        todoLayout->addStretch(1);

        load();
    }

    void load() override
    {
        KOPrefs::instance()->eventViewsPreferences()->setAgendaViewIcons(
            mAgendaIconComboBox->checkedIcons());
        KOPrefs::instance()->eventViewsPreferences()->setMonthViewIcons(
            mMonthIconComboBox->checkedIcons());

        mEnableToolTipsCheckBox->setChecked(KOPrefs::instance()->enableToolTips());
        mTodosUseCategoryColorsCheckBox->setChecked(KOPrefs::instance()->todosUseCategoryColors());
        mRecordTodosInJournalsCheckBox->setChecked(KOPrefs::instance()->recordTodosInJournals());
        mSortCompletedTodosSeparatelyCheckBox->setChecked(KOPrefs::instance()->sortCompletedTodosSeparately());
        mColorMonthBusyDaysEnabledCheckBox->setChecked(KOPrefs::instance()->colorMonthBusyDaysEnabled());
        mDailyRecurCheckbox->setChecked(KOPrefs::instance()->dailyRecur());
        mWeeklyRecurCheckbox->setChecked(KOPrefs::instance()->weeklyRecur());
        mHighlightTodosCheckbox->setChecked(KOPrefs::instance()->highlightTodos());
        mHighlightJournalsCheckbox->setChecked(KOPrefs::instance()->highlightJournals());
        mWeekNumbersShowWorkCheckbox->setChecked(KOPrefs::instance()->weekNumbersShowWork());
        mShowTimeInMonthViewCheckBox->setChecked(KOPrefs::instance()->showTimeInMonthView());
        mEnableMonthItemIconsCheckBox->setChecked(KOPrefs::instance()->enableMonthItemIcons());
        mShowTodosMonthViewCheckBox->setChecked(KOPrefs::instance()->showTodosMonthView());
        mShowJournalsMonthViewCheckBox->setChecked(KOPrefs::instance()->showJournalsMonthView());
        mHourSize->setValue(KOPrefs::instance()->hourSize());
        mEnableAgendaItemIconsCheckbox->setChecked(KOPrefs::instance()->enableAgendaItemIcons());
        mShowTodosAgendaViewCheckbox->setChecked(KOPrefs::instance()->showTodosAgendaView());
        mMarcusBainsEnabledCheckbox->setChecked(KOPrefs::instance()->marcusBainsEnabled());
        mMarcusBainsShowSecondsCheckbox->setChecked(KOPrefs::instance()->marcusBainsShowSeconds());
        mSelectionStartsEditorCheckbox->setChecked(KOPrefs::instance()->selectionStartsEditor());
        mColorBusyDaysEnabledCheckBox->setChecked(KOPrefs::instance()->colorBusyDaysEnabled());
        mNextDay->setValue(KOPrefs::instance()->nextXDays());
    }

    void save() override
    {
        KOPrefs::instance()->setEnableToolTips(mEnableToolTipsCheckBox->isChecked());
        KOPrefs::instance()->setTodosUseCategoryColors(mTodosUseCategoryColorsCheckBox->isChecked());
        KOPrefs::instance()->setRecordTodosInJournals(mRecordTodosInJournalsCheckBox->isChecked());
        KOPrefs::instance()->setSortCompletedTodosSeparately(mSortCompletedTodosSeparatelyCheckBox->isChecked());
        KOPrefs::instance()->setColorMonthBusyDaysEnabled(mColorMonthBusyDaysEnabledCheckBox->isChecked());
        KOPrefs::instance()->setDailyRecur(mDailyRecurCheckbox->isChecked());
        KOPrefs::instance()->setWeeklyRecur(mWeeklyRecurCheckbox->isChecked());
        KOPrefs::instance()->setHighlightTodos(mHighlightTodosCheckbox->isChecked());
        KOPrefs::instance()->setHighlightJournals(mHighlightJournalsCheckbox->isChecked());
        KOPrefs::instance()->setWeekNumbersShowWork(mWeekNumbersShowWorkCheckbox->isChecked());
        KOPrefs::instance()->setShowTimeInMonthView(mShowTimeInMonthViewCheckBox->isChecked());
        KOPrefs::instance()->setEnableMonthItemIcons(mEnableMonthItemIconsCheckBox->isChecked());
        KOPrefs::instance()->setShowTodosMonthView(mShowTodosMonthViewCheckBox->isChecked());
        KOPrefs::instance()->setShowJournalsMonthView(mShowJournalsMonthViewCheckBox->isChecked());
        KOPrefs::instance()->setHourSize(mHourSize->value());
        KOPrefs::instance()->setEnableAgendaItemIcons(mEnableAgendaItemIconsCheckbox->isChecked());
        KOPrefs::instance()->setShowTodosAgendaView(mShowTodosAgendaViewCheckbox->isChecked());
        KOPrefs::instance()->setMarcusBainsEnabled(mMarcusBainsEnabledCheckbox->isChecked());
        KOPrefs::instance()->setMarcusBainsShowSeconds(mMarcusBainsShowSecondsCheckbox->isChecked());
        KOPrefs::instance()->setSelectionStartsEditor(mSelectionStartsEditorCheckbox->isChecked());
        KOPrefs::instance()->setColorBusyDaysEnabled(mColorBusyDaysEnabledCheckBox->isChecked());
        KOPrefs::instance()->setNextXDays(mNextDay->value());

    }
private:
    void slotConfigChanged() {
        Q_EMIT markAsChanged();
    }
private:
    KItemIconCheckCombo *mMonthIconComboBox = nullptr;
    KItemIconCheckCombo *mAgendaIconComboBox = nullptr;
    QCheckBox *mEnableToolTipsCheckBox = nullptr;
    QCheckBox *mTodosUseCategoryColorsCheckBox = nullptr;
    QCheckBox *mRecordTodosInJournalsCheckBox = nullptr;
    QCheckBox *mSortCompletedTodosSeparatelyCheckBox = nullptr;
    QCheckBox *mColorMonthBusyDaysEnabledCheckBox = nullptr;
    QCheckBox *mDailyRecurCheckbox = nullptr;
    QCheckBox *mWeeklyRecurCheckbox = nullptr;
    QCheckBox *mHighlightTodosCheckbox = nullptr;
    QCheckBox *mHighlightJournalsCheckbox = nullptr;
    QCheckBox *mWeekNumbersShowWorkCheckbox = nullptr;
    QCheckBox *mShowTimeInMonthViewCheckBox = nullptr;
    QCheckBox *mEnableMonthItemIconsCheckBox = nullptr;
    QCheckBox *mShowTodosMonthViewCheckBox = nullptr;
    QCheckBox *mShowJournalsMonthViewCheckBox = nullptr;
    QCheckBox *mEnableAgendaItemIconsCheckbox = nullptr;
    QCheckBox *mShowTodosAgendaViewCheckbox = nullptr;
    QCheckBox *mMarcusBainsEnabledCheckbox = nullptr;
    QCheckBox *mMarcusBainsShowSecondsCheckbox = nullptr;
    QCheckBox *mSelectionStartsEditorCheckbox = nullptr;
    QCheckBox *mColorBusyDaysEnabledCheckBox = nullptr;
    QSpinBox *mNextDay = nullptr;
    QSpinBox *mHourSize = nullptr;
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

FontPreviewButton::FontPreviewButton(const QString &labelStr, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *label = new QLabel(labelStr, this);
    mainLayout->addWidget(label);

    mPreview = new QLabel(parent);
    mPreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mainLayout->addWidget(mPreview);

    QPushButton *button = new QPushButton(i18n("Choose..."), this);
    mainLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &FontPreviewButton::selectFont);
}

void FontPreviewButton::setFont(const QFont &font)
{
    mPreview->setFont(font);
}

QFont FontPreviewButton::font() const
{
    return mPreview->font();
}

void FontPreviewButton::setPreviewText(const QString &str)
{
    mPreview->setText(str);
}

void FontPreviewButton::selectFont()
{
    bool ok;
    QFont myFont = QFontDialog::getFont(&ok, mPreview->font());
    if (ok) {
        mPreview->setFont(myFont);
        Q_EMIT changed();
    }
}

KOPrefsDialogColorsAndFonts::KOPrefsDialogColorsAndFonts(QWidget *parent)
    : KCModule(parent)
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
    mUseSystemColorCheckBox = new QCheckBox(KOPrefs::instance()->useSystemColorItem()->label(), colorFrame);
    QObject::connect(mUseSystemColorCheckBox, &QCheckBox::toggled,
                     this, &KOPrefsDialogColorsAndFonts::useSystemColorToggle);
    colorLayout->addWidget(mUseSystemColorCheckBox, 1, 0, 1, 2);

    // agenda view background color
    mAgendaBgColorButton = new KColorButton(this);
    mButtonsDisable.push_back(mAgendaBgColorButton);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->agendaGridBackgroundColorItem()->label(), this), 2, 0);
    colorLayout->addWidget(mAgendaBgColorButton, 2, 1);

    mViewBgBusyColorButton = new KColorButton(this);
    mButtonsDisable.push_back(mViewBgBusyColorButton);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->viewBgBusyColorItem()->label(), this), 3, 0);
    colorLayout->addWidget(mViewBgBusyColorButton, 3, 1);

    // working hours color
    mAgendaGridWorkHoursBackgroundColorButton = new KColorButton(this);
    mButtonsDisable.push_back(mAgendaGridWorkHoursBackgroundColorButton);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->workingHoursColorItem()->label(), this), 4, 0);
    colorLayout->addWidget(mAgendaGridWorkHoursBackgroundColorButton, 4, 1);

    // agenda view Marcus Bains line color
    mAgendaMarcusBainsLineLineColorButton = new KColorButton(this);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->agendaMarcusBainsLineLineColorItem()->label(), this), 5, 0);
    colorLayout->addWidget(mAgendaMarcusBainsLineLineColorButton, 5, 1);

    // Holiday Color
    mAgendaHolidaysBackgroundColorButton = new KColorButton(this);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->agendaHolidaysBackgroundColorItem()->label(), this), 6, 0);
    colorLayout->addWidget(mAgendaHolidaysBackgroundColorButton, 6, 1);

    // Todo due today color
    mTodoDueTodayColorButton = new KColorButton(this);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->todoDueTodayColorItem()->label(), this), 7, 0);
    colorLayout->addWidget(mTodoDueTodayColorButton, 7, 1);

    // Todo overdue color
    mTodoOverdueColorButton = new KColorButton(this);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->todoOverdueColorItem()->label(), this), 8, 0);
    colorLayout->addWidget(mTodoOverdueColorButton, 8, 1);

    // categories colors
    QGroupBox *categoryGroup = new QGroupBox(i18nc("@title:group", "Categories"), colorFrame);
    colorLayout->addWidget(categoryGroup, 9, 0, 1, 2);

    QGridLayout *categoryLayout = new QGridLayout;
    categoryGroup->setLayout(categoryLayout);

    mUnsetCategoryColorButton = new KColorButton(this);
    categoryLayout->addWidget(new QLabel(CalendarSupport::KCalPrefs::instance()->unsetCategoryColorItem()->label(), this), 0, 0);
    categoryLayout->addWidget(mUnsetCategoryColorButton, 0, 1);

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

    QVBoxLayout *fontLayout = new QVBoxLayout(fontFrame);

    mTimeBarFontButton = new FontPreviewButton(KOPrefs::instance()->agendaTimeLabelsFontItem()->label(), this);
    mTimeBarFontButton->setPreviewText(QLocale().toString(QTime(12, 34), QLocale::ShortFormat));
    fontLayout->addWidget(mTimeBarFontButton);
    connect(mTimeBarFontButton, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    mMonthViewFont = new FontPreviewButton(KOPrefs::instance()->monthViewFontItem()->label(), this);
    mMonthViewFont->setPreviewText(QLocale().toString(QTime(12, 34), QLocale::ShortFormat) + QLatin1Char(' ')
                                   +i18nc("@label", "Event text"));
    fontLayout->addWidget(mMonthViewFont);
    connect(mMonthViewFont, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    mAgendaViewFont = new FontPreviewButton(KOPrefs::instance()->agendaViewFontItem()->label(), this);
    mAgendaViewFont->setPreviewText(i18nc("@label", "Event text"));
    fontLayout->addWidget(mAgendaViewFont);
    connect(mAgendaViewFont, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    mMarcusBainsFont = new FontPreviewButton(KOPrefs::instance()->agendaMarcusBainsLineFontItem()->label(), this);
    mMarcusBainsFont->setPreviewText(QLocale().toString(QTime(12, 34, 23), QLocale::ShortFormat));
    fontLayout->addWidget(mMarcusBainsFont);
    connect(mMarcusBainsFont, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    fontLayout->addStretch(1);
    load();
}

void KOPrefsDialogColorsAndFonts::save()
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

    KOPrefs::instance()->setUseSystemColor(mUseSystemColorCheckBox->isChecked());

    KOPrefs::instance()->setAgendaGridBackgroundColor(mAgendaBgColorButton->color());
    KOPrefs::instance()->setViewBgBusyColor(mViewBgBusyColorButton->color());
    KOPrefs::instance()->setWorkingHoursColor(mAgendaGridWorkHoursBackgroundColorButton->color());
    KOPrefs::instance()->setAgendaMarcusBainsLineLineColor(mAgendaMarcusBainsLineLineColorButton->color());
    KOPrefs::instance()->setAgendaHolidaysBackgroundColor(mAgendaHolidaysBackgroundColorButton->color());
    KOPrefs::instance()->setTodoDueTodayColor(mTodoDueTodayColorButton->color());
    KOPrefs::instance()->setTodoOverdueColor(mTodoOverdueColorButton->color());
    CalendarSupport::KCalPrefs::instance()->setUnsetCategoryColor(mUnsetCategoryColorButton->color());
    KOPrefs::instance()->setAgendaTimeLabelsFont(mTimeBarFontButton->font());
    KOPrefs::instance()->setMonthViewFont(mMonthViewFont->font());
    KOPrefs::instance()->setAgendaViewFont(mAgendaViewFont->font());
    KOPrefs::instance()->setAgendaMarcusBainsLineFont(mMarcusBainsFont->font());
}

void KOPrefsDialogColorsAndFonts::load()
{
    updateCategories();
    updateResources();
    mUseSystemColorCheckBox->setChecked(KOPrefs::instance()->useSystemColor());
    mAgendaBgColorButton->setColor(KOPrefs::instance()->agendaGridBackgroundColor());
    mViewBgBusyColorButton->setColor(KOPrefs::instance()->viewBgBusyColor());
    mAgendaGridWorkHoursBackgroundColorButton->setColor(KOPrefs::instance()->workingHoursColor());
    mAgendaMarcusBainsLineLineColorButton->setColor(KOPrefs::instance()->agendaMarcusBainsLineLineColor());
    mAgendaHolidaysBackgroundColorButton->setColor(KOPrefs::instance()->agendaHolidaysBackgroundColor());
    mTodoDueTodayColorButton->setColor(KOPrefs::instance()->todoDueTodayColor());
    mTodoOverdueColorButton->setColor(KOPrefs::instance()->todoOverdueColor());
    mUnsetCategoryColorButton->setColor(CalendarSupport::KCalPrefs::instance()->unsetCategoryColor());
    mTimeBarFontButton->setFont(KOPrefs::instance()->agendaTimeLabelsFont());
    mMonthViewFont->setFont(KOPrefs::instance()->monthViewFont());
    mAgendaViewFont->setFont(KOPrefs::instance()->agendaViewFont());
    mMarcusBainsFont->setFont(KOPrefs::instance()->agendaMarcusBainsLineFont());
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
    slotConfigChanged();
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
    slotConfigChanged();
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

void KOPrefsDialogColorsAndFonts::slotConfigChanged()
{
    Q_EMIT markAsChanged();
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

