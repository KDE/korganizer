/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Eitzenberger Thomas <thomas.eitzenberger@siemens.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KODAYMAT_H
#define _KODAYMAT_H

#include <qstring.h>
#include <qframe.h>
#include <qcolor.h>
#include <qpen.h>
#include <qdatetime.h>
#include <qtooltip.h>
#include <qmap.h>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class KODayMatrix;

/**
 *  small helper class to dynamically show tooltips inside the day matrix.
 *  This class asks the day matrix object for a appropriate label which
 *  is in our special case the name of the holiday or null if this day is no holiday.
 */
class DynamicTip : public QToolTip
{
public:

    /**
     * Constructor that expects a KODayMatrix object as parent.
     *
     * @param parent the parent KODayMatrix control.
     */
    DynamicTip(QWidget* parent );

protected:

    /**
     * Qt's callback to ask the object to provide an approrpiate text for the
     * tooltip to be shown.
     *
     * @param pos coordinates of the mouse.
     */
    void maybeTip( const QPoint & pos);

private:

    /** the parent control this tooltip is designed for. */
    KODayMatrix* matrix;
};

#include <calendarsystem/kcalendarsystem.h>
#include <libkcal/calendar.h>

using namespace KCal;

/**
 *  replacement for kdpdatebuton.cpp that used 42 widgets for the day matrix to be displayed.
 *  Cornelius thought this was a waste of memory and a lot of overhead.
 *  In addition the selection was not very intuitive so I decided to rewrite it using a QFrame
 *  that draws the labels and allows for dragging selection while maintaining nearly full
 *  compatibility in behaviour with its predecessor.
 *
 *  The following functionality has been changed:
 *
 *  o when shifting events in the agenda view from one day to another the day matrix is updated now
 *  o TODO ET dragging an event to the matrix will MOVE not COPY the event to the new date.
 *  o no support for Ctrl+click to create groups of dates
 *    (This has not really been supported in the predecessor. It was not very intuitive nor was it
 *     user friendly.)
 *    This feature has been replaced with dragging a selection on the matrix. The matrix will
 *    automatically choose the appropriate selection (e.g. you are not any longer able to select
 *    two distinct groups of date selections as in the old class)
 *  o now that you can select more then a week it can happen that not all selected days are
 *    displayed in the matrix. However this is preferred to the alternative which would mean to
 *    adjust the selection and leave some days undisplayed while scrolling through the months
 *
 *  @short day matrix widget of the KDateNavigator
 *
 *  @author Eitzenberger Thomas
 */
class KODayMatrix: public QFrame {

    Q_OBJECT

public:

    /** constructor to create a day matrix widget.
     *
     *  @param parent widget that is the parent of the day matrix. Normally this should
     *                be a KDateNavigator
     *  @param calendar instance of a calendar on which all calculations are based
     *  @param date start date of the matrix (is expected to be already fixed). It is
     *              assumed that this date is the first week day to be shown in the matrix.
     *  @param name name of the widget
     */
    KODayMatrix(QWidget *parent, Calendar* calendar, QDate date, const char *name, KCalendarSystem* calSys);

    /** destructor that deallocates all dynamically allocated private members.
     */
    ~KODayMatrix();

    /** updates the day matrix to start with the given date. Does all the necessary
     *  checks for holidays or events on a day and stores them for display later on.
     *  Does NOT update the view visually. Call repaint() for this.
     *
     *  @param actdate recalculates the day matrix to show NUMDAYS starting from this
     *                 date.
     */
    void updateView(QDate actdate);

    /** Recalculates all the flags of the days in the matrix like holidays or events
     *  on a day (Actually calls above method with the actual startdate).
     */
    void updateView();

    /** returns the QDate object associated with day indexed by the
     *  supplied offset.
     */
     const QDate& getDate(int offset);

    /** returns the official name of this holy day or 0 if there is no label
     *  for this day.
     */
    QString getHolidayLabel(int offset);

    /** adds all actual selected days from mSelStart to mSelEnd to the supplied
     *  DateList.
     */
    void addSelectedDaysTo(DateList&);

    /** sets the actual to be displayed selection in the day matrix starting from
     *  start and ending with end. Theview must be manually updated by calling
     *  repaint. (?)
     */
    void setSelectedDaysFrom(const QDate& start, const QDate& end);

/*
public slots:

    void setStartDate(QDate);
*/

signals:

    /** emitted if the user selects a block of days with the mouse by dragging a rectangle
     *  inside the matrix
     *
     *  @param daylist list of days that have been selected by the user
     */
    void selected(const DateList daylist);

    /** emitted if the user has dropped an event inside the matrix
     *
     *  @param event the dropped calendar event
     */
    void eventDropped(Event *event);

protected:

    void paintEvent(QPaintEvent *ev);

    void mousePressEvent (QMouseEvent* e);

    void mouseReleaseEvent (QMouseEvent* e);

    void mouseMoveEvent (QMouseEvent* e);

    void dragEnterEvent(QDragEnterEvent *);

    void dragMoveEvent(QDragMoveEvent *);

    void dragLeaveEvent(QDragLeaveEvent *);

    void dropEvent(QDropEvent *);

    void resizeEvent(QResizeEvent *);

private:

    /** returns the index of the day located at the matrix's widget (x,y) position.
     *
     *  @param x horizontal coordinate
     *  @param y vertical coordinate
     */
    int getDayIndexFrom(int x, int y);

    /** calculates a "shaded" color from the supplied color object.
     *  (Copied from Cornelius's kdpdatebutton.cpp)
     *
     *  @param color source based on which a shaded color should be calculated.
     */
    QColor getShadedColor(QColor color);

    /** number of days to be displayed. For now there is no support for any other number then 42.
        so change it at your own risk :o) */
    static const int NUMDAYS;

    /** calendar instance to be queried for holidays, events, ... */
    Calendar  *mCalendar;

    /** starting date of the matrix */
    QDate     startdate;

    /** array of day labels to optimeize drawing performance. */
    QString   *daylbls;

    /** array of days displayed to reduce memory consumption by
        subsequently calling QDate::addDays(). */
    QDate     *days;

    /** array of storing the number of events on a given day.
      *  used for drawing a bold font if there is at least one event on that day.
      */
    int      *events;

    /** stores holiday names of the days shown in the matrix. */
    QMap<int,QString>  mHolidays;

    /** indey of today or -1 if today is not visible in the matrix. */
    int       today;

    /** index of day where dragged selection was initiated.
        used to detect "negative" timely selections */
    int       mSelInit;

    /** if mSelStart has this value it indicates that there is no
        actual selection in the matrix. */
    static const int NOSELECTION;

    /** index of first selected day. */
    int       mSelStart;

    /** index of last selected day. */
    int       mSelEnd;

    /** dynamic tooltip to handle mouse dependent tips for each day in the matrix. */
    DynamicTip* mToolTip;


    /** default background colour of the matrix. */
    QColor    mDefaultBackColor;

    /** default text color of the matrix. */
    QColor    mDefaultTextColor;

    /** default text color for days not in the actual month. */
    QColor    mDefaultTextColorShaded;

    /** default text color for holidays not in the actual month. */
    QColor    mHolidayColorShaded;

    /** text color for selected days. */
    QColor    mSelectedDaysColor;

    /** default width of the frame drawn around today if it is visible in the matrix. */
    int       mTodayMarginWidth;

    /** stores actual size of each day in the widget so that I dont need to ask this data
     *  on every repaint.
     */
    QRect     daysize;

    KCalendarSystem* mCalendarSystem;
};

#endif
