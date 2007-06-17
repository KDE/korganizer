/*
    This file is part of KOrganizer.
    Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
*/

#include "potdwidget.h"
#include "potdwidget.moc"

#include <ktoolinvocation.h>
#include <klocale.h>
#include <kio/scheduler.h>
#include <kdebug.h>

#include <qdom.h>

POTDWidget::POTDWidget(QWidget* parent)
  : KUrlLabel( parent )
{
  connect( this, SIGNAL( leftClickedUrl(const QString&) ),
           this, SLOT( invokeBrowser(const QString&) ) );
}

POTDWidget::~POTDWidget()
{
}

void POTDWidget::loadPOTD( const QDate &date )
{
  KUrl *url = new KUrl( "http://commons.wikimedia.org/wiki/Template:Potd/"
                        + date.toString(Qt::ISODate) + "?action=raw" );

  KJob *job = KIO::storedGet( *url, false, false );
// TODO: change to KIO::get() and KIO::TransferJob?
//  KIO::Scheduler::scheduleJob(job);

  connect( job,  SIGNAL( result(KJob *) ),
           this, SLOT( gotFileName(KJob *) ) );
}

void POTDWidget::gotFileName( KJob* job )
{
  if ( job->error() )
  {
    kWarning() << "POTD: could not get POTD file name: " << job->errorString() << endl;
    return;
  }

  // First step completed: we now know the POTD's file name
  KIO::StoredTransferJob* const storedJob =
    static_cast<KIO::StoredTransferJob*>( job );
  mFileName =
    QString::fromUtf8( storedJob->data().data(), storedJob->data().size() );
  kDebug() << "POTD: got POTD file name: " << mFileName << endl;

  KUrl *url =
    new KUrl( "http://commons.wikimedia.org/wiki/Image:" + mFileName );

  setUrl( url->url() );

  KJob *imgJob = KIO::storedGet( *url, false, false );
// TODO: change to KIO::get() and KIO::TransferJob?
//  KIO::Scheduler::scheduleJob(imgJob);

  connect( imgJob,  SIGNAL( result(KJob *) ),
           this, SLOT( gotImagePageUrl(KJob *) ) );
}

void POTDWidget::gotImagePageUrl(KJob* job)
{
  if (job->error())
  {
    kWarning() << "POTD: could not get POTD image page: " 
               << job->errorString() << endl;
    return;
  }

  // Get the image URL from the image page's source code
  // and transform it to get an appropriate thumbnail size
  KIO::StoredTransferJob* const storedJob =
    static_cast<KIO::StoredTransferJob*>( job );
  
  QDomDocument imgPage;
  if( !imgPage.setContent( QString::fromUtf8( storedJob->data().data(),
                                              storedJob->data().size() ) ) ) {
    kWarning() << "Wikipedia returned an invalid XML page for image "
               << mFileName << endl;
    return;
  }

  QDomNodeList divs = imgPage.elementsByTagName("div");
  QString wikipediaLanguage = KGlobal::locale()->language();
  wikipediaLanguage.replace( QRegExp("^([^_][^_]*)_.*$"), "\\1" );
  for ( int i=0; i<divs.length(); i++ ) {
    if ( QString( divs.item(i).attributes().namedItem("class").nodeValue() ) ==
           QString( "description " + wikipediaLanguage ) ) {
      // TODO: delete the <span> tag before the real description text
      mDescription = QString( divs.item(i).toElement().text() );
      setToolTip( mDescription );
//      break; // todo: make this more reliable (for now we use the last desc.,
//             // but it may not be the POTD desc...)
    }
  }


  // We go through all links and stop at the first right-looking candidate
  QDomNodeList links = imgPage.elementsByTagName("a");
  for ( int i=0; i<links.length(); i++ ) {
    QString href =
      QString( links.item(i).attributes().namedItem("href").nodeValue() );
    if ( href.startsWith("http://upload.wikimedia.org/wikipedia/commons/") ) {
      mImagePageUrl = href;
      break;
    }
  }
 
  kDebug() << "POTD: got POTD image page source: " << mImagePageUrl << endl;

  QString thumbUrl = mImagePageUrl.url();
  // From e.g. http://upload.wikimedia.org/wikipedia/commons/6/66/Agasthiyamalai_range_and_Tirunelveli_rainshadow.jpg
  // to http://upload.wikimedia.org/wikipedia/commons/thumb/6/66/Agasthiyamalai_range_and_Tirunelveli_rainshadow.jpg/800px-Agasthiyamalai_range_and_Tirunelveli_rainshadow.jpg
  //TODO: use parent's width or find sth better? (300px?)
  QWidget* p = static_cast<QWidget*>( parent() );
  kDebug() << "POTD: parent w: " << p->width() << endl;
//  kDebug() << QString::number(2 * 50 * int(p->width()/50+1)) << endl;
  thumbUrl.replace(
    QRegExp("http://upload.wikimedia.org/wikipedia/commons/(.*)/([^/]*)"),
    "http://upload.wikimedia.org/wikipedia/commons/thumb/\\1/\\2/" 
      + QString::number(100) + "px-\\2"
    );

  kDebug() << "POTD: got POTD thumbnail URL: " << thumbUrl << endl;
  mThumbUrl = thumbUrl;

  KJob *potdJob = KIO::storedGet(mThumbUrl, false, false);
// TODO: change to KIO::get() and KIO::TransferJob?
//  KIO::Scheduler::scheduleJob(imgJob);

  connect(potdJob, SIGNAL(result(KJob *)),
          this, SLOT(gotPOTD(KJob *)));
}

void POTDWidget::gotPOTD(KJob* job)
{
  if (job->error())
  {
    kWarning() << "POTD: could not get POTD: " << job->errorString() << endl;
    return;
  }

  // First step completed: we now know the POTD's file name
  KIO::StoredTransferJob* const storedJob =
    static_cast<KIO::StoredTransferJob*>( job );
  QPixmap *p = new QPixmap();
  if ( p->loadFromData(storedJob->data()) ) {
    kDebug() << "POTD: got POTD. " << endl;
    setPixmap(*p);
  //FIXME: thumbnail is fixed at 100px for now
  }
}


void POTDWidget::invokeBrowser( const QString &url ) {
  KToolInvocation::invokeBrowser( url );
}

