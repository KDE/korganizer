/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
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

#include "urihandler.h"
#include <knodeinterface.h>
#include <kmailinterface.h>
#include <korganizerinterface.h>
#include <coreinterface.h>

#include <libkdepim/kdepimprotocols.h>

#include <kiconloader.h>
#include <krun.h>
#include <kapplication.h>
#include <kshell.h>
#include <kdebug.h>
#include <ktoolinvocation.h>

#include <QObject>

bool UriHandler::process( const QString &uri )
{
  kDebug() << uri;

  if ( uri.startsWith( KDEPIMPROTOCOL_EMAIL ) ) {
    // make sure kmail is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "kmail" );

    // parse string, show
    int colon = uri.indexOf( ':' );
    // extract 'number' from 'kmail:<number>/<id>'
    QString serialNumberStr = uri.mid( colon + 1 );
    serialNumberStr = serialNumberStr.left( serialNumberStr.indexOf( '/' ) );

    org::kde::kmail::kmail kmail(
      "org.kde.kmail", "/KMail", QDBusConnection::sessionBus() );
    kmail.showMail( serialNumberStr.toUInt(), QString() );
    return true;
  } else if ( uri.startsWith( "mailto:" ) ) {
    KToolInvocation::invokeMailer( uri.mid(7), QString() );
    return true;
  } else if ( uri.startsWith( KDEPIMPROTOCOL_CONTACT ) ) {
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered(
           "org.kde.kaddressbook" ) ) {
      kapp->updateRemoteUserTimestamp( "org.kde.kaddressbook" );
      org::kde::KAddressbook::Core kaddressbook(
        "org.kde.kaddressbook", "/KAddressBook", QDBusConnection::sessionBus() );
      kaddressbook.showContactEditor( uri.mid( ::qstrlen( KDEPIMPROTOCOL_CONTACT ) ) );
      return true;
    } else {
      /*
        KaddressBook is not already running.  Pass it the UID of the contact via
        the command line while starting it - it is neater.
        We start it without its main interface
      */
      QString iconPath =
        KIconLoader::global()->iconPath( "view-pim-contacts", KIconLoader::SizeSmall );
      QString tmpStr = "kaddressbook --editor-only --uid ";
      tmpStr += KShell::quoteArg( uri.mid( ::qstrlen( KDEPIMPROTOCOL_CONTACT ) ) );
      KRun::runCommand( tmpStr, "KAddressBook", iconPath, 0 );
      return true;
    }
  } else if ( uri.startsWith( KDEPIMPROTOCOL_INCIDENCE ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "korganizer" );

    // we must work around KUrl breakage (it doesn't know about URNs)
    QString uid = KUrl::fromPercentEncoding( uri.toLatin1() ).mid( 11 );
    OrgKdeKorganizerKorganizerInterface korganizerIface(
      "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );

    return korganizerIface.showIncidence( uid );
  } else if ( uri.startsWith( KDEPIMPROTOCOL_NEWSARTICLE ) ) {
    KToolInvocation::startServiceByDesktopPath( "knode" );
    org::kde::knode knode(
      "org.kde.knode", "/KNode", QDBusConnection::sessionBus() );
    knode.openURL( uri );
  } else {  // no special URI, let KDE handle it
    new KRun( KUrl( uri ), 0 );
  }

  return false;
}
