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

#include <kdebug.h>
#include <KMimeType>
#include <KStandardDirs>
#include <KTempDir>
#include <KZip>

#include <QtCore/QFile>

#include "theme.moc"

using namespace KOrg;

void Theme::useThemeFrom( const KUrl &url )
{
  if ( url.isEmpty() )
    return;

  QFile* file = new QFile( url.path() ); // FIXME: does it work with remote URLs?
  kDebug() << file->fileName() << endl;
  if ( ( ! file->open(QFile::ReadOnly | QFile::Text) ) || ( ! url.isLocalFile() ) ) {
    //TODO: KMessageBox "invalid file"
    kDebug() << "Theme: can't import: invalid file: (1) " << url.path() << endl;
    return;
  }

  KMimeType::Ptr mimeType;
  mimeType = KMimeType::findByUrl( url );
  if ( ( mimeType->name() != "application/xml" )
       && ( mimeType->name() != "application/zip" ) ) {
    //TODO: KMessageBox "invalid file"
    kDebug() << "Theme: can't import: invalid file: (2) " << url.path() << endl;
    return;
  }

  if ( mimeType->name() == "application/zip" ) {
    QString tempPath = KStandardDirs::locateLocal( "tmp", "korganizer-theme" );

    KTempDir *tempDir = new KTempDir( tempPath );

    KZip *zip = new KZip( url.path() );

    if ( ! zip->open(QIODevice::ReadOnly) ) {
      //TODO: KMessageBox "invalid file"
      kDebug() << "Theme: can't import: invalid file: (3) " << url.path() << endl;
      return;
    }

    const KArchiveDirectory *dir = zip->directory();
    if ( dir == 0 ) {
      //TODO: KMessageBox "invalid file"
      kDebug() << "Theme: can't import: invalid file: (4) " << url.path() << endl;
      return;
    }

    dir->copyTo( tempDir->name() );

    file = new QFile( tempDir->name() + "/theme.xml" );

    if ( ! file->open(QFile::ReadOnly | QFile::Text) ) {
      //TODO: KMessageBox "invalid file"
      kDebug() << "Theme: can't import: invalid file: (5) " << url.path() << endl;
      return;
    }

    KMimeType::Ptr mimeType;
    mimeType = KMimeType::findByUrl( tempDir->name() + "/theme.xml" );
    if ( mimeType->name() != "application/xml" ) {
      //TODO: KMessageBox "invalid file"
      kDebug() << "Theme: can't import: invalid file: (6) " << url.path() << endl;
      return;
    }
  }

  clearCurrentTheme();
  ThemeImporter reader( file );
}

void Theme::saveThemeTo( const KUrl &url )
{
}

void Theme::clearCurrentTheme()
{
  foreach ( QString viewType, Theme::themableViews() ) {
    KConfigGroup( KSharedConfig::openConfig(), "Theme/" + viewType + " view" ).deleteGroup();
  }
}

const QStringList Theme::themableViews( const QString &viewType )
{
  QStringList l;
  l.append( "Agenda" );
  l.append( "Month" );
  // TODO:  TodoView?
  if ( l.contains( viewType ) ) {
    return QStringList( viewType );
  }
  else if ( viewType.isEmpty() ) {
    return l;
  }
  else {
    return QStringList();
  }
}
