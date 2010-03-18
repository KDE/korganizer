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
#ifndef AUTOCHECKTREEWIDGET_H
#define AUTOCHECKTREEWIDGET_H

#include <QTreeWidget>

/**
  A tree widget which supports auto selecting child items, when clicking
  an item of the tree.

  Further you can find an item by passing a path as QStringList with
  the method itemByPath().

  @author Mathias Soeken <msoeken@tzi.de>
 */
class AutoCheckTreeWidget : public QTreeWidget {
  Q_OBJECT

  Q_PROPERTY( bool autoCheckChildren
              READ autoCheckChildren
              WRITE setAutoCheckChildren )

  Q_PROPERTY( bool autoCheck
              READ autoCheck
              WRITE setAutoCheck )

  public:
    /**
      Default constructor. The default behavior is like a QTreeWidget, so you
      have to activate the autoCheckChildren property manually.
     */
    AutoCheckTreeWidget( QWidget *parent = 0 );


    ~AutoCheckTreeWidget();

    /**
      Returns QTreeWidgetItem which matches the path, if available.

      @param path The path
      @returns a item which is represented by the path, if available.
     */
    QTreeWidgetItem *itemByPath( const QStringList &path ) const;

    /**
      Returns a path by a given item as QStringList.

      @param item The item
      @returns a string list which is the represented path of the item.
     */
    QStringList pathByItem( QTreeWidgetItem *item) const;

    /**
      @returns whether autoCheckChildren is enabled or not.
               Default value is false.
     */
    bool autoCheckChildren() const;

    /**
      enables or disables autoCheckChildren behavior.

      @param autoCheckChildren if true, children of items are auto checked or
                               not, otherwise. Default value is false.
     */
    void setAutoCheckChildren( bool autoCheckChildren );

    /**
      @returns whether newly added items have checkboxes by default.
               Default value is true.
     */
    bool autoCheck() const;

    /**
      Sets whether newly added items have checkboxes by default.

      @param autoCheck if true, newly added items have unchecked checkboxes
                       by default, otherwise not. Default value is true.
     */
    void setAutoCheck( bool autoCheck );

  protected:
    QTreeWidgetItem *findItem( QTreeWidgetItem *parent, const QString &text ) const;

  protected Q_SLOTS:
    void slotRowsInserted( const QModelIndex &parent, int start, int end );
    void slotDataChanged( const QModelIndex &topLeft,
                          const QModelIndex &bottomRight );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

#endif
