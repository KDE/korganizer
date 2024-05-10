/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Eitzenberger Thomas <thomas.eitzenberger@siemens.at>
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <Akonadi/CollectionCalendar>

#include <KCalendarCore/IncidenceBase> //for KCalendarCore::DateList typedef

#include <QDate>
#include <QFrame>

/**
 *  Replacement for kdpdatebuton.cpp that used 42 widgets for the day
 *  matrix to be displayed. Cornelius thought this was a waste of memory
 *  and a lot of overhead. In addition the selection was not very intuitive
 *  so I decided to rewrite it using a QFrame that draws the labels and
 *  allows for dragging selection while maintaining nearly full compatibility
 *  in behavior with its predecessor.
 *
 *  The following functionality has been changed:
 *
 *  o when shifting events in the agenda view from one day to another
 *    the day matrix is updated now
 *
 *  o dragging an event to the matrix will MOVE not COPY the event to the
 *    new date.
 *
 *  o no support for Ctrl+click to create groups of dates
 *    (This has not really been supported in the predecessor.
 *    It was not very intuitive nor was it user friendly.)
 *    This feature has been replaced with dragging a selection on the matrix.
 *    The matrix will automatically choose the appropriate selection (e.g. you
 *    are not any longer able to select two distinct groups of date selections
 *    as in the old class)
 *
 *  o now that you can select more than a week it can happen that not all
 *    selected days are displayed in the matrix. However this is preferred
 *    to the alternative which would mean to adjust the selection and leave
 *    some days undisplayed while scrolling through the months
 *
 *  @short day matrix widget of the KDateNavigator
 *
 *  @author Eitzenberger Thomas
 */
class KODayMatrix : public QFrame, public KCalendarCore::Calendar::CalendarObserver
{
    Q_OBJECT
public:
    /** constructor to create a day matrix widget.
     *
     *  @param parent widget that is the parent of the day matrix.
     *  Normally this should be a KDateNavigator
     */
    explicit KODayMatrix(QWidget *parent);

    /** destructor that deallocates all dynamically allocated private members.
     */
    ~KODayMatrix() override;

    /** returns the first and last date of the 6*7 matrix that displays @p month
     * @param month The month we want to get matrix boundaries
     */
    [[nodiscard]] static QPair<QDate, QDate> matrixLimits(QDate month);

    /**
      Associate a calendar with this day matrix. If there is a calendar, the
      day matrix will accept drops and days with events will be highlighted.
    */
    void addCalendar(const Akonadi::CollectionCalendar::Ptr &calendar);
    void removeCalendar(const Akonadi::CollectionCalendar::Ptr &calendar);

    /** updates the day matrix to start with the given date. Does all the
     *  necessary checks for holidays or events on a day and stores them
     *  for display later on.
     *  Does NOT update the view visually. Call repaint() for this.
     *
     *  @param actdate recalculates the day matrix to show NUMDAYS starting
     *  from this date.
     */
    void updateView(QDate actdate);

    /**
      Update incidence states of dates. Depending of the preferences days with
      incidences are highlighted in some way.
    */
    void updateIncidences();

    /**
     * Returns the QDate object associated with day indexed by the supplied
     * offset.
     */
    const QDate &getDate(int offset) const;

    /**
     * Returns the official name of this holy day or 0 if there is no label
     * for this day.
     */
    QString getHolidayLabel(int offset) const;

    /**
     * Adds all actual selected days from mSelStart to mSelEnd to the supplied
     * DateList.
     */
    void addSelectedDaysTo(KCalendarCore::DateList &);

    /**
     * Sets the actual to be displayed selection in the day matrix starting
     * from start and ending with end. Theview must be manually updated by
     * calling repaint. (?)
     * @param start start of the new selection
     * @param end end date of the new selection
     */
    void setSelectedDaysFrom(QDate start, QDate end);

    /**
      Clear all selections.
    */
    void clearSelection();

    /** Is today visible in the view? Keep this in sync with
     * the values today (below) can take.
     */
    [[nodiscard]] bool isTodayVisible() const
    {
        return mToday >= 0;
    }

    /**
     * If today is visible, then we can find out if today is
     * near the beginning or the end of the month.
     * This is dependent on today remaining the index
     * in the array of visible dates and going from
     * top left (0) to bottom right (41).
     */
    [[nodiscard]] bool isBeginningOfMonth() const
    {
        return mToday <= 8;
    }

    [[nodiscard]] bool isEndOfMonth() const
    {
        return mToday >= 27;
    }

    /**
     *  Reimplemented from Akonadi::ETMCalendar
     *  They set mPendingChanges to true
     */
    void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence) override;
    void calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &incidence) override;
    void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Calendar *calendar) override;

    /** Sets which incidences should be highlighted */
    void setHighlightMode(bool highlightEvents, bool highlightTodos, bool highlightJournals);
    void setUpdateNeeded();
