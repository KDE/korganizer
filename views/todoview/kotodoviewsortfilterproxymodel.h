/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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

#ifndef KOTODOVIEWSORTFILTERPROXYMODEL_H
#define KOTODOVIEWSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QStringList>
#include <Qt>

class QModelIndex;

class KOTodoViewSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

  public:
    KOTodoViewSortFilterProxyModel( QObject *parent = 0 );

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
    const QStringList& categories() const { return mCategories; }

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

  public Q_SLOTS:
    void setCategoryFilter( const QStringList &categories );

  private:
    QStringList mCategories;
    Qt::SortOrder mSortOrder;
};

#endif
