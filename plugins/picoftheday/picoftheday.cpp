/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "picoftheday.h"

#include <QtCore/QSize>
#include <QtXml/QDomDocument>

#include <KConfig>
#include <KDebug>
#include <KIO/Scheduler>
#include <KLocale>
#include <KStandardDirs>

#include "koglobals.h"

#include "configdialog.h"

#include "picoftheday.moc"

using namespace KOrg::CalendarDecoration;

class PicofthedayFactory : public DecorationFactory {
  public:
    Decoration *create() { return new Picoftheday; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_picoftheday, PicofthedayFactory )


Picoftheday::Picoftheday()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals ); // TODO: access via korg's std method?
  KConfigGroup config( &_config, "Picture of the Day Plugin" );
  mThumbSize = config.readEntry( "InitialThumbnailSize", QSize(120,120) );
}

Picoftheday::~Picoftheday()
{
}

/*void Picoftheday::configure( QWidget *parent )
{
  ConfigDialog dlg( parent );
}*/

QString Picoftheday::info()
{
  return i18n("<qt>This plugin provides the Wikipedia "
              "<i>Picture of the Day</i>.</qt>");
}

Element::List Picoftheday::createDayElements( const QDate &date )
{
  Element::List elements;

  POTDElement *element = new POTDElement( date, mThumbSize );
  elements.append( element );

  return elements;
}

////////////////////////////////////////////////////////////////////////////////

POTDElement::POTDElement( const QDate &date, const QSize &initialThumbSize )
  : mDate( date ), mThumbSize( initialThumbSize )
{
  setShortText( i18n("Loading…") );
  setLongText( i18n("<qt>Loading <i>Picture of the Day</i>…</qt>") );
  download();
}

/** First step of three in the download process */
void POTDElement::download()
{
  KUrl url = KUrl( "http://commons.wikimedia.org/wiki/Template:Potd/"
                   + mDate.toString(Qt::ISODate) + "?action=raw" );
               // The file at that URL contains the file name for the POTD

  KIO::SimpleJob *job = KIO::storedGet( url, false, false );
  KIO::Scheduler::scheduleJob( job );

  connect( job,  SIGNAL( result(KJob *) ),
           this, SLOT( downloadStep1Result(KJob *) ) );
}

/**
  Give it a job which fetched the raw page,
  and it'll give you the image file name hiding in it.
 */
void POTDElement::downloadStep1Result( KJob* job )
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

/** Second step of three in the download process */
void POTDElement::getImagePage()
{
  mUrl = KUrl( "http://commons.wikimedia.org/wiki/Image:" + mFileName );
  // We'll find the info to get the thumbnail we want on the POTD's image page

  emit gotNewUrl( mUrl );
  mShortText = i18n("Picture Page");
  emit gotNewShortText( mShortText );

  KIO::SimpleJob *job = KIO::storedGet( mUrl, false, false );
  KIO::Scheduler::scheduleJob( job );

  connect( job,  SIGNAL( result(KJob *) ),
           this, SLOT( downloadStep2Result(KJob *) ) );
}

/**
  Give it a job which fetched the image page,
  and it'll give you the appropriate thumbnail URL.
 */
void POTDElement::downloadStep2Result( KJob* job )
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
      mLongText = mDescription;
      emit gotNewLongText( mLongText );
//      break; // TODO: make this more reliable (for now we use the last desc.,
//             // but it may not be the POTD desc...)
    }
  }

  // We go through all links and stop at the first right-looking candidate
  QDomNodeList links = imgPage.elementsByTagName("a");
  for ( int i=0; i<links.length(); i++ ) {
    QString href = links.item(i).attributes().namedItem("href").nodeValue();
    if ( href.startsWith("http://upload.wikimedia.org/wikipedia/commons/") ) {
      mFullSizeImageUrl = href;
      break;
    }
  }

  // We get the image's width/height ratio
  mHWRatio = 1.0;
  QDomNodeList images = imgPage.elementsByTagName("img");
  for ( int i=0; i<links.length(); i++ ) {
    QDomNamedNodeMap attr = images.item(i).attributes();
    QString src = attr.namedItem("src").nodeValue();

    if ( src.startsWith( thumbnailUrl( mFullSizeImageUrl ).url() ) ) {
      if ( ( attr.namedItem("height").nodeValue().toInt() != 0 )
           && ( attr.namedItem("width").nodeValue().toInt() != 0 ) ) {
        mHWRatio = attr.namedItem("height").nodeValue().toFloat()
                  / attr.namedItem("width").nodeValue().toFloat();
      }
      break;
    }

  }
  kDebug() << "picoftheday Plugin: h/w ratio: " << mHWRatio << endl;

  kDebug() << "picoftheday Plugin: got POTD image page source: "
           << mFullSizeImageUrl << endl;

  getThumbnail();
}

