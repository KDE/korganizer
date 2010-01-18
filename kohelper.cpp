/*
  This file is part of KOrganizer.

  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "kohelper.h"
#include "koprefs.h"

#include <Akonadi/Collection>
#include <akonadi/item.h>

#include <QDate>

using namespace Akonadi;

QColor KOHelper::getTextColor( const QColor &c )
{
  double luminance = ( c.red() * 0.299 ) + ( c.green() * 0.587 ) + ( c.blue() * 0.114 );
  return ( luminance > 128.0 ) ? QColor( 0, 0, 0 ) : QColor( 255, 255, 255 );
}

QColor KOHelper::resourceColor( const Collection &coll ) {
  if ( !coll.isValid() )
    return QColor();
  const QString id = QString::number( coll.id() );
  return KOPrefs::instance()->resourceColor( id );
}

QColor KOHelper::resourceColor( const Item &item ) {
  if ( !item.isValid() )
    return QColor();
  const QString id = QString::number( item.storageCollectionId() );
  return KOPrefs::instance()->resourceColor( id );
}



qint64 KOHelper::yearDiff( const QDate &start, const QDate &end )
{
  return static_cast<qint64>( start.daysTo( end ) / 365.25 );
}

