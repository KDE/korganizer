/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koprefs.h"
#include "kocore.h"

#include "categoryconfig.h"

#include <kmime/kmime_header_parsing.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstringhandler.h>
#include <ksystemtimezone.h>

#include <QDir>
#include <QString>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QStringList>

#include <time.h>
#include <unistd.h>

using namespace IncidenceEditors;
using namespace KPIMIdentities;

class KOPrefsPrivate {
  public:
    KOPrefsPrivate() : prefs( new KOPrefs ) {}
    ~KOPrefsPrivate() { delete prefs; }
    KOPrefs* prefs;
};

K_GLOBAL_STATIC( KOPrefsPrivate, sInstance )

KOPrefs::KOPrefs() : KOPrefsBase()
{
  mDefaultCategoryColor = QColor( 151, 235, 121 );
  mDefaultResourceColor = QColor(); //Default is a color invalid

  mDefaultAgendaTimeLabelsFont = KGlobalSettings::generalFont();
  // make a large default time bar font, at least 16 points.
  mDefaultAgendaTimeLabelsFont.setPointSize(
    qMax( mDefaultAgendaTimeLabelsFont.pointSize() + 4, 16 ) );

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize(
    qMax( mDefaultMonthViewFont.pointSize() - 2, 6 ) );

  KConfigSkeleton::setCurrentGroup( "General" );

  addItemPath( "Html Export File", mHtmlExportFile,
      QDir::homePath() + '/' + i18nc( "Default export file", "calendar.html" ) );

  agendaTimeLabelsFontItem()->setDefaultValue( mDefaultAgendaTimeLabelsFont );
  monthViewFontItem()->setDefaultValue( mDefaultMonthViewFont );
}

KOPrefs::~KOPrefs()
{
  kDebug();
}

KOPrefs *KOPrefs::instance()
{
  if ( !sInstance.exists() ) {
    sInstance->prefs->readConfig();
  }

  return sInstance->prefs;
}

void KOPrefs::usrSetDefaults()
{
  setAgendaTimeLabelsFont( mDefaultAgendaTimeLabelsFont );
  setMonthViewFont( mDefaultMonthViewFont );

  KConfigSkeleton::usrSetDefaults();
}

void KOPrefs::usrReadConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  // Note that the [Category Colors] group was removed after 3.2 due to
  // an algorithm change. That's why we now use [Category Colors2]

  // Category colors
  KConfigGroup colorsConfig( config(), "Category Colors2" );
  CategoryConfig cc( this );
  const QStringList cats = cc.customCategories();
  Q_FOREACH( const QString& i, cats ) {
    QColor c = colorsConfig.readEntry( i, mDefaultCategoryColor );
    if ( c != mDefaultCategoryColor ) {
      setCategoryColor( i, c );
    }
  }

  // Resource colors
  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  const QStringList colorKeyList = rColorsConfig.keyList();

  QStringList::ConstIterator it3;
  for ( it3 = colorKeyList.begin(); it3 != colorKeyList.end(); ++it3 ) {
    QColor color = rColorsConfig.readEntry( *it3, mDefaultResourceColor );
    //kDebug() << "key:" << (*it3) << "value:" << color;
    setResourceColor( *it3, color );
  }

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  setTimeScaleTimezones( timeScaleConfig.readEntry( "Timescale Timezones", QStringList() ) );

  KConfigSkeleton::usrReadConfig();
}

void KOPrefs::usrWriteConfig()
{
  KConfigGroup generalConfig( config(), "General" );

  KConfigGroup colorsConfig( config(), "Category Colors2" );
  QHash<QString, QColor>::const_iterator i = mCategoryColors.constBegin();
  while ( i != mCategoryColors.constEnd() ) {
    colorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  i = mResourceColors.constBegin();
  while ( i != mResourceColors.constEnd() ) {
    rColorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

#if 0
  if ( mRememberRetrievePw ) {
    config()->writeEntry( "Retrieve Server Password",
                          KStringHandler::obscure( mRetrievePassword ) );
  } else {
    config()->deleteEntry( "Retrieve Server Password" );
  }
#endif

  KConfigGroup timeScaleConfig( config(), "Timescale" );
  timeScaleConfig.writeEntry( "Timescale Timezones", timeScaleTimezones() );

  KConfigSkeleton::usrWriteConfig();
}

void KOPrefs::setCategoryColor( const QString &cat, const QColor &color )
{
  mCategoryColors.insert( cat, color );
}

QColor KOPrefs::categoryColor( const QString &cat ) const
{
  QColor color;

  if ( !cat.isEmpty() ) {
    color = mCategoryColors.value( cat );
  }

  if ( color.isValid() ) {
    return color;
  } else {
    return mDefaultCategoryColor;
  }
}

bool KOPrefs::hasCategoryColor( const QString &cat ) const
{
    return mCategoryColors[ cat ].isValid();
}

void KOPrefs::setResourceColor ( const QString &cal, const QColor &color )
{
  // kDebug() << cal << "color:" << color.name();
  mResourceColors.insert( cal, color );
}

QColor KOPrefs::resourceColor( const QString &cal )
{
  QColor color;
  if ( !cal.isEmpty() ) {
    if ( mResourceColors.contains( cal ) ) {
      color = mResourceColors.value( cal );
      if ( !color.isValid() )
        return color;
    }
  } else {
    return mDefaultResourceColor;
  }

  // assign default color if enabled
  if ( !cal.isEmpty() && !color.isValid() && assignDefaultResourceColors() ) {
    QColor defColor( 0x37, 0x7A, 0xBC );
    if ( defaultResourceColorSeed() > 0 &&
         defaultResourceColorSeed() - 1 < (int)defaultResourceColors().size() ) {
        defColor = QColor( defaultResourceColors()[defaultResourceColorSeed()-1] );
    } else {
        int h, s, v;
        defColor.getHsv( &h, &s, &v );
        h = ( defaultResourceColorSeed() % 12 ) * 30;
        s -= s * static_cast<int>( ( ( defaultResourceColorSeed() / 12 ) % 2 ) * 0.5 );
        defColor.setHsv( h, s, v );
    }
    setDefaultResourceColorSeed( defaultResourceColorSeed() + 1 );
    setResourceColor( cal, defColor );
    color = mResourceColors[cal];
  }

  if ( color.isValid() ) {
    return color;
  } else {
    return mDefaultResourceColor;
  }
}

QStringList KOPrefs::timeScaleTimezones() const
{
  return mTimeScaleTimeZones;
}

void KOPrefs::setTimeScaleTimezones( const QStringList &list )
{
  mTimeScaleTimeZones = list;
}

