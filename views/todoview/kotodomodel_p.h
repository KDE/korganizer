/*
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>
  Copyright (c) 2012 Sérgio Martins <iamsergio@gmail.com>

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

#ifndef TODOMODEL_P_H_
#define TODOMODEL_P_H_

#include "kotodomodel.h"
#include <Akonadi/Item>

#include <QString>
#include <QModelIndex>

namespace CalendarSupport {
  class Calendar;
  class IncidenceChanger;
}

class KOTodoModel::Private : public QObject
{
  Q_OBJECT
public:
  Private( KOTodoModel *qq );
  void expandTodoIfNeeded( const Akonadi::Item &item );

  //TODO: O(N) complexity, see if the profiler complains about this
  Akonadi::Item findItemByUid( const QString &uid, const QModelIndex &parent ) const;

public:
  CalendarSupport::Calendar *m_calendar;
  CalendarSupport::IncidenceChanger *m_changer;

  //For adjusting persistent indexes
  QList<QPersistentModelIndex> m_layoutChangePersistentIndexes;
  QModelIndexList m_persistentIndexes;
  QList<int> m_columns;
private Q_SLOTS:
  void onDataChanged( const QModelIndex &begin, const QModelIndex &end );
  void onHeaderDataChanged( Qt::Orientation orientation, int first, int last );

  void onRowsAboutToBeInserted( const QModelIndex &parent, int begin, int end );
  void onRowsInserted( const QModelIndex &parent, int begin, int end );
  void onRowsAboutToBeRemoved( const QModelIndex &parent, int begin, int end );
  void onRowsRemoved( const QModelIndex &parent, int begin, int end );
  void onRowsAboutToBeMoved( const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                             const QModelIndex &destinationParent, int destinationRow );
  void onRowsMoved( const QModelIndex &, int, int, const QModelIndex &, int );

  void onModelAboutToBeReset();
  void onModelReset();
  void onLayoutAboutToBeChanged();
  void onLayoutChanged();
private:
  KOTodoModel *q;
};

#endif
