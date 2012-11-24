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

#ifndef KOTODOMODEL_H_
#define KOTODOMODEL_H_

#include <Akonadi/EntityTreeModel>
#include <QAbstractProxyModel>

namespace CalendarSupport {
  class IncidenceChanger;
  class Calendar;
}
class QMimeData;
class KOTodoModel : public QAbstractProxyModel {
  Q_OBJECT
public:
  /** This enum defines all columns this model provides */
  enum {
    SummaryColumn = 0,
    RecurColumn,
    PriorityColumn,
    PercentColumn,
    DueDateColumn,
    CategoriesColumn,
    DescriptionColumn,
    CalendarColumn,
    ColumnCount // Just for iteration/column count purposes. Always keep at the end of enum.
  };

  /** This enum defines the user defined roles of the items in this model */
  enum { //TODO: Check if this number should heigher
    TodoRole = Akonadi::EntityTreeModel::UserRole + 1,
    IsRichTextRole
  };
  explicit KOTodoModel( QObject *parent = 0 );
  ~KOTodoModel();

  /**reimp*/ int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  /**reimp*/ int columnCount( const QModelIndex &parent = QModelIndex() ) const;

  /**reimp*/ void setSourceModel( QAbstractItemModel *sourceModel );

  /**reimp*/ QVariant data( const QModelIndex &index, int role ) const;
  /**reimp*/ bool setData( const QModelIndex &index, const QVariant &value, int role );
  /**reimp*/ QVariant headerData( int section, Qt::Orientation, int role ) const;

  /**reimp*/ void setCalendar( CalendarSupport::Calendar *calendar );
  /**reimp*/ void setIncidenceChanger( CalendarSupport::IncidenceChanger *changer );

  /**reimp*/ QMimeData* mimeData( const QModelIndexList &indexes ) const;
  /**reimp*/ bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                                int row, int column, const QModelIndex &parent );
  /**reimp*/ QStringList mimeTypes() const;
  /**reimp*/ Qt::DropActions supportedDropActions() const;
  /**reimp*/ Qt::ItemFlags flags( const QModelIndex &index ) const;
  /**reimp*/ QModelIndex parent( const QModelIndex &child ) const;

  /**reimp*/ QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const;
  /**reimp*/ QModelIndex mapToSource( const QModelIndex &proxyIndex ) const;
  /**reimp*/ QModelIndex index( int row, int column,
                                const QModelIndex &parent = QModelIndex() ) const;
  /**reimp*/ QModelIndex buddy( const QModelIndex &index ) const;
Q_SIGNALS:
  void expandIndex( const QModelIndex &index );
private:
  class Private;
  Private *const d;
};

#endif
