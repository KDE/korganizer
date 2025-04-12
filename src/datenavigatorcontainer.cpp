/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "datenavigatorcontainer.h"
#include "kodaymatrix.h"
#include "koglobals.h"
#include "widgets/kdatenavigator.h"
#include "widgets/navigatorbar.h"

#include <KLocalizedString>

#include <QTimer>

DateNavigatorContainer::DateNavigatorContainer(QWidget *parent)
    : QFrame(parent)
    , mNavigatorView(new KDateNavigator(this))
{
    mNavigatorView->setWhatsThis(
        i18n("<qt><p>Select the dates you want to "
             "display in KOrganizer's main view here. Hold the "
             "mouse button to select more than one day.</p>"
             "<p>Press the top buttons to browse to the next "
             "/ previous months or years.</p>"
             "<p>Each line shows a week. The number in the left "
             "column is the number of the week in the year. "
             "Press it to select the whole week.</p>"
             "</qt>"));

    connectNavigatorView(mNavigatorView);
}

DateNavigatorContainer::~DateNavigatorContainer()
{
    qDeleteAll(mExtraViews);
}

void DateNavigatorContainer::connectNavigatorView(KDateNavigator *v)
{
    connect(v, &KDateNavigator::datesSelected, this, &DateNavigatorContainer::handleDatesSelectedSignal);

    connect(v, &KDateNavigator::incidenceDropped, this, &DateNavigatorContainer::incidenceDropped);
    connect(v, &KDateNavigator::incidenceDroppedMove, this, &DateNavigatorContainer::incidenceDroppedMove);

    connect(v, &KDateNavigator::newEventSignal, this, &DateNavigatorContainer::newEventSignal);
    connect(v, &KDateNavigator::newTodoSignal, this, &DateNavigatorContainer::newTodoSignal);
    connect(v, &KDateNavigator::newJournalSignal, this, &DateNavigatorContainer::newJournalSignal);

    connect(v, &KDateNavigator::weekClicked, this, &DateNavigatorContainer::handleWeekClickedSignal);

    connect(v, &KDateNavigator::goPrevious, this, &DateNavigatorContainer::goPrevious);
    connect(v, &KDateNavigator::goNext, this, &DateNavigatorContainer::goNext);

    connect(v, &KDateNavigator::nextYearClicked, this, &DateNavigatorContainer::nextYearClicked);
    connect(v, &KDateNavigator::prevYearClicked, this, &DateNavigatorContainer::prevYearClicked);

    connect(v, &KDateNavigator::prevMonthClicked, this, &DateNavigatorContainer::goPrevMonth);
    connect(v, &KDateNavigator::nextMonthClicked, this, &DateNavigatorContainer::goNextMonth);

    connect(v, &KDateNavigator::monthSelected, this, &DateNavigatorContainer::monthSelected);
    connect(v, &KDateNavigator::yearSelected, this, &DateNavigatorContainer::yearSelected);
}

void DateNavigatorContainer::addCalendar(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mNavigatorView->addCalendar(calendar);
    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->addCalendar(calendar);
        }
    }
    mCalendars.push_back(calendar);
}

void DateNavigatorContainer::removeCalendar(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mNavigatorView->removeCalendar(calendar);
    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->removeCalendar(calendar);
        }
    }
    mCalendars.removeOne(calendar);
}

// TODO_Recurrence: let the navigators update just once, and tell them that
// if data has changed or just the selection (because then the list of dayss
// with events doesn't have to be updated if the month stayed the same
void DateNavigatorContainer::updateDayMatrix()
{
    mNavigatorView->updateDayMatrix();
    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->updateDayMatrix();
        }
    }
}

void DateNavigatorContainer::updateToday()
{
    mNavigatorView->updateToday();
    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->updateToday();
        }
    }
}

void DateNavigatorContainer::setUpdateNeeded()
{
    mNavigatorView->setUpdateNeeded();
    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->setUpdateNeeded();
        }
    }
}

void DateNavigatorContainer::updateView()
{
    mNavigatorView->updateView();
    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->setUpdateNeeded();
        }
    }
}

void DateNavigatorContainer::updateConfig()
{
    mNavigatorView->updateConfig();
    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->updateConfig();
        }
    }
}

