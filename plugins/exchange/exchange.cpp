/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qfile.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <kurl.h>
#include <kdebug.h>
#include <krfcdate.h>
#include <kio/job.h>
// #include <kio/jobclasses.h>
// #include <kdirlister.h>

#include <mimelib/string.h>
#include <mimelib/message.h>
#include <mimelib/body.h>
#include <mimelib/bodypart.h>
#include <mimelib/headers.h>
#include <mimelib/mediatyp.h>
#include <mimelib/addrlist.h>
#include <mimelib/mboxlist.h>
#include <mimelib/text.h>

#include <qfile.h>
#include <qinputdialog.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qcstring.h>
#include <qregexp.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kio/slave.h>
#include <kio/scheduler.h>
#include <kio/slavebase.h>
#include <kio/davjob.h>
#include <kio/http.h>

#include <libkcal/icalformat.h>
#include <libkcal/icalformatimpl.h>
extern "C" {
  #include <ical.h>
}

#include "korganizer/korganizer.h"
#include "korganizer/calendarview.h"

#include "exchange.h"
#include "exchangedialog.h"
#include "exchangeconfig.h"
#include "exchangeprogress.h"

class ExchangeFactory : public KOrg::PartFactory {
  public:
    KOrg::Part *create(KOrg::MainWindow *parent, const char *name)
    {
      return new Exchange(parent,name);
    }
};

extern "C" { 
  void *init_libkorg_exchange()
  {
    kdDebug() << "Registering Exchange...\n";
    return (new ExchangeFactory);
  }
}

Exchange::Exchange(KOrg::MainWindow *parent, const char *name) :
  KOrg::Part(parent,name)
{
  kdDebug() << "Creating Exchange...\n";
  setXMLFile("plugins/exchangeui.rc");
  new KAction(i18n("Download..."), 0, this, SLOT(download()),
              actionCollection(), "exchange_download");
  new KAction(i18n("Test"), 0, this, SLOT(test()),
              actionCollection(), "exchange_test");
  KAction *action = new KAction(i18n("Upload Event..."), 0, this, SLOT(upload()),
                                actionCollection(), "exchange_upload");
  QObject::connect(mainWindow()->view(),SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  new KAction(i18n("Configure..."), 0, this, SLOT(configure()),
              actionCollection(), "exchange_configure");
  }

Exchange::~Exchange()
{
  kdDebug() << "Entering Exchange destructor" << endl;
  kdDebug() << "Finished Exchange destructor" << endl;
}

QString Exchange::info()
{
  return i18n("This plugin provides calendar events from an Exchange Server.");
}

QDomElement addElement( QDomDocument& doc, QDomNode& node, const QString& ns, const QString& tag )
{
  QDomElement el = doc.createElementNS( ns, tag );
  node.appendChild( el );
  return el;
}

QDomElement addElement( QDomDocument& doc, QDomNode& node, const QString& ns, const QString& tag, const QString& text )
{
  QDomElement el = doc.createElementNS( ns, tag );
  QDomText textnode = doc.createTextNode( text );
  el.appendChild( textnode );
  node.appendChild( el );
  return el;
}

void Exchange::upload()
{
  kdDebug() << "Called Exchange::upload()" << endl;

  m_currentUpload = mainWindow()->view()->currentSelection();
  if ( ! m_currentUpload )
  {
    KMessageBox::information( 0L, "Please select an appointment", "Exchange Plugin" );
    return;
  }
  KMessageBox::information( 0L, "Exchange Upload is HIGHLY EXPERIMENTAL!", "Exchange Plugin" );
  
  m_currentUploadNumber = 0;

  kdDebug() << "Trying to add appointment " << m_currentUpload->summary() << endl;

  findUid( m_currentUpload->uid() );
}

void Exchange::findUid( QString const& uid )
{
  QString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n";

  kdDebug() << "Find uid query: " << endl << query << endl;
  
  KIO::DavJob* job = KIO::davSearch( getCalendarURL(), "DAV:", "sql", query, false );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotFindUidResult(KIO::Job *)));
}

