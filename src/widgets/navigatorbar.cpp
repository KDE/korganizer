/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2006-2026 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "navigatorbar.h"
#include "koglobals.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLocale>
#include <QMenu>
#include <QToolButton>

NavigatorBar::NavigatorBar(QWidget *parent)
    : QWidget(parent)
    , mMonth(new QToolButton(this))
    , mYear(new QToolButton(this))
{
    QFont tfont = font();
    tfont.setPointSize(10);
    tfont.setBold(true);

    const bool isRTL = KOGlobals::self()->reverseLayout();

    mFullWindow = createNavigationButton(QStringLiteral("view-fullscreen"),
                                         i18nc("@info:tooltip", "Show/hide the sidebar in this view"),
                                         i18nc("@info::whatsthis",
                                               "Click this button to show/hide the sidebar in the current view. "
                                               "Each view can show/hide the sidebar independently."));

    mToday = createNavigationButton(QStringLiteral("go-jump-today"),
                                    i18nc("@info:tooltip", "Scroll the view to today"),
                                    i18nc("@info::whatsthis", "Scrolls the current view to today's date."));

    mPrevYear = createNavigationButton(isRTL ? QStringLiteral("go-last") : QStringLiteral("go-first"),
                                       i18nc("@info:tooltip", "Scroll backward a year"),
                                       i18nc("@info::whatsthis",
                                             "Click this button to scroll the display to the "
                                             "same approximate day of the previous year"));

    mPrevMonth = createNavigationButton(isRTL ? QStringLiteral("arrow-right-double") : QStringLiteral("arrow-left-double"),
                                        i18nc("@info:tooltip", "Scroll backward a month"),
                                        i18nc("@info::whatsthis",
                                              "Click this button to scroll the display to the "
                                              "same approximate date of the previous month"));

    mPrevWeek = createNavigationButton(isRTL ? QStringLiteral("arrow-right") : QStringLiteral("arrow-left"),
                                       i18nc("@info:tooltip", "Scroll backward a week"),
                                       i18nc("@info::whatsthis", "Click this button to scroll the display to back 1 week in time"));

    mNextWeek = createNavigationButton(isRTL ? QStringLiteral("arrow-left") : QStringLiteral("arrow-right"),
                                       i18nc("@info:tooltip", "Scroll forward a week"),
                                       i18nc("@info::whatsthis", "Click this button to scroll the display to forward 1 week in time"));

    mNextMonth = createNavigationButton(isRTL ? QStringLiteral("arrow-left-double") : QStringLiteral("arrow-right-double"),
                                        i18nc("@info:tooltip", "Scroll forward a month"),
                                        i18nc("@info::whatsthis",
                                              "Click this button to scroll the display to the "
                                              "same approximate date of the next month"));

    mNextYear = createNavigationButton(isRTL ? QStringLiteral("go-first") : QStringLiteral("go-last"),
                                       i18nc("@info:tooltip", "Scroll forward a year"),
                                       i18nc("@info::whatsthis",
                                             "Click this button to scroll the display to the "
                                             "same approximate day of the next year"));

    // Create month name button
    mMonth->setPopupMode(QToolButton::InstantPopup);
    mMonth->setAutoRaise(true);
    mMonth->setFont(tfont);
    mMonth->setToolTip(i18nc("@info:tooltip", "Select a month"));

    // Create year button
    mYear->setPopupMode(QToolButton::InstantPopup);
    mYear->setAutoRaise(true);
    mYear->setFont(tfont);
    mYear->setToolTip(i18nc("@info:tooltip", "Select a year"));

    // set up control frame layout
    auto ctrlLayout = new QHBoxLayout(this);
    ctrlLayout->setContentsMargins({});
    ctrlLayout->setSpacing(0);
    ctrlLayout->addWidget(mFullWindow);
    ctrlLayout->addStretch();
    ctrlLayout->addWidget(mPrevYear);
    ctrlLayout->addWidget(mPrevMonth);
    ctrlLayout->addWidget(mPrevWeek);
    ctrlLayout->addWidget(mMonth);
    ctrlLayout->addWidget(mYear);
    ctrlLayout->addWidget(mNextWeek);
    ctrlLayout->addWidget(mNextMonth);
    ctrlLayout->addWidget(mNextYear);
    ctrlLayout->addWidget(mToday);
    ctrlLayout->addStretch();

    connect(mFullWindow, &QToolButton::clicked, this, &NavigatorBar::fullWindowClicked);
    connect(mToday, &QToolButton::clicked, this, &NavigatorBar::todayClicked);
    connect(mPrevYear, &QToolButton::clicked, this, &NavigatorBar::prevYearClicked);
    connect(mPrevMonth, &QToolButton::clicked, this, &NavigatorBar::prevMonthClicked);
    connect(mPrevWeek, &QToolButton::clicked, this, &NavigatorBar::prevWeekClicked);
    connect(mNextWeek, &QToolButton::clicked, this, &NavigatorBar::nextWeekClicked);
    connect(mNextMonth, &QToolButton::clicked, this, &NavigatorBar::nextMonthClicked);
    connect(mNextYear, &QToolButton::clicked, this, &NavigatorBar::nextYearClicked);
    connect(mMonth, &QToolButton::clicked, this, &NavigatorBar::selectMonthFromMenu);
    connect(mYear, &QToolButton::clicked, this, &NavigatorBar::selectYearFromMenu);
}

