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
