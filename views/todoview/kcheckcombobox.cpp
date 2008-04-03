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

#include "kcheckcombobox.h"

#include <KDebug>

#include <QListView>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QKeyEvent>

KCheckComboBox::KCheckComboBox( QWidget* parent ) : QComboBox( parent )
{
  mSeparator = QLatin1String( "," );
  mIgnoreHide = false;

  connect( this, SIGNAL(activated(int)), this, SLOT(toggleCheckState(int)) );
  connect( model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
           this, SLOT(updateCheckedItems(const QModelIndex &, const QModelIndex &)) );

  // read-only contents
  setEditable( true );
  lineEdit()->setReadOnly( true );
  setInsertPolicy( QComboBox::NoInsert );

  view()->installEventFilter( this );
  view()->viewport()->installEventFilter( this );
  this->installEventFilter( this );

  updateCheckedItems();
}

KCheckComboBox::~KCheckComboBox()
{}

void KCheckComboBox::hidePopup()
{
  if (!mIgnoreHide) QComboBox::hidePopup();
  mIgnoreHide = false;
}

Qt::CheckState KCheckComboBox::itemCheckState( int index ) const
{
  return static_cast<Qt::CheckState>( itemData( index, Qt::CheckStateRole ).toInt() );
}

void KCheckComboBox::setItemCheckState( int index, Qt::CheckState state )
{
  setItemData( index, state, Qt::CheckStateRole );
}

QStringList KCheckComboBox::checkedItems() const
{
  QStringList items;
  if ( model() ) {
    QModelIndex index = model()->index( 0, modelColumn(), rootModelIndex() );
    QModelIndexList indexes = model()->match( index, Qt::CheckStateRole,
                                              Qt::Checked, -1, Qt::MatchExactly );
    foreach ( const QModelIndex& index, indexes ) {
      items += index.data().toString();
    }
  }
  return items;
}

void KCheckComboBox::setCheckedItems( const QStringList& items )
{
  for ( int r = 0; r < model()->rowCount( rootModelIndex() ); ++r ) {
    QModelIndex indx = model()->index( r, modelColumn(), rootModelIndex() );
    QString text = indx.data().toString();
    bool found = items.indexOf( text ) == -1 ? false : true;
    model()->setData( indx, found ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
  }
  updateCheckedItems();
}

QString KCheckComboBox::defaultText() const
{
  return mDefaultText;
}

void KCheckComboBox::setDefaultText( const QString& text )
{
  if ( mDefaultText != text ) {
    mDefaultText = text;
    updateCheckedItems();
  }
}

QString KCheckComboBox::separator() const
{
  return mSeparator;
}

void KCheckComboBox::setSeparator( const QString& separator )
{
  if ( mSeparator != separator ) {
    mSeparator = separator;
    updateCheckedItems();
  }
}

bool KCheckComboBox::eventFilter( QObject* receiver, QEvent* event )
{
  switch ( event->type() )
  {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );
      if ( receiver == this &&
           (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) ) {
        showPopup();
        return true;
      }
      break;
    }
    case QEvent::MouseButtonRelease:
      mIgnoreHide = receiver == view() || receiver == view()->viewport();
      break;
    case QEvent::Wheel:
      if ( receiver == this ) {
        // discard mouse wheel events on the combo box
        return true;
      }
      break;
    default:
      break;
  }
  return QComboBox::eventFilter( receiver, event );
}

void KCheckComboBox::updateCheckedItems( const QModelIndex &topLeft,
                                         const QModelIndex &bottomRight )
{
  Q_UNUSED( topLeft );
  Q_UNUSED( bottomRight );

  QStringList items = checkedItems();
  QString text;
  if ( items.isEmpty() ) {
    text = mDefaultText;
  } else {
    text = items.join( mSeparator );
  }

  if ( text != lineEdit()->text() ) {
    setEditText( text );
    emit checkedItemsChanged( items );
  }
}

void KCheckComboBox::toggleCheckState( int index )
{
  QVariant value = itemData( index, Qt::CheckStateRole );
  if ( value.isValid() ) {
    Qt::CheckState state = static_cast<Qt::CheckState>( value.toInt() );
    setItemData( index, state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
  }
}

#include "kcheckcombobox.moc"
