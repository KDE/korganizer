/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "provider.h"
#include "provider.moc"

using namespace KNS;

Provider::Provider() : mNoUpload( false )
{
}

Provider::Provider( const QDomElement &e ) : mNoUpload( false )
{
  parseDomElement( e );
}

Provider::~Provider()
{
}


void Provider::setName( const QString &name )
{
  mName = name;
}

QString Provider::name() const
{
  return mName;
}


void Provider::setIcon( const KURL &url )
{
  mIcon = url;
}

KURL Provider::icon() const
{
  return mIcon;
}


void Provider::setDownloadUrl( const KURL &url )
{
  mDownloadUrl = url;
}

KURL Provider::downloadUrl() const
{
  return mDownloadUrl;
}


void Provider::setUploadUrl( const KURL &url )
{
  mUploadUrl = url;
}

KURL Provider::uploadUrl() const
{
  return mUploadUrl;
}


void Provider::setNoUploadUrl( const KURL &url )
{
  mNoUploadUrl = url;
}

KURL Provider::noUploadUrl() const
{
  return mNoUploadUrl;
}


void Provider::setNoUpload( bool enabled )
{
  mNoUpload = enabled;
}

bool Provider::noUpload() const
{
  return mNoUpload;
}


void Provider::parseDomElement( const QDomElement &element )
{
  if ( element.tagName() != "provider" ) return;

  setDownloadUrl( KURL( element.attribute("downloadurl") ) );
  setUploadUrl( KURL( element.attribute("uploadurl") ) );
  setNoUploadUrl( KURL( element.attribute("nouploadurl") ) );
  setIcon( KURL( element.attribute("icon") ) );

  QDomNode n;
  for ( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement p = n.toElement();
    
    if ( p.tagName() == "noupload" ) setNoUpload( true );
    if ( p.tagName() == "title" ) setName( p.text().stripWhiteSpace() );
  }
}

QDomElement Provider::createDomElement( QDomDocument &doc, QDomElement &parent )
{
  QDomElement entry = doc.createElement( "stuff" );
  parent.appendChild( entry );

  QDomElement n = doc.createElement( "name" );
  n.appendChild( doc.createTextNode( name() ) );
  entry.appendChild( n );
  
  return entry;
}


ProviderLoader::ProviderLoader( QWidget *parentWidget ) :
  mParentWidget( parentWidget )
{
  mProviders.setAutoDelete( true );
}

void ProviderLoader::load( const QString &type )
{
  kdDebug(5850) << "ProviderLoader::load()" << endl;

  mProviders.clear();
  mJobData = "";

  KConfig *cfg = KGlobal::config();
  cfg->setGroup("KNewStuff");

  QString providersUrl = cfg->readEntry( "ProvidersUrl" );

  if ( providersUrl.isEmpty() ) {
    // FIXME: Replace the default by the real one.
    QString server = cfg->readEntry( "MasterServer",
                                     "http://korganizer.kde.org" );
  
    providersUrl = server + "/knewstuff/" + type + "/providers.xml";
  }

  kdDebug(5850) << "ProviderLoader::load(): providersUrl: " << providersUrl << endl;
  
  KIO::TransferJob *job = KIO::get( KURL( providersUrl ) );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotJobResult( KIO::Job * ) ) );
  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );

//  job->dumpObjectInfo();
}

void ProviderLoader::slotJobData( KIO::Job *, const QByteArray &data )
{
  kdDebug(5850) << "ProviderLoader::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  QCString str( data, data.size() + 1 );

  mJobData.append( QString::fromUtf8( str ) );
}

void ProviderLoader::slotJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( mParentWidget );
  }

  kdDebug(5850) << "--PROVIDERS-START--" << endl << mJobData << "--PROV_END--"
            << endl;

  QDomDocument doc;
  if ( !doc.setContent( mJobData ) ) {
    KMessageBox::error( mParentWidget, i18n("Error parsing providers list.") );
    return;
  }

  QDomElement providers = doc.documentElement();

  if ( providers.isNull() ) {
    kdDebug(5850) << "No document in Providers.xml." << endl;
  }

  QDomNode n;
  for ( n = providers.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement p = n.toElement();
    
    mProviders.append( new Provider( p ) );
  }
  
  emit providersLoaded( &mProviders );
}
