/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <time.h>
#include <unistd.h>

#include <qdir.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kemailsettings.h>
#include <kstaticdeleter.h>
#include <kstringhandler.h>

#include "koprefs.h"
#include <libkpimidentities/identitymanager.h>
#include <libkpimidentities/identity.h>
#include <libemailfunctions/email.h>
#include <kabc/stdaddressbook.h>
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
  mCategoryColors.setAutoDelete(true);

  mDefaultCategoryColor = QColor(151, 235, 121);

  mDefaultMonthViewFont = KGlobalSettings::generalFont();
  // make it a bit smaller
  mDefaultMonthViewFont.setPointSize(mDefaultMonthViewFont.pointSize()-2);

  KConfigSkeleton::setCurrentGroup("General");

  addItemPath("Html Export File",mHtmlExportFile,
      QDir::homeDirPath() + "/" + i18n("Default export file", "calendar.html"));

  monthViewFontItem()->setDefaultValue( mDefaultMonthViewFont );
  eventColorItem()->setDefaultValue( mDefaultCategoryColor );
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
  mName = settings.getSetting(KEMailSettings::RealName);
  mEmail = settings.getSetting(KEMailSettings::EmailAddress);
  fillMailDefaults();

  mMonthViewFont = mDefaultMonthViewFont;

  setTimeZoneIdDefault();

  KPimPrefs::usrSetDefaults();
}

void KOPrefs::fillMailDefaults()
{
  QString defaultEmail = i18n("nobody@nowhere");
  if (mEmail.isEmpty())
    mEmail = defaultEmail;
  if ( mEmail == defaultEmail ) { // from the line above, or from using korganizer previously
    // No korg settings - but maybe there's a kcontrol[/kmail] setting available
    KEMailSettings settings;
    if ( !settings.getSetting( KEMailSettings::EmailAddress ).isEmpty() )
      mEmailControlCenter = true;
  }
  if (mName.isEmpty()) mName = i18n("Anonymous");
}

void KOPrefs::setTimeZoneIdDefault()
{
  QString zone;

  char zonefilebuf[100];
  int len = readlink("/etc/localtime",zonefilebuf,100);
  if (len > 0 && len < 100) {
    zonefilebuf[len] = '\0';
    zone = zonefilebuf;
    zone = zone.mid(zone.find("zoneinfo/") + 9);
  } else {
    tzset();
    zone = tzname[0];
  }

  kdDebug () << "----- time zone: " << zone << endl;

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

  QStringList::Iterator it;
  for (it = mCustomCategories.begin();it != mCustomCategories.end();++it ) {
    setCategoryColor(*it,mDefaultCategoryColor);
  }
}


void KOPrefs::usrReadConfig()
{
  config()->setGroup("General");
  mCustomCategories = config()->readListEntry("Custom Categories");
  if (mCustomCategories.isEmpty()) setCategoryDefaults();

  config()->setGroup("Personal Settings");
  mName = config()->readEntry("user_name");
  mEmail = config()->readEntry("user_email");
  fillMailDefaults();

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
    setCategoryColor(*it,config()->readColorEntry(*it, &*it2));
  }

  if (mTimeZoneId.isEmpty()) {
    setTimeZoneIdDefault();
  }

  config()->setGroup("FreeBusy");
#if 0
  if( mRememberRetrievePw )
    mRetrievePassword = KStringHandler::obscure( config()->readEntry( "Retrieve Server Password" ) );
#endif
  KPimPrefs::usrReadConfig();
}


void KOPrefs::usrWriteConfig()
{
  config()->setGroup("General");
  config()->writeEntry("Custom Categories",mCustomCategories);

  config()->setGroup("Personal Settings");
  config()->writeEntry("user_name",mName);
  config()->writeEntry("user_email",mEmail);

  config()->setGroup("Category Colors2");
  QDictIterator<QColor> it(mCategoryColors);
  while (it.current()) {
    config()->writeEntry(it.currentKey(),*(it.current()));
    ++it;
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

void KOPrefs::setCategoryColor(QString cat,const QColor & color)
{
  mCategoryColors.replace( cat, new QColor( color ) );
}

QColor *KOPrefs::categoryColor(QString cat)
{
  QColor *color = 0;

  if ( !cat.isEmpty() ) color = mCategoryColors[ cat ];

  if ( color ) return color;
  else return &mDefaultCategoryColor;
}

void KOPrefs::setFullName(const QString &name)
{
  mName = name;
}

void KOPrefs::setEmail(const QString &email)
{
  mEmail = email;
}

QString KOPrefs::fullName()
{
  if (mEmailControlCenter) {
    KEMailSettings settings;
    return settings.getSetting(KEMailSettings::RealName);
  } else {
    return mName;
  }
}

QString KOPrefs::email()
{
  if (mEmailControlCenter) {
    KEMailSettings settings;
    return settings.getSetting(KEMailSettings::EmailAddress);
  } else {
    return mEmail;
  }
}

QStringList KOPrefs::allEmails()
{
  // Grab emails from the email identities
  QStringList lst = KOCore::self()->identityManager()->allEmails();
  // Add emails configured in korganizer
  lst += mAdditionalMails;
  // Add emails from the user's kaddressbook entry
  lst += KABC::StdAddressBook::self()->whoAmI().emails();

  // Warning, this list could contain duplicates.
  return lst;
}

QStringList KOPrefs::fullEmails()
{
  QStringList fullEmails;
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
  if ( KOCore::self()->identityManager()->thatIsMe( _email ) )
    return true;
  // in case email contains a full name, strip it out
  QString email = KPIM::getEmailAddr( _email );
  if ( mAdditionalMails.find( email ) != mAdditionalMails.end() )
    return true;
  QStringList lst = KABC::StdAddressBook::self()->whoAmI().emails();
  if ( lst.find( email ) != lst.end() )
    return true;
  return false;
}
