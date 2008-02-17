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

#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>

#include <kpimutils/email.h>
#include <kabc/stdaddressbook.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <k3staticdeleter.h>
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

KOPrefs *KOPrefs::mInstance = 0;
static K3StaticDeleter<KOPrefs> insd;

QColor getTextColor( const QColor &c )
{
  float luminance = ( c.red() * 0.299 ) + ( c.green() * 0.587 ) + ( c.blue() * 0.114 );
  return ( luminance > 128.0 ) ? QColor( 0, 0, 0 ) : QColor( 255, 255, 255 );
}

KOPrefs::KOPrefs() :
  KOPrefsBase()
{
  mDefaultCategoryColor = QColor( 151, 235, 121 );
  mDefaultResourceColor = QColor();//Default is a color invalid

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
  agendaCalendarItemsEventsBackgroundColorItem()->setDefaultValue( mDefaultCategoryColor );
}

KOPrefs::~KOPrefs()
{
  kDebug(5850) <<"KOPrefs::~KOPrefs()";
}

KOPrefs *KOPrefs::instance()
{
  if ( !mInstance ) {
    insd.setObject( mInstance, new KOPrefs() );
    mInstance->readConfig();
  }

  return mInstance;
}

void KOPrefs::usrSetDefaults()
{
  // Default should be set a bit smarter, respecting username and locale
  // settings for example.

  KEMailSettings settings;
  QString tmp = settings.getSetting( KEMailSettings::RealName );
  if ( !tmp.isEmpty() ) {
    setUserName( tmp );
  }
  tmp = settings.getSetting( KEMailSettings::EmailAddress );
  if ( !tmp.isEmpty() ) {
    setUserEmail( tmp );
  }
  fillMailDefaults();

  setAgendaTimeLabelsFont( mDefaultAgendaTimeLabelsFont );
  setMonthViewFont( mDefaultMonthViewFont );

  setTimeZoneDefault();

  KPimPrefs::usrSetDefaults();
}

void KOPrefs::fillMailDefaults()
{
  userEmailItem()->swapDefault();
  QString defEmail = userEmailItem()->value();
  userEmailItem()->swapDefault();

  if ( userEmail() == defEmail ) {
    // No korg settings - but maybe there's a kcontrol[/kmail] setting available
    KEMailSettings settings;
    if ( !settings.getSetting( KEMailSettings::EmailAddress ).isEmpty() ) {
      mEmailControlCenter = true;
    }
  }
}

void KOPrefs::setTimeZoneDefault()
{
  KTimeZone zone = KSystemTimeZones::local();
  if ( !zone.isValid() ) {
    kError() <<"KSystemTimeZones::local() return 0";
    return;
  }

  kDebug (5850) <<"----- time zone:" << zone.name();

  mTimeSpec = zone;
}

KDateTime::Spec KOPrefs::timeSpec()
{
  if (!mTimeSpec.isValid()) {
    // Read time zone from config file
    mTimeSpec = KPimPrefs::timeSpec();
  }
  return mTimeSpec;
}

void KOPrefs::setTimeSpec( const KDateTime::Spec &spec )
{
  mTimeSpec = spec;
}

void KOPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();

  mCustomCategories
    << i18nc( "incidence category: appointment", "Appointment" )
    << i18nc( "incidence category: ", "Business" )
    << i18nc( "incidence category", "Meeting" )
    << i18nc( "incidence category: phone call","Phone Call" )
    << i18nc( "incidence category", "Education" )
    << i18nc( "incidence category", "Holiday" )
    << i18nc( "incidence category", "Vacation" )
    << i18nc( "incidence category", "Special Occasion" )
    << i18nc( "incidence category", "Personal" )
    << i18nc( "incidence category", "Travel" )
    << i18nc( "incidence category", "Miscellaneous" )
    << i18nc( "incidence category", "Birthday" );
}

void KOPrefs::usrReadConfig()
{
  KConfigGroup generalConfig( config(), "General" );
  mCustomCategories = generalConfig.readEntry( "Custom Categories", QStringList() );
  if ( mCustomCategories.isEmpty() ) {
    setCategoryDefaults();
  }

  // old category colors, ignore if they have the old default
  // should be removed a few versions after 3.2...
  KConfigGroup colorsConfig( config(), "Category Colors");
  QList<QColor> oldCategoryColors;
  QStringList::Iterator it;
  for ( it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    QColor c = colorsConfig.readEntry( *it, mDefaultCategoryColor );
    oldCategoryColors.append( ( c == QColor( 196, 196, 196 ) ) ?
                              mDefaultCategoryColor : c );
  }

  // new category colors
  KConfigGroup colors2Config( config(), "Category Colors2");
  QList<QColor>::Iterator it2;
  for (it = mCustomCategories.begin(), it2 = oldCategoryColors.begin();
       it != mCustomCategories.end(); ++it, ++it2 ) {
      QColor c = config()->group(QString()).readEntry(*it, *it2);
      if ( c != mDefaultCategoryColor )
          setCategoryColor(*it,c);
  }

  KConfigGroup rColorsConfig( config(), "Resources Colors");
  const QStringList colorKeyList = rColorsConfig.keyList();

  QStringList::ConstIterator it3;
  for ( it3 = colorKeyList.begin(); it3 != colorKeyList.end(); ++it3 ) {
    QColor color = rColorsConfig.readEntry( *it3, mDefaultResourceColor );
    kDebug(5850)<<"KOPrefs::usrReadConfig: key:" << (*it3) << "value:"
                << color;
    setResourceColor( *it3, color );
  }

  if (!mTimeSpec.isValid()) {
    setTimeZoneDefault();
  }

#if 0
  config()->setGroup("FreeBusy");
  if ( mRememberRetrievePw ) {
    mRetrievePassword =
      KStringHandler::obscure( config()->readEntry( "Retrieve Server Password" ) );
  }
#endif
  KConfigGroup timeScaleConfig( config(), "Timescale" );
  setTimeScaleTimezones( timeScaleConfig.readEntry( "Timescale Timezones", QStringList() ) );


  KPimPrefs::usrReadConfig();
  fillMailDefaults();
}

