/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <qapplication.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <kcalendarsystem.h>

#ifndef KORG_NOKALARMD
#include "kalarmdclient.h"
#endif
#include "simplealarmclient.h"

#include "koglobals.h"

class NopAlarmClient : public AlarmClient
{
  public:
    void startDaemon() {}
    bool setCalendars( const QStringList & ) { return false; }
    bool addCalendar( const QString & ) { return false; }
    bool removeCalendar( const QString & ) { return false; }
    bool reloadCalendar( const QString & ) { return false; }
};

KOGlobals *KOGlobals::mSelf = 0;

KOGlobals *KOGlobals::self()
{
  if (!mSelf) {
    mSelf = new KOGlobals;
  }
  
  return mSelf;
}

KOGlobals::KOGlobals()
{
  KConfig *cfg = KOGlobals::config();

  cfg->setGroup("General");
  mCalendarSystem = KGlobal::locale()->calendar();

  cfg->setGroup("AlarmDaemon");
  QString alarmClient = cfg->readEntry( "Daemon", "kalarmd" );
  if ( alarmClient == "simple" ) {
    mAlarmClient = new SimpleAlarmClient;
#ifndef KORG_NOKALARMD
  } else if ( alarmClient == "kalarmd" ) {
    mAlarmClient = new KalarmdClient;
#endif
  } else {
    mAlarmClient = new NopAlarmClient;
  }
}

KConfig* KOGlobals::config()
{
  static KConfig *mConfig = 0;
  if (!mConfig)
    mConfig = new KConfig( locateLocal( "config", "korganizerrc" ) );
  return mConfig;
}

KOGlobals::~KOGlobals()
{
  delete mAlarmClient;
}

const KCalendarSystem *KOGlobals::calendarSystem() const
{
  return mCalendarSystem;
}

AlarmClient *KOGlobals::alarmClient() const
{
  return mAlarmClient;
}

void KOGlobals::fitDialogToScreen( QWidget *wid, bool force )
{
  bool resized = false;

  int w = wid->frameSize().width();
  int h = wid->frameSize().height();  

  if ( w > QApplication::desktop()->size().width() ) {
    w = QApplication::desktop()->size().width();
    resized = true;
  }
  if ( h > QApplication::desktop()->size().height() - 30 ) {
    h = QApplication::desktop()->size().height() - 30;
    resized = true;
  }
  
  if ( resized || force ) {
    wid->resize( w, h );
    wid->move( 0, 15 );
    if ( force ) wid->setFixedSize( w, h );
  }
}

bool KOGlobals::reverseLayout()
{
#if QT_VERSION >= 0x030000
  return QApplication::reverseLayout();
#else
  return false;
#endif
}
