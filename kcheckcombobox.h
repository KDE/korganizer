/*
  This file is part of libkdepim.

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

#ifndef KCHECKCOMBOBOX_H
#define KCHECKCOMBOBOX_H

#include <KComboBox>
#include <QModelIndex>

class KCheckComboBox : public KComboBox
{
  Q_OBJECT

  Q_PROPERTY( QString separator READ separator WRITE setSeparator )
  Q_PROPERTY( QString defaultText READ defaultText WRITE setDefaultText )
  Q_PROPERTY( QStringList checkedItems READ checkedItems WRITE setCheckedItems )

  public:
    explicit KCheckComboBox( QWidget *parent = 0 );
    virtual ~KCheckComboBox();

    virtual void hidePopup();

    QString defaultText() const;
    void setDefaultText( const QString &text );

    Qt::CheckState itemCheckState( int index ) const;
    void setItemCheckState( int index, Qt::CheckState state );

    QString separator() const;
    void setSeparator( const QString &separator );

    QStringList checkedItems() const;

    virtual bool eventFilter( QObject *receiver, QEvent *event );

  public Q_SLOTS:
    void setCheckedItems( const QStringList &items );

  Q_SIGNALS:
    void checkedItemsChanged( const QStringList &items );

  protected:
    void keyPressEvent( QKeyEvent *event );
    void wheelEvent( QWheelEvent *event );

  private Q_SLOTS:
    void updateCheckedItems( const QModelIndex &topLeft = QModelIndex(),
                             const QModelIndex &bottomRight = QModelIndex() );
    void toggleCheckState( int pos );
    void toggleCheckState( const QModelIndex &index );

  private:
    QString mSeparator;
    QString mDefaultText;
    bool mIgnoreHide;
};

#endif // KCHECKCOMBOBOX_H
