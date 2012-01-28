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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koeventviewerdialog.h"
#include "korganizerinterface.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/next/incidenceviewer.h>

#include <Akonadi/Item>

#include <KLocale>
#include <KToolInvocation>

KOEventViewerDialog::KOEventViewerDialog( CalendarSupport::Calendar *calendar, QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Event Viewer" ) );
  setButtons( Close | User1 | User2 );
  setModal( false );
  setButtonGuiItem( User1, KGuiItem( i18n( "Edit..." ), KIcon( "document-edit" ) ) );
  setButtonGuiItem( User2, KGuiItem( i18n( "Show in Context" ) ) );
  mEventViewer = new CalendarSupport::IncidenceViewer( calendar, this );
  setMainWidget( mEventViewer );

  resize( QSize( 500, 520 ).expandedTo( minimumSizeHint() ) );

  connect( this, SIGNAL(finished()), this, SLOT(delayedDestruct()) );
  connect( this, SIGNAL(user1Clicked()), this, SLOT(editIncidence()) );
  connect( this, SIGNAL(user2Clicked()), this, SLOT(showIncidenceContext()) );
}

KOEventViewerDialog::~KOEventViewerDialog()
{
  delete mEventViewer;
}

void KOEventViewerDialog::setIncidence( const Akonadi::Item &incidence, const QDate &date )
{
  mEventViewer->setIncidence( incidence, date );
}

void KOEventViewerDialog::addText( const QString &text )
{
  mEventViewer->setHeaderText( text );
}

void KOEventViewerDialog::editIncidence()
{
  const Akonadi::Item item = mEventViewer->item();

  if ( CalendarSupport::hasIncidence( item ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "korganizer" );

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
    korganizerIface.editIncidence( QString::number( item.id() ) );
  }
}

void KOEventViewerDialog::showIncidenceContext()
{
  const Akonadi::Item item = mEventViewer->item();

  if ( CalendarSupport::hasIncidence( item ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "korganizer" );

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
    korganizerIface.showIncidenceContext( QString::number( item.id() ) );
  }
}

#include "koeventviewerdialog.moc"
