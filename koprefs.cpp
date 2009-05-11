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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <time.h>
#include <unistd.h>

#include <qdir.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qmap.h>
#include <qstringlist.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <kstaticdeleter.h>
#include <kstringhandler.h>

#include <libkmime/kmime_header_parsing.h>

#include "koprefs.h"
#include <libkpimidentities/identitymanager.h>
#include <libkpimidentities/identity.h>
#include <libemailfunctions/email.h>
#include <kabc/stdaddressbook.h>
#include <ktimezones.h>
#include "kocore.h"

KOPrefs *KOPrefs::mInstance = 0;
static KStaticDeleter<KOPrefs> insd;

QColor getTextColor(const QColor &c)
{
  float luminance = (c.red() * 0.299) + (c.green() * 0.587) + (c.blue() * 0.114);
  return (luminance > 128.0) ? QColor( 0, 0 ,0 ) : QColor( 255, 255 ,255 );
}


KOPrefs::KOPrefs() :
  KOPrefsBase()
{
  mCategoryColors.setAutoDelete( true );
  mResourceColors.setAutoDelete( true );

  mDefaultCategoryColor = QColor( 151, 235, 121 );

  mDefaultResourceColor = QColor();//Default is a color invalid

  mDefaultTimeBarFont = KGlobalSettings::generalFont();
  // make a large default time bar font, at least 16 points.
  mDefaultTimeBarFont.setPointSize(
    QMAX( mDefaultTimeBarFont.pointSize() + 4, 16 ) );

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize( mDefaultMonthViewFont.pointSize() - 2 );

  KConfigSkeleton::setCurrentGroup( "General" );

  addItemPath( "Html Export File", mHtmlExportFile,
   QDir::homeDirPath() + "/" + i18n( "Default export file", "calendar.html" ) );

  timeBarFontItem()->setDefaultValue( mDefaultTimeBarFont );
  monthViewFontItem()->setDefaultValue( mDefaultMonthViewFont );

  // Load it now, not deep within some painting code
  mMyAddrBookMails = KABC::StdAddressBook::self()->whoAmI().emails();
}


KOPrefs::~KOPrefs()
{
  kdDebug(5850) << "KOPrefs::~KOPrefs()" << endl;
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
  QString tmp = settings.getSetting(KEMailSettings::RealName);
  if ( !tmp.isEmpty() ) setUserName( tmp );
  tmp = settings.getSetting(KEMailSettings::EmailAddress);
  if ( !tmp.isEmpty() ) setUserEmail( tmp );
  fillMailDefaults();

  mTimeBarFont = mDefaultTimeBarFont;
  mMonthViewFont = mDefaultMonthViewFont;

  setTimeZoneIdDefault();

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
    if ( !settings.getSetting( KEMailSettings::EmailAddress ).isEmpty() )
      mEmailControlCenter = true;
  }
}

void KOPrefs::setTimeZoneIdDefault()
{
  QString zone;

  zone = KTimezones().local()->name();

  kdDebug() << "----- time zone: " << zone << endl;

  mTimeZoneId = zone;
}

void KOPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();

  mCustomCategories << i18n("Appointment") << i18n("Business")
      << i18n("Meeting") << i18n("Phone Call") << i18n("Education")
      << i18n("Holiday") << i18n("Vacation") << i18n("Special Occasion")
      << i18n("Personal") << i18n("Travel") << i18n("Miscellaneous")
      << i18n("Birthday");
}


void KOPrefs::usrReadConfig()
{
  config()->setGroup("General");
  mCustomCategories = config()->readListEntry("Custom Categories");
  if (mCustomCategories.isEmpty()) setCategoryDefaults();

  // old category colors, ignore if they have the old default
  // should be removed a few versions after 3.2...
  config()->setGroup("Category Colors");
  QValueList<QColor> oldCategoryColors;
  QStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    QColor c = config()->readColorEntry(*it, &mDefaultCategoryColor);
    oldCategoryColors.append( (c == QColor(196,196,196)) ?
                              mDefaultCategoryColor : c);
  }

  // new category colors
  config()->setGroup("Category Colors2");
  QValueList<QColor>::Iterator it2;
  for (it = mCustomCategories.begin(), it2 = oldCategoryColors.begin();
       it != mCustomCategories.end(); ++it, ++it2 ) {
      QColor c = config()->readColorEntry(*it, &*it2);
      if ( c != mDefaultCategoryColor )
          setCategoryColor(*it,c);
  }

  config()->setGroup( "Resources Colors" );
  QMap<QString, QString> map = config()->entryMap( "Resources Colors" );

  QMapIterator<QString, QString> it3;
  for( it3 = map.begin(); it3 != map.end(); ++it3 ) {
    kdDebug(5850)<< "KOPrefs::usrReadConfig: key: " << it3.key() << " value: "
      << it3.data()<<endl;
    setResourceColor( it3.key(), config()->readColorEntry( it3.key(),
      &mDefaultResourceColor ) );
  }


  if (mTimeZoneId.isEmpty()) {
    setTimeZoneIdDefault();
  }

#if 0
  config()->setGroup("FreeBusy");
  if( mRememberRetrievePw )
    mRetrievePassword = KStringHandler::obscure( config()->readEntry( "Retrieve Server Password" ) );
#endif
  KPimPrefs::usrReadConfig();
  fillMailDefaults();
}


