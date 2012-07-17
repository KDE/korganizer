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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "picoftheday.h"
#include "configdialog.h"

#include <KConfig>
#include <KDebug>
#include <KIO/Scheduler>

#include <QDomDocument>

class PicofthedayFactory : public DecorationFactory
{
  public:
    Decoration *createPluginFactory() { return new Picoftheday; }
};

K_EXPORT_PLUGIN( PicofthedayFactory )

Picoftheday::Picoftheday()
{
  KConfig _config( "korganizerrc" );
  KConfigGroup config( &_config, "Picture of the Day Plugin" );
  mThumbSize = config.readEntry( "InitialThumbnailSize", QSize( 120, 60 ) );
}

Picoftheday::~Picoftheday()
{
}

void Picoftheday::configure( QWidget *parent )
{
  ConfigDialog dlg( parent );
  dlg.exec();
}

QString Picoftheday::info() const
{
  return i18n( "<qt>This plugin provides the Wikipedia "
               "<i>Picture of the Day</i>.</qt>" );
}

Element::List Picoftheday::createDayElements( const QDate &date )
{
  Element::List elements;

  POTDElement *element = new POTDElement( "main element", date, mThumbSize );
  elements.append( element );

  return elements;
}

////////////////////////////////////////////////////////////////////////////////

POTDElement::POTDElement( const QString &id, const QDate &date,
                          const QSize &initialThumbSize )
  : StoredElement( id ), mDate( date ), mThumbSize( initialThumbSize ),
    mFirstStepCompleted( false ),
    mSecondStepCompleted( false ),
    mFirstStepJob( 0 ), mSecondStepJob( 0 ), mThirdStepJob( 0 )
{
  setShortText( i18n( "Loading..." ) );
  setLongText( i18n( "<qt>Loading <i>Picture of the Day</i>...</qt>" ) );

  mTimer = new QTimer( this );
  mTimer->setSingleShot( true );

  step1StartDownload();
}

/** First step of three in the download process */
void POTDElement::step1StartDownload()
{
  // Start downloading the picture
  if ( !mFirstStepCompleted && !mFirstStepJob ) {
    KUrl url = KUrl( "http://en.wikipedia.org/w/index.php?title=Template:POTD/" +
                     mDate.toString( Qt::ISODate ) + "&action=raw" );
                // The file at that URL contains the file name for the POTD

    mFirstStepJob = KIO::storedGet( url, KIO::NoReload, KIO::HideProgressInfo );
    KIO::Scheduler::setJobPriority( mFirstStepJob, 1 );

    connect( mFirstStepJob, SIGNAL(result(KJob*)),
             this, SLOT(step1Result(KJob*)) );
    connect( this, SIGNAL(step1Success()),
             this, SLOT(step2GetImagePage()) );
  }
}

/**
  Give it a job which fetched the raw page,
  and it'll give you the image file name hiding in it.
 */
void POTDElement::step1Result( KJob *job )
{
  if ( job->error() ) {
    kWarning() << "POTD:" << mDate << ": could not get POTD file name:" << job->errorString();
    kDebug() << "POTD:" << mDate << ": file name:" << mFileName;
    kDebug() << "POTD:" << mDate << ": full-size image:" << mFullSizeImageUrl.url();
    kDebug() << "POTD:" << mDate << ": thumbnail:" << mThumbUrl.url();
    mFirstStepCompleted = false;
    return;
  }

  // First step completed: we now know the POTD's file name
  KIO::StoredTransferJob *const transferJob = static_cast<KIO::StoredTransferJob*>( job );
  const QStringList lines =
    QString::fromUtf8( transferJob->data().data(), transferJob->data().size() ).split( '\n' );

  Q_FOREACH( const QString &line, lines ) {
    if ( line.startsWith( "|image=" ) ) {
      mFileName = line;
      break;
    }
  }
  mFileName = mFileName.remove( "|image=" ).replace( ' ', '_' );

  Q_FOREACH( const QString &line, lines ) {
    if ( line.startsWith( "|texttitle=" ) ) {
      mDescription = line;
      break;
    }
  }
  mDescription = mDescription.remove( "|texttitle=" );
  if ( !mDescription.isEmpty() ) {
    mLongText = mDescription;
  } else {
    mLongText = mFileName;
  }
  //TODO: uncomment the next line when string freeze is over
  //mLongText = i18n( "Wikipedia POTD: %1", mLongText );
  emit gotNewLongText( mLongText );

  kDebug() << "FILENAME=" << mFileName;
  kDebug() << "DESCRIPTION=" << mDescription;

  mFirstStepCompleted = true;
  mFirstStepJob = 0;
  emit step1Success();
}

