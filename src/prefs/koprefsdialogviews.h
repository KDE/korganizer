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


#ifndef KOPREFSDIALOGVIEWS_H
#define KOPREFSDIALOGVIEWS_H

#include <KCModule>

class KItemIconCheckCombo;
class QCheckBox;
class QSpinBox;

class KOPrefsDialogViews : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogViews(QWidget *parent);

    void load() override;

    void save() override;
private:
    void slotConfigChanged();
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

#endif // KOPREFSDIALOGVIEWS_H
