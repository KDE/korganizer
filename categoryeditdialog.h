/*
  This file is part of libkdepim.

  Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
#ifndef CATEGORYEDITDIALOG_H
#define CATEGORYEDITDIALOG_H

#include "korganizer_export.h"

#include <KDialog>

class QTreeWidgetItem;

namespace Ui {
  class CategoryEditDialog_base;
}

namespace IncidenceEditors {
  class CategoryConfig;
}

class KORGANIZER_CORE_EXPORT CategoryEditDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit CategoryEditDialog( IncidenceEditors::CategoryConfig *categoryConfig, QWidget *parent = 0 );
    ~CategoryEditDialog();

  public Q_SLOTS:
    void reload();
    virtual void show();

  protected Q_SLOTS:
    void slotOk();
    void slotApply();
    void slotCancel();
    void slotTextChanged( const QString &text );
    void slotSelectionChanged();
    void add();
    void addSubcategory();
    void remove();
    void editItem( QTreeWidgetItem *item );
    void expandIfToplevel( QTreeWidgetItem *item );

  Q_SIGNALS:
    void categoryConfigChanged();

  protected:
    void fillList();

  private:
    void deleteItem( QTreeWidgetItem *item, QList<QTreeWidgetItem *> &to_remove );
    IncidenceEditors::CategoryConfig *mCategoryConfig;
    Ui::CategoryEditDialog_base *mWidgets;
};

#endif
