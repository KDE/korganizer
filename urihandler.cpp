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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qobject.h>
#include <qobject.h>

#include <libkdepim/kdepimprotocols.h>

#include "urihandler.h"

#ifndef KORG_NODCOP
#include <dcopclient.h>
#include "kmailIface_stub.h"
#include "knodeiface_stub.h"
#include "korganizeriface_stub.h"
#endif

#include <kiconloader.h>
#include <krun.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kdebug.h>
#include <ktoolinvocation.h>

bool UriHandler::process( const QString &uri )
{
  kDebug(5850) << "UriHandler::process(): " << uri << endl;

#ifndef KORG_NODCOP
  if ( uri.startsWith( KDEPIMPROTOCOL_EMAIL ) ) {
    // make sure kmail is running or the part is shown
    KToolInvocation::startServiceByDesktopPath("kmail");

    // parse string, show
    int colon = uri.find( ':' );
    // extract 'number' from 'kmail:<number>/<id>'
    QString serialNumberStr = uri.mid( colon + 1 );
    serialNumberStr = serialNumberStr.left( serialNumberStr.find( '/' ) );

    KMailIface_stub kmailIface( "kmail", "KMailIface" );
    kmailIface.showMail( serialNumberStr.toUInt(), QString() );
    return true;
  } else if ( uri.startsWith( "mailto:" ) ) {
    KToolInvocation::invokeMailer( uri.mid(7), QString() );
    return true;
  } else if ( uri.startsWith( KDEPIMPROTOCOL_CONTACT ) ) {
    DCOPClient *client = KApplication::kApplication()->dcopClient();
    const QByteArray noParamData;
    const QByteArray paramData;
    QByteArray replyData;
    DCOPCString replyTypeStr;
    bool foundAbbrowser = client->call( "kaddressbook", "KAddressBookIface",
                                        "interfaces()",  noParamData,
                                        replyTypeStr, replyData );
    if ( foundAbbrowser ) {
      //KAddressbook is already running, so just DCOP to it to bring up the contact editor
      kapp->updateRemoteUserTimestamp("kaddressbook");
      DCOPRef kaddressbook( "kaddressbook", "KAddressBookIface" );
      kaddressbook.send( "showContactEditor", 
                         uri.mid( ::qstrlen( KDEPIMPROTOCOL_CONTACT ) ) );
      return true;
    } else {
      /*
        KaddressBook is not already running.  Pass it the UID of the contact via the command line while starting it - its neater.
        We start it without its main interface
      */
      KIconLoader *iconLoader = new KIconLoader();
      QString iconPath = iconLoader->iconPath( "go", KIcon::Small );
      QString tmpStr = "kaddressbook --editor-only --uid ";
      tmpStr += KProcess::quote( uri.mid( ::qstrlen( KDEPIMPROTOCOL_CONTACT ) ) 
      );
      KRun::runCommand( tmpStr, "KAddressBook", iconPath );
      return true;
    }
  } else if ( uri.startsWith( KDEPIMPROTOCOL_INCIDENCE ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath("korganizer");

    // we must work around KUrl breakage (it doesn't know about URNs)
    QString uid = KUrl::decode_string( uri ).mid( 11 );
    
    KOrganizerIface_stub korganizerIface( "korganizer", "KOrganizerIface" );
    return korganizerIface.showIncidence( uid );
  } else if ( uri.startsWith( KDEPIMPROTOCOL_NEWSARTICLE ) ) {
    KToolInvocation::startServiceByDesktopPath( "knode" );
    
    KNodeIface_stub knodeIface( "knode", "KNodeIface" );
    knodeIface.openURL( uri );
  } else {  // no special URI, let KDE handle it
    new KRun(KUrl( uri ),0L);
  }
#endif /* KORG_NODCOP */
  
  return false;
}
