/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA  02111-1307, USA.

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

#include <libkcal/incidencebase.h>
#include <libkcal/attendee.h>
#include <libkcal/freebusy.h>
#include <libkcal/journal.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include <kio/job.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kabc/stdaddressbook.h> 
#include <kabc/addressee.h> 

#include <qfile.h>
#include <qbuffer.h>
#include <qregexp.h>
#include <qdir.h>

using namespace KCal;

FreeBusyDownloadJob::FreeBusyDownloadJob( const QString &email, const KURL &url,
                                          FreeBusyManager *manager,
                                          const char *name )
  : QObject( manager, name ), mManager( manager ), mEmail( email )
{
  KIO::Job *job = KIO::get( url, false, false );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );
  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotData( KIO::Job *, const QByteArray & ) ) );
}

FreeBusyDownloadJob::~FreeBusyDownloadJob()
{
}


void FreeBusyDownloadJob::slotData( KIO::Job *, const QByteArray &data )
{
  QByteArray tmp = data;
  tmp.resize( tmp.size() + 1 );
  tmp[tmp.size()-1] = 0;
  mFreeBusyData += tmp;
}

void FreeBusyDownloadJob::slotResult( KIO::Job *job )
{
  kdDebug(5850) << "FreeBusyDownloadJob::slotResult() " << mEmail << endl;

  if( job->error() ) {
    kdDebug(5850) << "FreeBusyDownloadJob::slotResult() job error :-(" << endl;
  }

  FreeBusy *fb = mManager->iCalToFreeBusy( mFreeBusyData );
  if ( fb ) {
    Person p = fb->organizer();
    p.setEmail( mEmail );
    mManager->saveFreeBusy( fb, p );
  }
  emit freeBusyDownloaded( fb, mEmail );
  // PENDING(steffen): Is this safe?
  //job->deleteLater();
  delete this;
}


FreeBusyManager::FreeBusyManager( QObject *parent, const char *name )
  : QObject( parent, name ),
    mCalendar( 0 ), mTimerID( 0 ), mUploadingFreeBusy( false )
{
}

void FreeBusyManager::setCalendar( KCal::Calendar *c )
{
  mCalendar = c;
  if ( mCalendar ) {
    mFormat.setTimeZone( mCalendar->timeZoneId(), true );
  }
}

KCal::FreeBusy *FreeBusyManager::ownerFreeBusy()
{
  QDateTime start = QDateTime::currentDateTime();
  QDateTime end = start.addDays( KOPrefs::instance()->mFreeBusyPublishDays );

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
  return mFormat.createScheduleMessage( freebusy, Scheduler::Publish );
}

void FreeBusyManager::slotPerhapsUploadFB()
{
  // user has automtic uploading disabled, bail out
  if ( !KOPrefs::instance()->freeBusyPublishAuto() ||
       KOPrefs::instance()->freeBusyPublishUrl().isEmpty() )
     return;
  if( mTimerID != 0 )
    // A timer is already running, so we don't need to do anything
    return;

  int now = static_cast<int>( QDateTime::currentDateTime().toTime_t() );
  int eta = static_cast<int>( mNextUploadTime.toTime_t() ) - now;

  if( !mUploadingFreeBusy ) {
    // Not currently uploading
    if( mNextUploadTime.isNull() ||
        QDateTime::currentDateTime() > mNextUploadTime ) {
      // No uploading have been done in this session, or delay time is over
      publishFreeBusy();
      return;
    }

    // We're in the delay time and no timer is running. Start one
    if( eta <= 0 ) {
      // Sanity check failed - better do the upload
      publishFreeBusy();
      return;
    }
  } else {
    // We are currently uploading the FB list. Start the timer
    if( eta <= 0 ) {
      kdDebug(5850) << "This shouldn't happen! eta <= 0\n";
      eta = 10; // whatever
    }
  }

  // Start the timer
  mTimerID = startTimer( eta * 1000 );

  if( mTimerID == 0 )
    // startTimer failed - better do the upload
    publishFreeBusy();
}

// This is used for delayed Free/Busy list uploading
void FreeBusyManager::timerEvent( QTimerEvent* )
{
  publishFreeBusy();
}

