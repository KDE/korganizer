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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "urihandler.h"

#include <libkcal/attachment.h>
#include <libkcal/calendarresources.h>
#include <libkcal/incidence.h>
using namespace KCal;

#ifndef KORG_NODCOP
#include <dcopclient.h>
#include "kmailIface_stub.h"
#endif

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <krun.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kio/netaccess.h>

#include <qfile.h>

static Attachment *findAttachment( const QString &name, const QString &uid )
{
  CalendarResources *cal = new CalendarResources( "UTC" );
  cal->readConfig();
  cal->load();
  Incidence *incidence = cal->incidence( uid );
  if ( !incidence ) {
    KMessageBox::sorry(
      0,
      i18n( "The incidence that owns the attachment named \"%1\" could not be found. "
            "Perhaps it was removed from your calendar?" ).arg( name ) );
    return 0;
  }

  // get the attachment by name from the incidence
  Attachment::List as = incidence->attachments();
  Attachment *a = 0;
  if ( as.count() > 0 ) {
    Attachment::List::ConstIterator it;
    for ( it = as.begin(); it != as.end(); ++it ) {
      if ( (*it)->label() == name ) {
        a = *it;
        break;
      }
    }
  }

  if ( !a ) {
    KMessageBox::error(
      0,
      i18n( "No attachment named \"%1\" found in the incidence." ).arg( name ) );
    return 0;
  }

  if ( a->isUri() ) {
    if ( !KIO::NetAccess::exists( a->uri(), true, 0 ) ) {
      KMessageBox::sorry(
        0,
        i18n( "The attachment \"%1\" is a web link that is inaccessible from this computer. " ).
        arg( KURL::decode_string( a->uri() ) ) );
      return 0;
    }
  }
  return a;
}

bool UriHandler::openAttachment( const QString &name, const QString &uid )
{
  Attachment *a = findAttachment( name, uid );
  if ( !a ) {
    return false;
  }

  if ( a->isUri() ) {
    kapp->invokeBrowser( a->uri() );
  } else {
    // put the attachment in a temporary file and launch it
    KTempFile *file;
    QStringList patterns = KMimeType::mimeType( a->mimeType() )->patterns();
    if ( !patterns.empty() ) {
      file = new KTempFile( QString::null,
                            QString( patterns.first() ).remove( '*' ),0600 );
    } else {
      file = new KTempFile( QString::null, QString::null, 0600 );
    }
    file->file()->open( IO_WriteOnly );
    QTextStream stream( file->file() );
    stream.writeRawBytes( a->decodedData().data(), a->size() );
    file->close();

    bool stat = KRun::runURL( KURL( file->name() ), a->mimeType(), 0, true );
    delete file;
    return stat;
  }
  return true;
}

bool UriHandler::saveAsAttachment( const QString &name, const QString &uid )
{
  Attachment *a = findAttachment( name, uid );
  if ( !a ) {
    return false;
  }

  // get the saveas file name
  QString saveAsFile =
    KFileDialog::getSaveFileName( name,
                                  QString::null, 0,
                                  i18n( "Save Attachment" ));
  if ( saveAsFile.isEmpty() ||
       ( QFile( saveAsFile ).exists() &&
         ( KMessageBox::warningYesNo(
             0,
             i18n( "%1 already exists. Do you want to overwrite it?").
             arg( saveAsFile ) ) == KMessageBox::No ) ) ) {
    return false;
  }

  bool stat = false;
  if ( a->isUri() ) {
    // save the attachment url
    stat = KIO::NetAccess::file_copy( a->uri(), KURL( saveAsFile ), -1, true );
  } else {
    // put the attachment in a temporary file and save it
    KTempFile *file;
    QStringList patterns = KMimeType::mimeType( a->mimeType() )->patterns();
    if ( !patterns.empty() ) {
      file = new KTempFile( QString::null,
                            QString( patterns.first() ).remove( '*' ),0600 );
    } else {
      file = new KTempFile( QString::null, QString::null, 0600 );
    }
    file->file()->open( IO_WriteOnly );
    QTextStream stream( file->file() );
    stream.writeRawBytes( a->decodedData().data(), a->size() );
    file->close();

    stat = KIO::NetAccess::file_copy( KURL( file->name() ), KURL( saveAsFile ), -1, true );

    delete file;
  }
  return stat;
}

QString UriHandler::attachmentNameFromUri( const QString &uri )
{
  QString tmp;
  if ( uri.startsWith( "ATTACH:" ) ) {
    tmp = uri.mid( 9 ).section( ':', -1, -1 );
  }
  return tmp;
}

QString UriHandler::uidFromUri( const QString &uri )
{
  QString tmp;
  if ( uri.startsWith( "ATTACH:" ) ) {
    tmp = uri.mid( 9 ).section( ':', 0, 0 );
  } else if ( uri.startsWith( "uid:" ) ) {
    tmp = uri.mid( 6 );
  }
  return tmp;
}

bool UriHandler::process( const QString &uri )
{
  kdDebug(5850) << "UriHandler::process(): " << uri << endl;

#ifndef KORG_NODCOP
  if ( uri.startsWith( "kmail:" ) ) {

    // make sure kmail is running or the part is shown
    kapp->startServiceByDesktopPath("kmail");

    // parse string, show
    int colon = uri.find( ':' );
    // extract 'number' from 'kmail:<number>/<id>'
    QString serialNumberStr = uri.mid( colon + 1 );
    serialNumberStr = serialNumberStr.left( serialNumberStr.find( '/' ) );

    KMailIface_stub kmailIface( "kmail", "KMailIface" );
    kmailIface.showMail( serialNumberStr.toUInt(), QString() );
    return true;

  } else if ( uri.startsWith( "mailto:" ) ) {

    KApplication::kApplication()->invokeMailer( uri.mid(7), QString::null );
    return true;

  } else if ( uri.startsWith( "uid:" ) ) {

    QString uid = uidFromUri( uri );
    DCOPClient *client = KApplication::kApplication()->dcopClient();
    const QByteArray noParamData;
    const QByteArray paramData;
    QByteArray replyData;
    QCString replyTypeStr;
    bool foundAbbrowser = client->call( "kaddressbook", "KAddressBookIface",
                                        "interfaces()",  noParamData,
                                        replyTypeStr, replyData );
    if ( foundAbbrowser ) {
      // KAddressbook is already running, so just DCOP to it to bring up the contact editor
#if KDE_IS_VERSION( 3, 2, 90 )
      kapp->updateRemoteUserTimestamp("kaddressbook");
#endif
      DCOPRef kaddressbook( "kaddressbook", "KAddressBookIface" );
      kaddressbook.send( "showContactEditor", uid );
      return true;
    } else {
      // KaddressBook is not already running.
      // Pass it the UID of the contact via the command line while starting it - its neater.
      // We start it without its main interface
      QString iconPath = KGlobal::iconLoader()->iconPath( "go", KIcon::Small );
      QString tmpStr = "kaddressbook --editor-only --uid ";
      tmpStr += KProcess::quote( uid );
      KRun::runCommand( tmpStr, "KAddressBook", iconPath );
      return true;
    }

  } else if ( uri.startsWith( "ATTACH:" ) ) {

    // a calendar incidence attachment
    return openAttachment( attachmentNameFromUri( uri ), uidFromUri( uri ) );

  } else {  // no special URI, let KDE handle it
    new KRun( KURL( uri ) );
  }
#endif

  return false;
}
