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
#include <kstaticdeleter.h>
#include <kiconloader.h>

#include <kcalendarsystem.h>
#include <kholidays.h>

#include "alarmclient.h"

#include "koglobals.h"
#include "koprefs.h"
#include "korganizer_part.h"

class NopAlarmClient : public AlarmClient
{
  public:
    void startDaemon() {}
    void stopDaemon() {}
};

KOGlobals *KOGlobals::mSelf = 0;

static KStaticDeleter<KOGlobals> koGlobalsDeleter;

KOGlobals *KOGlobals::self()
{
  if ( !mSelf ) {
    koGlobalsDeleter.setObject( mSelf, new KOGlobals );
  }

  return mSelf;
}

KOGlobals::KOGlobals()
  : mHolidays(0)
{
  // Needed to distinguish from global KInstance
  // in case we are a KPart
  mOwnInstance = new KInstance( "korganizer" );
  mOwnInstance->config()->setGroup( "General" );

  mAlarmClient = new AlarmClient;
}

KConfig* KOGlobals::config() const
{
  return mOwnInstance->config();
}

KOGlobals::~KOGlobals()
{
  delete mAlarmClient;
  delete mOwnInstance;
  delete mHolidays;
}

const KCalendarSystem *KOGlobals::calendarSystem() const
{
  return KGlobal::locale()->calendar();
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

  QRect desk = KGlobalSettings::desktopGeometry( wid );
  if ( w > desk.width() ) {
    w = desk.width();
    resized = true;
  }
  // FIXME: ugly hack.  Is the -30 really to circumvent the size of kicker?!
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

QPixmap KOGlobals::smallIcon( const QString& name )
{
  return SmallIcon( name, mOwnInstance );
}

QIconSet KOGlobals::smallIconSet( const QString& name, int size )
{
  return SmallIconSet( name, size, mOwnInstance );
}

QString KOGlobals::holiday( const QDate &date )
{
  if ( mHolidays ) return mHolidays->shortText( date );
  else return QString::null;
}

bool KOGlobals::isWorkDay( const QDate &date )
{
  int mask( ~( KOPrefs::instance()->mWorkWeekMask ) );

  bool nonWorkDay = ( mask & ( 1 << ( date.dayOfWeek() - 1 ) ) );

  nonWorkDay = nonWorkDay
               || ( KOPrefs::instance()->mExcludeHolidays
                    && ( mHolidays
                         && ( mHolidays->category( date ) == KHolidays::HOLIDAY ) ) );

  return !nonWorkDay;
}

void KOGlobals::setHolidays( KHolidays *h )
{
  delete mHolidays;
  mHolidays = h;
}

KHolidays *KOGlobals::holidays() const
{
  return mHolidays;
}