/*!
  This method is called when the user has selected to publish its
  free/busy list or when the delay have passed.
*/
void FreeBusyManager::publishFreeBusy()
{
  // Already uploading? Skip this one then.
  if ( mUploadingFreeBusy )
    return;
  KURL targetURL ( KOPrefs::instance()->freeBusyPublishUrl() );
  if ( targetURL.isEmpty() )  {
    KMessageBox::sorry( 0,
      i18n( "<qt>No URL configured for uploading your free/busy list! Please "
            "set it in KOrganizer's configuration dialog, \"Free/Busy\" page. "
            "<br>Contact your system administrator for the exact URL and the "
            "account details."
            "</qt>" ), i18n("No Free/Busy upload URL") );
    return;
  }
  targetURL.setUser( KOPrefs::instance()->mFreeBusyPublishUser );
  targetURL.setPass( KOPrefs::instance()->mFreeBusyPublishPassword );
  
  mUploadingFreeBusy = true;

  // If we have a timer running, it should be stopped now
  if( mTimerID != 0 ) {
    killTimer( mTimerID );
    mTimerID = 0;
  }

  // Save the time of the next free/busy uploading
  mNextUploadTime = QDateTime::currentDateTime();
  if( KOPrefs::instance()->mFreeBusyPublishDelay > 0 )
    mNextUploadTime = mNextUploadTime.addSecs(
        KOPrefs::instance()->mFreeBusyPublishDelay * 60 );

  QString messageText = ownerFreeBusyAsString();

  // We need to massage the list a bit so that Outlook understands
  // it.
  messageText = messageText.replace( QRegExp( "ORGANIZER\\s*:MAILTO:" ),
                                     "ORGANIZER:" );

  // Create a local temp file and save the message to it
  KTempFile tempFile;
  QTextStream *textStream = tempFile.textStream();
  if( textStream ) {
    *textStream << messageText;
    tempFile.close();

#if 0
    QString defaultEmail = KOCore()::self()->email();
    QString emailHost = defaultEmail.mid( defaultEmail.find( '@' ) + 1 );

    // Put target string together
    KURL targetURL;
    if( KOPrefs::instance()->mPublishKolab ) {
      // we use Kolab
      QString server;
      if( KOPrefs::instance()->mPublishKolabServer == "%SERVER%" ||
	  KOPrefs::instance()->mPublishKolabServer.isEmpty() )
	server = emailHost;
      else
	server = KOPrefs::instance()->mPublishKolabServer;

      targetURL.setProtocol( "webdavs" );
      targetURL.setHost( server );

      QString fbname = KOPrefs::instance()->mPublishUserName;
      int at = fbname.find('@');
      if( at > 1 && fbname.length() > (uint)at ) {
	fbname = fbname.left(at);
      }
      targetURL.setPath( "/freebusy/" + fbname + ".ifb" );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    } else {
      // we use something else
      targetURL = KOPrefs::instance()->mPublishAnyURL.replace( "%SERVER%",
                                                               emailHost );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    }
#endif


    KURL src;
    src.setPath( tempFile.name() );

    kdDebug(5850) << "FreeBusyManager::publishFreeBusy(): " << targetURL << endl;

    KIO::Job * job = KIO::file_copy( src, targetURL, -1,
                                     true /*overwrite*/,
                                     false /*don't resume*/,
                                     false /*don't show progress info*/ );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotUploadFreeBusyResult( KIO::Job * ) ) );
  }
}

void FreeBusyManager::slotUploadFreeBusyResult(KIO::Job *_job)
{
    KIO::FileCopyJob* job = static_cast<KIO::FileCopyJob *>(_job);
    if ( job->error() )
        KMessageBox::sorry( 0,
          i18n( "<qt>The software could not upload your free/busy list to the "
                "URL '%1'. There might be a problem with the access rights, or "
                "you specified an incorrect URL. The system said: <em>%2</em>."
                "<br>Please check the URL or contact your system administrator."
                "</qt>" ).arg( job->destURL().prettyURL() )
                         .arg( job->errorString() ) );
    // Delete temp file
    KURL src = job->srcURL();
    Q_ASSERT( src.isLocalFile() );
    if( src.isLocalFile() )
        QFile::remove(src.path());
    mUploadingFreeBusy = false;
}

