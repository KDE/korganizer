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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koglobals.h"
#include "koprefs.h"

#include <KHolidays/kholidays/HolidayRegion>

#include <KGlobal>
#include <KIconLoader>

#include <QApplication>

class KOGlobalsSingletonPrivate
{
  public:
    KOGlobals instance;
};

K_GLOBAL_STATIC( KOGlobalsSingletonPrivate, sKOGlobalsSingletonPrivate )

KOGlobals *KOGlobals::self()
{
  return &sKOGlobalsSingletonPrivate->instance;
}

KOGlobals::KOGlobals()
  : mOwnInstance( "korganizer" ), mHolidays( 0 )
{
  KIconLoader::global()->addAppDir( QLatin1String("kdepim") );
}

KConfig *KOGlobals::config() const
{
  KSharedConfig::Ptr c = mOwnInstance.config();
  return c.data();
}

KOGlobals::~KOGlobals()
{
  delete mHolidays;
}

const KCalendarSystem *KOGlobals::calendarSystem() const
{
  return KGlobal::locale()->calendar();
}

bool KOGlobals::reverseLayout()
{
  return QApplication::isRightToLeft();
}

QPixmap KOGlobals::smallIcon( const QString &name ) const
{
  return SmallIcon( name );
}

QMap<QDate,QStringList> KOGlobals::holiday( const QDate &start, const QDate &end ) const
{
  QMap<QDate,QStringList> holidaysByDate;

  if ( !mHolidays ) {
    return holidaysByDate;
  }

  const KHolidays::Holiday::List list = mHolidays->holidays( start, end );
  for ( int i = 0; i < list.count(); ++i ) {
    const KHolidays::Holiday &h = list.at( i );
    holidaysByDate[h.date()].append( h.text() );
  }
  return holidaysByDate;
}

QList<QDate> KOGlobals::workDays( const QDate &startDate,
                                  const QDate &endDate ) const
{
  QList<QDate> result;

  const int mask( ~( KOPrefs::instance()->mWorkWeekMask ) );
  const int numDays = startDate.daysTo( endDate ) + 1;

  for ( int i = 0; i < numDays; ++i ) {
    const QDate date = startDate.addDays( i );
    if ( !( mask & ( 1 << ( date.dayOfWeek() - 1 ) ) ) ) {
      result.append( date );
    }
  }

  if ( mHolidays && KOPrefs::instance()->mExcludeHolidays ) {
    const KHolidays::Holiday::List list = mHolidays->holidays( startDate, endDate );
    for ( int i = 0; i < list.count(); ++i ) {
      const KHolidays::Holiday &h = list.at( i );
      const QString dateString = h.date().toString();
      if ( h.dayType() == KHolidays::Holiday::NonWorkday ) {
        result.removeAll( h.date() );
      }
    }
  }

  return result;
}

int KOGlobals::getWorkWeekMask()
{
  return KOPrefs::instance()->mWorkWeekMask;
}

void KOGlobals::setHolidays( KHolidays::HolidayRegion *h )
{
  delete mHolidays;
  mHolidays = h;
}

KHolidays::HolidayRegion *KOGlobals::holidays() const
{
  return mHolidays;
}