void DateNavigatorContainer::selectDates(const KCalendarCore::DateList &dateList, const QDate &preferredMonth)
{
    if (!dateList.isEmpty()) {
        QDate start(dateList.first());
        QDate end(dateList.last());
        QDate navfirst(mNavigatorView->startDate());
        QDate navsecond; // start of the second shown month if existent
        QDate navlast;
        if (!mExtraViews.isEmpty()) {
            navlast = mExtraViews.last()->endDate();
            navsecond = mExtraViews.first()->startDate();
        } else {
            navlast = mNavigatorView->endDate();
            navsecond = navfirst;
        }

        // If the datelist crosses months we won't know which month to show
        // so we read what's in preferredMonth
        const bool changingMonth = preferredMonth.isValid() && mNavigatorView->month().month() != preferredMonth.month();

        if (start < navfirst // <- start should always be visible
            || // end is not visible and we have a spare month at the beginning:
            (end > navlast && start >= navsecond) || changingMonth) {
            if (preferredMonth.isValid()) {
                setBaseDates(preferredMonth);
            } else {
                setBaseDates(start);
            }
        }

        if (!mIgnoreNavigatorUpdates) {
            mNavigatorView->selectDates(dateList);
            for (KDateNavigator *n : std::as_const(mExtraViews)) {
                if (n) {
                    n->selectDates(dateList);
                }
            }
        }
    }
}

void DateNavigatorContainer::setBaseDates(const QDate &start)
{
    QDate baseDate = start;
    if (!mIgnoreNavigatorUpdates) {
        mNavigatorView->setBaseDate(baseDate);
    }

    for (KDateNavigator *n : std::as_const(mExtraViews)) {
        baseDate = baseDate.addMonths(1);
        if (!mIgnoreNavigatorUpdates) {
            n->setBaseDate(baseDate);
        }
    }
}

void DateNavigatorContainer::resizeEvent(QResizeEvent *)
{
    /*
        qCDebug(KORGANIZER_LOG) << "DateNavigatorContainer::resizeEvent()";
        qCDebug(KORGANIZER_LOG) << "  CURRENT SIZE:" << size();
        qCDebug(KORGANIZER_LOG) << "  MINIMUM SIZEHINT:" << minimumSizeHint();
        qCDebug(KORGANIZER_LOG) << "  SIZEHINT:" << sizeHint();
        qCDebug(KORGANIZER_LOG) << "  MINIMUM SIZE:" << minimumSize();
    */
    QTimer::singleShot(0, this, &DateNavigatorContainer::resizeAllContents);
}

void DateNavigatorContainer::resizeAllContents()
{
    QSize minSize = mNavigatorView->minimumSizeHint();

    //  qCDebug(KORGANIZER_LOG) << "  NAVIGATORVIEW minimumSizeHint:" << minSize;

    int verticalCount = size().height() / minSize.height();
    int horizontalCount = size().width() / minSize.width();

    if (horizontalCount != mHorizontalCount || verticalCount != mVerticalCount) {
        int count = horizontalCount * verticalCount;
        if (count == 0) {
            return;
        }

        while (count > (mExtraViews.count() + 1)) {
            auto n = new KDateNavigator(this);
            mExtraViews.append(n);
            for (const auto &calendar : std::as_const(mCalendars)) {
                n->addCalendar(calendar);
            }
            connectNavigatorView(n);
        }

        while (count < (mExtraViews.count() + 1)) {
            delete (mExtraViews.last());
            mExtraViews.removeLast();
        }

        mHorizontalCount = horizontalCount;
        mVerticalCount = verticalCount;
        const KCalendarCore::DateList dates = mNavigatorView->selectedDates();
        if (!dates.isEmpty()) {
            setBaseDates(dates.first());
            selectDates(dates);
            for (KDateNavigator *n : std::as_const(mExtraViews)) {
                if (n) {
                    n->show();
                }
            }
        }
    }

    int height = size().height() / verticalCount;
    int width = size().width() / horizontalCount;

    NavigatorBar *bar = mNavigatorView->navigatorBar();
    if (horizontalCount > 1) {
        bar->showButtons(true, false);
    } else {
        bar->showButtons(true, true);
    }

    mNavigatorView->setGeometry((((KOGlobals::self()->reverseLayout()) ? (horizontalCount - 1) : 0) * width), 0, width, height);
    for (int i = 0; i < mExtraViews.count(); ++i) {
        int x = (i + 1) % horizontalCount;
        int y = (i + 1) / horizontalCount;

        KDateNavigator *view = mExtraViews.at(i);
        bar = view->navigatorBar();
        if (y > 0) {
            bar->showButtons(false, false);
        } else {
            if (x + 1 == horizontalCount) {
                bar->showButtons(false, true);
            } else {
                bar->showButtons(false, false);
            }
        }
        view->setGeometry((((KOGlobals::self()->reverseLayout()) ? (horizontalCount - 1 - x) : x) * width), y * height, width, height);
    }
}

