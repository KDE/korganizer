/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
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

#include "kdatenavigator.h"
#include "kodaymatrix.h"
#include "koglobals.h"
#include "widgets/navigatorbar.h"

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QWheelEvent>
#include <KLocalizedString>
#include <QFontDatabase>
#include <QLocale>

KDateNavigator::KDateNavigator(QWidget *parent)
    : QFrame(parent), mBaseDate(1970, 1, 1)
{
    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(0);

    mNavigatorBar = new NavigatorBar(this);
    topLayout->addWidget(mNavigatorBar, 0, 0, 1, 8);

    connect(mNavigatorBar, &NavigatorBar::prevYearClicked, this, &KDateNavigator::prevYearClicked);
    connect(mNavigatorBar, &NavigatorBar::prevMonthClicked, this, &KDateNavigator::prevMonthClicked);
    connect(mNavigatorBar, &NavigatorBar::nextMonthClicked, this, &KDateNavigator::nextMonthClicked);
    connect(mNavigatorBar, &NavigatorBar::nextYearClicked, this, &KDateNavigator::nextYearClicked);
    connect(mNavigatorBar, &NavigatorBar::monthSelected, this, &KDateNavigator::monthSelected);
    connect(mNavigatorBar, &NavigatorBar::yearSelected, this, &KDateNavigator::yearSelected);

    QString generalFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();

    // Set up the heading fields.
    for (int i = 0; i < 7; ++i) {
        mHeadings[i] = new QLabel(this);
        mHeadings[i]->setFont(QFont(generalFont, 10, QFont::Bold));
        mHeadings[i]->setAlignment(Qt::AlignCenter);

        topLayout->addWidget(mHeadings[i], 1, i + 1);
    }

    // Create the weeknumber labels
    for (int i = 0; i < 6; ++i) {
        mWeeknos[i] = new QLabel(this);
        mWeeknos[i]->setAlignment(Qt::AlignCenter);
        mWeeknos[i]->setFont(QFont(generalFont, 10));
        mWeeknos[i]->installEventFilter(this);

        topLayout->addWidget(mWeeknos[i], i + 2, 0);
    }

    mDayMatrix = new KODayMatrix(this);
    mDayMatrix->setObjectName(QStringLiteral("KDateNavigator::dayMatrix"));

    connect(mDayMatrix, &KODayMatrix::selected, this, &KDateNavigator::datesSelected);

    connect(mDayMatrix, &KODayMatrix::incidenceDropped, this, &KDateNavigator::incidenceDropped);
    connect(mDayMatrix, &KODayMatrix::incidenceDroppedMove, this, &KDateNavigator::incidenceDroppedMove);

    connect(mDayMatrix, &KODayMatrix::newEventSignal, this, &KDateNavigator::newEventSignal);
    connect(mDayMatrix, &KODayMatrix::newTodoSignal, this, &KDateNavigator::newTodoSignal);
    connect(mDayMatrix, &KODayMatrix::newJournalSignal, this, &KDateNavigator::newJournalSignal);

    topLayout->addWidget(mDayMatrix, 2, 1, 6, 7);

    // read settings from configuration file.
    updateConfig();
}

KDateNavigator::~KDateNavigator()
{
}

void KDateNavigator::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    if (mCalendar) {
        disconnect(mCalendar.data(), Q_NULLPTR, this, Q_NULLPTR);
    }

    mCalendar = calendar;

    if (mCalendar) {
        connect(mCalendar.data(), &Akonadi::ETMCalendar::calendarChanged, this, &KDateNavigator::setUpdateNeeded);
    }

    mDayMatrix->setCalendar(calendar);
}

void KDateNavigator::setBaseDate(const QDate &date)
{
    if (date != mBaseDate) {
        mBaseDate = date;

        updateDates();
        updateView();

        // Use the base date to show the monthname and year in the header
        KCalCore::DateList dates;
        dates.append(date);
        mNavigatorBar->selectDates(dates);

        update();
        mDayMatrix->update();
    }
}

QSizePolicy KDateNavigator::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
}

void KDateNavigator::updateToday()
{
    mDayMatrix->recalculateToday();
    mDayMatrix->update();
}

QDate KDateNavigator::startDate() const
{
    // Find the first day of the week of the current month.
    QDate dayone(mBaseDate.year(), mBaseDate.month(), mBaseDate.day());
    int d2 = dayone.day();
    dayone = dayone.addDays(-d2 + 1);

    int m_fstDayOfWkCalsys = dayone.dayOfWeek();
    int weekstart = QLocale().firstDayOfWeek();

    // If month begins on Monday and Monday is first day of week,
    // month should begin on second line. Sunday doesn't have this problem.
    int nextLine = m_fstDayOfWkCalsys <= weekstart ? 7 : 0;

    // update the matrix dates
    int index = weekstart - m_fstDayOfWkCalsys - nextLine;

    dayone = dayone.addDays(index);

    return dayone;
}

