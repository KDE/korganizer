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

#include <qcstring.h>
#include <qdom.h>
#include <qfileinfo.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "knewstuff.h"
#include "downloaddialog.h"
#include "uploaddialog.h"
#include "providerdialog.h"

#include "engine.h"
#include "engine.moc"

using namespace KNS;

Engine::Engine( KNewStuff *newStuff, const QString &type,
                QWidget *parentWidget ) :
  mParentWidget( parentWidget ), mNewStuffJobCount( 0 ), mDownloadDialog( 0 ),
  mUploadDialog( 0 ), mProviderDialog( 0 ), mUploadProvider( 0 ),
  mMetaUploaded( false ), mNewStuff( newStuff ), mType( type )
{
  mProviderLoader = new ProviderLoader( mParentWidget );

  mNewStuffList.setAutoDelete( true );
}

Engine::~Engine()
{
  delete mProviderLoader;

  delete mUploadDialog;
  delete mDownloadDialog;
}

void Engine::download()
{
  kdDebug() << "Engine::download()" << endl;

  connect( mProviderLoader,
           SIGNAL( providersLoaded( Provider::List * ) ),
           SLOT( getMetaInformation( Provider::List * ) ) );
  mProviderLoader->load( mType );
}

void Engine::getMetaInformation( Provider::List *providers )
{
  mProviderLoader->disconnect();

  mNewStuffJobCount = 0;

  if ( !mDownloadDialog ) {
    mDownloadDialog = new DownloadDialog( this, mParentWidget );
    mDownloadDialog->show();
  }
  mDownloadDialog->clear();

  Provider *p;
  for ( p = providers->first(); p; p = providers->next() ) {
    if ( p->downloadUrl().isEmpty() ) continue;

    KIO::TransferJob *job = KIO::get( p->downloadUrl() );
    connect( job, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotNewStuffJobResult( KIO::Job * ) ) );
    connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
             SLOT( slotNewStuffJobData( KIO::Job *, const QByteArray & ) ) );

    mNewStuffJobData.insert( job, "" );
    ++mNewStuffJobCount;
  }
}

void Engine::slotNewStuffJobData( KIO::Job *job, const QByteArray &data )
{
  if ( data.isEmpty() ) return;

  kdDebug() << "Engine:slotNewStuffJobData()" << endl;

  kdDebug() << "===START===" << endl << data.data() << "===END===" << endl;

  QString s = data;
  mNewStuffJobData[ job ].append( s );
}

void Engine::slotNewStuffJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error downloading new stuff descriptions." << endl;
    job->showErrorDialog( mParentWidget );
  } else {
    QString knewstuffDoc = QString::fromUtf8( mNewStuffJobData[ job ] );

    kdDebug() << "---START---" << endl << knewstuffDoc << "---END---" << endl;

    QDomDocument doc;
    if ( !doc.setContent( knewstuffDoc ) ) {
      kdDebug() << "Error parsing knewstuff.xml." << endl;
      return;
    } else {
      QDomElement knewstuff = doc.documentElement();

      if ( knewstuff.isNull() ) {
        kdDebug() << "No document in knewstuffproviders.xml." << endl;
      } else {
        QDomNode p;
        for ( p = knewstuff.firstChild(); !p.isNull(); p = p.nextSibling() ) {
          QDomElement stuff = p.toElement();
          if ( stuff.tagName() != "stuff" ) continue;

          Entry *entry = new Entry( stuff );
          mNewStuffList.append( entry );

          mDownloadDialog->show();

          mDownloadDialog->addEntry( entry );
    
          kdDebug() << "KNEWSTUFF: " << entry->name() << endl;

          kdDebug() << "  SUMMARY: " << entry->summary() << endl;
          kdDebug() << "  VERSION: " << entry->version() << endl;
          kdDebug() << "  RELEASEDATE: " << entry->releaseDate().toString() << endl;
          kdDebug() << "  RATING: " << entry->rating() << endl;

          kdDebug() << "  LANGS: " << entry->langs().join(", ") << endl;
        }
      }
    }
  }

  if ( --mNewStuffJobCount == 0 ) {
    mDownloadDialog->show();
    mDownloadDialog->raise();
  }
}

