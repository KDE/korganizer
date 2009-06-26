/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klar�vdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "freebusymanager.h"
#include "koprefs.h"
#include "mailscheduler.h"

#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>

#include <kcal/incidencebase.h>
#include <kcal/attendee.h>
#include <kcal/freebusy.h>
#include <kcal/journal.h>
#include <kcal/calendarlocal.h>
#include <kcal/icalformat.h>

#include <kio/job.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <QFile>
#include <QBuffer>
#include <QRegExp>
#include <QDir>
#include <QTimerEvent>
#include <QTextStream>
#include <QByteArray>

using namespace KCal;

FreeBusyDownloadJob::FreeBusyDownloadJob( const QString &email, const KUrl &url,
                                          FreeBusyManager *manager )
  : QObject( manager ), mManager( manager ), mEmail( email )
{
  KIO::Job *job = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
  connect( job, SIGNAL(result(KJob *)), SLOT(slotResult(KJob *)) );
  connect( job, SIGNAL(data(KIO::Job *,const QByteArray &)),
           SLOT(slotData(KIO::Job *,const QByteArray &)) );
}

FreeBusyDownloadJob::~FreeBusyDownloadJob()
{
}

void FreeBusyDownloadJob::slotData( KIO::Job *, const QByteArray &data )
{
  mFreeBusyData += data;
}

void FreeBusyDownloadJob::slotResult( KJob *job )
{
  kDebug() << mEmail;

  if ( job->error() ) {
    kDebug() << "job error :-(";
  }

  FreeBusy *fb = mManager->iCalToFreeBusy( mFreeBusyData );
  if ( fb ) {
    Person p = fb->organizer();
    p.setEmail( mEmail );
    mManager->saveFreeBusy( fb, p );
  }
  emit freeBusyDownloaded( fb, mEmail );
  deleteLater();
}

////

FreeBusyManager::FreeBusyManager( QObject *parent ) :
  QObject( parent ),
  mCalendar( 0 ), mTimerID( 0 ), mUploadingFreeBusy( false ),
  mBrokenUrl( false )
{
}

void FreeBusyManager::setCalendar( KCal::Calendar *c )
{
  mCalendar = c;
  if ( mCalendar ) {
    mFormat.setTimeSpec( mCalendar->timeSpec() );
  }
}

KCal::FreeBusy *FreeBusyManager::ownerFreeBusy()
{
  KDateTime start = KDateTime::currentUtcDateTime();
  KDateTime end = start.addDays( KOPrefs::instance()->mFreeBusyPublishDays );

  FreeBusy *freebusy = new FreeBusy( mCalendar, start, end );
  freebusy->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                  KOPrefs::instance()->email() ) );

  return freebusy;
}

QString FreeBusyManager::ownerFreeBusyAsString()
{
  FreeBusy *freebusy = ownerFreeBusy();

  QString result = freeBusyToIcal( freebusy );

  delete freebusy;

  return result;
}

QString FreeBusyManager::freeBusyToIcal( KCal::FreeBusy *freebusy )
{
  return mFormat.createScheduleMessage( freebusy, iTIPPublish );
}

void FreeBusyManager::slotPerhapsUploadFB()
{
  // user has automatic uploading disabled, bail out
  if ( !KOPrefs::instance()->freeBusyPublishAuto() ||
       KOPrefs::instance()->freeBusyPublishUrl().isEmpty() ) {
     return;
  }

  if( mTimerID != 0 ) {
    // A timer is already running, so we don't need to do anything
    return;
  }

  int now = static_cast<int>( QDateTime::currentDateTime().toTime_t() );
  int eta = static_cast<int>( mNextUploadTime.toTime_t() ) - now;

  if ( !mUploadingFreeBusy ) {
    // Not currently uploading
    if ( mNextUploadTime.isNull() ||
         QDateTime::currentDateTime() > mNextUploadTime ) {
      // No uploading have been done in this session, or delay time is over
      publishFreeBusy();
      return;
    }

    // We're in the delay time and no timer is running. Start one
    if ( eta <= 0 ) {
      // Sanity check failed - better do the upload
      publishFreeBusy();
      return;
    }
  } else {
    // We are currently uploading the FB list. Start the timer
    if ( eta <= 0 ) {
      kDebug() << "This shouldn't happen! eta <= 0";
      eta = 10; // whatever
    }
  }

  // Start the timer
  mTimerID = startTimer( eta * 1000 );

  if ( mTimerID == 0 ) {
    // startTimer failed - better do the upload
    publishFreeBusy();
  }
}

