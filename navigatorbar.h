/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef NAVIGATORBAR_H
#define NAVIGATORBAR_H

#include <libkcal/incidencebase.h>

#include <qwidget.h>

class QPushButton;
class QFrame;
class QLabel;

class NavigatorBar: public QWidget
{
    Q_OBJECT
  public:
    NavigatorBar( const QDate & date, QWidget *parent = 0, const char *name = 0 );
    ~NavigatorBar();

  public slots:
    void selectDates( const KCal::DateList & );

  signals:
    void goNextMonth();
    void goPrevMonth();
    void goNextYear();
    void goPrevYear();

  private:
    QFrame *mCtrlFrame;

    QPushButton *mPrevYear;
    QPushButton *mPrevMonth;
    QPushButton *mNextMonth;
    QPushButton *mNextYear;

    QLabel *mDateLabel;
};

#endif
