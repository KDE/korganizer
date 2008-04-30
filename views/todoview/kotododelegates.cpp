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

#include "kcheckcombobox.h"
#include "kotodomodel.h"

#include <libkdepim/kdateedit.h>
#include <libkdepim/categoryhierarchyreader.h>

#include <kcal/calendar.h>
#include <kcal/calfilter.h>

#include <kcolorscheme.h>
#include <kcombobox.h>
#include <kdebug.h>

#include <QApplication>
#include <QSlider>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyleOptionProgressBar>
#include <QSize>
#include <QEvent>
#include <QPaintEvent>
#include <QTextDocument>
#include <QFont>
#include <QFontMetrics>

using namespace KCal;
using namespace KPIM;

// ---------------- COMPLETION DELEGATE --------------------------
// ---------------------------------------------------------------

KOTodoCompleteDelegate::KOTodoCompleteDelegate( QObject *parent )
 : QStyledItemDelegate( parent )
{
}

KOTodoCompleteDelegate::~KOTodoCompleteDelegate()
{
}

void KOTodoCompleteDelegate::paint( QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index ) const
{
  QStyle *style;

  QStyleOptionViewItemV4 opt = option;
  initStyleOption( &opt, index );

  style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawPrimitive( QStyle::PE_PanelItemViewItem, &opt, painter );

  // TODO QTreeView does not set State_Editing. Qt task id 205051
  // check if a newer version of Qt fixes this
  if ( !( opt.state & QStyle::State_Editing ) ) {
    QRect rect = opt.rect;

    rect.adjust( 4, 3, -6, -3 );

    QStyleOptionProgressBar pbOption;

    pbOption.palette = opt.palette;
    pbOption.state = opt.state;
    pbOption.direction = opt.direction;
    pbOption.fontMetrics = opt.fontMetrics;

    pbOption.rect = rect;
    pbOption.maximum = 100;
    pbOption.minimum = 0;
    pbOption.progress = index.data().toInt();
    pbOption.text = index.data().toString() + QString::fromAscii( "%" );
    pbOption.textAlignment = Qt::AlignCenter;
    pbOption.textVisible = true;

    style->drawControl( QStyle::CE_ProgressBar, &pbOption, painter );
  }
}

QSize KOTodoCompleteDelegate::sizeHint( const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  return QSize( 80, 20 );
}

QWidget *KOTodoCompleteDelegate::createEditor( QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  QSlider *slider = new QSlider( parent );

  slider->setRange( 0, 100 );
  slider->setOrientation( Qt::Horizontal );

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
  Q_UNUSED( option );
  Q_UNUSED( index );

  editor->setGeometry( option.rect );
}

// ---------------- PRIORITY DELEGATE ----------------------------
// ---------------------------------------------------------------

KOTodoPriorityDelegate::KOTodoPriorityDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
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
  QStyledItemDelegate::paint( painter, option, index );
}

QSize KOTodoPriorityDelegate::sizeHint( const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
  return QStyledItemDelegate::sizeHint( option, index );
}

QWidget *KOTodoPriorityDelegate::createEditor( QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  KComboBox *combo = new KComboBox( parent );

  combo->addItem( i18nc( "@action:inmenu Unspecified priority", "unspecified" ) );
  combo->addItem( i18nc( "@action:inmenu highest priority", "1 (highest)" ) );
  combo->addItem( i18nc( "@action:inmenu", "2" ) );
  combo->addItem( i18nc( "@action:inmenu", "3" ) );
  combo->addItem( i18nc( "@action:inmenu", "4" ) );
  combo->addItem( i18nc( "@action:inmenu medium priority", "5 (medium)" ) );
  combo->addItem( i18nc( "@action:inmenu", "6" ) );
  combo->addItem( i18nc( "@action:inmenu", "7" ) );
  combo->addItem( i18nc( "@action:inmenu", "8" ) );
  combo->addItem( i18nc( "@action:inmenu lowest priority", "9 (lowest)" ) );

  return combo;
}

void KOTodoPriorityDelegate::setEditorData( QWidget *editor,
                                            const QModelIndex &index ) const
{
  KComboBox *combo = static_cast<KComboBox *>( editor );

  combo->setCurrentIndex( index.data( Qt::EditRole ).toInt() );
}

void KOTodoPriorityDelegate::setModelData( QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index ) const
{
  KComboBox *combo = static_cast<KComboBox *>( editor );

  model->setData( index, combo->currentIndex() );
}

void KOTodoPriorityDelegate::updateEditorGeometry( QWidget *editor,
                                                   const QStyleOptionViewItem &option,
                                                   const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  editor->setGeometry( option.rect );
}

// ---------------- DUE DATE DELEGATE ----------------------------
// ---------------------------------------------------------------

KOTodoDueDateDelegate::KOTodoDueDateDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

KOTodoDueDateDelegate::~KOTodoDueDateDelegate()
{
}

