/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "theme.h"
#include "themeimporter.h"
#include "koprefs.h"

#include <KIO/NetAccess>
#include <QDebug>
#include <KMimeType>
#include <KZip>

#include <QtCore/QFile>
#include <QStandardPaths>


using namespace KOrg;

void Theme::useThemeFrom( const KUrl &url )
{
  if ( url.isEmpty() ) {
    return;
  }

  if( !url.isLocalFile() ) {
      qDebug() << "can't import (1) : only local files are supported" << url.prettyUrl();
      return;
  }

  QFile *file = new QFile( url.toLocalFile() );
  qDebug() << file->fileName();
  if ( !file->open( QFile::ReadOnly | QFile::Text ) ) {
    //TODO: KMessageBox "invalid file"
    qDebug() << "can't import: invalid file: (1)" << url.toLocalFile();
    delete file;
    return;
  }

  KMimeType::Ptr mimeType;
  mimeType = KMimeType::findByUrl( url );

  if ( mimeType->name() == "application/zip" ) {
    KZip *zip = new KZip( url.toLocalFile() );

    if ( !zip->open( QIODevice::ReadOnly ) ) {
      //TODO: KMessageBox "invalid file"
      qDebug() << "can't import: invalid file: (3)" << url.toLocalFile();
      delete zip;
      delete file;
      return;
    }

    const KArchiveDirectory *dir = zip->directory();
    if ( dir == 0 ) {
      //TODO: KMessageBox "invalid file"
      qDebug() << "can't import: invalid file: (4)" << url.toLocalFile();
      delete zip;
      delete file;
      return;
    }

    if ( ! KIO::NetAccess::del( QUrl::fromLocalFile( storageDir().absolutePath() ),
                                0 ) ) {
      qWarning() << "could not delete stale theme files";
    }
    dir->copyTo( storageDir().path() );

    delete file;

    file = new QFile( storageDir().path() + "/theme.xml" );

    if ( !file->open( QFile::ReadOnly | QFile::Text ) ) {
      //TODO: KMessageBox "invalid file"
      qDebug() << "can't import: invalid file: (5)" << url.toLocalFile();
      delete file;
      delete zip;
      return;
    }

    KMimeType::Ptr mimeType;
    mimeType = KMimeType::findByUrl( storageDir().path() + "/theme.xml" );
    if ( mimeType->name() != "application/xml" ) {
      //TODO: KMessageBox "invalid file"
      qDebug() << "can't import: invalid file: (6)" << url.toLocalFile();
      delete zip;
      delete file;
      return;
    }
  } else if ( mimeType->name() == "application/xml" ) {
    KIO::NetAccess::file_copy( url.toLocalFile(), storageDir().path() + '/', 0 );
    delete file;
  } else {
    //TODO: KMessageBox "invalid file"
    qDebug() << "can't import: invalid file: (2)" << url.toLocalFile();
    delete file;
    return;
  }

  clearCurrentTheme();
  ThemeImporter reader( file );
}

void Theme::saveThemeTo( const KUrl &url )
{
  const QString path = url.isLocalFile() ? url.toLocalFile() : url.path();

  KZip *zip = new KZip( path );

  if ( ! zip->open( QIODevice::WriteOnly ) ) {
      //TODO: KMessageBox "no write permission"
    qDebug() << "can't export: no write permission:" << path;
    return;
  }
  if ( ! zip->addLocalDirectory( storageDir().absolutePath(), QString() ) ) {
      //TODO: KMessageBox "could not add theme files"
    qDebug() << "can't export: could not add theme files to:" << path;
    return;
  }
  if ( ! zip->close() ) {
      //TODO: KMessageBox "could not write theme file"
    qDebug() << "can't export: could not close theme file:" << path;
    return;
  }
}

void Theme::clearCurrentTheme()
{
  foreach ( const QString &viewType, Theme::themableViews() ) {
    KSharedConfig::Ptr conf = KSharedConfig::openConfig();
    KConfigGroup( conf, "Theme/" + viewType + " view" ).deleteGroup();
  }
}

const QDir Theme::storageDir()
{
  QDir *dir = new QDir( QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + "theme" ) ;
  return *dir;
}

const QStringList Theme::themableViews( const QString &viewType )
{
  QStringList l;
  l.append( "Agenda" );
  l.append( "Month" );
  // TODO:  TodoView?
  if ( l.contains( viewType ) ) {
    return QStringList( viewType );
  } else if ( viewType.isEmpty() ) {
    return l;
  } else {
    return QStringList();
  }
}
