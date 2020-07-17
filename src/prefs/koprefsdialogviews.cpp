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

#include "koprefsdialogviews.h"

#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include "widgets/kitemiconcheckcombo.h"
#include "kocore.h"
#include "koglobals.h"
#include "koprefs.h"
#include <KLocalizedString>


extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigviews(QWidget *parent, const char *)
{
    return new KOPrefsDialogViews(parent);
}
}

KOPrefsDialogViews::KOPrefsDialogViews(QWidget *parent)
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
    connect(mNextDay, &QSpinBox::valueChanged,
            this, &KOPrefsDialogViews::slotConfigChanged);


    nextDaysLayout->addWidget(new QLabel(KOPrefs::instance()->nextXDaysItem()->label(), this));
    nextDaysLayout->addWidget(mNextDay);
    nextDaysLayout->addStretch(1);


    mEnableToolTipsCheckBox = new QCheckBox(KOPrefs::instance()->enableToolTipsItem()->label(), this);
    connect(mEnableToolTipsCheckBox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);

    mTodosUseCategoryColorsCheckBox = new QCheckBox(KOPrefs::instance()->todosUseCategoryColorsItem()->label(), this);
    connect(mTodosUseCategoryColorsCheckBox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);

    gdisplayLayout->addWidget(mEnableToolTipsCheckBox);
    gdisplayLayout->addWidget(mTodosUseCategoryColorsCheckBox);
    gdisplayBox->setLayout(gdisplayLayout);
    generalLayout->addWidget(gdisplayBox);

    // GroupBox: Views->General->Date Navigator
    QVBoxLayout *datenavLayout = new QVBoxLayout;
    QGroupBox *datenavBox = new QGroupBox(i18nc("@title:group", "Date Navigator"));
    mDailyRecurCheckbox = new QCheckBox(KOPrefs::instance()->dailyRecurItem()->label(), this);
    connect(mDailyRecurCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
    mWeeklyRecurCheckbox = new QCheckBox(KOPrefs::instance()->weeklyRecurItem()->label(), this);
    connect(mWeeklyRecurCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
    mHighlightTodosCheckbox = new QCheckBox(KOPrefs::instance()->highlightTodosItem()->label(), this);
    connect(mHighlightTodosCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
    mHighlightJournalsCheckbox = new QCheckBox(KOPrefs::instance()->highlightJournalsItem()->label(), this);
    connect(mHighlightJournalsCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
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
    connect(mEnableAgendaItemIconsCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
    adisplayLayout->addWidget(mEnableAgendaItemIconsCheckbox);
    mShowTodosAgendaViewCheckbox = new QCheckBox(KOPrefs::instance()->showTodosAgendaViewItem()->label(), this);
    connect(mShowTodosAgendaViewCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
    adisplayLayout->addWidget(mShowTodosAgendaViewCheckbox);
    mMarcusBainsEnabledCheckbox = new QCheckBox(KOPrefs::instance()->marcusBainsEnabledItem()->label(), this);
    connect(mMarcusBainsEnabledCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
    adisplayLayout->addWidget(mMarcusBainsEnabledCheckbox);
    mMarcusBainsShowSecondsCheckbox = new QCheckBox(KOPrefs::instance()->marcusBainsShowSecondsItem()->label(), this);
    connect(mMarcusBainsShowSecondsCheckbox, &QCheckBox::clicked,
            this, &KOPrefsDialogViews::slotConfigChanged);
    adisplayLayout->addWidget(mMarcusBainsShowSecondsCheckbox);
    connect(mMarcusBainsShowSecondsCheckbox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
    mSelectionStartsEditorCheckbox = new QCheckBox(KOPrefs::instance()->selectionStartsEditorItem()->label(), this);
    connect(mSelectionStartsEditorCheckbox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
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
    connect(mColorBusyDaysEnabledCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
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
    connect(mShowTimeInMonthViewCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
    mdisplayLayout->addWidget(mShowTimeInMonthViewCheckBox);
    mEnableMonthItemIconsCheckBox = new QCheckBox(KOPrefs::instance()->enableMonthItemIconsItem()->label(), this);
    connect(mEnableMonthItemIconsCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
    mdisplayLayout->addWidget(mEnableMonthItemIconsCheckBox);
    mShowTodosMonthViewCheckBox = new QCheckBox(KOPrefs::instance()->showTodosMonthViewItem()->label(), this);
    connect(mShowTodosMonthViewCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
    mdisplayLayout->addWidget(mShowTodosMonthViewCheckBox);
    mShowJournalsMonthViewCheckBox = new QCheckBox(KOPrefs::instance()->showJournalsMonthViewItem()->label(), this);
    connect(mShowJournalsMonthViewCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
    mdisplayLayout->addWidget(mShowJournalsMonthViewCheckBox);
    mdisplayBox->setLayout(mdisplayLayout);

    mMonthIconComboBox->setCheckedIcons(
                KOPrefs::instance()->eventViewsPreferences()->monthViewIcons());
    mdisplayLayout->addWidget(mMonthIconComboBox);

    monthLayout->addWidget(mdisplayBox);

    mColorMonthBusyDaysEnabledCheckBox = new QCheckBox(KOPrefs::instance()->colorMonthBusyDaysEnabledItem()->label(), this);
    connect(mColorMonthBusyDaysEnabledCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
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
    connect(mSortCompletedTodosSeparatelyCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);

    tdisplayLayout->addWidget(mSortCompletedTodosSeparatelyCheckBox);
    tdisplayBox->setLayout(tdisplayLayout);
    todoLayout->addWidget(tdisplayBox);

    // GroupBox: Views->Todo View->Other
    QVBoxLayout *otherLayout = new QVBoxLayout;
    QGroupBox *otherBox = new QGroupBox(i18nc("@title:group", "Other Options"));
    mRecordTodosInJournalsCheckBox = new QCheckBox(KOPrefs::instance()->recordTodosInJournalsItem()->label(), this);
    connect(mRecordTodosInJournalsCheckBox, &QAbstractButton::toggled,
            this, &QWidget::setEnabled);
    otherLayout->addWidget(mRecordTodosInJournalsCheckBox);
    otherBox->setLayout(otherLayout);
    todoLayout->addWidget(otherBox);
    todoLayout->addStretch(1);

    load();
}

void KOPrefsDialogViews::load()
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

void KOPrefsDialogViews::save()
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

void KOPrefsDialogViews::slotConfigChanged()
{
    Q_EMIT markAsChanged();
}
