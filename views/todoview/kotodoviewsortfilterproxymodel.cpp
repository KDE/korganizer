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
  51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kotodoviewsortfilterproxymodel.h"
#include "kotodomodel.h"
#include "koprefs.h"

#include <KDebug>
#include <QModelIndex>

KOTodoViewSortFilterProxyModel::KOTodoViewSortFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
}

void KOTodoViewSortFilterProxyModel::sort( int column, Qt::SortOrder order )
{
  mSortOrder = order;
  QSortFilterProxyModel::sort( column, order );
}

bool KOTodoViewSortFilterProxyModel::filterAcceptsRow(
                  int source_row, const QModelIndex &source_parent ) const
{
  bool ret = QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );

  if ( ret && !mCategories.isEmpty() ) {
    QStringList categories =
      sourceModel()->index( source_row, KOTodoModel::CategoriesColumn, source_parent ).
      data( Qt::EditRole ).toStringList();
    foreach ( const QString &category, categories ) {
      if ( mCategories.contains( category ) ) {
        return true;
      }
    }
    ret = false;
  }

  // check if one of the children is accepted, and accept this node too if so
  QModelIndex cur = sourceModel()->index( source_row, KOTodoModel::SummaryColumn, source_parent );
  for ( int r = 0; r < cur.model()->rowCount( cur ); ++r ) {
    if ( filterAcceptsRow( r, cur ) ) {
      return true;
    }
  }

  return ret;
}

bool KOTodoViewSortFilterProxyModel::lessThan( const QModelIndex &left,
                                               const QModelIndex &right ) const
{
  if ( KOPrefs::instance()->sortCompletedTodosSeparately() ) {
    QModelIndex cLeft = left.sibling( left.row(), KOTodoModel::PercentColumn );
    QModelIndex cRight = right.sibling( right.row(), KOTodoModel::PercentColumn );

    if ( cRight.data( Qt::EditRole ).toInt() == 100 &&
         cLeft.data( Qt::EditRole ).toInt() != 100 ) {
      return mSortOrder == Qt::AscendingOrder ? true : false;
    } else if ( cRight.data( Qt::EditRole ).toInt() != 100 &&
                cLeft.data( Qt::EditRole ).toInt() == 100 ) {
      return mSortOrder == Qt::AscendingOrder ? false : true;
    }
  }

  // To-dos without due date should appear last when sorting ascending,
  // so you can see the most urgent tasks first. (bug #174763)
  if ( right.column() == KOTodoModel::DueDateColumn ) {
    bool leftIsEmpty  = sourceModel()->data( left  ).toString().isEmpty();
    bool rightIsEmpty = sourceModel()->data( right ).toString().isEmpty();

    if ( leftIsEmpty || rightIsEmpty ) {
      return rightIsEmpty;
    }
  }

  return QSortFilterProxyModel::lessThan( left, right );
}

void KOTodoViewSortFilterProxyModel::setCategoryFilter( const QStringList &categories )
{
  mCategories = categories;
  invalidateFilter();
}

#include "kotodoviewsortfilterproxymodel.moc"
