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

#ifndef KOTODOMODEL_H
#define KOTODOMODEL_H

#include <QAbstractItemModel>
#include <QWidget>
#include <QString>
#include <QHash>

namespace KCal
{
class Calendar;
class Incidence;
class Todo;

#ifndef KORG_NODND
class DndFactory;
#endif
}
namespace KOrg
{
class IncidenceChangerBase;
}

using namespace KCal;
using namespace KOrg;

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
      RecurColumn = 1,
      PriorityColumn = 2,
      PercentColumn = 3,
      DueDateColumn = 4,
      CategoriesColumn = 5,
      DescriptionColumn = 6
    };

    /** This enum defines the user defined roles of the items in this model */
    enum {
      TodoRole = Qt::UserRole + 1,
      IsRichTextRole
    };

  public:
    explicit KOTodoModel( Calendar *cal, QObject *parent = 0 );
    virtual ~KOTodoModel();

    /** Set the calendar */
    void setCalendar( Calendar *cal );
    /** Resets the model and deletes the whole todo tree */
    void clearTodos();
    /** Reloads all todos from the Calendar provided during construction */
    void reloadTodos();
    /** Reloads only the specified todo (if the incidence is a todo) */
    void processChange( Incidence *incidence, int action );

    /** Creates a new todo with the given text as summary under the given parent */
    QModelIndex addTodo( const QString &summary,
                         const QModelIndex &parent = QModelIndex() );
    /** Copy the todo with the given index to the given date */
    void copyTodo( const QModelIndex &index, const QDate &date );

    /** Sets the incidence changer used to edit incidences (todos)
     *
     * @param changer Pointer to the changer to use.
     */
    void setIncidenceChanger( IncidenceChangerBase *changer )
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

#ifndef KORG_NODND
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;
    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action,
                               int row, int column, const QModelIndex &parent );
#endif

  public Q_SLOTS:
    /** Sets if the model should provide a hirarchical view on todos or not.
     *
     * If this is set to true, no parental relationship will be generated. All
     * todos will be in a single list. If this is false, todos are grouped
     * under their parents.
     *
     * @param flatView Whether to display todos as list or as tree.
     */
    void setFlatView( bool flatView );

  Q_SIGNALS:

    /**
     * This signal is emitted when the view should expand all parents of
     * this item. This is emitted when a todo was added that is overdue
     * or due today.
     */
    void expandIndex( const QModelIndex &index );

  private:
    struct TodoTreeNode;

    /** Create a model index for a given todo tree node. */
    QModelIndex getModelIndex( TodoTreeNode *todoTree ) const;

    /** Move the TodoTreeNode if the relationship of the todo's parent has
     *  changed and return a model index to the current node.
     */
    QModelIndex moveIfParentChanged( TodoTreeNode *curNode, Todo *todo,
                                     bool addParentIfMissing );

   /** Recursively find a todo.
    *
    * @param todo Pointer to the todo to find.
    * @return Pointer to the TodoTreeNode node which represents the todo
    *         searched for or 0 if not found.
    */
    TodoTreeNode *findTodo( const Todo *todo ) const;

    /**
     * If the todo is overdue or due today, the expandIndex signal
     * is emitted so that the view can expand the parents of this
     * todo.
     *
     * @param todo the todo whose parents will be expanded if needed
     */
    void expandTodoIfNeeded( const Todo *todo );

    /** Insert a todo at the right place in the todo tree.
     *
     * @param todo The todo to insert in the tree.
     * @param checkRelated Whether to try to link the todo under its
     *                     related todo or not. This is used to break
     *                     recursively linked todos.
     * @return Pointer to the newly inserted TodoTreeNode node.
     */
    TodoTreeNode *insertTodo( Todo *todo, bool checkRelated = true );

    /** Count of columns each item has */
    const int mColumnCount;

    /** Calendar to get data from */
    Calendar *mCalendar;
    /** Root elements of the todo tree. */
    TodoTreeNode *mRootNode;
    /** Hash to speed up searching todo by their uid */
    QHash<QString, TodoTreeNode*> mTodoHash;

    /** This IncidenceChanger is used to change todos */
    IncidenceChangerBase *mChanger;

    /** Display the todos without hierarchy? */
    bool mFlatView;

#ifndef KORG_NODND
    DndFactory *mDndFactory;
#endif
};

#endif