QWidget *KOTodoDueDateDelegate::createEditor( QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  KDateEdit *dateEdit = new KDateEdit( parent );

  return dateEdit;
}

void KOTodoDueDateDelegate::setEditorData( QWidget *editor,
                                           const QModelIndex &index ) const
{
  KDateEdit *dateEdit = static_cast<KDateEdit *>( editor );

  dateEdit->setDate( index.data( Qt::EditRole ).toDate() );
}

void KOTodoDueDateDelegate::setModelData( QWidget *editor,
                                          QAbstractItemModel *model,
                                          const QModelIndex &index ) const
{
  KDateEdit *dateEdit = static_cast<KDateEdit *>( editor );

  model->setData( index, dateEdit->date() );
}

void KOTodoDueDateDelegate::updateEditorGeometry( QWidget *editor,
                                                  const QStyleOptionViewItem &option,
                                                  const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  editor->setGeometry( option.rect );
}

// ---------------- CATEGORIES DELEGATE --------------------------
// ---------------------------------------------------------------

KOTodoCategoriesDelegate::KOTodoCategoriesDelegate( Calendar *cal, QObject *parent )
  : QStyledItemDelegate( parent )
{
  setCalendar( cal );
}

KOTodoCategoriesDelegate::~KOTodoCategoriesDelegate()
{
}

QWidget *KOTodoCategoriesDelegate::createEditor( QWidget *parent,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  KCheckComboBox *combo = new KCheckComboBox( parent );
  QStringList categories;

  if ( mCalendar ) {
    CalFilter *filter = mCalendar->filter();
    if ( filter->criteria() & CalFilter::ShowCategories ) {
      categories = filter->categoryList();
      categories.sort();
    } else {
      categories = KOPrefs::instance()->mCustomCategories;
      QStringList filterCategories = filter->categoryList();
      categories.sort();
      filterCategories.sort();

      QStringList::Iterator it = categories.begin();
      QStringList::Iterator jt = filterCategories.begin();
      while ( it != categories.end() && jt != filterCategories.end() ) {
        if ( *it == *jt ) {
          it = categories.erase( it );
          jt++;
        } else if ( *it < *jt ) {
          it++;
        } else if ( *it > *jt ) {
          jt++;
        }
      }
    }
  }

  CategoryHierarchyReaderQComboBox( combo ).read( categories );
  // TODO test again with newer version of Qt, it it manages then to move
  // the popup together with the combobox.
  //combo->showPopup();
  return combo;
}

void KOTodoCategoriesDelegate::setEditorData( QWidget *editor,
                                              const QModelIndex &index ) const
{
  KCheckComboBox *combo = static_cast<KCheckComboBox *>( editor );

  combo->setCheckedItems( index.data( Qt::EditRole ).toStringList() );
}

void KOTodoCategoriesDelegate::setModelData( QWidget *editor,
                                             QAbstractItemModel *model,
                                             const QModelIndex &index ) const
{
  KCheckComboBox *combo = static_cast<KCheckComboBox *>( editor );

  model->setData( index, combo->checkedItems() );
}

void KOTodoCategoriesDelegate::updateEditorGeometry( QWidget *editor,
                                                     const QStyleOptionViewItem &option,
                                                     const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  editor->setGeometry( option.rect );
}

void KOTodoCategoriesDelegate::setCalendar( Calendar *cal )
{
  mCalendar = cal;
}

// ---------------- DESCRIPTION DELEGATE -------------------------
// ---------------------------------------------------------------

KOTodoDescriptionDelegate::KOTodoDescriptionDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

KOTodoDescriptionDelegate::~KOTodoDescriptionDelegate()
{
}

void KOTodoDescriptionDelegate::paint( QPainter *painter,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index ) const
{
  if ( index.data( KOTodoModel::IsRichDescriptionRole ).toBool() ) {
    QStyle *style;

    QStyleOptionViewItemV4 opt = option;
    initStyleOption( &opt, index );

    style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &opt, painter );

    painter->save();
    QTextDocument tmp;
    tmp.setHtml( index.data().toString() );

    painter->translate( opt.rect.topLeft() );
    QRect rect = opt.rect;
    rect.moveTo( 0, 0 );
    tmp.setTextWidth( rect.width() );
    tmp.drawContents( painter, rect );

    painter->restore();
  } else {
    QStyledItemDelegate::paint( painter, option, index );
  }
}

QSize KOTodoDescriptionDelegate::sizeHint( const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
  QSize ret;
  if ( index.data( KOTodoModel::IsRichDescriptionRole ).toBool() ) {
    QTextDocument tmp;
    tmp.setHtml( index.data().toString() );
    ret = tmp.size().toSize();
  } else {
    ret = QStyledItemDelegate::sizeHint( option, index );
  }
  // limit height to max. 2 lines
  // TODO add graphical hint when truncating! make configurable height?
  if ( ret.height() > option.fontMetrics.height() * 2 ) {
    ret.setHeight( option.fontMetrics.height() * 2 );
  }
  return ret;
}

#include "kotododelegates.moc"