void Exchange::slotFindUidResult( KIO::Job * job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search uid result: " << endl << response.toString() << endl;

  QDomElement item = response.documentElement().firstChild().toElement();
  QDomElement hrefElement = item.namedItem( "href" ).toElement();
  if ( item.isNull() || hrefElement.isNull() ) {
    // No appointment with this UID in exchange database
    // Create a new filename for this appointment and store it there
    tryExist();
    return;
  }
  // The appointment is already in the exchange database
  // Overwrite it with the new data
  QString href = hrefElement.text();
  KURL url(href);
  url.setProtocol("webdav");
  kdDebug() << "Found URL with identical uid: " << url.prettyURL() << endl;

  startUpload( url );  
}  

void Exchange::tryExist()
{
  // FIXME: we should first check if current's uid is already in the Exchange database
  // Maybe use locking?
  KURL url = getCalendarURL();
  if ( m_currentUploadNumber == 0 )
    url.addPath( m_currentUpload->summary() + ".EML" );
  else
    url.addPath( m_currentUpload->summary() + "-" + QString::number( m_currentUploadNumber ) + ".EML" );

  kdDebug() << "Trying to see whether " << url.prettyURL() << " exists" << endl;

  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propfind" );
  QDomElement prop = addElement( doc, root, "DAV:", "prop" );
  addElement( doc, prop, "DAV:", "displayname" );
  addElement( doc, prop, "urn:schemas:calendar", "uid" );

  KIO::DavJob* job = KIO::davPropFind( url, doc, "0", false );
  job->addMetaData( "errorPage", "false" );
  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotPropFindResult( KIO::Job * ) ) );
}

void Exchange::slotPropFindResult( KIO::Job *job )
{
  int error = job->error(); 
  kdDebug() << "PROPFIND error: " << error << endl;
  if ( error && error != KIO::ERR_DOES_NOT_EXIST )
  {
    job->showErrorDialog( 0L );
    emit finishDownload();
    return;
  }

  if ( !error ) 
  {
    // File exist, try another one
    m_currentUploadNumber++;
    tryExist();
    return;
  }

  // We got a 404 error, resource doesn't exist yet, create it
  // FIXME: race condition possible if resource is created under
  // our nose.

  KURL url = getCalendarURL();
  if ( m_currentUploadNumber == 0 )
    url.addPath( m_currentUpload->summary() + ".EML" );
  else
    url.addPath( m_currentUpload->summary() + "-" + QString::number( m_currentUploadNumber ) + ".EML" );

  startUpload( url );
}

QString timezoneid( int offset ) {
  switch ( offset ) {
    case 0: return "0";
    case -60: return "3";
    case -120: return "5";
    case -180: return "51";
    case -210: return "25";
    case -240: return "24"; // Abu Dhabi
    case -270: return "48"; // Kabul
    case -300: return "47"; // Islamabad
    case -330: return "23"; // Bombay
    case -360: return "46"; // Dhaka
    case -420: return "22"; // Bangkok
    case -480: return "45"; // Beijing
    case -540: return "20"; // Tokyo
    case -570: return "44"; // Darwin
    case -600: return "18"; // Brisbane
    case -660: return "41"; // Solomon Islands
    case -720: return "17"; // Auckland
    case 60: return "29"; // Azores
    case 120: return "30"; // Mid Atlantic
    case 180: return "8"; // Brasilia
    case 210: return "28";  // Newfoundland
    case 240: return "9"; // Atlantic time Canada
    case 300: return "10"; // Eastern
    case 360: return "11"; // Central time
    case 420: return "12"; // Mountain time
    case 480: return "13"; // Pacific time
    case 540: return "14"; // Alaska time
    case 600: return "15"; // Hawaii
    case 660: return "16"; // Midway Island
    case 720: return "39"; // Eniwetok
    default: return "52"; // Invalid time zone
  }
}


