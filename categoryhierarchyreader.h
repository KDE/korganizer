/*
    This file is part of libkdepim.

    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
#ifndef CATEGORYHIERARCHYREADER_H
#define CATEGORYHIERARCHYREADER_H

#include "korganizer_export.h"

class QComboBox;
class QStringList;
class QString;
class QTreeWidget;
class QTreeWidgetItem;

class KORGANIZER_CORE_EXPORT CategoryHierarchyReader
{
  public:
    void read( QStringList categories );
    virtual ~CategoryHierarchyReader() { }
    static QStringList path( QString string );
  protected:
    CategoryHierarchyReader() { }
    virtual void clear() = 0;
    virtual void goUp() = 0;
    virtual void addChild( const QString &label ) = 0;
    virtual int depth() const = 0;
};

class KORGANIZER_CORE_EXPORT CategoryHierarchyReaderQComboBox : public CategoryHierarchyReader
{
  public:
    CategoryHierarchyReaderQComboBox( QComboBox *box )
        : mBox( box ), mCurrentDepth( 0 ) { }
    virtual ~CategoryHierarchyReaderQComboBox() { }

  protected:
    virtual void clear();
    virtual void goUp();
    virtual void addChild( const QString &label );
    virtual int depth() const;
  private:
    QComboBox *mBox;
    int mCurrentDepth;
};

class CategoryHierarchyReaderQTreeWidget : public CategoryHierarchyReader
{
  public:
    CategoryHierarchyReaderQTreeWidget( QTreeWidget *tree )
  : mTree( tree ), mItem( 0 ), mCurrentDepth( 0 ) {}
    virtual ~CategoryHierarchyReaderQTreeWidget() {}

  protected:
    virtual void clear();
    virtual void goUp();
    virtual void addChild( const QString &label );
    virtual int depth() const;

  private:
    QTreeWidget *mTree;
    QTreeWidgetItem *mItem;
    int mCurrentDepth;
};

#endif
