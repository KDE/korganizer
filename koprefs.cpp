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

#include <kabc/stdaddressbook.h>
#include <kmime/kmime_header_parsing.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <kpimutils/email.h>

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

using namespace KPIMIdentities;

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
  if ( !mInstance ) {
    insd.setObject( mInstance, new KOPrefs() );

    // Load it now, not deep within some painting code
    // Don't load inside constructor since synchronous loading could
    // require resources to do a nested eventloop or similar and
    // cause re-entrance, resulting in more than once call to KOPrefs::instance()
    // before the constructor has finished and thus create more than once instance
    mInstance->mMyAddrBookMails = KABC::StdAddressBook::self()->whoAmI().emails();

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
    kError() << "KSystemTimeZones::local() return 0";
    return;
  }

  kDebug () << "----- time zone:" << zone.name();

  mTimeSpec = zone;
}

KDateTime::Spec KOPrefs::timeSpec()
{
  return KSystemTimeZones::local();
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

  // Note that the [Category Colors] group was removed after 3.2 due to
  // an algorithm change. That's why we now use [Category Colors2]

  // Category colors
  KConfigGroup colorsConfig( config(), "Category Colors2" );
  QStringList::Iterator it;
  for ( it = mCustomCategories.begin(); it != mCustomCategories.end(); ++it ) {
    QColor c = colorsConfig.readEntry( *it, mDefaultCategoryColor );
    if ( c != mDefaultCategoryColor ) {
      setCategoryColor( *it, c );
    }
  }

  // Resource colors
  KConfigGroup rColorsConfig( config(), "Resources Colors" );
  const QStringList colorKeyList = rColorsConfig.keyList();

  QStringList::ConstIterator it3;
  for ( it3 = colorKeyList.begin(); it3 != colorKeyList.end(); ++it3 ) {
    QColor color = rColorsConfig.readEntry( *it3, mDefaultResourceColor );
    kDebug() << "key:" << (*it3) << "value:" << color;
    setResourceColor( *it3, color );
  }

  if ( !mTimeSpec.isValid() ) {
    setTimeZoneDefault();
  }

#if 0
  config()->setGroup( "FreeBusy" );
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
  KConfigGroup generalConfig( config(), "General" );
  generalConfig.writeEntry( "Custom Categories", mCustomCategories );

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

bool KOPrefs::hasCategoryColor( const QString &cat ) const
{
    return mCategoryColors[ cat ].isValid();
}

void KOPrefs::setResourceColor ( const QString &cal, const QColor &color )
{
  kDebug() << cal << "color:" << color.name();
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
    if ( defaultResourceColorSeed() > 0 &&
         defaultResourceColorSeed() - 1 < (int)defaultResourceColors().size() ) {
        defColor = QColor( defaultResourceColors()[defaultResourceColorSeed()-1] );
    } else {
        int h, s, v;
        defColor.getHsv( &h, &s, &v );
        h = ( defaultResourceColorSeed() % 12 ) * 30;
        s -= s * ( ( defaultResourceColorSeed() / 12 ) % 2 ) * 0.5;
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
  QString tusername;
  if ( mEmailControlCenter ) {
    KEMailSettings settings;
    tusername = settings.getSetting( KEMailSettings::RealName );
  } else {
    tusername = userName();
  }

  // Quote the username as it might contain commas and other quotable chars.
  tusername = KPIMUtils::quoteNameIfNecessary( tusername );

  QString tname, temail;
  // ignore the return value from extractEmailAddressAndName() because
  // it will always be false since tusername does not contain "@domain".
  KPIMUtils::extractEmailAddressAndName( tusername, temail, tname );
  return tname;
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
  QStringList lst = KOCore::self()->identityManager()->allEmails();
  // Add emails configured in korganizer
  lst += mAdditionalMails;
  // Add emails from the user's kaddressbook entry
  lst += mMyAddrBookMails;
  // Add the email entered as the userEmail here
  lst += email();

  // Warning, this list could contain duplicates.
  return lst;
}

QStringList KOPrefs::fullEmails()
{
  QStringList fullEmails;
  // The user name and email from the config dialog:
  fullEmails << QString( "%1 <%2>" ).arg( fullName() ).arg( email() );

  QStringList::Iterator it;
  // Grab emails from the email identities
  IdentityManager *idmanager = KOCore::self()->identityManager();
  QStringList lst = idmanager->identities();
  IdentityManager::ConstIterator it1;
  for ( it1 = idmanager->begin(); it1 != idmanager->end(); ++it1 ) {
    fullEmails << (*it1).fullEmailAddr();
  }
  // Add emails configured in korganizer
  lst = mAdditionalMails;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << QString( "%1 <%2>" ).arg( fullName() ).arg( *it );
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
  // NOTE: this method is called for every created agenda view item,
  // so we need to keep performance in mind

  /* identityManager()->thatIsMe() is quite expensive since it does parsing of
     _email in a way which is unnecessarily complex for what we can have here,
     so we do that ourselves. This makes sense since this

  if ( KOCore::self()->identityManager()->thatIsMe( _email ) ) {
    return true;
  }
  */

  // in case email contains a full name, strip it out.
  // the below is the simpler but slower version of the following code:
  // const QString email = KPIM::getEmailAddress( _email );
  const QByteArray tmp = _email.toUtf8();
  const char *cursor = tmp.constData();
  const char *end = tmp.data() + tmp.length();
  KMime::Types::Mailbox mbox;
  KMime::HeaderParsing::parseMailbox( cursor, end, mbox );
  const QString email = mbox.addrSpec().asString();

  if ( this->email() == email ) {
    return true;
  }

  for ( IdentityManager::ConstIterator it = KOCore::self()->identityManager()->begin();
        it != KOCore::self()->identityManager()->end(); ++it ) {
    if ( email == (*it).emailAddr() ) {
      return true;
    }
  }

  if ( mAdditionalMails.contains( email ) ) {
    return true;
  }
  QStringList lst = mMyAddrBookMails;
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