void Engine::download( Entry *entry )
{
  kdDebug() << "Engine::download(entry)" << endl;

  KURL source = entry->payload();
  mDownloadDestination = mNewStuff->downloadDestination( entry );
  KURL destination = KURL( mDownloadDestination );

  kdDebug() << "  SOURCE: " << source.url() << endl;
  kdDebug() << "  DESTINATION: " << destination.url() << endl;

  KIO::FileCopyJob *job = KIO::file_copy( source, destination );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotDownloadJobResult( KIO::Job * ) ) );
}

void Engine::slotDownloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error downloading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    return;
  }

  if ( mNewStuff->install( mDownloadDestination ) ) {
    KMessageBox::information( mParentWidget,
                              i18n("Successfully installed hot new stuff.") );
  } else {
    KMessageBox::error( mParentWidget,
                        i18n("Failed to install hot new stuff.") );
  }
}

void Engine::upload()
{
  connect( mProviderLoader,
           SIGNAL( providersLoaded( Provider::List * ) ),
           SLOT( selectUploadProvider( Provider::List * ) ) );
  mProviderLoader->load( mType );
}

void Engine::selectUploadProvider( Provider::List *providers )
{
  kdDebug() << "Engine:selectUploadProvider()" << endl;

  mProviderLoader->disconnect();

  if ( !mProviderDialog ) {
    mProviderDialog = new ProviderDialog( this, mParentWidget );
  }
  mProviderDialog->show();
  mProviderDialog->raise();

  for( Provider *p = providers->first(); p; p = providers->next() ) {
    mProviderDialog->addProvider( p );
  }
}

void Engine::requestMetaInformation( Provider *provider )
{
  mUploadProvider = provider;

  if ( !mUploadDialog ) {
    mUploadDialog = new UploadDialog( this, mParentWidget );
  }
  mUploadDialog->show();
  mUploadDialog->raise();
}

void Engine::upload( Entry *entry )
{
  mUploadEntry = entry;

  QString lang = entry->langs().first();

  QString uploadFile = mNewStuff->createUploadFile();
  if ( uploadFile.isEmpty() ) {
    KMessageBox::error( mParentWidget, i18n("Can't create file to upload.") );
    return;
  }

  QFileInfo fi( uploadFile );

  entry->setPayload( fi.fileName(), lang );

  KURL destination = mUploadProvider->uploadUrl();
  destination.setFileName( fi.fileName() );

  KIO::FileCopyJob *job = KIO::file_copy( uploadFile, destination );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUploadPayloadJobResult( KIO::Job * ) ) );
}

void Engine::slotUploadPayloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error uploading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    return;
  }

  mMetaUploaded = false;

  KURL metaDestination = mUploadProvider->uploadUrl();
  metaDestination.setFileName( mUploadEntry->fullName() );
  
  KIO::TransferJob *newjob = KIO::put( metaDestination, -1, false, false );
  connect( newjob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUploadMetaJobResult( KIO::Job * ) ) );
  connect( newjob, SIGNAL( dataReq( KIO::Job *, QByteArray & ) ),
           SLOT( slotUploadMetaJobDataReq( KIO::Job *, QByteArray & ) ) );
}

void Engine::slotUploadMetaJobDataReq( KIO::Job *, QByteArray &data )
{
  if ( mMetaUploaded ) return;
  
  QDomDocument doc("knewstuff");
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement de = doc.createElement("knewstuff");
  doc.appendChild( de );

  de.appendChild( mUploadEntry->createDomElement( doc, de ) );
  
  kdDebug() << "--DOM START--" << endl << doc.toString() << endl;

  data = doc.toString().utf8();

  mMetaUploaded = true;
}

void Engine::slotUploadMetaJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug() << "Error uploading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    return;
  }

  KMessageBox::information( mParentWidget,
                            i18n("Successfully uploaded new stuff.") );
}
