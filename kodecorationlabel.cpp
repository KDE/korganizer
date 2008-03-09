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

#include <KToolInvocation>

#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>

#include "kodecorationlabel.moc"

KODecorationLabel::KODecorationLabel( KOrg::CalendarDecoration::Element *e,
                                      QWidget *parent )
  : QLabel( parent ), mAutomaticSqueeze( true ), mDecorationElement( e ),
    mShortText( e->shortText() ), mLongText( e->longText() ),
    mExtensiveText( e->extensiveText() )
{
  mPixmap = e->newPixmap( size() );
  mUrl = e->url();
  setUrl( e->url() );

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

  squeezeContentsToLabel();
}

KODecorationLabel::~KODecorationLabel()
{
}

void KODecorationLabel::mouseReleaseEvent( QMouseEvent *event )
{
  QLabel::mouseReleaseEvent( event );

  switch ( event->button() ) {
    case Qt::LeftButton:
      if ( ! mUrl.isEmpty() ) {
        KToolInvocation::invokeBrowser( mUrl.url() );
        setForegroundRole( QPalette::LinkVisited );
      }
      break;
    case Qt::MidButton:
    case Qt::RightButton:
    default:
      break;
  }
}

void KODecorationLabel::resizeEvent( QResizeEvent *event )
{
  mPixmap = mDecorationElement->newPixmap( event->size() );
  QLabel::resizeEvent( event );
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
  mPixmap = pixmap.scaled( size(), Qt::KeepAspectRatio );
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
  if ( !mAutomaticSqueeze ) { // The content type to use has been set manually
    return;
  }

  QFontMetrics fm( fontMetrics() );

  int labelWidth = size().width();
  int longTextWidth = fm.width(mLongText);
  int extensiveTextWidth = fm.width(mExtensiveText);

  if ( ! mPixmap.isNull() ) {
    usePixmap( true );
  } else if ( ( !mExtensiveText.isEmpty() ) && ( extensiveTextWidth <= labelWidth ) ) {
    useExtensiveText( true );
  } else if ( ( !mLongText.isEmpty() ) && ( longTextWidth <= labelWidth ) ) {
    useLongText( true );
  } else {
    useShortText( true );
  }

  setAlignment( Qt::AlignCenter );
  setWordWrap( true );
  QSize msh = QLabel::minimumSizeHint();
  msh.setHeight( fontMetrics().lineSpacing() );
  msh.setWidth( 0 );
  setMinimumSize( msh );
  setSizePolicy( sizePolicy().horizontalPolicy(),
                 QSizePolicy::MinimumExpanding );
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
  QLabel::setPixmap( mPixmap );
  setToolTip( mExtensiveText.isEmpty() ? mLongText : mExtensiveText );
}

void KODecorationLabel::useShortText( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setText( mShortText );
  setToolTip( mExtensiveText.isEmpty() ? mLongText : mExtensiveText );
}