public Q_SLOTS:
    /**
     * Recalculates all the flags of the days in the matrix like holidays or
     * events on a day (Actually calls above method with the actual startdate).
     */
    void updateView();

    /**
     * Calculates which square in the matrix should be highlighted to indicate
     * the square is on "today".
     */
    void recalculateToday();

    /**
     * Handle resource changes.
     */
    void resourcesChanged();

Q_SIGNALS:
    /**
     * Emitted if the user selects a block of days with the mouse by dragging
     * a rectangle inside the matrix
     *
     * @param daylist list of days that have been selected by the user
     */
    void selected(const KCalendarCore::DateList &daylist);

    void newEventSignal(const QDate &date, const QDate &Date);
    void newTodoSignal(const QDate &date);
    void newJournalSignal(const QDate &date);

    /**
     * Emitted if the user has dropped an incidence (event or todo) inside
     * the matrix.
     *
     * @param incidence the dropped calendar incidence
     * @param dt QDate that has been selected
     */
    void incidenceDropped(const Akonadi::Item &item, const QDate &dt);

    /**
     * Emitted if the user has dropped an event inside the matrix and chose
     * to move it instead of copy
     *
     * @param oldincidence the new calendar incidence
     * @param dt QDate that has been selected
     */
    void incidenceDroppedMove(const Akonadi::Item &item, const QDate &dt);

protected:
    bool event(QEvent *e) override;

    void paintEvent(QPaintEvent *ev) override;

    void mousePressEvent(QMouseEvent *e) override;

    void mouseReleaseEvent(QMouseEvent *e) override;

    void mouseMoveEvent(QMouseEvent *e) override;

    void resizeEvent(QResizeEvent *) override;

    void dragEnterEvent(QDragEnterEvent *e) override;

    void dragMoveEvent(QDragMoveEvent *e) override;

    void dragLeaveEvent(QDragLeaveEvent *e) override;

    void dropEvent(QDropEvent *e) override;

private:
    /**
     * Pop-up a context menu for creating a new Event, To-do, or Journal.
     */
    void popupMenu(const QDate &date, const QDate &date2);

    /** returns the index of the day located at the matrix's widget (x,y) position.
     *
     *  @param x horizontal coordinate
     *  @param y vertical coordinate
     */
    int getDayIndexFrom(int x, int y) const;

    /** calculates a "shaded" color from the supplied color object.
     *  (Copied from Cornelius's kdpdatebutton.cpp)
     *
     *  @param color source based on which a shaded color should be calculated.
     */
    QColor getShadedColor(const QColor &color) const;

    /** updates mEvent list with all days that have events */
    void updateEvents();

    /** updates mEvent list with all days that have to-dos with due date */
    void updateTodos();

    /** updates mEvent list with all days that have journals */
    void updateJournals();

    /** number of days to be displayed. For now there is no support for any
        other number than 42. so change it at your own risk :o) */
    static const int NUMDAYS;

    /** calendar instance to be queried for holidays, events, … */
    QList<Akonadi::CollectionCalendar::Ptr> mCalendars;

    /** starting date of the matrix */
    QDate mStartDate;

    /** array of day labels to optimeize drawing performance. */
    QString *mDayLabels = nullptr;

    /** array of days displayed to reduce memory consumption by
        subsequently calling QDate::addDays(). */
    QDate *mDays = nullptr;

    /** List for storing days which should be drawn using bold font. */
    QList<QDate> mEvents;

    /** stores holiday names of the days shown in the matrix. */
    QMap<int, QString> mHolidays;

    /** index of today or -1 if today is not visible in the matrix. */
    int mToday;

    /** index of day where dragged selection was initiated.
        used to detect "negative" timely selections */
    int mSelInit;

    /** if mSelStart has this value it indicates that there is no
        actual selection in the matrix. */
    static const int NOSELECTION;

    /** index of first selected day. */
    int mSelStart;

    /** index of last selected day. */
    int mSelEnd;

    /** default width of the frame drawn around today if it is visible
        in the matrix. */
    int mTodayMarginWidth;

    /** stores actual size of each day in the widget so we don't need to
     *  ask on every repaint.
     */
    QRect mDaySize;

    /**
     * Indicate pending calendar changes.
     */
    bool mPendingChanges = false;

    /** Whether days with events are highlighted */
    bool mHighlightEvents = false;

    /** Whether days with to-dos (with due date) are highlighted */
    bool mHighlightTodos = false;

    /** Whether days with journals are highlighted */
    bool mHighlightJournals = false;
};
