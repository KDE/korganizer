/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "urihandler.h"

#ifndef KORG_NODCOP
#include <dcopclient.h>
#include "kmailIface_stub.h"
#endif

#include <kiconloader.h>
#include <krun.h>
#include <kprocess.h>
#include <kdebug.h>

bool UriHandler::process( const QString &uri )
{
  kdDebug(5850) << "UriHandler::process(): " << uri << endl;

#ifndef KORG_NODCOP
  if ( uri.startsWith( "kmail:" ) ) {
    int pos = uri.find( "/" );
    if ( pos > 5 ) {
      QString serialNumberStr = uri.mid( 6, pos - 6 );
      QString messageId = uri.mid( pos + 1 );
      kdDebug() << "SERIALNUMBERSTR: " << serialNumberStr << " MESSAGEID: "
                << messageId << endl;
      Q_UINT32 serialNumber = serialNumberStr.toUInt();
      kdDebug() << "SERIALNUMBER: " << serialNumber << endl;
      KMailIface_stub kmailIface( "kmail", "KMailIface" );
      kmailIface.showMail( serialNumber, messageId );
      return true;
    }
  } else if ( uri.startsWith( "mailto:" ) ) {
    KApplication::kApplication()->invokeMailer( uri.mid(7), QString::null );
    return true;
  } else if ( uri.startsWith( "uid:" ) ) {
    DCOPClient *client = KApplication::kApplication()->dcopClient();
    const QByteArray noParamData;
    const QByteArray paramData;
    QByteArray replyData;
    QCString replyTypeStr;
    bool foundAbbrowser = client->call( "kaddressbook", "KAddressBookIface",
                                        "interfaces()",  noParamData,
                                        replyTypeStr, replyData );
    if ( foundAbbrowser ) {
      //KAddressbook is already running, so just DCOP to it to bring up the contact editor
      //client->send("kaddressbook","KAddressBookIface",
      QDataStream arg( paramData, IO_WriteOnly );
      arg << uri.mid( 6 );
      client->send( "kaddressbook", "KAddressBookIface",
                    "showContactEditor( QString )", paramData );
      return true;
    } else {
      /*
        KaddressBook is not already running.  Pass it the UID of the contact via the command line while starting it - its neater.
        We start it without its main interface
      */
      KIconLoader *iconLoader = new KIconLoader();
      QString iconPath = iconLoader->iconPath( "go", KIcon::Small );
      QString tmpStr = "kaddressbook --editor-only --uid ";
      tmpStr += KProcess::quote( uri.mid( 6 ) );
      KRun::runCommand( tmpStr, "KAddressBook", iconPath );
      return true;
    }
  }
  else {  // no special URI, let KDE handle it
    KRun *run = new KRun(KURL( uri ));
  }
#endif
  
  return false;
}
