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
  mParentWidget( parentWidget ), mDownloadDialog( 0 ),
  mUploadDialog( 0 ), mProviderDialog( 0 ), mUploadProvider( 0 ),
  mNewStuff( newStuff ), mType( type )
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
  kdDebug(5850) << "Engine::download()" << endl;

  connect( mProviderLoader,
           SIGNAL( providersLoaded( Provider::List * ) ),
           SLOT( getMetaInformation( Provider::List * ) ) );
  mProviderLoader->load( mType );
}

void Engine::getMetaInformation( Provider::List *providers )
{
  mProviderLoader->disconnect();

  mNewStuffJobData.clear();

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
  }
}

void Engine::slotNewStuffJobData( KIO::Job *job, const QByteArray &data )
{
  if ( data.isEmpty() ) return;

  kdDebug(5850) << "Engine:slotNewStuffJobData()" << endl;

  kdDebug(5850) << "===START===" << endl << data.data() << "===END===" << endl;

  mNewStuffJobData[ job ].append( data );
}

void Engine::slotNewStuffJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error downloading new stuff descriptions." << endl;
    job->showErrorDialog( mParentWidget );
  } else {
    QString knewstuffDoc = QString::fromUtf8( mNewStuffJobData[ job ] );

    kdDebug(5850) << "---START---" << endl << knewstuffDoc << "---END---" << endl;

    QDomDocument doc;
    if ( !doc.setContent( knewstuffDoc ) ) {
      kdDebug(5850) << "Error parsing knewstuff.xml." << endl;
      return;
    } else {
      QDomElement knewstuff = doc.documentElement();

      if ( knewstuff.isNull() ) {
        kdDebug(5850) << "No document in knewstuffproviders.xml." << endl;
      } else {
        QDomNode p;
        for ( p = knewstuff.firstChild(); !p.isNull(); p = p.nextSibling() ) {
          QDomElement stuff = p.toElement();
          if ( stuff.tagName() != "stuff" ) continue;

          Entry *entry = new Entry( stuff );
          mNewStuffList.append( entry );

          mDownloadDialog->show();

          mDownloadDialog->addEntry( entry );
    
          kdDebug(5850) << "KNEWSTUFF: " << entry->name() << endl;

          kdDebug(5850) << "  SUMMARY: " << entry->summary() << endl;
          kdDebug(5850) << "  VERSION: " << entry->version() << endl;
          kdDebug(5850) << "  RELEASEDATE: " << entry->releaseDate().toString() << endl;
          kdDebug(5850) << "  RATING: " << entry->rating() << endl;

          kdDebug(5850) << "  LANGS: " << entry->langs().join(", ") << endl;
        }
      }
    }
  }
  
  mNewStuffJobData.remove( job );

  if ( mNewStuffJobData.count() == 0 ) {
    mDownloadDialog->show();
    mDownloadDialog->raise();
  }
}

void Engine::download( Entry *entry )
{
  kdDebug(5850) << "Engine::download(entry)" << endl;

  KURL source = entry->payload();
  mDownloadDestination = mNewStuff->downloadDestination( entry );

  if ( mDownloadDestination.isEmpty() ) {
    kdDebug(5850) << "Empty downloadDestination. Cancelling download." << endl;
    return;
  }

  KURL destination = KURL( mDownloadDestination );

  kdDebug(5850) << "  SOURCE: " << source.url() << endl;
  kdDebug(5850) << "  DESTINATION: " << destination.url() << endl;

  KIO::FileCopyJob *job = KIO::file_copy( source, destination, -1, true );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotDownloadJobResult( KIO::Job * ) ) );
}

void Engine::slotDownloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error downloading new stuff payload." << endl;
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
  kdDebug(5850) << "Engine:selectUploadProvider()" << endl;

  mProviderLoader->disconnect();

  if ( !mProviderDialog ) {
    mProviderDialog = new ProviderDialog( this, mParentWidget );
  }

  mProviderDialog->clear();
  
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
  QString uploadFile = entry->fullName();
  uploadFile = locateLocal( "data", "korganizer/upload/" + uploadFile );

  if ( !mNewStuff->createUploadFile( uploadFile ) ) {
    KMessageBox::error( mParentWidget, i18n("Unable to create file to upload") );
    return;
  }

  QString lang = entry->langs().first();
  QFileInfo fi( uploadFile );
  entry->setPayload( fi.fileName(), lang );

  if ( !createMetaFile( entry ) ) return;

  QString text = i18n("The files to be uploaded have been created at:\n");
  text.append( uploadFile + "\n" );
  text.append( mUploadMetaFile + "\n" );

  QString caption = i18n("Upload files");

  if ( mUploadProvider->noUpload() ) {
    KURL noUploadUrl = mUploadProvider->noUploadUrl();
    if ( noUploadUrl.isEmpty() ) {
      text.append( i18n("Please upload the files manually.") ); 
      KMessageBox::information( mParentWidget, text, caption );
    } else {
      int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                               i18n("Upload Info..."),
                                               i18n("Close") );
      if ( result == KMessageBox::Yes ) {
        kapp->invokeBrowser( noUploadUrl.url() );
      }
    }
  } else {
    int result = KMessageBox::questionYesNo( mParentWidget, text, caption,
                                             i18n("Upload"), i18n("Cancel") );
    if ( result == KMessageBox::Yes ) {
      KURL destination = mUploadProvider->uploadUrl();
      destination.setFileName( fi.fileName() );

      KIO::FileCopyJob *job = KIO::file_copy( uploadFile, destination );
      connect( job, SIGNAL( result( KIO::Job * ) ),
               SLOT( slotUploadPayloadJobResult( KIO::Job * ) ) );
    }
  }
}

bool Engine::createMetaFile( Entry *entry )
{
  QDomDocument doc("knewstuff");
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement de = doc.createElement("knewstuff");
  doc.appendChild( de );

  de.appendChild( entry->createDomElement( doc, de ) );
  
  kdDebug(5850) << "--DOM START--" << endl << doc.toString()
            << "--DOM_END--" << endl;

  mUploadMetaFile = entry->fullName() + ".meta";
  mUploadMetaFile = locateLocal( "data", "korganizer/upload/" + mUploadMetaFile );

  QFile f( mUploadMetaFile );
  if ( !f.open( IO_WriteOnly ) ) {
    mUploadMetaFile = QString::null;
    return false;
  }
  
  QTextStream ts( &f );
  ts.setEncoding( QTextStream::UnicodeUTF8 );
  ts << doc.toString();
  
  f.close();
  
  return true;
}

void Engine::slotUploadPayloadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error uploading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    return;
  }

  QFileInfo fi( mUploadMetaFile );

  KURL metaDestination = mUploadProvider->uploadUrl();
  metaDestination.setFileName( fi.fileName() );

  KIO::FileCopyJob *newJob = KIO::file_copy( mUploadMetaFile, metaDestination );
  connect( newJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUploadMetaJobResult( KIO::Job * ) ) );
}

void Engine::slotUploadMetaJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    kdDebug(5850) << "Error uploading new stuff payload." << endl;
    job->showErrorDialog( mParentWidget );
    return;
  }

  KMessageBox::information( mParentWidget,
                            i18n("Successfully uploaded new stuff.") );
}
