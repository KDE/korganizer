/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

#include <kdebug.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>

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
  if( job->error() ) {
    kdDebug(5850) << "FreeBusyDownloadJob::slotResult() job error :-(" << endl;
  }

  FreeBusy *fb = mManager->iCalToFreeBusy( mFreeBusyData );
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
}

KCal::FreeBusy *FreeBusyManager::ownerFreeBusy()
{
  QDateTime start = QDateTime::currentDateTime();
  QDateTime end = start.addDays( KOPrefs::instance()->mPublishFreeBusyDays );

  FreeBusy *freebusy = new FreeBusy( mCalendar, start, end );
  freebusy->setOrganizer( KOPrefs::instance()->email() );

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
  if ( !KOPrefs::instance()->autoPublish() )
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
  mUploadingFreeBusy = true;

  // If we have a timer running, it should be stopped now
  if( mTimerID != 0 ) {
    killTimer( mTimerID );
    mTimerID = 0;
  }

  // Save the time of the next free/busy uploading
  mNextUploadTime = QDateTime::currentDateTime();
  if( KOPrefs::instance()->mPublishDelay > 0 )
    mNextUploadTime = mNextUploadTime.addSecs( KOPrefs::instance()->mPublishDelay * 60 );

  QString messageText = ownerFreeBusyAsString();

  // We need to massage the list a bit so that Outlook understands
  // it.
  messageText = messageText.replace( QRegExp( "ORGANIZER\\s*:MAILTO:" ),
                                     "ORGANIZER:" );

  QString emailHost = KOPrefs::instance()->email().mid(
      KOPrefs::instance()->email().find( '@' ) + 1 );

  // Create a local temp file and save the message to it
  KTempFile tempFile;
  QTextStream *textStream = tempFile.textStream();
  if( textStream ) {
    *textStream << messageText;
    tempFile.close();

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
      targetURL.setPath( "/freebusy/" + fbname + ".vfb" );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    } else {
      // we use something else
      targetURL = KOPrefs::instance()->mPublishAnyURL.replace( "%SERVER%",
                                                               emailHost );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    }

    KURL src;
    src.setPath( tempFile.name() );
    
    kdDebug() << "FreeBusyManager::publishFreeBusy(): " << targetURL << endl;
    
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
          i18n( "<qt>The software could not upload your free/busy list to the URL %1. There might be a problem with the access rights, or you specified an incorrect URL. The system said: <em>%2</em>.<br>Please check the URL or contact your system administrator.</qt>" ).arg( job->destURL().prettyURL() ).arg( job->errorString() ) );
    // Delete temp file
    KURL src = job->srcURL();
    Q_ASSERT( src.isLocalFile() );
    if( src.isLocalFile() )
        QFile::remove(src.path());
    mUploadingFreeBusy = false;
}

bool FreeBusyManager::retrieveFreeBusy( const QString &email )
{
  kdDebug() << "FreeBusyManager::retrieveFreeBusy(): " << email << endl;

  if( email == KOPrefs::instance()->email() ) {
    // Don't download our own free-busy list from the net
    kdDebug() << "freebusy of owner" << endl;
    emit freeBusyRetrieved( ownerFreeBusy(), email );
    return true;
  }

  // Check for cached copy of free/busy list
  KCal::FreeBusy *fb = loadFreeBusy( email );
  if ( fb ) {
    emit freeBusyRetrieved( fb, email );
  }

  // Don't download free/busy if the user does not want it.
  if( !KOPrefs::instance()->mRetrieveFreeBusy )
    return false;

  KURL sourceURL = freeBusyUrl( email );

  FreeBusyDownloadJob* job = new FreeBusyDownloadJob( email, sourceURL, this,
                                                      "freebusy_download_job" );
  connect( job, SIGNAL( freeBusyDownloaded( KCal::FreeBusy *, const QString & ) ),
	   SIGNAL( freeBusyRetrieved( KCal::FreeBusy *, const QString & ) ) );

  return true;
}