QDate KDateNavigator::endDate() const
{
    return startDate().addDays(6 * 7);
}

void KDateNavigator::setHighlightMode(bool highlightEvents,
                                      bool highlightTodos,
                                      bool highlightJournals) const
{

    mDayMatrix->setHighlightMode(highlightEvents, highlightTodos, highlightJournals);
}

void KDateNavigator::updateDates()
{
    QDate dayone = startDate();

    mDayMatrix->updateView(dayone);

    // set the week numbers.
    for (int i = 0; i < 6; ++i) {
        // Use QDate's weekNumber method to determine the week number!
        QDate dtStart = mDayMatrix->getDate(i * 7);
        QDate dtEnd = mDayMatrix->getDate((i + 1) * 7 - 1);
        const int weeknumstart = dtStart.weekNumber();
        const int weeknumend = dtEnd.weekNumber();
        QString weeknum;

        if (weeknumstart != weeknumend) {
            weeknum = i18nc("start/end week number of line in date picker", "%1/%2",
                            weeknumstart, weeknumend);
        } else {
            weeknum.setNum(weeknumstart);
        }
        mWeeknos[i]->setText(weeknum);
        mWeeknos[i]->setToolTip(i18n("Scroll to week number %1", weeknum));
        mWeeknos[i]->setWhatsThis(
            i18n("Click here to scroll the display to week number %1 "
                 "of the currently displayed year.", weeknum));
    }

// each updateDates is followed by an updateView -> repaint is issued there !
//  mDayMatrix->repaint();
}

void KDateNavigator::updateDayMatrix()
{
    mDayMatrix->updateView();
    mDayMatrix->update();
}

void KDateNavigator::setUpdateNeeded()
{
    mDayMatrix->setUpdateNeeded();
}

QDate KDateNavigator::month() const
{
    QDate firstCell = startDate();

    if (firstCell.day() == 1) {
        return firstCell;
    } else {
        firstCell.setDate(firstCell.year(), firstCell.month(), 1);
        return firstCell.addMonths(1);
    }
}

void KDateNavigator::updateView()
{
    updateDayMatrix();
    update();
}

void KDateNavigator::updateConfig()
{
    int weekstart = QLocale().firstDayOfWeek();
    for (int i = 0; i < 7; ++i) {
        const int day = weekstart + i <= 7 ? weekstart + i : (weekstart + i) % 7;
        QString dayName = QLocale().dayName(day, QLocale::ShortFormat);
        QString longDayName = QLocale().dayName(day, QLocale::LongFormat);
        mHeadings[i]->setText(dayName);
        mHeadings[i]->setToolTip(i18n("%1", longDayName));
        mHeadings[i]->setWhatsThis(
            i18n("A column header of the %1 dates in the month.", longDayName));
    }
    mDayMatrix->setUpdateNeeded();
    updateDayMatrix();
    update();
    // FIXME: Use actual config setting here
//  setShowWeekNums( true );
}

void KDateNavigator::setShowWeekNums(bool enabled)
{
    for (int i = 0; i < 6; ++i) {
        if (enabled) {
            mWeeknos[i]->show();
        } else {
            mWeeknos[i]->hide();
        }
    }
}

void KDateNavigator::selectMonthHelper(int monthDifference)
{
    QDate baseDateNextMonth = mBaseDate.addMonths(monthDifference);

    KCalCore::DateList newSelection = mSelectedDates;
    for (int i = 0; i < mSelectedDates.count(); ++i) {
        newSelection[i] = newSelection[i].addMonths(monthDifference);
    }

    setBaseDate(baseDateNextMonth);
    mSelectedDates = newSelection;
    mDayMatrix->setSelectedDaysFrom(*(newSelection.begin()),
                                    *(--newSelection.end()));
    updateView();
}

void KDateNavigator::selectNextMonth()
{
    selectMonthHelper(1);
}

void KDateNavigator::selectPreviousMonth()
{
    selectMonthHelper(-1);
}

void KDateNavigator::selectDates(const KCalCore::DateList &dateList)
{
    if (dateList.count() > 0) {
        mSelectedDates = dateList;

        updateDates();

        mDayMatrix->setSelectedDaysFrom(*(dateList.begin()),
                                        *(--dateList.end()));

        updateView();
    }
}

void KDateNavigator::wheelEvent(QWheelEvent *e)
{
    if (e->delta() > 0) {
        Q_EMIT goPrevious();
    } else {
        Q_EMIT goNext();
    }
    e->accept();
}

bool KDateNavigator::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress) {
        int i;
        for (i = 0; i < 6; ++i) {
            if (o == mWeeknos[i]) {
                const QDate weekstart = mDayMatrix->getDate(i * 7);
                Q_EMIT weekClicked(weekstart, month());
                break;
            }
        }
        return true;
    } else {
        return false;
    }
}

