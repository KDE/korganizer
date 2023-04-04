/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "koprefsdialogtime.h"
#include "koglobals.h"
#include "koprefs.h"
#include <CalendarSupport/KCalPrefs>
#include <KComboBox>
#include <KHolidays/HolidayRegion>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KTimeComboBox>
#include <KUrlRequester>
#include <Libkdepim/KCheckComboBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimeEdit>
#include <QVBoxLayout>

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogTime, "korganizer_configtime.json")

KOPrefsDialogTime::KOPrefsDialogTime(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Korganizer::KPrefsModule(KOPrefs::instance(), parent, data, args)
{
    auto layout = new QVBoxLayout(widget());
    auto tabWidget = new QTabWidget(widget());
    layout->addWidget(tabWidget);

    auto regionalPage = new QFrame(widget());
    tabWidget->addTab(regionalPage, QIcon::fromTheme(QStringLiteral("flag")), i18nc("@title:tab", "Regional"));

    auto regionalLayout = new QGridLayout(regionalPage);

    auto datetimeGroupBox = new QGroupBox(i18nc("@title:group", "General Time and Date"), regionalPage);
    regionalLayout->addWidget(datetimeGroupBox, 0, 0);

    auto datetimeLayout = new QGridLayout(datetimeGroupBox);

    Korganizer::KPrefsWidTime *dayBegins = addWidTime(KOPrefs::instance()->dayBeginsItem(), regionalPage);
    datetimeLayout->addWidget(dayBegins->label(), 1, 0);
    datetimeLayout->addWidget(dayBegins->timeEdit(), 1, 1);

    auto holidaysGroupBox = new QGroupBox(i18nc("@title:group", "Holidays"), regionalPage);
    regionalLayout->addWidget(holidaysGroupBox, 1, 0);

    auto holidaysLayout = new QGridLayout(holidaysGroupBox);

    // holiday region selection
    auto holidayRegBox = new QWidget(regionalPage);
    auto holidayRegBoxHBoxLayout = new QHBoxLayout(holidayRegBox);
    holidayRegBoxHBoxLayout->setContentsMargins({});
    holidaysLayout->addWidget(holidayRegBox, 1, 0, 1, 2);

    auto holidayLabel = new QLabel(i18nc("@label", "Use holiday region:"), holidayRegBox);
    holidayLabel->setToolTip(KOPrefs::instance()->holidaysItem()->toolTip());
    holidayLabel->setWhatsThis(KOPrefs::instance()->holidaysItem()->whatsThis());

    mHolidayCheckCombo = new KPIM::KCheckComboBox(holidayRegBox);
    holidayRegBoxHBoxLayout->addWidget(mHolidayCheckCombo);
    connect(mHolidayCheckCombo, &KPIM::KCheckComboBox::checkedItemsChanged, this, &KOPrefsDialogTime::slotWidChanged);

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
            label = i18nc("@item:inlistbox Holiday region, region language", "%1 (%2)", name, languageName);
        }
        regionsMap.emplace_back(label, regionCode);
    }
    std::sort(regionsMap.begin(), regionsMap.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.first < rhs.first;
    });

    mHolidayCheckCombo->clear();
    mHolidayCheckCombo->setDefaultText(i18nc("@item:inlistbox", "Select Holiday Regions"));
    for (const auto &entry : regionsMap) {
        mHolidayCheckCombo->addItem(entry.first, entry.second);
    }

    // QString regionStr = KHolidays::HolidayRegion::defaultRegionCode();
    const auto holidays = KOGlobals::self()->holidays();
    for (KHolidays::HolidayRegion *region : holidays) {
        const QString regionStr = region->regionCode();
        mHolidayCheckCombo->setItemCheckState(mHolidayCheckCombo->findData(regionStr), Qt::Checked);
    }

    auto workingHoursGroupBox = new QGroupBox(i18nc("@title:group", "Working Period"), regionalPage);
    regionalLayout->addWidget(workingHoursGroupBox, 2, 0);

    QBoxLayout *workingHoursLayout = new QVBoxLayout(workingHoursGroupBox);

    QBoxLayout *workDaysLayout = new QHBoxLayout;
    workingHoursLayout->addLayout(workDaysLayout);

    // Respect start of week setting
    int weekStart = QLocale().firstDayOfWeek();
    for (int i = 0; i < 7; ++i) {
        QString weekDayName = QLocale().dayName((i + weekStart + 6) % 7 + 1, QLocale::ShortFormat);
        int index = (i + weekStart + 6) % 7;
        mWorkDays[index] = new QCheckBox(weekDayName);
        mWorkDays[index]->setWhatsThis(i18nc("@info:whatsthis",
                                             "Check this box to make KOrganizer mark the "
                                             "working hours for this day of the week. "
                                             "If this is a work day for you, check "
                                             "this box, or the working hours will not be "
                                             "marked with color."));

        connect(mWorkDays[index], &QCheckBox::stateChanged, this, &Korganizer::KPrefsModule::slotWidChanged);

        workDaysLayout->addWidget(mWorkDays[index]);
    }

    Korganizer::KPrefsWidCombo *firstDayCombo = addWidCombo(KOPrefs::instance()->weekStartDayItem(), workingHoursGroupBox);
    auto firstDayLayout = new QHBoxLayout;
    workingHoursLayout->addLayout(firstDayLayout);
    QStringList days;
    days << i18nc("@item:inlistbox", "Monday") << i18nc("@item:inlistbox", "Tuesday") << i18nc("@item:inlistbox", "Wednesday")
         << i18nc("@item:inlistbox", "Thursday") << i18nc("@item:inlistbox", "Friday") << i18nc("@item:inlistbox", "Saturday")
         << i18nc("@item:inlistbox", "Sunday");
    firstDayCombo->comboBox()->addItems(days);

    firstDayLayout->addWidget(firstDayCombo->label());
    firstDayLayout->addWidget(firstDayCombo->comboBox());

    Korganizer::KPrefsWidTime *workStart = addWidTime(KOPrefs::instance()->workingHoursStartItem());

    auto workStartLayout = new QHBoxLayout;
    workingHoursLayout->addLayout(workStartLayout);

    workStartLayout->addWidget(workStart->label());
    workStartLayout->addWidget(workStart->timeEdit());

    Korganizer::KPrefsWidTime *workEnd = addWidTime(KOPrefs::instance()->workingHoursEndItem());

    auto workEndLayout = new QHBoxLayout;
    workingHoursLayout->addLayout(workEndLayout);

    workEndLayout->addWidget(workEnd->label());
    workEndLayout->addWidget(workEnd->timeEdit());

    Korganizer::KPrefsWidBool *excludeHolidays = addWidBool(KOPrefs::instance()->excludeHolidaysItem());

    workingHoursLayout->addWidget(excludeHolidays->checkBox());

    regionalLayout->setRowStretch(4, 1);

    auto defaultPage = new QFrame(widget());
    tabWidget->addTab(defaultPage, QIcon::fromTheme(QStringLiteral("draw-eraser")), i18nc("@title:tab", "Default Values"));
    auto defaultLayout = new QGridLayout(defaultPage);

    auto timesGroupBox = new QGroupBox(i18nc("@title:group", "Appointments"), defaultPage);
    defaultLayout->addWidget(timesGroupBox, 0, 0);

    auto timesLayout = new QGridLayout(timesGroupBox);

    Korganizer::KPrefsWidTime *defaultTime = addWidTime(CalendarSupport::KCalPrefs::instance()->startTimeItem(), defaultPage);
    timesLayout->addWidget(defaultTime->label(), 0, 0);
    timesLayout->addWidget(defaultTime->timeEdit(), 0, 1);

    Korganizer::KPrefsWidDuration *defaultDuration =
        addWidDuration(CalendarSupport::KCalPrefs::instance()->defaultDurationItem(), QStringLiteral("hh:mm"), defaultPage);

    timesLayout->addWidget(defaultDuration->label(), 1, 0);
    timesLayout->addWidget(defaultDuration->timeEdit(), 1, 1);

    auto remindersGroupBox = new QGroupBox(i18nc("@title:group", "Reminders"), defaultPage);
    defaultLayout->addWidget(remindersGroupBox, 1, 0);

    auto remindersLayout = new QGridLayout(remindersGroupBox);

    auto reminderLabel = new QLabel(i18nc("@label", "Default reminder time:"), defaultPage);
    remindersLayout->addWidget(reminderLabel, 0, 0);
    reminderLabel->setWhatsThis(CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->whatsThis());
    mReminderTimeSpin = new QSpinBox(defaultPage);
    mReminderTimeSpin->setWhatsThis(CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->whatsThis());
    mReminderTimeSpin->setToolTip(CalendarSupport::KCalPrefs::instance()->reminderTimeItem()->toolTip());
    connect(mReminderTimeSpin, &QSpinBox::valueChanged, this, &KOPrefsDialogTime::slotWidChanged);
    remindersLayout->addWidget(mReminderTimeSpin, 0, 1);

    mReminderUnitsCombo = new KComboBox(defaultPage);
    mReminderUnitsCombo->setToolTip(CalendarSupport::KCalPrefs::instance()->reminderTimeUnitsItem()->toolTip());
    mReminderUnitsCombo->setWhatsThis(CalendarSupport::KCalPrefs::instance()->reminderTimeUnitsItem()->whatsThis());
    connect(mReminderUnitsCombo, &KComboBox::activated, this, &KOPrefsDialogTime::slotWidChanged);
    mReminderUnitsCombo->addItem(i18nc("@item:inlistbox reminder units in minutes", "minute(s)"));
    mReminderUnitsCombo->addItem(i18nc("@item:inlistbox reminder time units in hours", "hour(s)"));
    mReminderUnitsCombo->addItem(i18nc("@item:inlistbox reminder time units in days", "day(s)"));
    remindersLayout->addWidget(mReminderUnitsCombo, 0, 2);

    QCheckBox *cb = addWidBool(CalendarSupport::KCalPrefs::instance()->defaultAudioFileRemindersItem())->checkBox();

    if (CalendarSupport::KCalPrefs::instance()->audioFilePathItem()->value().isEmpty()) {
        const QString defAudioFile =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sound/") + QLatin1String("KDE-Sys-Warning.ogg"));
        CalendarSupport::KCalPrefs::instance()->audioFilePathItem()->setValue(defAudioFile);
    }
    QString filter = i18n(
        "*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra|"
        "Audio Files (*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra)");
    KUrlRequester *rq = addWidPath(CalendarSupport::KCalPrefs::instance()->audioFilePathItem(), nullptr, filter)->urlRequester();
    rq->setEnabled(cb->isChecked());

    connect(cb, &QCheckBox::toggled, rq, &KUrlRequester::setEnabled);

    auto audioFileRemindersBox = new QVBoxLayout;
    audioFileRemindersBox->addWidget(cb);
    audioFileRemindersBox->addWidget(rq);

    remindersLayout->addLayout(audioFileRemindersBox, 1, 0);
    remindersLayout->addWidget(addWidBool(CalendarSupport::KCalPrefs::instance()->defaultEventRemindersItem())->checkBox(), 2, 0);
    remindersLayout->addWidget(addWidBool(CalendarSupport::KCalPrefs::instance()->defaultTodoRemindersItem())->checkBox(), 3, 0);

    defaultLayout->setRowStretch(3, 1);
    load();
}

