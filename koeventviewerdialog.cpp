/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koeventviewerdialog.h"

#include "koeventviewer.h"

#include <klocale.h>

KOEventViewerDialog::KOEventViewerDialog( QWidget *parent, bool compact )
  : KDialog( parent )
{
  setCaption( i18n("Event Viewer") );
  setButtons( Ok | User1 | User2 );
  setModal( false );
  setButtonGuiItem( User1, i18n("Edit") );
  setButtonGuiItem( User2, i18n("Show in context") );
  mEventViewer = new KOEventViewer( this );
  setMainWidget( mEventViewer );

  // FIXME: Set a sensible size (based on the content?).
  if ( compact ) {
    setFixedSize( 240,284 );
    move( 0, 15 );
  } else {
    setMinimumSize( 300, 200 );
    resize( 320, 300 );
  }
  connect( this, SIGNAL(finished()), this, SLOT(delayedDestruct()) );
#ifdef KORG_NODCOP
  showButton( KDialog::User1, false );
  showButton( KDialog::User2, false );
#else
  connect( this, SIGNAL( user1Clicked() ), mEventViewer,
           SLOT( editIncidence() ) );
  connect( this, SIGNAL( user2Clicked() ), mEventViewer,
           SLOT( showIncidenceContext() ) );
#endif
}

KOEventViewerDialog::~KOEventViewerDialog()
{
}

void KOEventViewerDialog::addText( const QString &text )
{
  mEventViewer->addText(text);
}

#include "koeventviewerdialog.moc"