void Exchange::startUpload( KURL const& url )
{
  Event* event = static_cast<Event *>( m_currentUpload );
  if ( ! event ) {
    kdDebug() << "ERROR: trying to upload a non-Event Incidence" << endl;
    return;
  }

  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propertyupdate" );
  QDomElement set = addElement( doc, root, "DAV:", "set" );
  QDomElement prop = addElement( doc, set, "DAV:", "prop" );
  addElement( doc, prop, "DAV:", "contentclass", "urn:content-classes:appointment" );
  addElement( doc, prop, "http://schemas.microsoft.com/exchange/", "outlookmessageclass", "IPM.appointment" );
 // addElement( doc, prop, "urn:schemas:calendar:", "method", "Add" );
  addElement( doc, prop, "urn:schemas:calendar:", "alldayevent", 
      event->doesFloat() ? "1" : "0" );
  addElement( doc, prop, "urn:schemas:calendar:", "busystatus", 
      event->transparency() ? "Free" : "Busy" );
  // KLUDGE: somehow we need to take the opposite of the
  // value that localUTCOffset() supplies...
  int tzOffset = - KRFCDate::localUTCOffset(); 
  QString offsetString;
  if ( tzOffset==0 ) 
    offsetString = "Z";
  else if ( tzOffset > 0 ) 
    offsetString = QString( "+%1:%2" ).arg(tzOffset/60, 2).arg( tzOffset%60, 2 );
  else
    offsetString = QString( "-%1:%2" ).arg((-tzOffset)/60, 2).arg( (-tzOffset)%60, 2 );
  offsetString = offsetString.replace( QRegExp(" "), "0" );

  kdDebug() << "Timezone offset: " << tzOffset << " : " << offsetString << endl;
  addElement( doc, prop, "urn:schemas:calendar:", "dtstart", 
      event->dtStart().toString( "yyyy-MM-ddThh:mm:ss.zzz" )+ offsetString );
  //    event->dtStart().toString( "yyyy-MM-ddThh:mm:ss.zzzZ" ) );
  //    2002-06-04T08:00:00.000Z" );
  addElement( doc, prop, "urn:schemas:calendar:", "dtend", 
      event->dtEnd().toString( "yyyy-MM-ddThh:mm:ss.zzz" ) + offsetString );
//  addElement( doc, prop, "urn:schemas:calendar:", "meetingstatus", "confirmed" );
  addElement( doc, prop, "urn:schemas:httpmail:", "textdescription", event->description() );
  addElement( doc, prop, "urn:schemas:httpmail:", "subject", event->summary() );
  addElement( doc, prop, "urn:schemas:calendar:", "location", event->location() );
  // addElement( doc, prop, "urn:schemas:mailheader:", "subject", event->summary() );
  addElement( doc, prop, "urn:schemas:calendar:", "uid", event->uid() );

  KCal::Recurrence *recurrence = event->recurrence();
  kdDebug() << "Recurrence->doesRecur(): " << recurrence->doesRecur() << endl;
  if ( recurrence->doesRecur() != KCal::Recurrence::rNone ) {
    addElement( doc, prop, "urn:schemas:calendar:", "instancetype", "1" );
    ICalFormat *format = new ICalFormat();
    QString recurstr = format->toString( recurrence );
    // Strip leading "RRULE\n :" and whitespace
    recurstr = recurstr.replace( QRegExp("^[A-Z]*[\\s]*:"), "").stripWhiteSpace();
    kdDebug() << "Recurrence rule after replace: \"" << recurstr << "\"" << endl;
    delete format;
    QDomElement rrule = addElement( doc, prop, "urn:schemas:calendar:", "rrule" );
    addElement( doc, rrule, "xml:", "v", recurstr );
    addElement( doc, prop, "urn:schemas:calendar:", "timezoneid", timezoneid( tzOffset ) );
  } else {
    addElement( doc, prop, "urn:schemas:calendar:", "instancetype", "0" );
  }
  kdDebug() << doc.toString() << endl;

  KIO::DavJob *job2 = KIO::davPropPatch( url, doc, false );
  connect( job2, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotPatchResult( KIO::Job * ) ) );
}

