/*
    This file is part of KOrganizer.

    Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <qapplication.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include <kcalendarsystem.h>

#include "alarmclient.h"

#include "koglobals.h"

class NopAlarmClient : public AlarmClient
{
  public:
    void startDaemon() {}
    void stopDaemon() {}
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

  mAlarmClient = new AlarmClient;
}

KConfig* KOGlobals::config()
{
  static KConfig *mConfig = 0;
  if ( !mConfig )
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

  QRect desk = KGlobalSettings::desktopGeometry(wid);
  if ( w > desk.width() ) {
    w = desk.width();
    resized = true;
  }
  // Yuck this hack is ugly.  Is the -30 really to circumvent the size of
  // kicker?!
  if ( h > desk.height() - 30 ) {
    h = desk.height() - 30;
    resized = true;
  }
  
  if ( resized || force ) {
    wid->resize( w, h );
    wid->move( desk.x(), desk.y()+15 );
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