// This is used for delayed Free/Busy list uploading
void FreeBusyManager::timerEvent( QTimerEvent * )
{
  publishFreeBusy();
}

void FreeBusyManager::setBrokenUrl( bool isBroken )
{
  mBrokenUrl = isBroken;
}

/*!
  This method is called when the user has selected to publish its
  free/busy list or when the delay have passed.
*/
void FreeBusyManager::publishFreeBusy()
{
  // Already uploading? Skip this one then.
  if ( mUploadingFreeBusy ) {
    return;
  }
  KUrl targetURL ( KOPrefs::instance()->freeBusyPublishUrl() );
  if ( targetURL.isEmpty() )  {
    KMessageBox::sorry(
      0,
      i18n( "<qt><p>No URL configured for uploading your free/busy list. "
            "Please set it in KOrganizer's configuration dialog, on the "
            "\"Free/Busy\" page.</p>"
            "<p>Contact your system administrator for the exact URL and the "
            "account details.</p></qt>" ),
      i18n( "No Free/Busy Upload URL" ) );
    return;
  }

  if ( mBrokenUrl ) {
     // Url is invalid, don't try again
    return;
  }
  if ( !targetURL.isValid() ) {
    KMessageBox::sorry(
      0,
      i18n( "<qt>The target URL '%1' provided is invalid.</qt>", targetURL.prettyUrl() ),
      i18n( "Invalid URL" ) );
    mBrokenUrl = true;
    return;
  }
  targetURL.setUser( KOPrefs::instance()->mFreeBusyPublishUser );
  targetURL.setPass( KOPrefs::instance()->mFreeBusyPublishPassword );

  mUploadingFreeBusy = true;

  // If we have a timer running, it should be stopped now
  if ( mTimerID != 0 ) {
    killTimer( mTimerID );
    mTimerID = 0;
  }

  // Save the time of the next free/busy uploading
  mNextUploadTime = QDateTime::currentDateTime();
  if ( KOPrefs::instance()->mFreeBusyPublishDelay > 0 ) {
    mNextUploadTime = mNextUploadTime.addSecs( KOPrefs::instance()->mFreeBusyPublishDelay * 60 );
  }

  QString messageText = ownerFreeBusyAsString();

  // We need to massage the list a bit so that Outlook understands
  // it.
  messageText = messageText.replace( QRegExp( "ORGANIZER\\s*:MAILTO:" ), "ORGANIZER:" );

  // Create a local temp file and save the message to it
  KTemporaryFile tempFile;
  tempFile.setAutoRemove( false );
  if ( tempFile.open() ) {
    QTextStream textStream ( &tempFile );
    textStream << messageText;
    textStream.flush();

#if 0
    QString defaultEmail = KOCore()::self()->email();
    QString emailHost = defaultEmail.mid( defaultEmail.indexOf( '@' ) + 1 );

    // Put target string together
    KUrl targetURL;
    if( KOPrefs::instance()->mPublishKolab ) {
      // we use Kolab
      QString server;
      if ( KOPrefs::instance()->mPublishKolabServer == QLatin1String( "%SERVER%" ) ||
           KOPrefs::instance()->mPublishKolabServer.isEmpty() ) {
        server = emailHost;
      } else {
        server = KOPrefs::instance()->mPublishKolabServer;
      }

      targetURL.setProtocol( "webdavs" );
      targetURL.setHost( server );

      QString fbname = KOPrefs::instance()->mPublishUserName;
      int at = fbname.indexOf( '@' );
      if ( at > 1 && fbname.length() > (uint)at ) {
        fbname = fbname.left(at);
      }
      targetURL.setPath( "/freebusy/" + fbname + ".ifb" );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    } else {
      // we use something else
      targetURL = KOPrefs::instance()->mPublishAnyURL.replace( "%SERVER%", emailHost );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    }
#endif

    KUrl src;
    src.setPath( tempFile.fileName() );

    kDebug() << targetURL;

    KIO::Job *job = KIO::file_copy( src, targetURL, -1, KIO::Overwrite | KIO::HideProgressInfo );
    connect( job, SIGNAL(result(KJob *)), SLOT(slotUploadFreeBusyResult(KJob *)) );
  }
}

