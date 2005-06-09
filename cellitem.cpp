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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "cellitem.h"

#include <klocale.h>
#include <kdebug.h>

#include <qintdict.h>

using namespace KOrg;

QString CellItem::label() const
{
  return i18n("<undefined>");
}

QPtrList<CellItem> CellItem::placeItem( QPtrList<CellItem> cells,
                                        CellItem *placeItem )
{
  kdDebug(5855) << "Placing " << placeItem->label() << endl;

  QPtrList<KOrg::CellItem> conflictItems;
  int maxSubCells = 0;
  QIntDict<KOrg::CellItem> subCellDict;

  // Find all items which are in same cell
  QPtrListIterator<KOrg::CellItem> it2( cells );
  for( it2.toFirst(); it2.current(); ++it2 ) {
    KOrg::CellItem *item = it2.current();
    if ( item == placeItem ) continue;

    if ( item->overlaps( placeItem ) ) {
      kdDebug(5855) << "  Overlaps: " << item->label() << endl;

      conflictItems.append( item );
      if ( item->subCells() > maxSubCells ) maxSubCells = item->subCells();
      subCellDict.insert( item->subCell(), item );
    }
  }

  if ( conflictItems.count() > 0 ) {
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

    QPtrListIterator<KOrg::CellItem> it3( conflictItems );
    for( it3.toFirst(); it3.current(); ++it3 ) {
      (*it3)->setSubCells( maxSubCells );
    }
    // Todo: Adapt subCells of items conflicting with conflicting items
  } else {
    kdDebug(5855) << "  no conflicts" << endl;
    placeItem->setSubCell( 0 );
    placeItem->setSubCells( 1 );
  }
  
  return conflictItems;
}
