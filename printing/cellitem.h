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
#ifndef KORG_CELLITEM_H
#define KORG_CELLITEM_H

#include <qstring.h>
#include <qptrlist.h>

namespace KOrg {

class CellItem
{
  public:
    CellItem()
      : mSubCells( 0 ), mSubCell( -1 )
    {
    }
  
    void setSubCells( int v ) { mSubCells = v; }
    int subCells() const { return mSubCells; }
    
    void setSubCell( int v ) { mSubCell = v; }
    int subCell() const { return mSubCell; }
    
    virtual bool overlaps( CellItem *other ) const = 0;

    virtual QString label() const;

    /**
      Place item \arg placeItem into stripe containing items \arg cells in a
      way that items don't overlap.
      
      \return Placed items
    */
    static QPtrList<CellItem> placeItem( QPtrList<CellItem> cells,
                                         CellItem *placeItem );
    
  private:
    int mSubCells;
    int mSubCell;
};

}

#endif
