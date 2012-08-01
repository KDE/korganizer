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

#ifndef KORG_VIEWS_KOTODOMODEL_H
#define KORG_VIEWS_KOTODOMODEL_H

#include <akonadi/calendar/incidencechanger.h>
#include <Akonadi/Item>

#include <KCalCore/Todo>
#include <QAbstractItemModel>

namespace CalendarSupport {
  class Calendar;
}

namespace Akonadi {
  class IncidenceChanger;
}

/**
	@author Thomas Thrainer
*/
class KOTodoModel : public QAbstractItemModel
{
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
    enum {
      TodoRole = Qt::UserRole + 1,
      IsRichTextRole
    };

  public:
    explicit KOTodoModel( QObject *parent = 0 );
    virtual ~KOTodoModel();

    /** Set the calendar */
    void setCalendar( CalendarSupport::Calendar *cal );
    /** Resets the model and deletes the whole todo tree */
    void clearTodos();
    /** Reloads all todos from the Calendar provided during construction */
    void reloadTodos();
    /** Reloads only the specified todo (if the incidence is a todo) */
    void processChange( const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType );

    Akonadi::Item todoForIndex( const QModelIndex &idx ) const;

    /** Sets the incidence changer used to edit incidences (todos)
     *
     * @param changer Pointer to the changer to use.
     */
    void setIncidenceChanger( Akonadi::IncidenceChanger *changer )
    { mChanger = changer; }

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent ) const;
    virtual QModelIndex parent( const QModelIndex &child ) const;

    virtual int columnCount( const QModelIndex &parent ) const;
    virtual int rowCount( const QModelIndex &parent ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual QVariant headerData( int section, Qt::Orientation,
                                 int role ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value,
                          int role );

    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;
    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                               int row, int column, const QModelIndex &parent );

  public Q_SLOTS:
    /** Sets if the model should provide a hierarchical view on todos or not.
     *
     * If this is set to true, no parental relationship will be generated. All
     * todos will be in a single list. If this is false, todos are grouped
     * under their parents.
     *
     * @param flatView Whether to display todos as list or as tree.
     */
    void setFlatView( bool flatView );

    /** Sets if the model should provide a full window view or not.
     *
     * @param fullView Whether to display the todo list in a full window.
     */
    void setFullView( bool fullView );

  Q_SIGNALS:

    /**
     * This signal is emitted when the view should expand all parents of
     * this item. This is emitted when a todo was added that is overdue
     * or due today.
     */
    void expandIndex( const QModelIndex &index );
    void flatViewChanged( bool enabled );
    void fullViewChanged( bool enabled );

  private:
    struct TodoTreeNode;

    /** Create a model index for a given todo tree node. */
    QModelIndex getModelIndex( TodoTreeNode *todoTree ) const;

    /** Move the TodoTreeNode if the relationship of the todo's parent has
     *  changed and return a model index to the current node.
     */
    QModelIndex moveIfParentChanged( TodoTreeNode *curNode, const Akonadi::Item &todo,
                                     bool addParentIfMissing );

    /** Recursively find a todo.
     *
     * @param uid uid to the todo to find.
     * @return Pointer to the TodoTreeNode node which represents the todo
     *         searched for or 0 if not found.
     */
    TodoTreeNode *findTodo( const QString &uid ) const;

    /**
     * If the todo is overdue or due today, the expandIndex signal
     * is emitted so that the view can expand the parents of this
     * todo.
     *
     * @param todo the todo whose parents will be expanded if needed
     */
    void expandTodoIfNeeded( const Akonadi::Item &todo );

    /**
     * Returns true if there's a loop, e.g.: t1 is parent of
     * t2 which is parent of t3 which is parent of t1
     *
     * @param todo the todo that will be checked
     */
    bool isInHierarchyLoop( const KCalCore::Todo::Ptr &todo ) const;

    /** Insert a todo at the right place in the todo tree.
     *
     * @param todo The todo to insert in the tree.
     * @param checkRelated Whether to try to link the todo under its
     *                     related todo or not. This is used to break
     *                     recursively linked todos.
     * @return Pointer to the newly inserted TodoTreeNode node.
     */
    TodoTreeNode *insertTodo( const Akonadi::Item &todo, bool checkRelated = true );

    /** Count of columns each item has */
    const int mColumnCount;

    /** Calendar to get data from */
    CalendarSupport::Calendar *mCalendar;
    /** Root elements of the todo tree. */
    TodoTreeNode *mRootNode;
    /** Hash to speed up searching todo by their uid */
    QHash<Akonadi::Item::Id, TodoTreeNode*> mTodoHash;

    QHash<QString, TodoTreeNode*> mTodoUidHash;

    /** This Akonadi::IncidenceChanger is used to change todos */
    Akonadi::IncidenceChanger *mChanger;

    /** Display the todos without hierarchy? */
    bool mFlatView;

    /** Display the todos in a full window? */
    bool mFullView;
};

#endif
