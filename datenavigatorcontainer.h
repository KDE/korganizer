/*
  This file is part of KOrganizer.

  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef DATENAVIGATORCONTAINER_H
#define DATENAVIGATORCONTAINER_H

#include <kcal/calendar.h>
using namespace KCal;

#include <QFrame>
#include <QList>

class KDateNavigator;
class QResizeEvent;

class DateNavigatorContainer: public QFrame
{
  Q_OBJECT
  public:
    DateNavigatorContainer( QWidget *parent = 0 );
    ~DateNavigatorContainer();

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar( Calendar * );

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

  public slots:
    void selectDates( const KCal::DateList & );
    void selectNextMonth();
    void selectPreviousMonth();
    void updateView();
    void updateConfig();
    void updateDayMatrix();
    void updateToday();

  signals:
    void datesSelected( const KCal::DateList & );
    void incidenceDropped( Incidence *, const QDate & );
    void incidenceDroppedMove( Incidence *, const QDate & );
    void newEventSignal( const QDate & );
    void newTodoSignal( const QDate & );
    void newJournalSignal( const QDate & );
    void weekClicked( const QDate &);

    void goPrevious();
    void goNext();

    void goNextMonth();
    void goPrevMonth();
    void goNextYear();
    void goPrevYear();

    void goMonth( int month );
    void goYear( int year );

  protected:
    void resizeEvent( QResizeEvent * );
    void setBaseDates( const QDate &start );
    void connectNavigatorView( KDateNavigator *v );
  protected slots:
    /** Resizes all the child elements after the size of the widget
        changed. This slot is called by a QTimer::singleShot from
        resizeEvent. This makes the UI seem more responsive, since
        the other parts of the splitter are resized earlier now */
    void resizeAllContents();

  private:

    KDateNavigator *mNavigatorView;

    KCal::Calendar *mCalendar;

    QList<KDateNavigator*> mExtraViews;

    int mHorizontalCount;
    int mVerticalCount;

    bool mIgnoreNavigatorUpdates;
};

#endif
