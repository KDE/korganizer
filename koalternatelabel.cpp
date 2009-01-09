/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koalternatelabel.h"

#include "koalternatelabel.moc"

KOAlternateLabel::KOAlternateLabel( const QString &shortlabel,
                                    const QString &longlabel,
                                    const QString &extensivelabel,
                                    QWidget *parent )
  : QLabel( parent ), mTextTypeFixed( false ), mShortText( shortlabel ),
    mLongText( longlabel ), mExtensiveText( extensivelabel )
{
  setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  if ( mExtensiveText.isEmpty() ) {
    mExtensiveText = mLongText;
  }
  squeezeTextToLabel();
}

KOAlternateLabel::~KOAlternateLabel()
{
}

void KOAlternateLabel::useShortText()
{
  mTextTypeFixed = true;
  QLabel::setText( mShortText );
  setToolTip( mExtensiveText );
}

void KOAlternateLabel::useLongText()
{
  mTextTypeFixed = true;
  QLabel::setText( mLongText );
  this->setToolTip( mExtensiveText );
}

void KOAlternateLabel::useExtensiveText()
{
  mTextTypeFixed = true;
  QLabel::setText( mExtensiveText );
  this->setToolTip( "" );
}

void KOAlternateLabel::useDefaultText()
{
  mTextTypeFixed = false;
  squeezeTextToLabel();
}

void KOAlternateLabel::squeezeTextToLabel()
{
  if ( mTextTypeFixed ) {
    return;
  }

  QFontMetrics fm( fontMetrics() );
  int labelWidth = size().width();
  int textWidth = fm.width( mLongText );
  int longTextWidth = fm.width( mExtensiveText );
  if ( longTextWidth <= labelWidth ) {
    QLabel::setText( mExtensiveText );
    this->setToolTip( "" );
  } else if ( textWidth <= labelWidth ) {
    QLabel::setText( mLongText );
    this->setToolTip( mExtensiveText );
  } else {
    QLabel::setText( mShortText );
    this->setToolTip( mExtensiveText );
  }
}

void KOAlternateLabel::resizeEvent( QResizeEvent * )
{
  squeezeTextToLabel();
}

QSize KOAlternateLabel::minimumSizeHint() const
{
  QSize sh = QLabel::minimumSizeHint();
  sh.setWidth( -1 );
  return sh;
}

void KOAlternateLabel::setText( const QString &text )
{
  mLongText = text;
  squeezeTextToLabel();
}