/** Second step of three in the download process */
void POTDElement::step2GetImagePage()
{
  if ( !mSecondStepCompleted && !mSecondStepJob ) {
    mUrl = KUrl( "http://en.wikipedia.org/wiki/File:" + mFileName );
    // We'll find the info to get the thumbnail we want on the POTD's image page

    emit gotNewUrl( mUrl );
    mShortText = i18n( "Picture Page" );
    emit gotNewShortText( mShortText );

    mSecondStepJob = KIO::storedGet( mUrl, KIO::NoReload, KIO::HideProgressInfo );
    KIO::Scheduler::setJobPriority( mSecondStepJob, 1 );

    connect( mSecondStepJob, SIGNAL(result(KJob*)),
             this, SLOT(step2Result(KJob*)) );
    connect( this, SIGNAL(step2Success()), SLOT(step3GetThumbnail()) );
  }
}

/**
  Give it a job which fetched the image page,
  and it'll give you the appropriate thumbnail URL.
 */
void POTDElement::step2Result( KJob *job )
{
  if ( job->error() ) {
    kWarning() << "POTD:" << mDate << ": could not get POTD image page:" << job->errorString();
    kDebug() << "POTD:" << mDate << ": file name:" << mFileName;
    kDebug() << "POTD:" << mDate << ": full-size image:" << mFullSizeImageUrl.url();
    kDebug() << "POTD:" << mDate << ": thumbnail:" << mThumbUrl.url();
    mSecondStepCompleted = false;
    return;
  }

  // Get the image URL from the image page's source code
  // and transform it to get an appropriate thumbnail size
  KIO::StoredTransferJob *const transferJob = static_cast<KIO::StoredTransferJob*>( job );

  QDomDocument imgPage;
  if ( !imgPage.setContent( QString::fromUtf8( transferJob->data().data(),
                                               transferJob->data().size() ) ) ) {
    kWarning() << "POTD:" << mDate << ": Wikipedia returned an invalid XML page for image"
               << mFileName;
    return;
  }

  // We go through all links and stop at the first right-looking candidate
  QDomNodeList links = imgPage.elementsByTagName( "a" );
  for ( uint i=0; i<links.length(); i++ ) {
    QString href = links.item(i).attributes().namedItem( "href" ).nodeValue();
    if ( href.startsWith(
           QLatin1String( "//upload.wikimedia.org/wikipedia/commons/" ) ) ) {
      mFullSizeImageUrl = href;
      break;
    }
  }

  // We get the image's width/height ratio
  mHWRatio = 1.0;
  QDomNodeList images = imgPage.elementsByTagName( "img" );
  for ( uint i=0; i<links.length(); i++ ) {
    QDomNamedNodeMap attr = images.item( i ).attributes();
    QString src = attr.namedItem( "src" ).nodeValue();

    if ( src.startsWith( thumbnailUrl( mFullSizeImageUrl ).url() ) ) {
      if ( ( attr.namedItem( "height" ).nodeValue().toInt() != 0 ) &&
           ( attr.namedItem( "width" ).nodeValue().toInt() != 0 ) ) {
        mHWRatio = attr.namedItem( "height" ).nodeValue().toFloat() /
                   attr.namedItem( "width" ).nodeValue().toFloat();
      }
      break;
    }

  }
  kDebug() << "POTD:" << mDate << ": h/w ratio:" << mHWRatio;
  kDebug() << "POTD:" << mDate << ": got POTD image page source:" << mFullSizeImageUrl;

  if ( !mFullSizeImageUrl.isEmpty() ) {
    mSecondStepCompleted = true;
    mSecondStepJob = 0;
    emit step2Success();
  }
}

KUrl POTDElement::thumbnailUrl( const KUrl &fullSizeUrl, const int width ) const
{
  QString thumbUrl = fullSizeUrl.url();
  if ( width != 0 ) {
    thumbUrl.replace( QRegExp( "//upload.wikimedia.org/wikipedia/commons/(.*)/([^/]*)" ),
                      "//upload.wikimedia.org/wikipedia/commons/thumb/\\1/\\2/" +
                      QString::number( width ) + "px-\\2" );
  } else {  // This will not return a valid thumbnail URL, but will at least
            // give some info (the beginning of the URL)
    thumbUrl.replace( QRegExp( "//upload.wikimedia.org/wikipedia/commons/(.*)/([^/]*)" ),
                      "//upload.wikimedia.org/wikipedia/commons/thumb/\\1/\\2" );
  }
  thumbUrl.replace( QRegExp( "^file:////" ), "http://" );
  return KUrl( thumbUrl );
}

