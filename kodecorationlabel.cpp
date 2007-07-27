/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kodecorationlabel.h"

#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>

#include <KToolInvocation>

#include "kodecorationlabel.moc"

KODecorationLabel::KODecorationLabel( KOrg::CalendarDecoration::Element *e,
                                      QWidget *parent )
  : QLabel( parent ), mAutomaticSqueeze( true ), mDecorationElement( e ),
    mShortText( e->shortText() ), mLongText( e->longText() ),
    mExtensiveText( e->extensiveText() )
{
  mPixmap = e->pixmap( size() );
  mUrl = e->url();
  setUrl( e->url() );

  //setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  setAlignment( Qt::AlignCenter );

  connect( e, SIGNAL( gotNewExtensiveText( const QString & ) ),
           this, SLOT( setExtensiveText( const QString & ) ) );
  connect( e, SIGNAL( gotNewLongText( const QString & ) ),
           this, SLOT( setLongText( const QString & ) ) );
  connect( e, SIGNAL( gotNewPixmap( const QPixmap & ) ),
           this, SLOT( setPixmap( const QPixmap & ) ) );
  connect( e, SIGNAL( gotNewShortText( const QString & ) ),
           this, SLOT( setShortText( const QString & ) ) );
  connect( e, SIGNAL( gotNewUrl( const KUrl & ) ),
           this, SLOT( setUrl( const KUrl & ) ) );
  squeezeContentsToLabel();
}

KODecorationLabel::KODecorationLabel( const QString &shortText,
                                      const QString &longText,
                                      const QString &extensiveText,
                                      const QPixmap &pixmap,
                                      const KUrl &url,
                                      QWidget *parent )
  : QLabel( parent ), mAutomaticSqueeze( true ), mShortText( shortText ),
    mLongText( longText ), mExtensiveText( extensiveText ),
    mPixmap( pixmap )
{
  setUrl( url );

  setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  squeezeContentsToLabel();
}

KODecorationLabel::~KODecorationLabel()
{
}

QSize KODecorationLabel::minimumSizeHint() const
{
  QSize sh = QLabel::minimumSizeHint();
  sh.setWidth(-1);
  sh.setHeight(120); //FIXME
  return sh;
}

void KODecorationLabel::mouseReleaseEvent( QMouseEvent *event )
{
  QLabel::mouseReleaseEvent( event );

  switch ( event->button() ) {
    case Qt::LeftButton:
      KToolInvocation::invokeBrowser( mUrl.url() );
      setForegroundRole( QPalette::LinkVisited );
      break;
    case Qt::MidButton:
    case Qt::RightButton:
    default:
      break;
  }
}

void KODecorationLabel::resizeEvent( QResizeEvent *event )
{
  kDebug() << "DecorationLabel got a resize event; old size "
           << event->oldSize() << "--> new size " << event->size() << endl;
  mPixmap = mDecorationElement->pixmap( event->size() ); //FIXME
  squeezeContentsToLabel();
}

void KODecorationLabel::setExtensiveText( const QString &text )
{
  mExtensiveText = text;
  squeezeContentsToLabel();
}

void KODecorationLabel::setLongText( const QString &text )
{
  mLongText = text;
  squeezeContentsToLabel();
}

void KODecorationLabel::setPixmap( const QPixmap &pixmap )
{
  mPixmap = pixmap;
  squeezeContentsToLabel();
}

void KODecorationLabel::setShortText( const QString &text )
{
  mShortText = text;
  squeezeContentsToLabel();
}

void KODecorationLabel::setText( const QString &text )
{
  setLongText( text );
}

void KODecorationLabel::setUrl( const KUrl &url )
{
  mUrl = url;
  QFont f = font();
  if ( url.isEmpty() ) {
    setForegroundRole( QPalette::WindowText );
    f.setUnderline( false );
    setCursor( QCursor( Qt::ArrowCursor ) );
  } else {
    setForegroundRole( QPalette::Link );
    f.setUnderline( true );
    setCursor( QCursor( Qt::PointingHandCursor ) );
  }
  setFont( f );
}

void KODecorationLabel::squeezeContentsToLabel()
{
  if ( !mAutomaticSqueeze )  // The content type to use has been set manually
    return;

  QFontMetrics fm( fontMetrics() );

  int labelWidth = size().width();
  int longTextWidth = fm.width(mLongText);
  int extensiveTextWidth = fm.width(mExtensiveText);

  if ( ! mPixmap.isNull() ) {
kDebug() << "using pixmap" << endl; //FIXME: remove these debug lines
kDebug() << "pixmap size: " << mPixmap.size() << endl;
    usePixmap( true );
  } else if ( ( !mExtensiveText.isEmpty() )
              && ( extensiveTextWidth <= labelWidth ) ) {
kDebug() << "using extenstext" << endl;
    useExtensiveText( true );
  } else if ( ( !mLongText.isEmpty() )
              && ( longTextWidth <= labelWidth ) ) {
kDebug() << "using longtext" << endl;
    useLongText( true );
  } else {
kDebug() << "using shorttext" << endl;
kDebug() << "pixmap size: " << mPixmap.size() << endl;
    useShortText( true );
  }
}

void KODecorationLabel::useDefaultText()
{
  mAutomaticSqueeze = false;
  squeezeContentsToLabel();
}

void KODecorationLabel::useExtensiveText( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setText( mExtensiveText );
  setToolTip( QString() );
}

void KODecorationLabel::useLongText( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setText( mLongText );
  setToolTip( mExtensiveText.isEmpty() ? QString() : mExtensiveText );
}

void KODecorationLabel::usePixmap( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setPixmap( mPixmap ); // .scaled( sizeHint(), Qt::KeepAspectRatio ) //FIXME: scale to the label's size!!
  kDebug() << "sizeHint " << sizeHint() << endl;
  kDebug() << "MINsizeHint " << minimumSizeHint() << endl;
  setToolTip( mExtensiveText.isEmpty() ? mLongText : mExtensiveText );
}

void KODecorationLabel::useShortText( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setText( mShortText );
  setToolTip( mExtensiveText.isEmpty() ? mLongText : mExtensiveText );
}
