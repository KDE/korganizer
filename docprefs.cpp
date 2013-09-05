/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "docprefs.h"

#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

KConfig *DocPrefs::mConfig = 0;

DocPrefs::DocPrefs( const QString &type )
{
  if ( !mConfig ) {
    mConfig = new KConfig(
      KStandardDirs::locateLocal( "data", QLatin1String("korganizer/docprefs.") + type + QLatin1String(".kconfig") ) );
  }
}

DocPrefs::~DocPrefs()
{
  mConfig->sync();
}

void DocPrefs::setDoc( const QString &identifier )
{
  mDocId = identifier;
}

QString DocPrefs::doc() const
{
  return mDocId;
}

bool DocPrefs::readBoolEntry( const QString &id ) const
{
  KConfigGroup docConfig( mConfig, mDocId );
  bool result = docConfig.readEntry( id, false );
  return result;
}

void DocPrefs::writeBoolEntry( const QString &id, bool value )
{
  KConfigGroup docConfig( mConfig, mDocId );
  docConfig.writeEntry( id, value );
}

int DocPrefs::readNumEntry( const QString &id ) const
{
  KConfigGroup docConfig( mConfig, mDocId );
  int result = docConfig.readEntry( id, 0 );
  return result;
}

void DocPrefs::writeNumEntry( const QString &id, int value )
{
  KConfigGroup docConfig( mConfig, mDocId );
  docConfig.writeEntry( id, value );
}
