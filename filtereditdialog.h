/*
  This file is part of KOrganizer.

  Copyright (c) 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2005 Thomas Zander <zander@kde.org>

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

#ifndef KORG_FILTEREDITDIALOG_H
#define KORG_FILTEREDITDIALOG_H

#include "ui_filteredit_base.h"

#include <KDialog>

class FilterEdit;

namespace KCalCore {
  class CalFilter;
}

namespace KPIM {
  class TagSelectionDialog;
}

/**
  This is the class to add/edit a calendar filter.

  @short Creates a dialog box to create/edit a calendar filter
  @author Cornelius Schumacher, Thomas Zander
*/
class FilterEditDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit FilterEditDialog( QList<KCalCore::CalFilter*> *, QWidget *parent=0 );
    virtual ~FilterEditDialog();

  signals:
    void filterChanged();
    void editCategories();

  public slots:
    void updateFilterList();
    void setDialogConsistent( bool consistent );

  private:
    FilterEdit *mFilterEdit;

  protected slots:
    void slotApply();
    void slotOk();
};

class FilterEdit : public QWidget, Ui::FilterEdit_base
{
  Q_OBJECT
  public:
    explicit FilterEdit( QList<KCalCore::CalFilter*> *filters, QWidget *parent );
    ~FilterEdit();

    void updateFilterList();
    void saveChanges();

  signals:
    void dataConsistent(bool);
    void filterChanged();
    void editCategories();

  private slots:
    void filterSelected();
    void bNewPressed();
    void bDeletePressed();
    void updateSelectedName( const QString & );
    void updateCategorySelection();
    void editCategorySelection();

  private:
    bool correctName(const QString &newText);
    void filterSelected( KCalCore::CalFilter *f );

    QString mNegativeBackground;
    QList<KCalCore::CalFilter*> *mFilters;
    KCalCore::CalFilter *mCurrent;
    KPIM::TagSelectionDialog *mCategorySelectDialog;
};

#endif
