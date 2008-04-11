/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef TIMELABELS_H
#define TIMELABELS_H

#include <qscrollview.h>
#include <qlabel.h>

class QHBox;
class QPushButton;
class QBoxLayout;

class KOAgenda;
class KOAgendaItem;
class KConfig;

class TimeLabels : public QScrollView
{
    Q_OBJECT
  public:
    TimeLabels( int rows, QWidget *parent = 0, const char *name = 0,
                WFlags f = 0 );

    /** Calculates the minimum width */
    virtual int minimumWidth() const;

    /** updates widget's internal state */
    void updateConfig();

    /**  */
    void setAgenda( KOAgenda *agenda );

    /**  */
    virtual void paintEvent( QPaintEvent *e );

  public slots:
    /** update time label positions */
    void positionChanged();
    void positionChanged( int pos );

  protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );

  private slots:
    /** update the position of the marker showing the mouse position */
    void mousePosChanged(const QPoint &pos);

    void showMousePos();
    void hideMousePos();

    void setCellHeight( double height );

  private:
    int mRows;
    double mCellHeight;
    int mMiniWidth;
    KOAgenda* mAgenda;

    QFrame *mMousePos;  // shows a marker for the current mouse position in y direction
};

#endif
