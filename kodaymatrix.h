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
*/
#ifndef _KODAYMAT_H
#define _KODAYMAT_H

#include <qstring.h>
#include <qframe.h>
#include <qcolor.h>
#include <qpen.h>
#include <qdatetime.h>
#include <qtooltip.h>

class KODayMatrix;

class DynamicTip : public QToolTip
{
public:
    DynamicTip(QWidget* parent );
protected:
    void maybeTip( const QPoint & );
private:
    KODayMatrix* matrix;
};

#include <libkcal/calendar.h>

using namespace KCal;

/** replacement for kdpdatebuton.cpp that used 42 widgets for the day matrix to be displayed.
    Cornelius thought this was a waste of memory and a lot of overhead.
    In addition the selection was not very intuitive so I decided to rewrite it using a QFrame
    that draws the labels and allows for dragging selection while maintaining nearly full
    compatibility in behaviour with its predecessor.

    The following functionality has been changed:

    o when shifting events in the agenda view from one day to another the day matrix is updated now
    o TODO ET dragging an event to the matrix will MOVE not COPY the event to the new date.
    o no support for Ctrl+click to create groups of dates
      (This has not really been supported in the predecessor. It was not very intuitive nor was it
       user friendly.)
      This feature has been replaced with dragging a selection on the matrix. The matrix will
      automatically choose the appropriate selection (e.g. you are not any longer able to select
      two distinct groups of date selections as in the old class)
    TODO ET continue....
*/
class KODayMatrix: public QFrame {

    Q_OBJECT

public:

    KODayMatrix(QWidget *parent, Calendar* calendar, QDate date, const char *name);

    ~KODayMatrix();

    void updateView(QDate actdate);

    void updateView();

    void paintEvent(QPaintEvent *ev);

    const QDate& getDate(int offset);

    const QString* getHolidayLabel(int offset);

    void addSelectedDaysTo(DateList&);

    void setSelectedDaysFrom(const QDate& start, const QDate& end);

public slots:

    void setStartDate(QDate);

signals:

    void selected(const DateList);

    void eventDropped(Event *);

protected:

    void mousePressEvent (QMouseEvent* e);

    void mouseReleaseEvent (QMouseEvent* e);

    void mouseMoveEvent (QMouseEvent* e);

    void dragEnterEvent(QDragEnterEvent *);
    
    void dragMoveEvent(QDragMoveEvent *);

    void dragLeaveEvent(QDragLeaveEvent *);

    void dropEvent(QDropEvent *);

private:

    int getDayIndexFrom(int x, int y);

    QColor getShadedColor(QColor color);

    /** number of days to be displayed. For now there is no support for any other number then 42.
        so change it at your own risk :o) */
    static const int NUMDAYS = 42;

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
    QString   **holidays;

    /** indey of today or -1 if today is not visible in the matrix. */
    int       today;

    /** index of day where dragged selection was initiated.
        used to detect "negative" timely selections */
    int       mSelInit;

    /** index of first selected day. */
    int       mSelStart;

    /** index of last selected day. */
    int       mSelEnd;

    /** dynamic tooltip to handle mouse dependent tips for each day in the matrix. */
    DynamicTip* mToolTip;



    /** pen to be used to draw rectangle around today label.
        created on the fly in paintEvent(). */
    QPen      *mTodayPen;

    /** default background colour of the matrix. */
    QColor    mDefaultBackColor;

    /** default text color of the matrix. */
    QColor    mDefaultTextColor;

    /** default text color for days not in the actual month. */
    QColor    mDefaultTextColorShaded;

    /** default text color for holidays not in the actual month. */
    QColor    mHolidayColorShaded;

    /** default width of the frame drawn around today if it is visible in the matrix. */
    int       mTodayMarginWidth;
};

#endif
