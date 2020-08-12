/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_DATENAVIGATORCONTAINER_H
#define KORG_DATENAVIGATORCONTAINER_H

#include <Akonadi/Calendar/ETMCalendar>

#include <QFrame>
#include <QDate>
class KDateNavigator;

class DateNavigatorContainer : public QFrame
{
    Q_OBJECT
public:
    explicit DateNavigatorContainer(QWidget *parent = nullptr);
    ~DateNavigatorContainer() override;

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar(const Akonadi::ETMCalendar::Ptr &);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setHighlightMode(bool highlightEvents, bool highlightTodos, bool highlightJournals) const;
    void setUpdateNeeded();

    /**
       Returns the month of the specified KDateNavigator.
       The first navigatorIndex is 0.
       Returns a QDate instead of uint so it can be easily feed to KCalendarSystem's
       functions.
       An invalid QDate() is returned if the index is too big or too small.
    */
    Q_REQUIRED_RESULT QDate monthOfNavigator(int navigatorIndex = 0) const;
public Q_SLOTS:

    /**
       preferredMonth is useful when the datelist crosses months, if different
       from -1, it has the month that the kdatenavigator should show in case
       of ambiguity
    */
    void selectDates(const KCalendarCore::DateList &, const QDate &preferredMonth = QDate());

    void updateView();
    void updateConfig();
    void updateDayMatrix();
    void updateToday();

    void goPrevMonth();
    void goNextMonth();

Q_SIGNALS:
    void datesSelected(const KCalendarCore::DateList &, const QDate &preferredMonth);
    void incidenceDropped(const Akonadi::Item &, const QDate &);
    void incidenceDroppedMove(const Akonadi::Item &, const QDate &);
    void newEventSignal(const QDate &);
    void newTodoSignal(const QDate &);
    void newJournalSignal(const QDate &);

    /**
     * @param preferredMonth Holds the month that should be selected when the
     * week crosses months. It's a QDate instead of uint so it can be easily
     * fed to KCalendarSystem's functions.
     */
    void weekClicked(const QDate &week, const QDate &preferredMonth);

    void goPrevious();
    void goNext();

    void nextYearClicked();
    void prevYearClicked();

    /** Signals that the previous month button has been clicked.

        @param currentMonth The month displayed on the first KDateNavigator.
               DateNavigator doesn't know anything abouts months, it just has
               a list of selected dates, so we must send this.
        @param selectionLowerLimit The first date of the first KDateNavigator.
        @param selectionUpperLimit The last date of the last KDateNavigator.
    */
    void prevMonthClicked(const QDate &currentMonth, const QDate &selectionLowerLimit, const QDate &selectionUpperLimit);

    void nextMonthClicked(const QDate &currentMonth, const QDate &selectionLowerLimit, const QDate &selectionUpperLimit);

    void monthSelected(int month);

    void yearSelected(int year);

protected:
    void resizeEvent(QResizeEvent *) override;
    void setBaseDates(const QDate &start);
    void connectNavigatorView(KDateNavigator *v);

protected Q_SLOTS:
    /**
     * Resizes all the child elements after the size of the widget changed.
     * This slot is called by a QTimer::singleShot from resizeEvent.
     * This makes the UI seem more responsive, since the other parts
     * of the splitter are resized earlier now.
     */
    void resizeAllContents();

private Q_SLOTS:
    void handleDatesSelectedSignal(const KCalendarCore::DateList &);
    void handleWeekClickedSignal(const QDate &, const QDate &);

private:
    /* Returns the first day of the first KDateNavigator, and the last day
       of the last KDateNavigator.

       @param monthOffset If you have two KDateNavigators displaying
       January and February and want to know the boundaries of,
       for e.g. displaying February and March, use monthOffset = 1.
    */
    QPair<QDate, QDate> dateLimits(int monthOffset = 0) const;

    /**
     * Returns the first KDateNavigator that displays date, or 0 if
     * no KDateNavigator displays it.
     */
    KDateNavigator *firstNavigatorForDate(const QDate &date) const;

    KDateNavigator *mNavigatorView = nullptr;

    Akonadi::ETMCalendar::Ptr mCalendar;

    QList<KDateNavigator *> mExtraViews;

    int mHorizontalCount = 1;
    int mVerticalCount = 1;

    bool mIgnoreNavigatorUpdates = false;
};

#endif
