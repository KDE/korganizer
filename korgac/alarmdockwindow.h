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
#ifndef ALARMDOCKWINDOW_H
#define ALARMDOCKWINDOW_H

#include <ksystemtray.h>

#include <qpixmap.h>

class AlarmDockWindow : public KSystemTray
{
    Q_OBJECT
  public:
    AlarmDockWindow( const char *name = 0 );
    virtual ~AlarmDockWindow();

    void enableAutostart( bool enabled );

  public slots:
    void toggleAlarmsEnabled();    
    void toggleAutostart();  
    void slotUpdate( int reminders );

  signals:
    void quitSignal();
    void suspendAllSignal();
    void dismissAllSignal();

  protected:
    void mousePressEvent( QMouseEvent * );
//    void closeEvent( QCloseEvent * );

  protected slots:
    void slotQuit();
    void slotSuspendAll();
    void slotDismissAll();

  private:
    QPixmap mPixmapEnabled;
    QPixmap mPixmapDisabled;

    int mAlarmsEnabledId;
    int mAutostartId;
    int mSuspendAll;
    int mDismissAll;
};

#endif
