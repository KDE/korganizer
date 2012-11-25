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

#include "kotodomodel_p.h"
#include "koglobals.h"
#include "koprefs.h"
#include "kohelper.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/calendaradaptor.h>
#include <calendarsupport/incidencechanger.h>
#include <KLocale>
#include <KCalCore/Todo>
#include <KCalCore/Event>
#include <KCalCore/Attachment>
#include <KCalUtils/IncidenceFormatter>
#include <KPIMUtils/Email>

#include <calendarsupport/dndfactory.h>
#include <KCalUtils/DndFactory>
#include <KCalUtils/ICalDrag>
#include <KCalUtils/VCalDrag>
#include <QMimeData>

#include <KMessageBox>

#include <QIcon>

struct SourceModelIndex
{
    SourceModelIndex(int _r, int _c, void *_p, QAbstractItemModel *_m)
      : r(_r), c(_c), p(_p), m(_m)
    {
    }

    operator QModelIndex() { return reinterpret_cast<QModelIndex&>(*this); }

    int r, c;
    void *p;
    const QAbstractItemModel *m;
};

static bool isDueToday( const KCalCore::Todo::Ptr &todo )
{
  return !todo->isCompleted() && todo->dtDue().date() == QDate::currentDate();
}

KOTodoModel::Private::Private( KOTodoModel *qq ) : QObject()
                                                 , m_calendar( 0 )
                                                 , q( qq )
{
}

// TODO: Call this.
void KOTodoModel::Private::expandTodoIfNeeded( const Akonadi::Item &item )
{
  Q_ASSERT( item.isValid() );
  KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
  Q_ASSERT( todo );
  if ( todo->isOverdue() || isDueToday( todo ) ) {
    const QModelIndexList indexes = Akonadi::EntityTreeModel::modelIndexesForItem( q, item );
    Q_ASSERT( !indexes.isEmpty() );
    emit q->expandIndex( indexes.first() );
  }
}