void Exchange::slotPatchResult( KIO::Job* job )
{
  kdDebug() << "Patch result" << endl;
  QDomDocument response = static_cast<KIO::DavJob *>( job )->response();
  kdDebug() << response.toString() << endl;
}

void Exchange::configure()
{
  kdDebug() << "Exchange::configure" << endl;
  ExchangeConfig dialog;
  
  if (dialog.exec() != QDialog::Accepted ) 
    return;
}

void Exchange::test()
{
  kdDebug() << "Entering test()" << endl;
  baseURL = KURL( "http://mail.tbm.tudelft.nl/janb/Calendar" );
  calendar = mainWindow()->view()->calendar();
  KURL url( "webdav://mail.tbm.tudelft.nl/janb/Calendar/TestTest.EML" );
/*
  kdDebug() << "GET url: " << url.prettyURL() << endl;
    
  KIO::TransferJob *job2 = KIO::get(url, false, false);
  KIO::Scheduler::scheduleJob(job2);
  job2->addMetaData("davHeader", "Translate: f\r\n");
  connect( job2, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(slotData(KIO::Job *, const QByteArray &)));
  connect( job2, SIGNAL( result ( KIO::Job * ) ), SLOT ( slotTransferResult( KIO:: Job * ) ) );
*/ 
/*
  QString query = // "<D:sql>\r\n"
        "SELECT *\r\n" 
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"DAV:displayname\" = \"IWB.EML\"\r\n";
        // "</D:sql>\r\n";

  kdDebug() << query << endl;
  KIO::DavJob* job = KIO::davSearch( baseURL, "DAV:", "sql", query, false );
*/
  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propfind" );
  addElement( doc, root, "DAV:", "allprop" );
//  QDomElement prop = addElement( doc, root, "DAV:", "prop" );
//  addElement( doc, prop, "urn:schemas:calendar:", "rrule" );
//  addElement( doc, prop, "urn:schemas:calendar:", "uid" );
//  addElement( doc, prop, "DAV:", "displayname" );


  KIO::DavJob* job = KIO::davPropFind( url, doc, "0", false );
  job->addMetaData( "errorPage", "false" );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotTestResult(KIO::Job *)));
}

void Exchange::slotTestResult( KIO::Job * job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Test result: " << endl << response.toString() << endl;
}
 
void Exchange::test2()
{
  kdDebug() << "Entering test2()" << endl;

}

KURL Exchange::getBaseURL()
{
  kapp->config()->setGroup("Calendar/Exchange Plugin");
  QString host = kapp->config()->readEntry( "host" );
  if ( host.isNull() ) {
    KURL url;
    return url;
  }
  QString user = kapp->config()->readEntry( "user" );
  if ( user.isNull() ) {
    KURL url;
    return url;
  }
  KURL url( "webdav://" + host + "/" + user );
  return url;
}

KURL Exchange::getCalendarURL()
{
  KURL url = getBaseURL();
  if ( url.isValid() )
    url.addPath( "Calendar" );
  return url;
}

