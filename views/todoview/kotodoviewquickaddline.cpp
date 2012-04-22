/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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
  51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kotodoviewquickaddline.h"

#include <KLocale>

#include <QKeyEvent>

KOTodoViewQuickAddLine::KOTodoViewQuickAddLine( QWidget *parent )
  : KLineEdit( parent )
{
  connect( this, SIGNAL(returnPressed()),
           this, SLOT(returnPressed()) );

  mClickMessage = i18n( "Click to add a new to-do" );
  setToolTip( mClickMessage );
}

void KOTodoViewQuickAddLine::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Return ) {
    mModifiers = event->modifiers();
  }

  KLineEdit::keyPressEvent( event );
}

void KOTodoViewQuickAddLine::returnPressed()
{
  // Workaround bug #217592 (disappearing cursor)
  unsetCursor();

  emit returnPressed( mModifiers );
}

void KOTodoViewQuickAddLine::resizeEvent ( QResizeEvent * event )
{
  KLineEdit::resizeEvent( event );

  setClickMessage( fontMetrics().elidedText(
                     mClickMessage,
                     Qt::ElideRight,
                     width() - clearButtonUsedSize().width() ) );
}

#include "kotodoviewquickaddline.moc"