Akonadi::Item KOTodoModel::Private::findItemByUid( const QString &uid,
                                                 const QModelIndex &parent ) const
{
  Akonadi::Item item;
  const int count = q->rowCount( parent );
  for ( int i=0; i<count; ++i ) {
    const QModelIndex currentIndex = q->index( i, 0, parent );
    Q_ASSERT( currentIndex.isValid() );
    item = q->data( currentIndex, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if ( item.isValid() ) {
      return item;
    } else {
      item = findItemByUid( uid, currentIndex );
      if ( item.isValid() )
        return item;
    }
  }

  return item;
}

void KOTodoModel::Private::onDataChanged( const QModelIndex &begin, const QModelIndex &end )
{
  Q_ASSERT( begin.isValid() );
  Q_ASSERT( end.isValid() );
  const QModelIndex proxyBegin = q->mapFromSource( begin );
  Q_ASSERT( proxyBegin.column() == 0 );
  const QModelIndex proxyEnd = q->mapFromSource( end );
  emit q->dataChanged( proxyBegin, proxyEnd.sibling( proxyEnd.row(), KOTodoModel::ColumnCount-1 ) );
}

void KOTodoModel::Private::onHeaderDataChanged( Qt::Orientation orientation, int first, int last )
{
  emit q->headerDataChanged( orientation, first, last );
}

void KOTodoModel::Private::onRowsAboutToBeInserted( const QModelIndex &parent, int begin, int end )
{
  q->beginInsertRows( q->mapFromSource( parent ), begin, end );
}

void KOTodoModel::Private::onRowsInserted( const QModelIndex &, int, int )
{
  q->endInsertRows();
}

void KOTodoModel::Private::onRowsAboutToBeRemoved( const QModelIndex &parent, int begin, int end )
{
  q->beginRemoveRows( q->mapFromSource( parent ), begin, end );
}

void KOTodoModel::Private::onRowsRemoved( const QModelIndex &, int, int )
{
  q->endRemoveRows();
}

void KOTodoModel::Private::onRowsAboutToBeMoved( const QModelIndex &sourceParent, int sourceStart,
                                                 int sourceEnd, const QModelIndex &destinationParent,
                                                 int destinationRow )
{
  Q_UNUSED( sourceParent );
  Q_UNUSED( sourceStart );
  Q_UNUSED( sourceEnd );
  Q_UNUSED( destinationParent );
  Q_UNUSED( destinationRow );
  /* Disabled for now, layoutAboutToBeChanged() is emitted
  q->beginMoveRows( q->mapFromSource( sourceParent ), sourceStart, sourceEnd,
                    q->mapFromSource( destinationParent ), destinationRow );
  */
}

void KOTodoModel::Private::onRowsMoved( const QModelIndex &, int, int, const QModelIndex &, int )
{
  /*q->endMoveRows();*/
}

void KOTodoModel::Private::onModelAboutToBeReset()
{
  q->beginResetModel();
}

void KOTodoModel::Private::onModelReset()
{
  q->endResetModel();
}

void KOTodoModel::Private::onLayoutAboutToBeChanged()
{
  Q_ASSERT( m_persistentIndexes.isEmpty() );
  Q_ASSERT( m_layoutChangePersistentIndexes.isEmpty() );
  Q_ASSERT( m_columns.isEmpty() );
  QModelIndexList persistentIndexes = q->persistentIndexList();
  foreach( const QPersistentModelIndex &persistentIndex, persistentIndexes ) {
    m_persistentIndexes << persistentIndex; // Stuff we have to update onLayoutChanged
    Q_ASSERT( persistentIndex.isValid() );
    QModelIndex index_col0 = q->createIndex( persistentIndex.row(), 0, persistentIndex.internalPointer() );
    const QPersistentModelIndex srcPersistentIndex = q->mapToSource( index_col0 );
    Q_ASSERT( srcPersistentIndex.isValid() );
    m_layoutChangePersistentIndexes << srcPersistentIndex;
    m_columns << persistentIndex.column();
  }
  emit q->layoutAboutToBeChanged();
}

void KOTodoModel::Private::onLayoutChanged()
{
  for ( int i = 0; i < m_persistentIndexes.size(); ++i ) {
    QModelIndex newIndex_col0 = q->mapFromSource( m_layoutChangePersistentIndexes.at( i ) );
    Q_ASSERT( newIndex_col0.isValid() );
    const int column = m_columns.at( i );
    QModelIndex newIndex = column == 0 ? newIndex_col0 :
                                         q->createIndex( newIndex_col0.row(), column, newIndex_col0.internalPointer() );
    q->changePersistentIndex( m_persistentIndexes.at( i ), newIndex );
  }

  m_layoutChangePersistentIndexes.clear();
  m_persistentIndexes.clear();
  m_columns.clear();
  emit q->layoutChanged();
}


KOTodoModel::KOTodoModel( QObject *parent ) : QAbstractProxyModel( parent )
                                            , d( new Private( this ) )
{
  setObjectName( "KOTodoModel" );
}

KOTodoModel::~KOTodoModel()
{
  delete d;
}

QVariant KOTodoModel::data( const QModelIndex &index, int role ) const
{
  Q_ASSERT( index.isValid() );
  if ( !index.isValid() || !d->m_calendar ) {
    return QVariant();
  }

  const QModelIndex sourceIndex = mapToSource( index.sibling( index.row(), 0 ) );
  if ( !sourceIndex.isValid() ) {
    return QVariant();
  }
  Q_ASSERT( sourceIndex.isValid() );
  const Akonadi::Item item = sourceIndex.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !item.isValid() ) {
    kWarning() << "Invalid index: " << sourceIndex;
    //Q_ASSERT( false );
    return QVariant();
  }
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( item );
  Q_ASSERT( todo );

  if ( role == Qt::DisplayRole ) {
    switch ( index.column() ) {
    case SummaryColumn:
      return QVariant( todo->summary() );
    case RecurColumn:
      return QVariant( todo->recurs() ?
                       i18nc( "yes, recurring to-do", "Yes" ) :
                       i18nc( "no, not a recurring to-do", "No" ) );
    case PriorityColumn:
      if ( todo->priority() == 0 ) {
        return QVariant( QString::fromAscii( "--" ) );
      }
      return QVariant( todo->priority() );
    case PercentColumn:
      return QVariant( todo->percentComplete() );
    case DueDateColumn:
      if ( todo->hasDueDate() && todo->dtDue().date().isValid() ) {
        return QVariant( KCalUtils::IncidenceFormatter::dateToString( todo->dtDue() ) );
      } else {
        return QVariant( QString() );
      }
    case CategoriesColumn:
    {
      QString categories = todo->categories().join(
        i18nc( "delimiter for joining category names", "," ) );
      return QVariant( categories );
    }
    case DescriptionColumn:
      return QVariant( todo->description() );
    case CalendarColumn:
      return QVariant( CalendarSupport::displayName( d->m_calendar, item.parentCollection() ) );
    }
    return QVariant();
  }

  if ( role == Qt::EditRole ) {
    switch ( index.column() ) {
    case SummaryColumn:
      return QVariant( todo->summary() );
    case RecurColumn:
      return QVariant( todo->recurs() );
    case PriorityColumn:
      return QVariant( todo->priority() );
    case PercentColumn:
      return QVariant( todo->percentComplete() );
    case DueDateColumn:
      return QVariant( todo->dtDue().date() );
    case CategoriesColumn:
      return QVariant( todo->categories() );
    case DescriptionColumn:
      return QVariant( todo->description() );
    case CalendarColumn:
      return QVariant( CalendarSupport::displayName( d->m_calendar, item.parentCollection() ) );
    }
    return QVariant();
  }

  // set the tooltip for every item
  if ( role == Qt::ToolTipRole ) {
    if ( KOPrefs::instance()->enableToolTips() ) {
      return QVariant( KCalUtils::IncidenceFormatter::toolTipStr(
                         CalendarSupport::displayName( d->m_calendar, item.parentCollection() ),
                         todo, QDate(), true,
                         CalendarSupport::KCalPrefs::instance()->timeSpec() ) );
    } else {
      return QVariant();
    }
  }

  // background colour for todos due today or overdue todos
  if ( role == Qt::BackgroundRole ) {
    if ( todo->isOverdue() ) {
      return QVariant(
        QBrush( KOPrefs::instance()->todoOverdueColor() ) );
    } else if ( isDueToday( todo ) ) {
      return QVariant(
        QBrush( KOPrefs::instance()->todoDueTodayColor() ) );
    }
  }

  // indicate if a row is checked (=completed) only in the first column
  if ( role == Qt::CheckStateRole && index.column() == 0 ) {
    if ( todo->isCompleted() ) {
      return QVariant( Qt::Checked );
    } else {
      return QVariant( Qt::Unchecked );
    }
  }

  // icon for recurring todos
  // It's in the summary column so you don't accidentally click
  // the checkbox ( which increments the next occurrence date ).
  if ( role == Qt::DecorationRole && index.column() == SummaryColumn ) {
    if ( todo->recurs() ) {
      return QVariant( QIcon( KOGlobals::self()->smallIcon( "task-recurring" ) ) );
    }
  }

  // category colour
  if ( role == Qt::DecorationRole && index.column() == SummaryColumn ) {
    QStringList categories = todo->categories();
    return categories.isEmpty() ? QVariant() : QVariant( CalendarSupport::KCalPrefs::instance()->categoryColor( categories.first() ) );
  } else if ( role == Qt::DecorationRole ) {
    return QVariant();
  }

  if ( role == TodoRole ) {
    QVariant ret( QMetaType::VoidStar );
    ret.setValue( item );
    return ret;
  }

  if ( role == IsRichTextRole ) {
    if ( index.column() == SummaryColumn ) {
      return QVariant( todo->summaryIsRich() );
    } else if ( index.column() == DescriptionColumn ) {
      return QVariant( todo->descriptionIsRich() );
    } else {
      return QVariant();
    }
  }

  if ( role == Qt::TextAlignmentRole ) {
    switch ( index.column() ) {
      // If you change this, change headerData() too.
      case RecurColumn:
      case PriorityColumn:
      case PercentColumn:
      case DueDateColumn:
        return QVariant( Qt::AlignHCenter | Qt::AlignVCenter );
    }
    return QVariant( Qt::AlignLeft | Qt::AlignVCenter );
  }

  if ( sourceModel() ) {
    return sourceModel()->data( mapToSource( index.sibling( index.row(), 0 ) ), role );
  }

  return QVariant();
}