void Exchange::download()
{
  ExchangeDialog dialog( mainWindow()->view()->startDate(), mainWindow()->view()->endDate() );
  
  if (dialog.exec() != QDialog::Accepted ) 
    return;

  QDate start = dialog.m_start->date();
  QDate end = dialog.m_end->date();
  
  baseURL = getCalendarURL();
  if ( ! baseURL.isValid() ) {
    KMessageBox::sorry( 0L, "Please configure the Exchange host and user", "Exchange Plugin" );
    return;
  }
	
  calendar = mainWindow()->view()->calendar();
  
  ExchangeProgress *progress;
  progress = new ExchangeProgress();
  
  connect( this, SIGNAL(startDownload()), progress, SLOT(slotTransferStarted()) );
  connect( this, SIGNAL(finishDownload()), progress, SLOT(slotTransferFinished()) );
  connect( progress, SIGNAL(complete( ExchangeProgress* )), this, SLOT(slotComplete( ExchangeProgress* )) );
    
  QString startString;
  startString.sprintf("%04i/%02i/%02i",start.year(),start.month(),start.day());
  QString endString;
  endString.sprintf("%04i/%02i/%02i",end.year(),end.month(),end.day());
  QString sql = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\", \"urn:schemas:calendar:uid\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:dtend\" > '" + startString + "'\r\n"
        "AND \"urn:schemas:calendar:dtstart\" < '" + endString + "'";
  
  kdDebug() << "Exchange download query: " << endl << sql << endl;

  emit startDownload();

  KIO::DavJob *job = KIO::davSearch( baseURL, "DAV:", "sql", sql, false );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotSearchResult(KIO::Job *)));
}

void Exchange::slotSearchResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit finishDownload();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search result: " << endl << response.toString() << endl;

  handleAppointments( response, true );
  
  emit finishDownload();
}

void Exchange::slotMasterResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    emit finishDownload();
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  kdDebug() << "Search (master) result: " << endl << response.toString() << endl;

  handleAppointments( response, false );
  
  emit finishDownload();
}

void Exchange::handleAppointments( const QDomDocument& response, bool recurrence ) {
  kdDebug() << "Entering handleAppointments" << endl;
  for( QDomElement item = response.documentElement().firstChild().toElement();
       !item.isNull();
       item = item.nextSibling().toElement() )
  {
    kdDebug() << "Current item:" << item.tagName() << endl;
    QDomNodeList propstats = item.elementsByTagNameNS( "DAV:", "propstat" );
    kdDebug() << "Item has " << propstats.count() << " propstat children" << endl; 
    for( uint i=0; i < propstats.count(); i++ )
    {
      QDomElement propstat = propstats.item(i).toElement();
      QDomElement prop = propstat.namedItem( "prop" ).toElement();
      if ( prop.isNull() )
      {
        kdDebug() << "Error: no <prop> in response" << endl;
	continue;
      }

      QDomElement instancetypeElement = prop.namedItem( "instancetype" ).toElement();
      if ( instancetypeElement.isNull() ) {
        kdDebug() << "Error: no instance type in Exchange server reply" << endl;
        continue;
      }
      int instanceType = instancetypeElement.text().toInt();
      kdDebug() << "Instance type: " << instanceType << endl;
    
      if ( recurrence && instanceType > 0 ) {
        QDomElement uidElement = prop.namedItem( "uid" ).toElement();
        if ( uidElement.isNull() ) {
          kdDebug() << "Error: no uid in Exchange server reply" << endl;
          continue;
        }
        QString uid = uidElement.text();
        if ( ! m_uids.contains( uid ) ) {
          m_uids[uid] = 1;
          handleRecurrence(uid);
        }
        continue;
      }

      QDomElement hrefElement = prop.namedItem( "href" ).toElement();
      if ( instancetypeElement.isNull() ) {
        kdDebug() << "Error: no href in Exchange server reply" << endl;
        continue;
      }
      QString href = hrefElement.text();
      KURL url(href);
      url.setProtocol("webdav");
      kdDebug() << "GET url: " << url.prettyURL() << endl;
    
      emit startDownload();
      KIO::TransferJob *job2 = KIO::get(url, false, false);
      KIO::Scheduler::scheduleJob(job2);
      job2->addMetaData("davHeader", "Translate: f\r\n");
      connect( job2, SIGNAL(data(KIO::Job *, const QByteArray &)), this, SLOT(slotData(KIO::Job *, const QByteArray &)));
      connect( job2, SIGNAL( result ( KIO::Job * ) ), SLOT ( slotTransferResult( KIO:: Job * ) ) );
    }
  }
}  

