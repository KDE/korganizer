/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KDATENAVIGATOR_H
#define KDATENAVIGATOR_H

#include <qframe.h>
#include <qdatetime.h>

#include <libkcal/incidencebase.h>

class QPushButton;
class QLabel;

class KCalendarSystem;
namespace KCal {
class Calendar;
class Incidence;
}
class NavigatorBar;
using namespace KCal;

class KODayMatrix;

class KDateNavigator: public QFrame
{
    Q_OBJECT
  public:
    KDateNavigator( QWidget *parent = 0, const char *name = 0 );
    ~KDateNavigator();

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar( Calendar * );

    void setBaseDate( const QDate & );

    KCal::DateList selectedDates() const { return mSelectedDates; }

    QSizePolicy sizePolicy () const;

    NavigatorBar *navigatorBar() const { return mNavigatorBar; }

  public slots:
    void selectDates( const KCal::DateList & );
    void updateView();
    void updateConfig();
    void updateDayMatrix();
    void updateToday();

  signals:
    void datesSelected( const KCal::DateList & );
    void incidenceDropped( Incidence *, const QDate & );
    void incidenceDroppedMove( Incidence *, const QDate & );
    void weekClicked( const QDate &);

    void goPrevious();
    void goNext();

    void goNextMonth();
    void goPrevMonth();
    void goNextYear();
    void goPrevYear();

    void goMonth( int month );

  protected:
    void updateDates();

    void wheelEvent( QWheelEvent * );

    bool eventFilter( QObject *,QEvent * );

    void setShowWeekNums( bool enabled );

  private:
    NavigatorBar *mNavigatorBar;

    QFrame *headingSep;
    QFrame *weeknumSep;
    QLabel *headings[ 7 ];
    QLabel *weeknos[ 7 ];

    KODayMatrix *mDayMatrix;

    KCal::DateList mSelectedDates;
    QDate mBaseDate;

    KCalendarSystem *mCalendarSystem;

    const QString *curHeaders;

    // Disabling copy constructor and assignment operator
    KDateNavigator( const KDateNavigator & );
    KDateNavigator &operator=( const KDateNavigator & );
};

#endif
