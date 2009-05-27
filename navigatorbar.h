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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef NAVIGATORBAR_H
#define NAVIGATORBAR_H

#include <kcal/incidencebase.h>

#include <QLabel>
#include <QMouseEvent>

class QToolButton;

class ActiveLabel : public QLabel
{
  Q_OBJECT
  public:
    explicit ActiveLabel( QWidget *parent );

  signals:
    void clicked();

  protected:
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void mouseReleaseEvent ( QMouseEvent * );
};

class NavigatorBar: public QWidget
{
  Q_OBJECT
  public:
    explicit NavigatorBar( QWidget *parent = 0 );
    ~NavigatorBar();

    void showButtons( bool left, bool right );

  public slots:
    void selectDates( const KCal::DateList & );

  signals:
    void goNextMonth();
    void goPrevMonth();
    void goNextYear();
    void goPrevYear();
    void goMonth( int month );

  protected:
    QToolButton *createNavigationButton( const QString &icon, const QString &toolTip );

  private slots:
    void selectMonth();

  private:
    bool mHasMinWidth;

    QDate mDate;

    QToolButton *mPrevYear;
    QToolButton *mPrevMonth;
    ActiveLabel *mMonth;
    QToolButton *mNextMonth;
    QToolButton *mNextYear;
};

#endif
