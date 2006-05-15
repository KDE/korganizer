/*
    This file is part of KOrganizer.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef CUSTOMLISTVIEWITEM_H
#define CUSTOMLISTVIEWITEM_H

#include <qmap.h>
#include <qstring.h>
#include <klistview.h>

template<class T>
class CustomListViewItem : public KListViewItem
{
  public:
    CustomListViewItem( T data, KListView *parent ) :
      KListViewItem( parent ), mData( data ) { updateItem(); };
    CustomListViewItem( T data, KListView *parent, KListViewItem* after ) :
      KListViewItem( parent, after ), mData( data ) { updateItem(); };
    ~CustomListViewItem() {};
    
    void updateItem() {};

    T data() const { return mData; }

    QString key(int column, bool) const
    {
      QMap<int,QString>::ConstIterator it = mKeyMap.find(column);
      if (it == mKeyMap.end()) return text(column);
      else return *it;
    }

    void setSortKey(int column,const QString &key)
    {
      mKeyMap.insert(column,key);
    }

  private:
    T mData;

    QMap<int,QString> mKeyMap;
};

#endif