bool FreeBusyManager::retrieveFreeBusy( const QString &email )
{
  kdDebug(5850) << "FreeBusyManager::retrieveFreeBusy(): " << email << endl;

  if( KOPrefs::instance()->thatIsMe( email ) ) {
    // Don't download our own free-busy list from the net
    kdDebug(5850) << "freebusy of owner" << endl;
    emit freeBusyRetrieved( ownerFreeBusy(), email );
    return true;
  }

  // Check for cached copy of free/busy list
  KCal::FreeBusy *fb = loadFreeBusy( email );
  if ( fb ) {
    emit freeBusyRetrieved( fb, email );
  }

  // Don't download free/busy if the user does not want it.
  if( !KOPrefs::instance()->mFreeBusyRetrieveAuto )
    return false;

  mRetrieveQueue.append( email );

  if ( mRetrieveQueue.count() > 1 ) return true;

  return processRetrieveQueue();
}

bool FreeBusyManager::processRetrieveQueue()
{
  if ( mRetrieveQueue.isEmpty() ) return true;

  QString email = mRetrieveQueue.first();
  mRetrieveQueue.pop_front();

  KURL sourceURL = freeBusyUrl( email );

  kdDebug(5850) << "FreeBusyManager::retrieveFreeBusy(): url: " << sourceURL.url()
            << endl;

  if ( !sourceURL.isValid() ) {
    kdDebug(5850) << "Invalid FB URL\n";
    return false;
  }

  FreeBusyDownloadJob *job = new FreeBusyDownloadJob( email, sourceURL, this,
                                                      "freebusy_download_job" );
  connect( job, SIGNAL( freeBusyDownloaded( KCal::FreeBusy *,
                                            const QString & ) ),
	   SIGNAL( freeBusyRetrieved( KCal::FreeBusy *, const QString & ) ) );
  connect( job, SIGNAL( freeBusyDownloaded( KCal::FreeBusy *,
                                            const QString & ) ),
           SLOT( processRetrieveQueue() ) );

  return true;
}

void FreeBusyManager::cancelRetrieval()
{
  mRetrieveQueue.clear();
}

KURL FreeBusyManager::freeBusyUrl( const QString &email )
{
  kdDebug(5850) << "FreeBusyManager::freeBusyUrl(): " << email << endl;

  // First check if there is a specific FB url for this email
  QString configFile = locateLocal( "data", "korganizer/freebusyurls" );
  KConfig cfg( configFile );

  cfg.setGroup( email );
  QString url = cfg.readEntry( "url" );
  if ( !url.isEmpty() ) {
    return KURL( url );
  }
  // Try with the url configurated by preferred email in kaddressbook
  KABC::Addressee::List list= KABC::StdAddressBook::self()->findByEmail( email );
  KABC::Addressee::List::Iterator it;
  QString pref;
  for ( it = list.begin(); it != list.end(); ++it ) {
    pref = (*it).preferredEmail();
    if ( !pref.isEmpty() && pref != email ) {
      kdDebug( 5850 ) << "FreeBusyManager::freeBusyUrl():" <<
        "Preferred email of " << email << " is " << pref << endl;
      cfg.setGroup( pref );
      url = cfg.readEntry ( "url" );
      if ( !url.isEmpty() )
        kdDebug( 5850 ) << "FreeBusyManager::freeBusyUrl():" <<
          "Taken url from preferred email:" << url << endl;
        return KURL( url );
    }
  }
  // None found. Check if we do automatic FB retrieving then
  if ( !KOPrefs::instance()->mFreeBusyRetrieveAuto )
    // No, so no FB list here
    return KURL();

  // Sanity check: Don't download if it's not a correct email
  // address (this also avoids downloading for "(empty email)").
  int emailpos = email.find( '@' );
  if( emailpos == -1 )
    return KURL();

  // Cut off everything left of the @ sign to get the user name.
  const QString emailName = email.left( emailpos );
  const QString emailHost = email.mid( emailpos + 1 );

  // Build the URL
  KURL sourceURL;
  sourceURL = KOPrefs::instance()->mFreeBusyRetrieveUrl;

  // Don't try to fetch free/busy data for users not on the specified servers
  // This tests if the hostnames match, or one is a subset of the other
  const QString hostDomain = sourceURL.host();
  if ( hostDomain != emailHost && !hostDomain.endsWith( '.' + emailHost )
       && !emailHost.endsWith( '.' + hostDomain ) ) {
    // Host names do not match
    kdDebug(5850) << "Host '" << sourceURL.host() << "' doesn't match email '"
      << email << "'" << endl; 
    return KURL();
}

  if ( KOPrefs::instance()->mFreeBusyFullDomainRetrieval )
    sourceURL.setFileName( email + ".ifb" );
  else
    sourceURL.setFileName( emailName + ".ifb" );
  sourceURL.setUser( KOPrefs::instance()->mFreeBusyRetrieveUser );
  sourceURL.setPass( KOPrefs::instance()->mFreeBusyRetrievePassword );

  return sourceURL;
}