KURL FreeBusyManager::freeBusyUrl( const QString &email )
{
  KURL sourceURL;

  // Sanity check: Don't download if it's not a correct email
  // address (this also avoids downloading for "(empty email)").
  int emailpos = email.find( '@' );
  if( emailpos == -1 )
    return false;

  // Cut off everything left of the @ sign to get the user name.
  QString emailName = email.left( emailpos );
  QString emailHost = email.mid( emailpos + 1 );

  // Put download string together
  if( KOPrefs::instance()->mRetrieveKolab ) {
    // we use Kolab
    QString server;
    if( KOPrefs::instance()->mRetrieveKolabServer == "%SERVER%" ||
	KOPrefs::instance()->mRetrieveKolabServer.isEmpty() )
      server = emailHost;
    else
      server = KOPrefs::instance()->mRetrieveKolabServer;

    sourceURL.setProtocol( "webdavs" );
    sourceURL.setHost( server );
    sourceURL.setPass( KOPrefs::instance()->mRetrievePassword );
    sourceURL.setUser( KOPrefs::instance()->mRetrieveUserName );
    sourceURL.setPath( QString::fromLatin1( "/freebusy/" ) + emailName +
		       QString::fromLatin1( ".vfb" ) );
  } else {
    // we use something else
    QString anyurl = KOPrefs::instance()->mRetrieveAnyURL;
    if( anyurl.contains( "%SERVER%" ) )
      anyurl.replace( "%SERVER%", emailHost );
    sourceURL = anyurl;
  }

  return sourceURL;
}

KCal::FreeBusy *FreeBusyManager::iCalToFreeBusy( const QCString &data )
{
  KCal::FreeBusy *fb = 0;
  QString freeBusyVCal = QString::fromUtf8( data );
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar,
                                                                 freeBusyVCal );
  if ( message ) {
    KCal::IncidenceBase *event = message->event();

    if ( !event ) {
      kdError() << "freebusy message has no event" << endl;
    } else {
      fb = dynamic_cast<KCal::FreeBusy *>( event );
      if ( fb ){
        saveFreeBusy( fb, fb->organizer() );
      } else {
        kdError() << "no freebusy" << endl;
      }
    }
  }
  return fb;
}

QString FreeBusyManager::freeBusyDir()
{
  return locateLocal( "data", "korganizer/freebusy" );
}

FreeBusy *FreeBusyManager::loadFreeBusy( const QString &email )
{
  kdDebug() << "FreeBusyManager::loadFreeBusy(): " << email << endl;

  QString fbd = freeBusyDir();

  QFile f( fbd + "/" + email + ".ifb" );
  if ( !f.exists() ) {
    kdDebug() << "FreeBusyManager::loadFreeBusy() " << f.name()
              << " doesn't exist." << endl;
    return 0;
  }
  
  if ( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "FreeBusyManager::loadFreeBusy() Unable to open file "
              << f.name() << endl;
    return 0;
  }
  
  QTextStream ts( &f );
  QString str = ts.read();

  return iCalToFreeBusy( str.utf8() );  
}

bool FreeBusyManager::saveFreeBusy( FreeBusy *freebusy, const QString &email )
{
  kdDebug() << "FreeBusyManager::saveFreeBusy(): " << email << endl;

  QString fbd = freeBusyDir();

  QDir freeBusyDirectory( fbd );
  if ( !freeBusyDirectory.exists() ) {
    kdDebug() << "Directory " << fbd << " does not exist!" << endl;
    kdDebug() << "Creating directory: " << fbd << endl;
    
    if( !freeBusyDirectory.mkdir( fbd, true ) ) {
      kdDebug() << "Could not create directory: " << fbd << endl;
      return false;
    }
  }

  QString filename( fbd );
  filename += "/";
  filename += email;
  filename += ".ifb";
  QFile f( filename );

  kdDebug() << "FreeBusyManager::saveFreeBusy(): filename: " << filename
            << endl;

  freebusy->clearAttendees();
  freebusy->setOrganizer( email );

  QString messageText = mFormat.createScheduleMessage( freebusy,
                                                       Scheduler::Publish );

  if ( !f.open( IO_ReadWrite ) ) {
    kdDebug() << "acceptFreeBusy: Can't open:" << filename << " for writing"
              << endl;
    return false;
  }
  QTextStream t( &f );
  t << messageText;
  f.close();

  return true;
}

#include "freebusymanager.moc"
