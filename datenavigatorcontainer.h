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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef DATENAVIGATORCONTAINER_H
#define DATENAVIGATORCONTAINER_H

class KDateNavigator;

class DateNavigatorContainer: public QWidget
{
    Q_OBJECT
  public:
    DateNavigatorContainer( QWidget *parent = 0, const char *name = 0 );
    ~DateNavigatorContainer();

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar( Calendar * );

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

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
    void resizeEvent( QResizeEvent * );

    void setBaseDates();
    void connectNavigatorView( KDateNavigator *v );

  private:
    KDateNavigator *mNavigatorView;

    KCal::Calendar *mCalendar;

    QPtrList<KDateNavigator> mExtraViews;

    int mHorizontalCount;
    int mVerticalCount;
};

#endif
