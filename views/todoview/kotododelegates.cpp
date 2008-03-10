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

#include "kotododelegates.h"
#include "koprefs.h"

#include <kcolorscheme.h>
#include <kdebug.h>

#include <QApplication>
#include <QSlider>
#include <QComboBox>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyleOptionProgressBar>
#include <QSize>

// ---------------- COMPLETION DELEGATE --------------------------
// ---------------------------------------------------------------

KOTodoCompleteDelegate::KOTodoCompleteDelegate( QObject *parent )
 : QItemDelegate( parent )
{
}

KOTodoCompleteDelegate::~KOTodoCompleteDelegate()
{
}

void KOTodoCompleteDelegate::paint( QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index ) const
{
  QRect rect = option.rect;

  if ( option.state & QStyle::State_Selected ) {
    painter->fillRect( rect, option.palette.highlight() );
  } else {
    painter->fillRect( rect, option.palette.base() );
  }

  rect.adjust( 4, 3, -6, -3 );

  QStyle *style = QApplication::style();
  QStyleOptionProgressBar pbOption;

  pbOption.palette = option.palette;
  pbOption.state = option.state;
  pbOption.direction = option.direction;
  pbOption.fontMetrics = option.fontMetrics;

  pbOption.rect = rect;
  pbOption.maximum = 100;
  pbOption.minimum = 0;
  pbOption.progress = index.data().toInt();
  pbOption.text = index.data().toString() + QString::fromAscii( "%" );
  pbOption.textAlignment = Qt::AlignCenter;
  pbOption.textVisible = true;

  style->drawControl( QStyle::CE_ProgressBar, &pbOption, painter );
}

QSize KOTodoCompleteDelegate::sizeHint( const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
  return QSize( 80, 20 );
}

QWidget *KOTodoCompleteDelegate::createEditor( QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index ) const
{
  QSlider *slider = new QSlider( parent );

  slider->setRange( 0, 100 );
  slider->setOrientation( Qt::Horizontal );

  QPalette palette = slider->palette();
  palette.setColor( QPalette::Base, palette.highlight().color() );
  slider->setPalette( palette );
  slider->setAutoFillBackground( true );

  return slider;
}

void KOTodoCompleteDelegate::setEditorData( QWidget *editor,
                                            const QModelIndex &index ) const
{
  QSlider *slider = static_cast<QSlider *>( editor );

  slider->setValue( index.data( Qt::EditRole ).toInt() );
}

void KOTodoCompleteDelegate::setModelData( QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index ) const
{
  QSlider *slider = static_cast<QSlider *>( editor );

  model->setData( index, slider->value() );
}

void KOTodoCompleteDelegate::updateEditorGeometry( QWidget *editor,
                                                   const QStyleOptionViewItem &option,
                                                   const QModelIndex &index ) const
{
  editor->setGeometry( option.rect );
}

// ---------------- PRIORITY DELEGATE ----------------------------
// ---------------------------------------------------------------

KOTodoPriorityDelegate::KOTodoPriorityDelegate( QObject *parent )
  : QItemDelegate( parent )
{
}

KOTodoPriorityDelegate::~KOTodoPriorityDelegate()
{
}

void KOTodoPriorityDelegate::paint( QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index ) const
{
  //TODO paint different priorities differently
  QItemDelegate::paint( painter, option, index );
}

QSize KOTodoPriorityDelegate::sizeHint( const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
  QItemDelegate::sizeHint( option, index );
}

QWidget *KOTodoPriorityDelegate::createEditor( QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index ) const
{
//TODO use a KComboBox???????????
  QComboBox *combo = new QComboBox( parent );

  combo->addItem( i18nc( "Unspecified priority", "unspecified" ) );
  combo->addItem( i18nc( "@action:inmenu highest priority", "1 (highest)" ) );
  combo->addItem( i18n( "2" ) );
  combo->addItem( i18n( "3" ) );
  combo->addItem( i18n( "4" ) );
  combo->addItem( i18nc( "@action:inmenu medium priority", "5 (medium)" ) );
  combo->addItem( i18n( "6" ) );
  combo->addItem( i18n( "7" ) );
  combo->addItem( i18n( "8" ) );
  combo->addItem( i18nc( "@action:inmenu lowest priority", "9 (lowest)" ) );

  return combo;
}

void KOTodoPriorityDelegate::setEditorData( QWidget *editor,
                                            const QModelIndex &index ) const
{
  QComboBox *combo = static_cast<QComboBox *>( editor );

  combo->setCurrentIndex( index.data( Qt::EditRole ).toInt() );
}

void KOTodoPriorityDelegate::setModelData( QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index ) const
{
  QComboBox *combo = static_cast<QComboBox *>( editor );

  model->setData( index, combo->currentIndex() );
}

void KOTodoPriorityDelegate::updateEditorGeometry( QWidget *editor,
                                                   const QStyleOptionViewItem &option,
                                                   const QModelIndex &index ) const
{
  editor->setGeometry( option.rect );
}

#include "kotododelegates.moc"
