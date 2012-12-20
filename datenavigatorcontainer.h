/*
  This file is part of KOrganizer.

  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  Author: Sergio Martins <sergio@kdab.com>

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

#ifndef KORG_DATENAVIGATORCONTAINER_H
#define KORG_DATENAVIGATORCONTAINER_H

#include <Akonadi/Calendar/ETMCalendar>

#include <QFrame>

class KDateNavigator;

class DateNavigatorContainer: public QFrame
{
  Q_OBJECT
  public:
    explicit DateNavigatorContainer( QWidget *parent = 0 );
    ~DateNavigatorContainer();

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar( const Akonadi::ETMCalendar::Ptr & );

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void setHighlightMode( bool highlightEvents,
                           bool highlightTodos,
                           bool highlightJournals ) const;
    void setUpdateNeeded();

    /**
       Returns the month of the specified KDateNavigator.
       The first navigatorIndex is 0.
       Returns a QDate instead of uint so it can be easily feed to KCalendarSystem's
       functions.
       An invalid QDate() is returned if the index is too big or too small.
    */
    QDate monthOfNavigator( int navigatorIndex = 0 ) const;
  public slots:

    /**
       preferredMonth is useful when the datelist crosses months, if different
       from -1, it has the month that the kdatenavigator should show in case
       of ambiguity
    */
    void selectDates( const KCalCore::DateList &, const QDate &preferredMonth = QDate() );

    void updateView();
    void updateConfig();
    void updateDayMatrix();
    void updateToday();

    void goPrevMonth();
    void goNextMonth();

  signals:
    void datesSelected( const KCalCore::DateList &, const QDate &preferredMonth );
    void incidenceDropped( const Akonadi::Item &, const QDate & );
    void incidenceDroppedMove( const Akonadi::Item &, const QDate & );
    void newEventSignal( const QDate & );
    void newTodoSignal( const QDate & );
    void newJournalSignal( const QDate & );

    /**
     * @param preferredMonth Holds the month that should be selected when the
     * week crosses months. It's a QDate instead of uint so it can be easily
     * fed to KCalendarSystem's functions.
     */
    void weekClicked( const QDate &week, const QDate &preferredMonth );

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
    void prevMonthClicked( const QDate &currentMonth,
                           const QDate &selectionLowerLimit,
                           const QDate &selectionUpperLimit );

    void nextMonthClicked( const QDate &currentMonth,
                           const QDate &selectionLowerLimit,
                           const QDate &selectionUpperLimit );

    void monthSelected( int month );

    void yearSelected( int year );

  protected:
    void resizeEvent( QResizeEvent * );
    void setBaseDates( const QDate &start );
    void connectNavigatorView( KDateNavigator *v );

  protected slots:
    /**
     * Resizes all the child elements after the size of the widget changed.
     * This slot is called by a QTimer::singleShot from resizeEvent.
     * This makes the UI seem more responsive, since the other parts
     * of the splitter are resized earlier now.
     */
    void resizeAllContents();

  private slots:
    void handleDatesSelectedSignal( const KCalCore::DateList & );
    void handleWeekClickedSignal( const QDate &, const QDate & );

  private:
    /* Returns the first day of the first KDateNavigator, and the last day
       of the last KDateNavigator.

       @param monthOffset If you have two KDateNavigators displaying
       January and February and want to know the boundaries of,
       for e.g. displaying February and March, use monthOffset = 1.
    */
    QPair<QDate,QDate> dateLimits( int monthOffset = 0 ) const;

    /**
     * Returns the first KDateNavigator that displays date, or 0 if
     * no KDateNavigator displays it.
     */
    KDateNavigator *firstNavigatorForDate( const QDate &date ) const;

    KDateNavigator *mNavigatorView;

    Akonadi::ETMCalendar::Ptr mCalendar;

    QList<KDateNavigator*> mExtraViews;

    int mHorizontalCount;
    int mVerticalCount;

    bool mIgnoreNavigatorUpdates;
};

#endif
