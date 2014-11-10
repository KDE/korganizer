/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "navigatorbar.h"
#include "koglobals.h"

#include <KCalendarSystem>
#include <KIconLoader>

#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>

NavigatorBar::NavigatorBar(QWidget *parent) : QWidget(parent)
{
    QFont tfont = font();
    tfont.setPointSize(10);
    tfont.setBold(false);

    bool isRTL = KOGlobals::self()->reverseLayout();

    mPrevYear = createNavigationButton(
                    isRTL ? QLatin1String("arrow-right-double") : QLatin1String("arrow-left-double"),
                    i18n("Scroll backward to the previous year"),
                    i18n("Click this button to scroll the display to the "
                         "same approximate day of the previous year"));

    mPrevMonth = createNavigationButton(
                     isRTL ? QLatin1String("arrow-right") : QLatin1String("arrow-left"),
                     i18n("Scroll backward to the previous month"),
                     i18n("Click this button to scroll the display to the "
                          "same approximate date of the previous month"));

    mNextMonth = createNavigationButton(
                     isRTL ? QLatin1String("arrow-left") : QLatin1String("arrow-right"),
                     i18n("Scroll forward to the next month"),
                     i18n("Click this button to scroll the display to the "
                          "same approximate date of the next month"));

    mNextYear = createNavigationButton(
                    isRTL ? QLatin1String("arrow-left-double") : QLatin1String("arrow-right-double"),
                    i18n("Scroll forward to the next year"),
                    i18n("Click this button to scroll the display to the "
                         "same approximate day of the next year"));

    // Create month name button
    mMonth = new QToolButton(this);
    mMonth->setPopupMode(QToolButton::InstantPopup);
    mMonth->setAutoRaise(true);
    mMonth->setFont(tfont);
    mMonth->setToolTip(i18n("Select a month"));

    // Create year button
    mYear = new QToolButton(this);
    mYear->setPopupMode(QToolButton::InstantPopup);
    mYear->setAutoRaise(true);
    mYear->setFont(tfont);
    mYear->setToolTip(i18n("Select a year"));

    // set up control frame layout
    QHBoxLayout *ctrlLayout = new QHBoxLayout(this);
    ctrlLayout->setMargin(0);
    ctrlLayout->addWidget(mPrevYear);
    ctrlLayout->addWidget(mPrevMonth);
    ctrlLayout->addStretch();
    ctrlLayout->addWidget(mMonth);
    ctrlLayout->addWidget(mYear);
    ctrlLayout->addStretch();
    ctrlLayout->addWidget(mNextMonth);
    ctrlLayout->addWidget(mNextYear);

    connect(mPrevYear, &QToolButton::clicked, this, &NavigatorBar::prevYearClicked);
    connect(mPrevMonth, &QToolButton::clicked, this, &NavigatorBar::prevMonthClicked);
    connect(mNextMonth, &QToolButton::clicked, this, &NavigatorBar::nextMonthClicked);
    connect(mNextYear, &QToolButton::clicked, this, &NavigatorBar::nextYearClicked);
    connect(mMonth, &QToolButton::clicked, this, &NavigatorBar::selectMonthFromMenu);
    connect(mYear, &QToolButton::clicked, this, &NavigatorBar::selectYearFromMenu);
}

NavigatorBar::~NavigatorBar()
{
}

void NavigatorBar::showButtons(bool left, bool right)
{
    if (left) {
        mPrevYear->show();
        mPrevMonth->show();
    } else {
        mPrevYear->hide();
        mPrevMonth->hide();
    }

    if (right) {
        mNextYear->show();
        mNextMonth->show();
    } else {
        mNextYear->hide();
        mNextMonth->hide();
    }
}

void NavigatorBar::selectDates(const KCalCore::DateList &dateList)
{
    if (dateList.count() > 0) {
        mDate = dateList.first();

        const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

        // set the label text at the top of the navigator
        mMonth->setText(i18nc("monthname", "%1", calSys->monthName(mDate)));
        mYear->setText(i18nc("4 digit year", "%1",
                             calSys->formatDate(mDate, KLocale::Year, KLocale::LongNumber)));
    }
}

void NavigatorBar::selectMonthFromMenu()
{
    // every year can have different month names (in some calendar systems)
    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    int month = calSys->month(mDate);
    int year = calSys->year(mDate);
    int months = calSys->monthsInYear(mDate);

    QMenu *menu = new QMenu(mMonth);
    QList<QAction *>act;

    QAction *activateAction = 0;
    for (int i = 1; i <= months; ++i) {
        QAction *monthAction = menu->addAction(calSys->monthName(i, year));
        act.append(monthAction);
        if (i == month) {
            activateAction = monthAction;
        }
    }
    if (activateAction) {
        menu->setActiveAction(activateAction);
    }
    month = 0;
    QAction *selectedAct = menu->exec(mMonth->mapToGlobal(QPoint(0, 0)));
    if (selectedAct && (selectedAct != activateAction)) {
        for (int i = 0; i < months; ++i) {
            if (act[i] == selectedAct) {
                month = i + 1;
            }
        }
    }
    qDeleteAll(act);
    act.clear();
    delete menu;

    if (month > 0) {
        emit monthSelected(month);
    }
}

void NavigatorBar::selectYearFromMenu()
{
    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    int year = calSys->year(mDate);
    int years = 11;  // odd number (show a few years ago -> a few years from now)
    int minYear = year - (years / 3);

    QMenu *menu = new QMenu(mYear);
    QList<QAction *>act;

    QString yearStr;
    QAction *activateAction = 0;
    int y = minYear;
    for (int i = 0; i < years; ++i) {
        QAction *yearAction = menu->addAction(yearStr.setNum(y));
        act.append(yearAction);
        if (y == year) {
            activateAction = yearAction;
        }
        y++;
    }
    if (activateAction) {
        menu->setActiveAction(activateAction);
    }
    year = 0;
    QAction *selectedAct = menu->exec(mYear->mapToGlobal(QPoint(0, 0)));
    if (selectedAct && (selectedAct != activateAction)) {
        int y = minYear;
        for (int i = 0; i < years; ++i) {
            if (act[i] == selectedAct) {
                year = y;
            }
            y++;
        }
    }
    qDeleteAll(act);
    act.clear();
    delete menu;

    if (year > 0) {
        emit yearSelected(year);
    }
}

QToolButton *NavigatorBar::createNavigationButton(const QString &icon,
        const QString &toolTip,
        const QString &whatsThis)
{
    QToolButton *button = new QToolButton(this);

    button->setIcon(
        KIconLoader::global()->loadIcon(icon, KIconLoader::Desktop, KIconLoader::SizeSmall));
    button->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setAutoRaise(true);
    button->setToolTip(toolTip);
    button->setWhatsThis(whatsThis);

    return button;
}