void FreeBusyManager::slotUploadFreeBusyResult( KJob *_job )
{
    KIO::FileCopyJob *job = static_cast<KIO::FileCopyJob *>( _job );
    if ( job->error() ) {
        KMessageBox::sorry(
          0,
          i18n( "<qt><p>The software could not upload your free/busy list to "
                "the URL '%1'. There might be a problem with the access "
                "rights, or you specified an incorrect URL. The system said: "
                "<em>%2</em>.</p>"
                "<p>Please check the URL or contact your system administrator."
                "</p></qt>", job->destUrl().prettyUrl(),
                job->errorString() ) );
    }
    // Delete temp file
    KUrl src = job->srcUrl();
    Q_ASSERT( src.isLocalFile() );
    if ( src.isLocalFile() ) {
      QFile::remove( src.path() );
    }
    mUploadingFreeBusy = false;
}

bool FreeBusyManager::retrieveFreeBusy( const QString &email, bool forceDownload )
{
  kDebug() << email;
  if ( email.isEmpty() ) {
    return false;
  }

  if ( KOPrefs::instance()->thatIsMe( email ) ) {
    // Don't download our own free-busy list from the net
    kDebug() << "freebusy of owner";
    emit freeBusyRetrieved( ownerFreeBusy(), email );
    return true;
  }

  // Check for cached copy of free/busy list
  KCal::FreeBusy *fb = loadFreeBusy( email );
  if ( fb ) {
    emit freeBusyRetrieved( fb, email );
  }

  // Don't download free/busy if the user does not want it.
  if ( !KOPrefs::instance()->mFreeBusyRetrieveAuto && !forceDownload ) {
    return false;
  }

  mRetrieveQueue.append( email );

  if ( mRetrieveQueue.count() > 1 ) {
    return true;
  }

  return processRetrieveQueue();
}

bool FreeBusyManager::processRetrieveQueue()
{
  if ( mRetrieveQueue.isEmpty() ) {
    return true;
  }

  QString email = mRetrieveQueue.first();
  mRetrieveQueue.pop_front();

  KUrl sourceURL = freeBusyUrl( email );

  kDebug() << "url:" << sourceURL;

  if ( !sourceURL.isValid() ) {
    kDebug() << "Invalid FB URL";
    return false;
  }

  FreeBusyDownloadJob *job = new FreeBusyDownloadJob( email, sourceURL, this );
  job->setObjectName( "freebusy_download_job" );
  connect( job, SIGNAL(freeBusyDownloaded(KCal::FreeBusy *,const QString &)),
           SIGNAL(freeBusyRetrieved(KCal::FreeBusy *,const QString &)) );
  connect( job, SIGNAL(freeBusyDownloaded(KCal::FreeBusy *,const QString &)),
           SLOT(processRetrieveQueue()) );

  return true;
}

void FreeBusyManager::cancelRetrieval()
{
  mRetrieveQueue.clear();
}

