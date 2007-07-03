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

POTDWidget::POTDWidget( QWidget* parent )
  : KUrlLabel( parent )
{
  mThumbSize = 120;
  resize( 120, 120 );
  mARMode = Qt::KeepAspectRatio;
  mDate = QDate::currentDate();
  connect( this, SIGNAL( leftClickedUrl(const QString&) ),
           this, SLOT( invokeBrowser(const QString&) ) );
}

POTDWidget::~POTDWidget()
{
}

void POTDWidget::setAspectRatioMode( const Qt::AspectRatioMode mode )
{
  mARMode = mode;
}

void POTDWidget::setThumbnailSize( const int size )
{
  mThumbSize = size;
}

void POTDWidget::setDate( const QDate &date )
{
  mDate = date;
}

void POTDWidget::downloadPOTD()
{
  KUrl *url = new KUrl( "http://commons.wikimedia.org/wiki/Template:Potd/"
                        + mDate.toString(Qt::ISODate) + "?action=raw" );
               // The file at that URL contains the file name for the POTD

  KIO::SimpleJob *job = KIO::storedGet( *url, false, false );
  KIO::Scheduler::scheduleJob( job );

  connect( job,  SIGNAL( result(KJob *) ),
           this, SLOT( downloadStep1Result(KJob *) ) );
}

/**
  Give it a job which fetched the raw page,
  and it'll give you the image file name hiding in it.
 */
void POTDWidget::downloadStep1Result( KJob* job )
{
  if ( job->error() )
  {
    kWarning() << "picoftheday Plugin: could not get POTD file name: "
               << job->errorString() << endl;
    return;
  }

  // First step completed: we now know the POTD's file name
  KIO::StoredTransferJob* const transferJob =
    static_cast<KIO::StoredTransferJob*>( job );
  mFileName = QString::fromUtf8( transferJob->data().data(),
                                 transferJob->data().size() );
  kDebug() << "picoftheday Plugin: got POTD file name: " << mFileName << endl;

  getImagePage();
}

void POTDWidget::getImagePage()
{
  KUrl *url =
    new KUrl( "http://commons.wikimedia.org/wiki/Image:" + mFileName );
  // We'll find the info to get the thumbnail we want on the POTD's image page

  setUrl( url->url() );

  KIO::SimpleJob *job = KIO::storedGet( *url, false, false );
  KIO::Scheduler::scheduleJob( job );

  connect( job,  SIGNAL( result(KJob *) ),
           this, SLOT( downloadStep2Result(KJob *) ) );
}

/**
  Give it a job which fetched the image page,
  and it'll give you the appropriate thumbnail URL.
 */
void POTDWidget::downloadStep2Result( KJob* job )
{
  if ( job->error() )
  {
    kWarning() << "picoftheday Plugin: could not get POTD image page: " 
               << job->errorString() << endl;
    return;
  }

  // Get the image URL from the image page's source code
  // and transform it to get an appropriate thumbnail size
  KIO::StoredTransferJob* const transferJob =
    static_cast<KIO::StoredTransferJob*>( job );
  
  QDomDocument imgPage;
  if ( !imgPage.setContent( QString::fromUtf8( transferJob->data().data(),
                                             transferJob->data().size() ) ) ) {
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
 
  kDebug() << "picoftheday Plugin: got POTD image page source: " 
           << mImagePageUrl << endl;

  generateThumbnailUrl();

  getThumbnail();
}

void POTDWidget::generateThumbnailUrl()
{
  QString thumbUrl = mImagePageUrl.url();
  thumbUrl.replace(
    QRegExp("http://upload.wikimedia.org/wikipedia/commons/(.*)/([^/]*)"),
    "http://upload.wikimedia.org/wikipedia/commons/thumb/\\1/\\2/" 
      + QString::number( mThumbSize+200 ) + "px-\\2"
    );

  kDebug() << "picoftheday Plugin: got POTD thumbnail URL: " 
           << thumbUrl << endl;
  mThumbUrl = thumbUrl;
}

void POTDWidget::getThumbnail()
{
  KIO::SimpleJob *job = KIO::storedGet( mThumbUrl, false, false );
  KIO::Scheduler::scheduleJob( job );

  connect( job, SIGNAL(result(KJob *)),
           this, SLOT(downloadStep3Result(KJob *)) );
}

void POTDWidget::downloadStep3Result( KJob* job )
{
  if (job->error())
  {
    kWarning() << "picoftheday Plugin: could not get POTD: "
               << job->errorString() << endl;
    return;
  }

  // First step completed: we now know the POTD's file name
  KIO::StoredTransferJob* const transferJob =
    static_cast<KIO::StoredTransferJob*>( job );
  if ( mPixmap.loadFromData( transferJob->data() ) ) {
    kDebug() << "picoftheday Plugin: got POTD. " << endl;
    setPixmap( mPixmap.scaled( mThumbSize, mThumbSize, mARMode ) );
  }
}

void POTDWidget::invokeBrowser( const QString &url ) {
  KToolInvocation::invokeBrowser( url );
}

//TODO: this is still a work-in-progress
void POTDWidget::resizeEvent( QResizeEvent *event )
{
/*  kDebug() << "picoftheday Plugin: I GOT A RESIZE EVENT! "
           << "new width: " << width() << " instead of " << event->oldSize().width()
           << "new height: " << height() << " instead of " << event->oldSize().height();*/
  mThumbSize = width();
  if ( ( width() > mPixmap.width() || height() > mPixmap.height() )
       && !mImagePageUrl.isEmpty() ) {
    int newThumbSize = qMax( width() + 200, mPixmap.width() );
//         FIXME TODO
//         resizeImage(&image, QSize(newWidth, newHeight));
//         update();

  generateThumbnailUrl();

  getThumbnail();

  }
     QWidget::resizeEvent( event );
}