/** Returns the thumbnail URL for a given width corresponding to a full-size
    image URL */
KUrl POTDElement::thumbnailUrl( const KUrl &fullSizeUrl, const int width ) const
{
  QString thumbUrl = fullSizeUrl.url();
  if ( width != 0 ) {
    thumbUrl.replace(
      QRegExp("http://upload.wikimedia.org/wikipedia/commons/(.*)/([^/]*)"),
      "http://upload.wikimedia.org/wikipedia/commons/thumb/\\1/\\2/"
        + QString::number( width ) + "px-\\2"
    );
  } else {  // This will not return a valid thumbnail URL, but will at least
            // give some info (the beginning of the URL)
    thumbUrl.replace(
      QRegExp("http://upload.wikimedia.org/wikipedia/commons/(.*)/([^/]*)"),
      "http://upload.wikimedia.org/wikipedia/commons/thumb/\\1/\\2"
    );
  }
  return KUrl( thumbUrl );
}

/** Third step of three in the downloading process */
void POTDElement::getThumbnail()
{
  int thumbWidth = mThumbSize.width();
  if ( mThumbSize.height() /* the requested height */
       < mThumbSize.width() * mHWRatio /* the thumbnail's height,
                                          based on the requested width
                                          and the image's ratio */
     ) {  /* We would download too much, as the downloaded picture would be
             taller than requested, so we adjust the width of the picture to
             be downloaded in consequence */
    thumbWidth /= ( ( mThumbSize.width() * mHWRatio ) / mThumbSize.height() );
  }
  kDebug() << "picoftheday Plugin: will download thumbnail of size "
           << QSize( thumbWidth, thumbWidth * mHWRatio ) << endl;
  QString thumbUrl = thumbnailUrl( mFullSizeImageUrl, thumbWidth ).url();

  kDebug() << "picoftheday Plugin: got POTD thumbnail URL: " 
           << thumbUrl << endl;
  mThumbUrl = thumbUrl;

  KIO::SimpleJob *job = KIO::storedGet( thumbUrl, false, false );
  KIO::Scheduler::scheduleJob( job );

  connect( job, SIGNAL(result(KJob *)),
           this, SLOT(downloadStep3Result(KJob *)) );
}

/**
  Give it a job which fetched the thumbnail,
  and it'll give the corresponding pixmap to you.
 */
void POTDElement::downloadStep3Result( KJob* job )
{
  if (job->error())
  {
    kWarning() << "picoftheday Plugin: could not get POTD: "
               << job->errorString() << endl;
    return;
  }

  // Last step completed: we get the pixmap from the transfer job's data
  KIO::StoredTransferJob* const transferJob =
    static_cast<KIO::StoredTransferJob*>( job );
  if ( mPixmap.loadFromData( transferJob->data() ) ) {
    kDebug() << "picoftheday Plugin: got POTD. " << endl;
    emit gotNewPixmap( mPixmap.scaled( mThumbSize, Qt::KeepAspectRatio,
                       Qt::SmoothTransformation ) );
  }
}

QPixmap POTDElement::pixmap( const QSize &size )
{
  kDebug() << "picoftheday Plugin: called for a new pixmap size ("
           << size << " instead of " << mThumbSize << ")" << endl;
  if ( mPixmap.width() < size.width() || mPixmap.height() < size.height() ) {
    setThumbnailSize( size );

    if ( mFullSizeImageUrl.url().isEmpty() ) {
      if ( mFileName.isEmpty() ) {
        download();
      } else {
        getImagePage();
      }
    } else {
      getThumbnail();
    }
  }
  return mPixmap.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
}

void POTDElement::setThumbnailSize( const QSize &size )
{
  mThumbSize = size;
}