void KOPrefs::usrWriteConfig()
{
  config()->setGroup("General");
  config()->writeEntry("Custom Categories",mCustomCategories);

  config()->setGroup("Category Colors2");
  QDictIterator<QColor> it(mCategoryColors);
  while (it.current()) {
    config()->writeEntry(it.currentKey(),*(it.current()));
    ++it;
  }

  config()->setGroup( "Resources Colors" );
  QDictIterator<QColor> it2( mResourceColors );
  while( it2.current() ) {
    config()->writeEntry( it2.currentKey(), *( it2.current() ) );
    ++it2;
  }

  if( !mFreeBusyPublishSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyPublishPasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }
  if( !mFreeBusyRetrieveSavePassword ) {
    KConfigSkeleton::ItemPassword *i = freeBusyRetrievePasswordItem();
    i->setValue( "" );
    i->writeConfig( config() );
  }

#if 0
  if( mRememberRetrievePw )
    config()->writeEntry( "Retrieve Server Password", KStringHandler::obscure( mRetrievePassword ) );
  else
    config()->deleteEntry( "Retrieve Server Password" );
#endif

  KPimPrefs::usrWriteConfig();
}

void KOPrefs::setCategoryColor( const QString &cat, const QColor & color)
{
  mCategoryColors.replace( cat, new QColor( color ) );
}

QColor *KOPrefs::categoryColor( const QString &cat )
{
  QColor *color = 0;

  if ( !cat.isEmpty() ) color = mCategoryColors[ cat ];

  if ( color ) return color;
  else return &mDefaultCategoryColor;
}


bool KOPrefs::hasCategoryColor( const QString& cat ) const
{
    return mCategoryColors[ cat ];
}

void KOPrefs::setResourceColor ( const QString &cal, const QColor &color )
{
  kdDebug(5850)<<"KOPrefs::setResourceColor: " << cal << " color: "<<
    color.name()<<endl;
  mResourceColors.replace( cal, new QColor( color ) );
}

QColor* KOPrefs::resourceColor( const QString &cal )
{
  QColor *color=0;
  if( !cal.isEmpty() ) color = mResourceColors[cal];

  // assign default color if enabled
  if ( !cal.isEmpty() && !color && assignDefaultResourceColors() ) {
    QColor defColor( 0x37, 0x7A, 0xBC );
    if ( defaultResourceColorSeed() > 0 && defaultResourceColorSeed() - 1 < (int)defaultResourceColors().size() ) {
        defColor = QColor( defaultResourceColors()[defaultResourceColorSeed()-1] );
    } else {
        int h, s, v;
        defColor.getHsv( h, s, v );
        h = ( defaultResourceColorSeed() % 12 ) * 30;
        s -= s * ( (defaultResourceColorSeed() / 12) % 2 ) * 0.5;
        defColor.setHsv( h, s, v );
    }
    setDefaultResourceColorSeed( defaultResourceColorSeed() + 1 );
    setResourceColor( cal, defColor );
    color = mResourceColors[cal];
  }

  if (color && color->isValid() )
    return color;
  else
    return &mDefaultResourceColor;
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
  tusername = KPIM::quoteNameIfNecessary( tusername );

  QString tname, temail;
  KPIM::getNameAndMail( tusername, tname, temail ); // ignore return value
                                                    // which is always false
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
  fullEmails << QString("%1 <%2>").arg( fullName() ).arg( email() );

  QStringList::Iterator it;
  // Grab emails from the email identities
  KPIM::IdentityManager *idmanager = KOCore::self()->identityManager();
  QStringList lst = idmanager->identities();
  KPIM::IdentityManager::ConstIterator it1;
  for ( it1 = idmanager->begin() ; it1 != idmanager->end() ; ++it1 ) {
    fullEmails << (*it1).fullEmailAddr();
  }
  // Add emails configured in korganizer
  lst = mAdditionalMails;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << QString("%1 <%2>").arg( fullName() ).arg( *it );
  }
  // Add emails from the user's kaddressbook entry
  KABC::Addressee me = KABC::StdAddressBook::self()->whoAmI();
  lst = me.emails();
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    fullEmails << me.fullEmail( *it );
  }

  // Warning, this list could contain duplicates.
  return fullEmails;
}

bool KOPrefs::thatIsMe( const QString& _email )
{
  // NOTE: this method is called for every created agenda view item, so we need to keep
  // performance in mind

  /* identityManager()->thatIsMe() is quite expensive since it does parsing of
     _email in a way which is unnecessarily complex for what we can have here,
     so we do that ourselves. This makes sense since this

  if ( KOCore::self()->identityManager()->thatIsMe( _email ) )
    return true;
  */

  // in case email contains a full name, strip it out
  // the below is the simpler but slower version of the following KMime code
  // const QString email = KPIM::getEmailAddress( _email );
  const QCString tmp = _email.utf8();
  const char *cursor = tmp.data();
  const char *end = tmp.data() + tmp.length();
  KMime::Types::Mailbox mbox;
  KMime::HeaderParsing::parseMailbox( cursor, end, mbox );
  const QString email = mbox.addrSpec.asString();

  for ( KPIM::IdentityManager::ConstIterator it = KOCore::self()->identityManager()->begin();
        it != KOCore::self()->identityManager()->end(); ++it ) {
    if ( email == (*it).emailAddr() )
      return true;
  }

  if ( mAdditionalMails.find( email ) != mAdditionalMails.end() )
    return true;
  QStringList lst = mMyAddrBookMails;
  if ( lst.find( email ) != lst.end() )
    return true;
  return false;
}