void KOPrefs::usrWriteConfig()
{
  KConfigGroup generalConfig( config(), "General");
  generalConfig.writeEntry( "Custom Categories", mCustomCategories );

  KConfigGroup colors2Config( config(), "Category Colors2");
  QHash<QString, QColor>::const_iterator i = mCategoryColors.constBegin();
  while ( i != mCategoryColors.constEnd() ) {
    colors2Config.writeEntry( i.key(), i.value() );
    ++i;
  }

  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  i = mResourceColors.constBegin();
  while (i != mResourceColors.constEnd()) {
    rColorsConfig.writeEntry( i.key(), i.value() );
    ++i;
  }

  if ( !mFreeBusyPublishSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyPublishPasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }
  if ( !mFreeBusyRetrieveSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyRetrievePasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
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

  KPimPrefs::usrWriteConfig();
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


bool KOPrefs::hasCategoryColor( const QString& cat ) const
{
    return mCategoryColors[ cat ].isValid();
}

void KOPrefs::setResourceColor ( const QString &cal, const QColor &color )
{
  kDebug(5850)<<"KOPrefs::setResourceColor:" << cal << "color:" <<
    color.name();
  mResourceColors.insert( cal, color );
}

QColor KOPrefs::resourceColor( const QString &cal )
{
  QColor color;
  if ( !cal.isEmpty() ) {
    color = mResourceColors.value( cal );
  }

  // assign default color if enabled
  if ( !cal.isEmpty() && !color.isValid() && assignDefaultResourceColors() ) {
    QColor defColor( 0x37, 0x7A, 0xBC );
    if ( defaultResourceColorSeed() > 0 && defaultResourceColorSeed() - 1 < (int)defaultResourceColors().size() ) {
        defColor = QColor( defaultResourceColors()[defaultResourceColorSeed()-1] );
    } else {
        int h, s, v;
        defColor.getHsv( &h, &s, &v );
        h = ( defaultResourceColorSeed() % 12 ) * 30;
        s -= s * ( (defaultResourceColorSeed() / 12) % 2 ) * 0.5;
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

QString KOPrefs::fullName()
{
  if ( mEmailControlCenter ) {
    KEMailSettings settings;
    return settings.getSetting( KEMailSettings::RealName );
  } else {
    return userName();
  }
}

QString KOPrefs::email()
{
  if ( mEmailControlCenter ) {
    KEMailSettings settings;
    return settings.getSetting( KEMailSettings::EmailAddress );
  } else {
    return userEmail();
  }
}

QStringList KOPrefs::allEmails()
{
  // Grab emails from the email identities
  kDebug(5850)<<" KOCore::self()->identityManager() :"<<KOCore::self()->identityManager();
  QStringList lst = KOCore::self()->identityManager()->allEmails();
  // Add emails configured in korganizer
  lst += mAdditionalMails;
  // Add emails from the user's kaddressbook entry
  lst += KABC::StdAddressBook::self( true )->whoAmI().emails();
  // Add the email entered as the userEmail here
  lst += email();

  // Warning, this list could contain duplicates.
  return lst;
}

QStringList KOPrefs::fullEmails()
{
  QStringList fullEmails;
  // The user name and email from the config dialog:
  fullEmails << QString("%1 <%2>").arg( fullName() ).arg( email() );

  QStringList::Iterator it;
  // Grab emails from the email identities
  KPIMIdentities::IdentityManager *idmanager = KOCore::self()->identityManager();
  QStringList lst = idmanager->identities();
  KPIMIdentities::IdentityManager::ConstIterator it1;
  for ( it1 = idmanager->begin() ; it1 != idmanager->end() ; ++it1 ) {
    fullEmails << (*it1).fullEmailAddr();
  }
  // Add emails configured in korganizer
  lst = mAdditionalMails;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << QString("%1 <%2>").arg( fullName() ).arg( *it );
  }
  // Add emails from the user's kaddressbook entry
  KABC::Addressee me = KABC::StdAddressBook::self( true )->whoAmI();
  lst = me.emails();
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << me.fullEmail( *it );
  }

  // Warning, this list could contain duplicates.
  return fullEmails;
}

bool KOPrefs::thatIsMe( const QString &_email )
{
  if ( KOCore::self()->identityManager()->thatIsMe( _email ) ) {
    return true;
  }

  // in case email contains a full name, strip it out
  QString email = KPIMUtils::extractEmailAddress( _email );
  if ( mAdditionalMails.contains( email ) ) {
    return true;
  }
  QStringList lst = KABC::StdAddressBook::self( true )->whoAmI().emails();
  if ( lst.contains( email ) ) {
    return true;
  }
  return false;
}

QStringList KOPrefs::timeScaleTimezones()
{
  return mTimeScaleTimeZones;
}

void KOPrefs::setTimeScaleTimezones( const QStringList &list )
{
  mTimeScaleTimeZones = list;
}