KUrl FreeBusyManager::freeBusyUrl( const QString &email ) const
{
  kDebug() << email;

  // First check if there is a specific FB url for this email
  QString configFile = KStandardDirs::locateLocal( "data", "korganizer/freebusyurls" );
  KConfig cfg( configFile );
  KConfigGroup group = cfg.group(email);
  QString url = group.readEntry( "url" );
  if ( !url.isEmpty() ) {
    return KUrl( url );
  }
  // Try with the url configurated by preferred email in kcontactmanager
  KABC::Addressee::List list= KABC::StdAddressBook::self( true )->findByEmail( email );
  KABC::Addressee::List::Iterator it;
  QString pref;
  for ( it = list.begin(); it != list.end(); ++it ) {
    pref = (*it).preferredEmail();
    if ( !pref.isEmpty() && pref != email ) {
      kDebug() << "Preferred email of" << email << "is" << pref;
      group = cfg.group( pref );
      url = group.readEntry ( "url" );
      if ( !url.isEmpty() ) {
        kDebug() << "Taken url from preferred email:" << url;
        return KUrl( url );
      }
    }
  }
  // None found. Check if we do automatic FB retrieving then
  if ( !KOPrefs::instance()->mFreeBusyRetrieveAuto ) {
    // No, so no FB list here
    return KUrl();
  }

  // Sanity check: Don't download if it's not a correct email
  // address (this also avoids downloading for "(empty email)").
  int emailpos = email.indexOf( '@' );
  if( emailpos == -1 ) {
     kDebug() << "No '@' found in" << email;
     return KUrl();
  }

  // Cut off everything left of the @ sign to get the user name.
  const QString emailName = email.left( emailpos );
  const QString emailHost = email.mid( emailpos + 1 );

  // Build the URL
  KUrl sourceURL;
  sourceURL = KOPrefs::instance()->mFreeBusyRetrieveUrl;

  if ( KOPrefs::instance()->mFreeBusyCheckHostname ) {
    // Don't try to fetch free/busy data for users not on the specified servers
    // This tests if the hostnames match, or one is a subset of the other
    const QString hostDomain = sourceURL.host();
    if ( hostDomain != emailHost &&
         !hostDomain.endsWith( '.' + emailHost ) &&
         !emailHost.endsWith( '.' + hostDomain ) ) {
      // Host names do not match
      kDebug() << "Host '" << sourceURL.host()
               << "' doesn't match email '" << email << '\'';
      return KUrl();
    }
  }

  if ( KOPrefs::instance()->mFreeBusyFullDomainRetrieval ) {
    sourceURL.setFileName( email + ".ifb" );
  } else {
    sourceURL.setFileName( emailName + ".ifb" );
  }
  sourceURL.setUser( KOPrefs::instance()->mFreeBusyRetrieveUser );
  sourceURL.setPass( KOPrefs::instance()->mFreeBusyRetrievePassword );

  return sourceURL;
}

KCal::FreeBusy *FreeBusyManager::iCalToFreeBusy( const QByteArray &data )
{
  kDebug() << data;

  QString freeBusyVCal = QString::fromUtf8( data );
  KCal::FreeBusy *fb = mFormat.parseFreeBusy( freeBusyVCal );
  if ( !fb ) {
    kDebug() << "Error parsing free/busy";
    kDebug() << freeBusyVCal;
  }
  return fb;
}

QString FreeBusyManager::freeBusyDir()
{
  return KStandardDirs::locateLocal( "data", "korganizer/freebusy" );
}

FreeBusy *FreeBusyManager::loadFreeBusy( const QString &email )
{
  kDebug() << email;

  QString fbd = freeBusyDir();

  QFile f( fbd + '/' + email + ".ifb" );
  if ( !f.exists() ) {
    kDebug() << f.fileName() << "doesn't exist.";
    return 0;
  }

  if ( !f.open( QIODevice::ReadOnly ) ) {
    kDebug() << "Unable to open file" << f.fileName();
    return 0;
  }

  QTextStream ts( &f );
  QString str = ts.readAll();

  return iCalToFreeBusy( str.toUtf8() );
}

bool FreeBusyManager::saveFreeBusy( FreeBusy *freebusy, const Person &person )
{
  kDebug() << person.fullName();

  QString fbd = freeBusyDir();

  QDir freeBusyDirectory( fbd );
  if ( !freeBusyDirectory.exists() ) {
    kDebug() << "Directory" << fbd <<" does not exist!";
    kDebug() << "Creating directory:" << fbd;

    if( !freeBusyDirectory.mkpath( fbd ) ) {
      kDebug() << "Could not create directory:" << fbd;
      return false;
    }
  }

  QString filename( fbd );
  filename += '/';
  filename += person.email();
  filename += ".ifb";
  QFile f( filename );

  kDebug() << "filename:" << filename;

  freebusy->clearAttendees();
  freebusy->setOrganizer( person );

  QString messageText = mFormat.createScheduleMessage( freebusy, iTIPPublish );

  if ( !f.open( QIODevice::ReadWrite ) ) {
    kDebug() << "acceptFreeBusy: Can't open:" << filename << "for writing";
    return false;
  }
  QTextStream t( &f );
  t << messageText;
  f.close();

  return true;
}

#include "freebusymanager.moc"
