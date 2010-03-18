/*
  This file is part of libkdepim.

  Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#ifndef CATEGORYSELECTDIALOG_H
#define CATEGORYSELECTDIALOG_H

#include "korganizer_export.h"

#include <KDialog>

class CategoryConfig;
class AutoCheckTreeWidget;
class CategorySelectWidgetBase;

class CategorySelectWidget : public QWidget
{
  Q_OBJECT
  public:
    explicit CategorySelectWidget( CategoryConfig* config, QWidget *parent=0 );
    ~CategorySelectWidget();

    void setCategories( const QStringList &categoryList = QStringList() );
    void setCategoryList( const QStringList &categories );

    void setSelected( const QStringList &selList );
    QStringList selectedCategories() const;
    QStringList selectedCategories( QString &categoriesStr );

    void setAutoselectChildren( bool autoselectChildren );

    void hideButton();
    void hideHeader();

    AutoCheckTreeWidget *listView() const;

  public Q_SLOTS:
    void clear();

  Q_SIGNALS:
    void editCategories();

  private:
    QStringList mCategoryList;
    CategorySelectWidgetBase *mWidgets;
    CategoryConfig *mCategoryConfig;
};

class KORGANIZER_CORE_EXPORT CategorySelectDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit CategorySelectDialog( CategoryConfig *cfg, QWidget *parent = 0 );
    ~CategorySelectDialog();

    QStringList selectedCategories() const;
    void setCategoryList( const QStringList &categories );

    void setAutoselectChildren( bool autoselectChildren );
    void setSelected( const QStringList &selList );

  public Q_SLOTS:
    void slotOk();
    void slotApply();
    void updateCategoryConfig();

  Q_SIGNALS:
    void categoriesSelected( const QString & );
    void categoriesSelected( const QStringList & );
    void editCategories();

  private:
    CategorySelectWidget *mWidgets;

    class CategorySelectDialogPrivate;
    CategorySelectDialogPrivate *d;
};

#endif