bool KOTodoModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_ASSERT( index.isValid() );
  if ( !d->m_changer ) {
    return false;
  }
  const QVariant oldValue = data( index, role );

  if ( oldValue == value ) {
    // Nothing changed, the user used one of the QStyledDelegate's editors but seted the old value
    // Lets just skip this then and avoid a roundtrip to akonadi, and avoid sending invitations
    return true;
  }

  const Akonadi::Item item = data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( item );

  if ( !item.isValid() || !todo ) {
    kWarning() << "KOTodoModel::setData() called, bug item is invalid or doesn't have payload";
    Q_ASSERT( false );
    return false;
  }

  if ( d->m_calendar->hasChangeRights( item ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );
    CalendarSupport::IncidenceChanger::WhatChanged modified =
      CalendarSupport::IncidenceChanger::UNKNOWN_MODIFIED;

    if ( role == Qt::CheckStateRole && index.column() == 0 ) {
      todo->setCompleted( static_cast<Qt::CheckState>( value.toInt() ) == Qt::Checked );
      if ( todo->recurs() ) {
        modified = CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED_WITH_RECURRENCE;
      } else {
        modified = CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED;
      }
    }

    if ( role == Qt::EditRole ) {
      switch ( index.column() ) {
        case SummaryColumn:
          if ( !value.toString().isEmpty() ) {
            todo->setSummary( value.toString() );
            modified = CalendarSupport::IncidenceChanger::SUMMARY_MODIFIED;
          }
          break;
        case PriorityColumn:
          todo->setPriority( value.toInt() );
          modified = CalendarSupport::IncidenceChanger::PRIORITY_MODIFIED;
          break;
        case PercentColumn:
          todo->setPercentComplete( value.toInt() );
          modified = CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED;
          break;
        case DueDateColumn:
          {
            KDateTime tmp = todo->dtDue();
            tmp.setDate( value.toDate() );
            todo->setDtDue( tmp );
            todo->setHasDueDate( value.toDate().isValid() );
            modified = CalendarSupport::IncidenceChanger::DATE_MODIFIED;
          }
          break;
        case CategoriesColumn:
          todo->setCategories( value.toStringList() );
          modified = CalendarSupport::IncidenceChanger::CATEGORY_MODIFIED;
          break;
        case DescriptionColumn:
          todo->setDescription( value.toString() );
          modified = CalendarSupport::IncidenceChanger::DESCRIPTION_MODIFIED;
          break;
      }
    }

    if ( modified != CalendarSupport::IncidenceChanger::UNKNOWN_MODIFIED ) {
      d->m_changer->changeIncidence( oldTodo, item, modified, 0 );
      // changeIncidence will eventually call the view's
      // changeIncidenceDisplay method, which in turn
      // will call processChange. processChange will then emit
      // dataChanged to the view, so we don't have to
      // do it here
    }

    return true;
  } else {
    if ( !( role == Qt::CheckStateRole && index.column() == 0 ) ) {
      KOHelper::showSaveIncidenceErrorMsg( 0, todo ); //TODO pass parent
    }
    return false;
  }
}

