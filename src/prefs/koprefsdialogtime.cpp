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
#include "koprefsdialogtime.h"
#include "koglobals.h"
#include <QSpinBox>
#include <KUrlRequester>
#include <KLocalizedString>
#include <KCheckComboBox>
#include <KTimeComboBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <KComboBox>
#include <CalendarSupport/KCalPrefs>
#include <KHolidays/HolidayRegion>
#include "koprefs.h"
extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigtime(QWidget *parent, const char *)
{
    return new KOPrefsDialogTime(parent);
}
}

KOPrefsDialogTime::KOPrefsDialogTime(QWidget *parent)
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
    std::vector<std::pair<QString, QString> > regionsMap;
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
    std::sort(regionsMap.begin(), regionsMap.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.first < rhs.first;
    });

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
    connect(mExcludeHolidaysCheckbox, &QCheckBox::clicked, this, &KOPrefsDialogTime::slotConfigChanged);
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
    connect(mDefaultAudioFileRemindersCheckBox, &QCheckBox::clicked, this, &KOPrefsDialogTime::slotConfigChanged);
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
    connect(mDefaultEventRemindersCheckBox, &QCheckBox::clicked, this, &KOPrefsDialogTime::slotConfigChanged);
    remindersLayout->addWidget(mDefaultEventRemindersCheckBox, 2, 0);
    mDefaultTodoRemindersCheckBox = new QCheckBox(CalendarSupport::KCalPrefs::instance()->defaultTodoRemindersItem()->label(), this);
    connect(mDefaultTodoRemindersCheckBox, &QCheckBox::clicked, this, &KOPrefsDialogTime::slotConfigChanged);
    remindersLayout->addWidget(mDefaultTodoRemindersCheckBox, 3, 0);

    defaultLayout->setRowStretch(3, 1);
    load();
}

void KOPrefsDialogTime::load()
{
    mFirstDayCombo->setCurrentIndex(KOPrefs::instance()->weekStartDay());
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

void KOPrefsDialogTime::save()
{
    KOPrefs::instance()->setWeekStartDay(mFirstDayCombo->currentIndex());

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

void KOPrefsDialogTime::slotConfigChanged()
{
    Q_EMIT markAsChanged();
}