void Exchange::handleRecurrence(QString uid) {
  kdDebug() << "Handling recurrence info for uid=" << uid << endl;
  QString query = 
        "SELECT \"DAV:href\", \"urn:schemas:calendar:instancetype\"\r\n"
        "FROM Scope('shallow traversal of \"\"')\r\n"
        "WHERE \"urn:schemas:calendar:uid\" = '" + uid + "'\r\n"
	" AND (\"urn:schemas:calendar:instancetype\" = 1)\r\n";
//	"      OR \"urn:schemas:calendar:instancetype\" = 3)\r\n" // FIXME: exception are not handled

  kdDebug() << "Exchange master query: " << endl << query << endl;

  emit startDownload();
 
  KIO::DavJob* job = KIO::davSearch( baseURL, "DAV:", "sql", query, false );
  connect(job, SIGNAL(result( KIO::Job * )), this, SLOT(slotMasterResult(KIO::Job *)));
}

void Exchange::slotData(KIO::Job *job, const QByteArray &data) {
  KURL url = static_cast<KIO::TransferJob *>(job)->url();
  kdDebug() << "Got data for " << url.prettyURL() << endl;
  
  if(data.size() != 0)
  {
    DwString *messageData;
    if ( !m_transferJobs.contains( url.url() ) ) 
    { 
      messageData = new DwString();
      m_transferJobs[url.url()] = messageData;
    } else {
      messageData = m_transferJobs[url.url()];
    }

    // DwString string(data.data(), data.size());
    messageData->append(data.data(), data.size());
    // kdDebug() << messageData->c_str() << endl;
    // delete string;
  }
}
   
void Exchange::slotTransferResult(KIO::Job *job) {
  KURL url = static_cast<KIO::TransferJob *>(job)->url();
  kdDebug() << "Transfer " << url.prettyURL() << " finished" << endl;
  
  if ( job->error() ) {
    job->showErrorDialog( 0L );
    if ( m_transferJobs.contains( url.url() ) ) {
      delete m_transferJobs[url.url()];
      m_transferJobs.remove( url.url() );
    }
    emit finishDownload();
    return;
  }

  // kdDebug() << "Message: " << messageData->c_str() << endl;
  if ( !m_transferJobs.contains( url.url() ) ) {
    kdDebug() << "WARNING: no data!" << endl;
    emit finishDownload();
    return;
  }
  
  DwString *messageData = m_transferJobs[url.url()];
  DwMessage msg( *messageData );
  msg.Parse();
  handlePart(&msg);
  DwBody body = msg.Body();
  for ( DwBodyPart *part=body.FirstBodyPart(); part; part = part->Next() )
  {
    handlePart(part);
  }
  m_transferJobs.remove( url.url() );
  delete messageData;
  emit finishDownload();
  kdDebug() << "Finished slotTransferREsult" << endl;
}

void Exchange::handlePart( DwEntity *part ) {
  // kdDebug() << "part text:" << endl << part->Body().AsString().c_str() << endl;
  DwMediaType contType = part->Headers().ContentType();
  if ( contType.TypeStr()=="text" && contType.SubtypeStr()=="calendar" ) {
    kdDebug() << "CALENDAR!" <<endl;
    kdDebug() << "VCalendar text:" << endl << "---- BEGIN ----" << endl << part->Body().AsString().c_str() << "---- END ---" << endl;
    ICalFormat *format = new ICalFormat();
    bool result = format->fromString( calendar, part->Body().AsString().c_str() );
    delete format;
    kdDebug() << "Result:" << result << endl;
  } else {
    kdDebug() << contType.TypeStr().c_str() << "/" << contType.SubtypeStr().c_str() << endl;
  }
}

void Exchange::slotComplete( ExchangeProgress *progress )
{
  kdDebug() << "Entering slotComplete()" << endl;
  calendar->setModified( true );
  disconnect( this, 0, progress, 0 );
  disconnect( progress, 0, this, 0 );
  progress->delayedDestruct();
}

