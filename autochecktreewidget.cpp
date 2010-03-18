/*
    This file is part of libkdepim.

    Copyright (c) 2007 Mathias Soeken <msoeken@tzi.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
    Boston, MA 02110-1301, USA.
*/
#include "autochecktreewidget.h"

#include <QAbstractItemModel>

//@cond PRIVATE
class AutoCheckTreeWidget::Private {
  public:
    Private() 
      : mAutoCheckChildren( false ),
        mAutoCheck( true ) {}

    bool mAutoCheckChildren;
    bool mAutoCheck;
};
//@endcond

AutoCheckTreeWidget::AutoCheckTreeWidget( QWidget *parent ) 
  : QTreeWidget( parent ), d( new Private() )
{
  connect( model(), SIGNAL( rowsInserted( const QModelIndex &, int, int ) ),
           this, SLOT( slotRowsInserted( const QModelIndex &, int, int ) ) );
  connect( model(), 
           SIGNAL( dataChanged( const QModelIndex &, const QModelIndex & ) ),
           this, 
           SLOT( slotDataChanged( const QModelIndex &, const QModelIndex & ) ) );
}

AutoCheckTreeWidget::~AutoCheckTreeWidget()
{
  delete d;
}

QTreeWidgetItem* AutoCheckTreeWidget::itemByPath( const QStringList &path ) const
{
  QStringList newPath = path;
  QTreeWidgetItem *item = 0;

  while ( newPath.count() ) {
    item = findItem( item, newPath.takeFirst() );
    if ( !item ) return 0;
  }

  return item;
}

QStringList AutoCheckTreeWidget::pathByItem( QTreeWidgetItem *item ) const
{
  QStringList path;
  QTreeWidgetItem *current = item;

  while ( current ) {
    path.prepend( current->text( 0 ) );
    current = current->parent();
  }

  return path;
}

bool AutoCheckTreeWidget::autoCheckChildren() const
{
  return d->mAutoCheckChildren;
}

void AutoCheckTreeWidget::setAutoCheckChildren( bool autoCheckChildren )
{
  d->mAutoCheckChildren = autoCheckChildren;
}

bool AutoCheckTreeWidget::autoCheck() const
{
  return d->mAutoCheck;
}

void AutoCheckTreeWidget::setAutoCheck( bool autoCheck )
{
  d->mAutoCheck = autoCheck;
}

QTreeWidgetItem* AutoCheckTreeWidget::findItem( QTreeWidgetItem *parent, const QString &text ) const
{
  if ( parent ) {
    for ( int i = 0; i < parent->childCount(); ++i ) {
      if ( parent->child( i )->text( 0 ) == text ) {
        return parent->child( i );
      }
    }
  } else {
    for ( int i = 0; i < topLevelItemCount(); ++i ) {
      if ( topLevelItem( i )->text( 0 ) == text ) {
        return topLevelItem( i );
      }
    }
  }

  return 0;
}

void AutoCheckTreeWidget::slotRowsInserted( const QModelIndex &parent,
                                            int start, int end )
{
  if ( d->mAutoCheck ) {
    QTreeWidgetItem *item = itemFromIndex( parent );
    QTreeWidgetItem *child;
    if ( item ) {
      for ( int i = start; i < qMax( end, item->childCount() ); ++i ) {
        child = item->child( i );
        child->setFlags( child->flags() | Qt::ItemIsUserCheckable );
        child->setCheckState( 0, Qt::Unchecked );
      }
    } else { /* top level item has been inserted */
      for ( int i = start; i < qMax( end, topLevelItemCount() ); ++i ) {
        child = topLevelItem( i );
        child->setFlags( child->flags() | Qt::ItemIsUserCheckable );
        child->setCheckState( 0, Qt::Unchecked );
      }
    }
  }
}

void AutoCheckTreeWidget::slotDataChanged( const QModelIndex &topLeft,
                                           const QModelIndex &bottomRight )
{
  if ( !d->mAutoCheckChildren ) return;

  QTreeWidgetItem *item1 = itemFromIndex( topLeft );
  QTreeWidgetItem *item2 = itemFromIndex( bottomRight );

  if ( item1 == item2 ) {
    for ( int i = 0; i < item1->childCount(); ++i ) {
      item1->child( i )->setCheckState( 0, item1->checkState( 0 ) );
    }
  }
}

#include "autochecktreewidget.moc"
