/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KOPREFSDIALOGTIME_H
#define KOPREFSDIALOGTIME_H

#include <KCModule>

class QSpinBox;
class KComboBox;
class QCheckBox;
class KUrlRequester;
class KTimeComboBox;
class QComboBox;
class QTimeEdit;
namespace KPIM {
class KCheckComboBox;
}

class KOPrefsDialogTime : public KCModule
{
public:
    KOPrefsDialogTime(QWidget *parent);

    void load() override;

    void save() override;
protected:
    void setCombo(KComboBox *combo, const QString &text, const QStringList *tags = nullptr);

private:
    void slotConfigChanged();
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

#endif // KOPREFSDIALOGTIME_H
