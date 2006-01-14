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

#include "cellitem.h"

#include <klocale.h>
#include <kdebug.h>

#include <QList>
#include <QMultiHash>

using namespace KOrg;

QString CellItem::label() const
{
  return i18n("<undefined>");
}

QList<CellItem*> CellItem::placeItem( QList<CellItem*> cells,
                                        CellItem *placeItem )
{
  kdDebug(5855) << "Placing " << placeItem->label() << endl;

  QList<KOrg::CellItem*> conflictItems;
  int maxSubCells = 0;
  QMultiHash<int,KOrg::CellItem*> subCellDict;

  // Find all items which are in same cell
  QList<KOrg::CellItem*>::iterator it;
  for( it = cells.begin(); it != cells.end(); ++it ) {
    KOrg::CellItem *item = *it;
    if ( item == placeItem ) continue;

    if ( item->overlaps( placeItem ) ) {
      kdDebug(5855) << "  Overlaps: " << item->label() << endl;

      conflictItems.append( item );
      if ( item->subCells() > maxSubCells ) maxSubCells = item->subCells();
      subCellDict.insert( item->subCell(), item );
    }
  }

  if ( !conflictItems.empty() ) {
    // Look for unused sub cell and insert item
    int i;
    for( i = 0; i < maxSubCells; ++i ) {
      kdDebug(5855) << "  Trying subcell " << i << endl;
      if ( !subCellDict.find( i ) ) {
        kdDebug(5855) << "  Use subcell " << i << endl;
        placeItem->setSubCell( i );
        break;
      }
    }
    if ( i == maxSubCells ) {
      kdDebug(5855) << "  New subcell " << i << endl;
      placeItem->setSubCell( maxSubCells );
      maxSubCells++;  // add new item to number of sub cells
    }

    kdDebug(5855) << "  Sub cells: " << maxSubCells << endl;

    // Write results to item to be placed
    conflictItems.append( placeItem );
    placeItem->setSubCells( maxSubCells );

    QList<KOrg::CellItem*>::iterator it;
    for ( it = conflictItems.begin(); it != conflictItems.end(); ++it ) {
      (*it)->setSubCells( maxSubCells );
    }
    // Todo: Adapt subCells of items conflicting with conflicting items
  } else {
    kdDebug(5855) << "  no conflicts" << endl;
    placeItem->setSubCell( 0 );
    placeItem->setSubCells( 1 );
  }
  
  return conflictItems;
}