NavigatorBar::~NavigatorBar() = default;

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

void NavigatorBar::selectDates(const KCalendarCore::DateList &dateList)
{
    if (!dateList.isEmpty()) {
        mDate = dateList.first();

        // set the label text at the top of the navigator
        mMonth->setText(i18nc("monthname", "%1", QLocale().standaloneMonthName(mDate.month(), QLocale::LongFormat)));
        mYear->setText(i18nc("4 digit year", "%1", QLocale().toString(mDate, QStringLiteral("yyyy"))));
    }
}

void NavigatorBar::selectMonthFromMenu()
{
    int month = mDate.month();
    const int months = 12;

    auto menu = new QMenu(mMonth);
    QList<QAction *> act;

    QAction *activateAction = nullptr;
    act.reserve(months);
    for (int i = 1; i <= months; ++i) {
        QAction *monthAction = menu->addAction(QLocale().standaloneMonthName(i, QLocale::LongFormat));
        act.append(monthAction);
        if (i == month) {
            activateAction = monthAction;
        }
    }
    if (activateAction) {
        menu->setActiveAction(activateAction);
    }
    month = 0;
    const QAction *selectedAct = menu->exec(mMonth->mapToGlobal(QPoint(0, 0)));
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
        Q_EMIT monthSelected(month);
    }
}

void NavigatorBar::selectYearFromMenu()
{
    int year = mDate.year();
    int const years = 11; // odd number (show a few years ago -> a few years from now)
    int const minYear = year - (years / 3);

    auto menu = new QMenu(mYear);
    QList<QAction *> act;
    act.reserve(years);

    QString yearStr;
    QAction *activateAction = nullptr;
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
    const QAction *selectedAct = menu->exec(mYear->mapToGlobal(QPoint(0, 0)));
    if (selectedAct && (selectedAct != activateAction)) {
        int yearCount = minYear;
        for (int i = 0; i < years; ++i) {
            if (act[i] == selectedAct) {
                year = yearCount;
            }
            yearCount++;
        }
    }
    qDeleteAll(act);
    act.clear();
    delete menu;

    if (year > 0) {
        Q_EMIT yearSelected(year);
    }
}

QToolButton *NavigatorBar::createNavigationButton(const QString &icon, const QString &toolTip, const QString &whatsThis)
{
    auto button = new QToolButton(this);

    button->setIcon(QIcon::fromTheme(icon));
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setAutoRaise(true);
    button->setToolTip(toolTip);
    button->setWhatsThis(whatsThis);

    return button;
}

void NavigatorBar::setSideBarMode(bool isShown)
{
    if (isShown) {
        mFullWindow->setIcon(QIcon::fromTheme(QStringLiteral("view-fullscreen")));
    } else {
        mFullWindow->setIcon(QIcon::fromTheme(QStringLiteral("view-restore")));
    }
}

#include "moc_navigatorbar.cpp"