KCal::FreeBusy *FreeBusyManager::iCalToFreeBusy( const QCString &data )
{
  kdDebug(5850) << "FreeBusyManager::iCalToFreeBusy()" << endl;

  QString freeBusyVCal = QString::fromUtf8( data );
  KCal::FreeBusy *fb = mFormat.parseFreeBusy( freeBusyVCal );
  if ( !fb ) {
    kdDebug(5850) << "FreeBusyManager::iCalToFreeBusy(): Error parsing free/busy"
              << endl;
    kdDebug(5850) << freeBusyVCal << endl;
  } 
  return fb;
}

QString FreeBusyManager::freeBusyDir()
{
  return locateLocal( "data", "korganizer/freebusy" );
}

FreeBusy *FreeBusyManager::loadFreeBusy( const QString &email )
{
  kdDebug(5850) << "FreeBusyManager::loadFreeBusy(): " << email << endl;

  QString fbd = freeBusyDir();

  QFile f( fbd + "/" + email + ".ifb" );
  if ( !f.exists() ) {
    kdDebug(5850) << "FreeBusyManager::loadFreeBusy() " << f.name()
              << " doesn't exist." << endl;
    return 0;
  }

  if ( !f.open( IO_ReadOnly ) ) {
    kdDebug(5850) << "FreeBusyManager::loadFreeBusy() Unable to open file "
              << f.name() << endl;
    return 0;
  }

  QTextStream ts( &f );
  QString str = ts.read();

  return iCalToFreeBusy( str.utf8() );
}

bool FreeBusyManager::saveFreeBusy( FreeBusy *freebusy, const Person &person )
{
  kdDebug(5850) << "FreeBusyManager::saveFreeBusy(): " << person.fullName() << endl;

  QString fbd = freeBusyDir();

  QDir freeBusyDirectory( fbd );
  if ( !freeBusyDirectory.exists() ) {
    kdDebug(5850) << "Directory " << fbd << " does not exist!" << endl;
    kdDebug(5850) << "Creating directory: " << fbd << endl;

    if( !freeBusyDirectory.mkdir( fbd, true ) ) {
      kdDebug(5850) << "Could not create directory: " << fbd << endl;
      return false;
    }
  }

  QString filename( fbd );
  filename += "/";
  filename += person.email();
  filename += ".ifb";
  QFile f( filename );

  kdDebug(5850) << "FreeBusyManager::saveFreeBusy(): filename: " << filename
            << endl;

  freebusy->clearAttendees();
  freebusy->setOrganizer( person );

  QString messageText = mFormat.createScheduleMessage( freebusy,
                                                       Scheduler::Publish );

  if ( !f.open( IO_ReadWrite ) ) {
    kdDebug(5850) << "acceptFreeBusy: Can't open:" << filename << " for writing"
              << endl;
    return false;
  }
  QTextStream t( &f );
  t << messageText;
  f.close();

  return true;
}

#include "freebusymanager.moc"
