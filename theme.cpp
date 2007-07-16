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
    //TODO: msg "invalid file"
    kDebug() << "Theme: can't import: invalid file: " << url.path() << endl;
    return;
  }

  clearCurrentTheme();
  ThemeImporter reader( file );
/*  ThemeImporter reader();
    if ( !reader.read( file ) ) {
    // FIXME: why doesn't this work?
    kWarning() << "Theme import: Parse error in file " << file->fileName() << " at line "
        << reader.lineNumber() << ", column " << reader.columnNumber() << ":" <<endl;
    kWarning() << reader.errorString() << endl;
  } else {
    kDebug() << "Theme: File loaded" << endl;
  }*/

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
