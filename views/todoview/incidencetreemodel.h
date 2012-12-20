/*
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

#ifndef INCIDENCE_TREEMODEL_H_
#define INCIDENCE_TREEMODEL_H_

#include <Akonadi/Item>
#include <QAbstractProxyModel>

class IncidenceTreeModel : public QAbstractProxyModel {
  Q_OBJECT
public:

  /**
   * Constructs a new IncidenceTreeModel.
   */
  explicit IncidenceTreeModel( QObject *parent = 0 );

  /**
   * Constructs a new IncidenceTreeModel which will only show incidences of
   * type @p mimeTypes. Common use case is a to-do tree.
   *
   * This constructor is offered for performance reasons. The filtering has
   * zero overhead, and we avoid stacking mime type filter proxy models.
   *
   * If you're more concerned about clean design than performance, use the default
   * constructor and stack a Akonadi::EntityMimeTypeFilterModel on top of this one.
   */
  explicit IncidenceTreeModel( const QStringList &mimeTypes, QObject *parent = 0 );

  ~IncidenceTreeModel();

  /**reimp*/ int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  /**reimp*/ int columnCount( const QModelIndex &parent = QModelIndex() ) const;

  /**reimp*/ QVariant data( const QModelIndex &index, int role ) const;

  /**reimp*/ QModelIndex index( int row, int column,
                                const QModelIndex &parent = QModelIndex() ) const;

  /**reimp*/ QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const;
  /**reimp*/ QModelIndex mapToSource( const QModelIndex &proxyIndex ) const;

  /**reimp*/ QModelIndex parent( const QModelIndex &child ) const;

  /**reimp*/ void setSourceModel( QAbstractItemModel *sourceModel );
  /**reimp*/ bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;

  /**
   * Returns the akonadi item containing the incidence with @p incidenceUid.
   */
  Akonadi::Item item( const QString &incidenceUid ) const;

Q_SIGNALS:
  /**
   * This signal is emitted whenever an index changes parent.
   * The view can then expand the parent if desired.
   * This is better than the view waiting for "rows moved" signals because those
   * signals are also sent when the model is initially populated.
   */
  void indexChangedParent( const QModelIndex &index );

  /**
   * Signals that we finished doing a batch of insertions.
   *
   * One rowsInserted() signal from the ETM, will make IncidenceTreeModel generate
   * several rowsInserted(), layoutChanged() or rowsMoved() signals.
   *
   * A tree view can use this signal to know when to call KViewStateSaver::restore()
   * to restore expansion states. Listening to rowsInserted() signals would be a
   * performance problem.
   */
  void batchInsertionFinished();

private:
  class Private;
  Private *const d;
};

#endif