/** Third step of three in the downloading process */
void POTDElement::step3GetThumbnail()
{
  if ( mThirdStepJob ) {
    mThirdStepJob->kill();
  }
  mThirdStepJob = 0;

  int thumbWidth = mThumbSize.width();
  int thumbHeight = static_cast<int>( thumbWidth * mHWRatio );
  if ( mThumbSize.height() < thumbHeight ) {
    /* if the requested height is less than the requested width * ratio
       we would download too much, as the downloaded picture would be
       taller than requested, so we adjust the width of the picture to
       be downloaded in consequence */
    thumbWidth /= ( thumbHeight / mThumbSize.height() );
    thumbHeight = static_cast<int>( thumbWidth * mHWRatio );
  }
  mDlThumbSize = QSize( thumbWidth, thumbHeight );
  kDebug() << "POTD:" << mDate << ": will download thumbnail of size" << mDlThumbSize;
  QString thumbUrl =
    QUrl::fromPercentEncoding(
      thumbnailUrl( mFullSizeImageUrl, thumbWidth ).url().toAscii() );

  kDebug() << "POTD:" << mDate << ": got POTD thumbnail URL:" << thumbUrl;
  mThumbUrl = thumbUrl;

  mThirdStepJob = KIO::storedGet( thumbUrl, KIO::NoReload, KIO::HideProgressInfo );
  kDebug() << "POTD:" << mDate << ": get" << thumbUrl;//FIXME
  KIO::Scheduler::setJobPriority( mThirdStepJob, 1 );

  connect( mThirdStepJob, SIGNAL(result(KJob*)),
           this, SLOT(step3Result(KJob*)) );
}

/**
  Give it a job which fetched the thumbnail,
  and it'll give the corresponding pixmap to you.
 */
void POTDElement::step3Result( KJob *job )
{
  if ( job != mThirdStepJob ) {
    return;
  }
  mThirdStepJob = 0;

  if ( job->error() ) {
    kWarning() << "POTD:" << mDate << ": could not get POTD:" << job->errorString();
    kDebug() << "POTD:" << mDate << ": file name:" << mFileName;
    kDebug() << "POTD:" << mDate << ": full-size image:" << mFullSizeImageUrl.url();
    kDebug() << "POTD:" << mDate << ": thumbnail:" << mThumbUrl.url();
    return;
  }

  // Last step completed: we get the pixmap from the transfer job's data
  KIO::StoredTransferJob* const transferJob = static_cast<KIO::StoredTransferJob*>( job );
  if ( mPixmap.loadFromData( transferJob->data() ) ) {
    kDebug() << "POTD:" << mDate << ": got POTD.";
    emit gotNewPixmap( mPixmap.scaled( mThumbSize, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation ) );
  }
}

QPixmap POTDElement::newPixmap( const QSize &size )
{
  if ( ( mThumbSize.width() < size.width() ) || ( mThumbSize.height() < size.height() ) ) {
    kDebug() << "POTD:" << mDate << ": called for a new pixmap size ("
             << size << "instead of" << mThumbSize << ", stored pixmap:"
             << mPixmap.size() << ")";
    setThumbnailSize( size );

    if ( !mFirstStepCompleted ) {
      step1StartDownload();  // First run, start from the beginning
    } else if ( ( mDlThumbSize.width() < size.width() ) &&
                ( mDlThumbSize.height() < size.height() ) ) {
      if ( mThirdStepJob ) {
        // Another download (for the old size) is already running;
        // we'll run after that
        disconnect( this, SIGNAL(step3Success()),
                    this, SLOT(step3GetThumbnail()) );
        connect( this, SIGNAL(step3Success()), SLOT(step3GetThumbnail()) );
      } else if ( mFirstStepJob || mSecondStepJob ) {
        // The download process did not get to step 3 yet, and will download
        // the correct size automagically
      } else {
        // We start a new thumbnail download a little later; the following code
        // is to avoid too frequent transfers e.g. when resizing
        mTimer->stop();
        disconnect( mTimer, SIGNAL(timeout()),
                   this, SLOT(step3GetThumbnail()) );
        connect( mTimer, SIGNAL(timeout()),
                 this, SLOT(step3GetThumbnail()) );
        mTimer->setSingleShot( true );
        mTimer->start( 1000 );
      }
    }
  }

  /* else, either we already got a sufficiently big pixmap (stored in mPixmap),
     or we will get one anytime soon (we are downloading it already) and we will
     actualize what we return here later via gotNewPixmap */
  if ( mPixmap.isNull() ) {
    return QPixmap();
  }
  return mPixmap.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
}

void POTDElement::setThumbnailSize( const QSize &size )
{
  mThumbSize = size;
}

#include "picoftheday.moc"
