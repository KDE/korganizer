/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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
    KItemIconCheckCombo *const mMonthIconComboBox;
    KItemIconCheckCombo *const mAgendaIconComboBox;
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