int KOTodoModel::rowCount( const QModelIndex &parent ) const
{
  if ( sourceModel() ) {
    if ( parent.isValid() ) {
      QModelIndex parent_col0 = createIndex( parent.row(), 0, parent.internalPointer() );
      return sourceModel()->rowCount( mapToSource( parent_col0 ) );
    } else {
      return sourceModel()->rowCount();
    }
  }
  return 0;
}

int KOTodoModel::columnCount( const QModelIndex & ) const
{
  return ColumnCount;
}

void KOTodoModel::setSourceModel( QAbstractItemModel *model )
{
  if ( model == sourceModel() )
    return;

  beginResetModel();

  if ( sourceModel() ) {
    disconnect( sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                d, SLOT(onDataChanged(QModelIndex,QModelIndex)) );
    disconnect( sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                d, SLOT(onHeaderDataChanged(Qt::Orientation,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                d, SLOT(onRowsInserted(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                d, SLOT(onRowsRemoved(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                d, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)) );

    disconnect( sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                d, SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                d, SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(modelAboutToBeReset()),
                d, SLOT(onModelAboutToBeReset()) );

    disconnect( sourceModel(), SIGNAL(modelReset()),
                d, SLOT(onModelReset()) );

    disconnect( sourceModel(), SIGNAL(layoutAboutToBeChanged()),
                d, SLOT(onLayoutAboutToBeChanged()) );

    disconnect( sourceModel(), SIGNAL(layoutChanged()),
                d, SLOT(onLayoutChanged()) );
  }

  QAbstractProxyModel::setSourceModel( model );

  if ( sourceModel() ) {
    connect( sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             d, SLOT(onDataChanged(QModelIndex,QModelIndex)) );

    connect( sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
             d, SLOT(onHeaderDataChanged(Qt::Orientation,int,int)) );

    connect( sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
             d, SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
             d, SLOT(onRowsInserted(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             d, SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
             d, SLOT(onRowsRemoved(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
             d, SLOT(onRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)) );

    connect( sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
             d, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)) );

    connect( sourceModel(), SIGNAL(modelAboutToBeReset()),
             d, SLOT(onModelAboutToBeReset()) );

    connect( sourceModel(), SIGNAL(modelReset()),
             d, SLOT(onModelReset()) );

    connect( sourceModel(), SIGNAL(layoutAboutToBeChanged()),
             d, SLOT(onLayoutAboutToBeChanged()) );

    connect( sourceModel(), SIGNAL(layoutChanged()),
             d, SLOT(onLayoutChanged()) );
  }

  endResetModel();
}

void KOTodoModel::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  d->m_changer = changer;
}

QVariant KOTodoModel::headerData( int column, Qt::Orientation orientation, int role ) const
{
  if ( orientation != Qt::Horizontal ) {
    return QVariant();
  }

  if ( role == Qt::DisplayRole ) {
    switch ( column ) {
    case SummaryColumn:
      return QVariant( i18n( "Summary" ) );
    case RecurColumn:
      return QVariant( i18n( "Recurs" ) );
    case PriorityColumn:
      return QVariant( i18n( "Priority" ) );
    case PercentColumn:
      return QVariant( i18nc( "@title:column percent complete", "Complete" ) );
    case DueDateColumn:
      return QVariant( i18n( "Due Date/Time" ) );
    case CategoriesColumn:
      return QVariant( i18n( "Categories" ) );
    case DescriptionColumn:
      return QVariant( i18n( "Description" ) );
    case CalendarColumn:
      return QVariant( i18n( "Calendar" ) );
    }
  }

  if ( role == Qt::TextAlignmentRole ) {
    switch ( column ) {
      // If you change this, change data() too.
      case RecurColumn:
      case PriorityColumn:
      case PercentColumn:
      case DueDateColumn:
        return QVariant( Qt::AlignHCenter );
    }
    return QVariant();
  }
  return QVariant();
}

void KOTodoModel::setCalendar( CalendarSupport::Calendar *calendar )
{
  d->m_calendar = calendar;
}

Qt::DropActions KOTodoModel::supportedDropActions() const
{
  // Qt::CopyAction not supported yet
  return Qt::MoveAction;
}

QStringList KOTodoModel::mimeTypes() const
{
  static QStringList list;
  if ( list.isEmpty() ) {
    list << KCalUtils::ICalDrag::mimeType() << KCalUtils::VCalDrag::mimeType();
  }
  return list;
}

QMimeData *KOTodoModel::mimeData( const QModelIndexList &indexes ) const
{
  Akonadi::Item::List items;
  Q_FOREACH ( const QModelIndex &index, indexes ) {
    const Akonadi::Item item = this->data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if ( item.isValid() && !items.contains( item ) ) {
      items.push_back( item );
    }
  }
  return CalendarSupport::createMimeData( items, d->m_calendar->timeSpec() );
}

bool KOTodoModel::dropMimeData( const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( row );
  Q_UNUSED( column );

  if ( action != Qt::MoveAction ) {
    kWarning() << "No action other than MoveAction currently supported!"; //TODO
    return false;
  }

  if ( d->m_calendar && d->m_changer &&
       ( KCalUtils::ICalDrag::canDecode( data ) || KCalUtils::VCalDrag::canDecode( data ) ) ) {
    CalendarSupport::DndFactory dndFactory (
      CalendarSupport::CalendarAdaptor::Ptr(
        new CalendarSupport::CalendarAdaptor( d->m_calendar, 0 ) ), true );
    KCalCore::Todo::Ptr t = dndFactory.createDropTodo( data );
    KCalCore::Event::Ptr e = dndFactory.createDropEvent( data );

    if ( t ) {
      // we don't want to change the created todo, but the one which is already
      // stored in our calendar / tree
      const Akonadi::Item item = d->findItemByUid( t->uid(), QModelIndex() );
      KCalCore::Todo::Ptr todo = CalendarSupport::todo( item );
      KCalCore::Todo::Ptr destTodo;
      if ( parent.isValid() ) {
        const Akonadi::Item parentItem = this->data( parent, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        if ( parentItem.isValid() ) {
          destTodo = CalendarSupport::todo( parentItem );
        }
      }

      KCalCore::Incidence::Ptr tmp = destTodo;
      while ( tmp ) {
        if ( tmp->uid() == todo->uid() ) {
          KMessageBox::information(
            0,
            i18n( "Cannot move to-do to itself or a child of itself." ),
            i18n( "Drop To-do" ), "NoDropTodoOntoItself" );
          return false;
        }
        const QString parentUid = tmp->relatedTo();
        tmp = CalendarSupport::incidence( d->m_calendar->itemForIncidenceUid( parentUid ) );
      }

      KCalCore::Todo::Ptr oldTodo = KCalCore::Todo::Ptr( todo->clone() );
      // destTodo is empty when we drag a to-do out of a relationship
      todo->setRelatedTo( destTodo ? destTodo->uid() : QString() );
      d->m_changer->changeIncidence( oldTodo, item,
                                 CalendarSupport::IncidenceChanger::RELATION_MODIFIED, 0 );

      // again, no need to emit dataChanged, that's done by processChange
      return true;

    } else if ( e ) {
      // TODO: Implement dropping an event onto a to-do: Generate a relationship to the event!
    } else {
      if ( !parent.isValid() ) {
        // TODO we should create a new todo with the data in the drop object
        kDebug() << "TODO: Create a new todo with the given data";
        return false;
      }

      const Akonadi::Item parentItem = this->data( parent, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      KCalCore::Todo::Ptr destTodo = CalendarSupport::todo( parentItem );

      if ( data->hasText() ) {
        QString text = data->text();

        KCalCore::Todo::Ptr oldTodo = KCalCore::Todo::Ptr( destTodo->clone() );

        if ( text.startsWith( QLatin1String( "file:" ) ) ) {
          destTodo->addAttachment( KCalCore::Attachment::Ptr( new KCalCore::Attachment( text ) ) );
        } else {
          QStringList emails = KPIMUtils::splitAddressList( text );
          for ( QStringList::ConstIterator it = emails.constBegin();
                it != emails.constEnd(); ++it ) {
            QString name, email, comment;
            if ( KPIMUtils::splitAddress( *it, name, email, comment ) ==
                 KPIMUtils::AddressOk ) {
              destTodo->addAttendee( KCalCore::Attendee::Ptr( new KCalCore::Attendee( name, email ) ) );
            }
          }
        }
        d->m_changer->changeIncidence( oldTodo, parentItem,
                                       CalendarSupport::IncidenceChanger::RELATION_MODIFIED, 0 );
        return true;
      }
    }
  }

  return false;
}

Qt::ItemFlags KOTodoModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return 0;

  Qt::ItemFlags ret = QAbstractItemModel::flags( index );

  const Akonadi::Item item = data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !item.isValid() ) {
    Q_ASSERT( mapToSource( index ).isValid() );
    kWarning() << "Item is invalid " << index;
    Q_ASSERT( false );
    return 0;
  }

  ret |= Qt::ItemIsDragEnabled;

  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( item );

  if ( d->m_calendar->hasChangeRights( item ) ) {
    // the following columns are editable:
    switch ( index.column() ) {
    case SummaryColumn:
    case PriorityColumn:
    case PercentColumn:
    case DueDateColumn:
    case CategoriesColumn:
      ret |= Qt::ItemIsEditable;
      break;
    case DescriptionColumn:
      if ( !todo->descriptionIsRich() ) {
        ret |= Qt::ItemIsEditable;
      }
      break;
    }
  }

  if ( index.column() == 0 ) {
    // whole rows should have checkboxes, so append the flag for the
    // first item of every row only. Also, only the first item of every
    // row should be used as a target for a drag and drop operation.
    ret |= Qt::ItemIsUserCheckable |
           Qt::ItemIsDropEnabled;
  }
  return ret;
}

QModelIndex KOTodoModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
  if ( !sourceModel() || !sourceIndex.isValid() )
    return QModelIndex();

  Q_ASSERT( sourceIndex.internalPointer() );

  return createIndex( sourceIndex.row(), 0, sourceIndex.internalPointer() );
}

QModelIndex KOTodoModel::mapToSource( const QModelIndex &proxyIndex ) const
{
  if ( !sourceModel() || !proxyIndex.isValid() )
    return QModelIndex();

  if ( proxyIndex.column() != 0 ) {
    kError() << "Map to source called with column>0, but source model only has 1 column";
    Q_ASSERT( false );
  }

  Q_ASSERT( proxyIndex.internalPointer() );

  // we convert to column 0
  const QModelIndex sourceIndex = SourceModelIndex( proxyIndex.row(), 0,
                                                    proxyIndex.internalPointer(), sourceModel() );

  return sourceIndex;
}

QModelIndex KOTodoModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !sourceModel() )
    return QModelIndex();

  Q_ASSERT( !parent.isValid() || parent.internalPointer() );
  QModelIndex parent_col0 = parent.isValid() ? createIndex( parent.row(), 0, parent.internalPointer() )
                                             : QModelIndex();

  // Lets preserve the original internalPointer
  const QModelIndex index = mapFromSource( sourceModel()->index( row, 0, mapToSource( parent_col0 ) ) );
  Q_ASSERT( !index.isValid() || index.internalPointer() );

  if ( index.isValid() ) {
    return createIndex( row, column, index.internalPointer() );
  }

  return QModelIndex();
}

QModelIndex KOTodoModel::parent( const QModelIndex &child ) const
{
  if ( !sourceModel() || !child.isValid() )
    return QModelIndex();

  Q_ASSERT( child.internalPointer() );
  const QModelIndex child_col0 = createIndex( child.row(), 0, child.internalPointer() );
  QModelIndex parentIndex = mapFromSource( sourceModel()->parent( mapToSource( child_col0 ) ) );

  Q_ASSERT( !parentIndex.isValid() || parentIndex.internalPointer() );
  if ( parentIndex.isValid() ) // preserve original column
    return createIndex( parentIndex.row(), child.column(), parentIndex.internalPointer() );
  return QModelIndex();
}

QModelIndex KOTodoModel::buddy( const QModelIndex &index ) const
{
  // We reimplement because the default implementation calls mapToSource() and
  // source model doesn't have the same number of columns.
  return index;
}