void KOPrefsDialogTime::usrReadConfig()
{
    mReminderTimeSpin->setValue(CalendarSupport::KCalPrefs::instance()->mReminderTime);
    mReminderUnitsCombo->setCurrentIndex(CalendarSupport::KCalPrefs::instance()->mReminderTimeUnits);
    for (int i = 0; i < 7; ++i) {
        mWorkDays[i]->setChecked((1 << i) & (KOPrefs::instance()->mWorkWeekMask));
    }
}

void KOPrefsDialogTime::usrWriteConfig()
{
    QStringList HolidayRegions;
    const auto checkedItems = mHolidayCheckCombo->checkedItems();
    for (const QString &str : checkedItems) {
        int index = mHolidayCheckCombo->findText(str);
        if (index >= 0) {
            HolidayRegions.append(mHolidayCheckCombo->itemData(index).toString());
        }
    }
    KOPrefs::instance()->mHolidays = HolidayRegions;

    CalendarSupport::KCalPrefs::instance()->mReminderTime = mReminderTimeSpin->value();
    CalendarSupport::KCalPrefs::instance()->mReminderTimeUnits = mReminderUnitsCombo->currentIndex();

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

void KOPrefsDialogTime::setCombo(KComboBox *combo, const QString &text, const QStringList *tags)
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

#include "koprefsdialogtime.moc"