QSize DateNavigatorContainer::minimumSizeHint() const
{
    return mNavigatorView->minimumSizeHint();
}

QSize DateNavigatorContainer::sizeHint() const
{
    return mNavigatorView->sizeHint();
}

void DateNavigatorContainer::setHighlightMode(bool highlightEvents, bool highlightTodos, bool highlightJournals) const
{
    mNavigatorView->setHighlightMode(highlightEvents, highlightTodos, highlightJournals);

    for (const KDateNavigator *n : std::as_const(mExtraViews)) {
        if (n) {
            n->setHighlightMode(highlightEvents, highlightTodos, highlightJournals);
        }
    }
}

void DateNavigatorContainer::goNextMonth()
{
    const QPair<QDate, QDate> p = dateLimits(1);

    Q_EMIT nextMonthClicked(mNavigatorView->month(), p.first, p.second);
}

void DateNavigatorContainer::goPrevMonth()
{
    const QPair<QDate, QDate> p = dateLimits(-1);

    Q_EMIT prevMonthClicked(mNavigatorView->month(), p.first, p.second);
}

QPair<QDate, QDate> DateNavigatorContainer::dateLimits(int offset) const
{
    QDate firstMonth;
    QDate lastMonth;
    if (mExtraViews.isEmpty()) {
        lastMonth = mNavigatorView->month();
    } else {
        lastMonth = mExtraViews.last()->month();
    }

    firstMonth = mNavigatorView->month().addMonths(offset);
    lastMonth = lastMonth.addMonths(offset);

    QPair<QDate, QDate> firstMonthBoundary = KODayMatrix::matrixLimits(firstMonth);
    QPair<QDate, QDate> lastMonthBoundary = KODayMatrix::matrixLimits(lastMonth);

    return qMakePair(firstMonthBoundary.first, lastMonthBoundary.second);
}

QDate DateNavigatorContainer::monthOfNavigator(int navigatorIndex) const
{
    if (navigatorIndex == 0) {
        return mNavigatorView->month();
    }

    if (navigatorIndex <= mExtraViews.count() && navigatorIndex >= 0) {
        return mExtraViews[navigatorIndex - 1]->month();
    } else {
        return {};
    }
}

void DateNavigatorContainer::handleDatesSelectedSignal(const KCalendarCore::DateList &dateList)
{
    Q_ASSERT(sender());
    // When we have more than one KDateNavigator, both can have the
    // same selection ( because they can share weeks )
    // The month that we send in the datesSelected() signal should be
    // the one belonging to the KDatenavigator with the earliest month
    const QDate firstDate = dateList.first();
    KDateNavigator *navigator = firstNavigatorForDate(firstDate);
    navigator = navigator ? navigator : qobject_cast<KDateNavigator *>(sender());

    Q_EMIT datesSelected(dateList, navigator->month());
}

void DateNavigatorContainer::handleWeekClickedSignal(const QDate &week, const QDate &)
{
    Q_ASSERT(sender());
    KDateNavigator *navigator = firstNavigatorForDate(week);
    navigator = navigator ? navigator : qobject_cast<KDateNavigator *>(sender());

    Q_EMIT weekClicked(week, navigator->month());
}

KDateNavigator *DateNavigatorContainer::firstNavigatorForDate(const QDate &date) const
{
    KDateNavigator *navigator = nullptr;
    if (date.isValid()) {
        QPair<QDate, QDate> limits = KODayMatrix::matrixLimits(mNavigatorView->month());

        if (date >= limits.first && date <= limits.second) {
            // The date is in the first navigator
            navigator = mNavigatorView;
        } else {
            for (KDateNavigator *nav : std::as_const(mExtraViews)) {
                if (nav) {
                    limits = KODayMatrix::matrixLimits(nav->month());
                    if (date >= limits.first && date <= limits.second) {
                        navigator = nav;
                        break;
                    }
                }
            }
        }
    }

    return navigator;
}

#include "moc_datenavigatorcontainer.cpp"
