/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
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

#include "koeventviewer.h"
#include "urihandler.h"
#include "korganizerinterface.h"
#include "koglobals.h"

#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/kpimprefs.h>

#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>

#include <kapplication.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kconfiggroup.h>

#include <QRegExp>

KOEventViewer::KOEventViewer( QWidget *parent )
  : KTextBrowser( parent ), mDefaultText( "" )
{
  mIncidence = 0;
  setNotifyClick( true );
  setMinimumHeight( 1 );
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::readSettings( KConfig *config )
{
  if ( config ) {
// With each restart of KOrganizer the font site gets halfed. What should this
// be good for?
#if 0
    config->setGroup( QString( "EventViewer-%1" ).arg( name() ) );
    int zoomFactor = config->readEntry( "ZoomFactor", fontPointSize() );
    zoomTo( zoomFactor / 2 );
    kDebug(5850) << " KOEventViewer: restoring the fontPointSize:" << fontPointSize()
                 << ", zoomFactor: " << zoomFactor;
#endif
  }
}

void KOEventViewer::writeSettings( KConfig *config )
{
  if ( config ) {
    kDebug() << "saving the zoomFactor:" << fontPointSize();
    KConfigGroup configGroup( config, QString( "EventViewer-%1" ).arg( objectName() ) );
    configGroup.writeEntry( "ZoomFactor", fontPointSize() );
  }
}

void KOEventViewer::setSource( const QUrl &name )
{
  QString uri = name.toString();
  // QTextBrowser for some reason insists on putting // or / in links,
  // this is a crude workaround
  if ( uri.startsWith( KDEPIMPROTOCOL_CONTACT ) ||
       uri.startsWith( KDEPIMPROTOCOL_EMAIL ) ||
       uri.startsWith( QString( KDEPIMPROTOCOL_INCIDENCE ).section( ':', 0, 0 ) ) ||
       uri.startsWith( KDEPIMPROTOCOL_NEWSARTICLE ) ||
       uri.startsWith( "mailto:" ) )
  {
    uri.replace( QRegExp( "^([^:]+:)/+" ), "\\1" );
  }

  UriHandler::process( uri );
}

bool KOEventViewer::appendIncidence( Incidence *incidence )
{
  addText( IncidenceFormatter::extensiveDisplayStr(
             incidence, KPIM::KPimPrefs::timeSpec() ) );
  return true;
}

void KOEventViewer::setIncidence( Incidence *incidence )
{
  clearEvents();
  if( incidence ) {
    appendIncidence( incidence );
    mIncidence = incidence;
  } else {
    clearEvents( true );
    mIncidence = 0;
  }
}

void KOEventViewer::clearEvents( bool now )
{
  mText = "";
  if ( now ) {
    setText( mDefaultText );
  }
}

void KOEventViewer::addText( const QString &text )
{
  mText.append( text );
  setText( mText );
}

void KOEventViewer::setDefaultText( const QString &text )
{
  mDefaultText = text;
}

void KOEventViewer::changeIncidenceDisplay( Incidence *incidence, int action )
{
  if ( mIncidence && ( incidence->uid() == mIncidence->uid() ) ) {
    switch ( action ) {
    case KOGlobals::INCIDENCEEDITED:
    {
      setIncidence( incidence );
      break;
    }
    case KOGlobals::INCIDENCEDELETED:
    {
      setIncidence( 0 );
      break;
    }
    }
  }
}

void KOEventViewer::editIncidence()
{
  if ( mIncidence ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "korganizer" );

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
    korganizerIface.editIncidence( mIncidence->uid() );
  }
}

void KOEventViewer::showIncidenceContext()
{
  if ( mIncidence ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "korganizer" );

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
    korganizerIface.showIncidenceContext( mIncidence->uid() );
  }
}

#include "koeventviewer.moc"
