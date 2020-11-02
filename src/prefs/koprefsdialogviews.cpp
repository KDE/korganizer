/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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
    : Korganizer::KPrefsModule(KOPrefs::instance(), parent)
    , mMonthIconComboBox(new KItemIconCheckCombo(KItemIconCheckCombo::MonthType, this))
    , mAgendaIconComboBox(new KItemIconCheckCombo(KItemIconCheckCombo::AgendaType, this))
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);
    auto *tabWidget = new QTabWidget(this);
    topTopLayout->addWidget(tabWidget);

    connect(mMonthIconComboBox, &KPIM::KCheckComboBox::checkedItemsChanged,
            this, &Korganizer::KPrefsModule::slotWidChanged);
    connect(mAgendaIconComboBox, &KPIM::KCheckComboBox::checkedItemsChanged,
            this, &Korganizer::KPrefsModule::slotWidChanged);

    // Tab: Views->General
    QFrame *generalFrame = new QFrame(this);
    tabWidget->addTab(generalFrame, QIcon::fromTheme(QStringLiteral("view-choose")),
                      i18nc("@title:tab general settings", "General"));

    QBoxLayout *generalLayout = new QVBoxLayout(generalFrame);

    // GroupBox: Views->General->Display Options
    auto *gdisplayLayout = new QVBoxLayout;
    QGroupBox *gdisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));

    QBoxLayout *nextDaysLayout = new QHBoxLayout;
    gdisplayLayout->addLayout(nextDaysLayout);

    Korganizer::KPrefsWidInt *nextDays
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
    auto *datenavLayout = new QVBoxLayout;
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
    auto *adisplayLayout = new QVBoxLayout;
    QGroupBox *adisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));

    auto *hourSizeLayout = new QHBoxLayout;
    adisplayLayout->addLayout(hourSizeLayout);

    Korganizer::KPrefsWidInt *hourSize
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
    Korganizer::KPrefsWidBool *marcusBainsEnabled
        = addWidBool(KOPrefs::instance()->marcusBainsEnabledItem());
    adisplayLayout->addWidget(marcusBainsEnabled->checkBox());

    Korganizer::KPrefsWidBool *marcusBainsShowSeconds
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
    auto *mdisplayLayout = new QVBoxLayout;
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
    auto *tdisplayLayout = new QVBoxLayout;
    QGroupBox *tdisplayBox = new QGroupBox(i18nc("@title:group", "Display Options"));
    tdisplayLayout->addWidget(
        addWidBool(KOPrefs::instance()->sortCompletedTodosSeparatelyItem())->checkBox());
    tdisplayBox->setLayout(tdisplayLayout);
    todoLayout->addWidget(tdisplayBox);

    // GroupBox: Views->Todo View->Other
    auto *otherLayout = new QVBoxLayout;
    QGroupBox *otherBox = new QGroupBox(i18nc("@title:group", "Other Options"));
    otherLayout->addWidget(
        addWidBool(KOPrefs::instance()->recordTodosInJournalsItem())->checkBox());
    otherBox->setLayout(otherLayout);
    todoLayout->addWidget(otherBox);
    todoLayout->addStretch(1);

    load();
}

void KOPrefsDialogViews::usrReadConfig()
{
    KOPrefs::instance()->eventViewsPreferences()->setAgendaViewIcons(
        mAgendaIconComboBox->checkedIcons());
    KOPrefs::instance()->eventViewsPreferences()->setMonthViewIcons(
        mMonthIconComboBox->checkedIcons());
}
