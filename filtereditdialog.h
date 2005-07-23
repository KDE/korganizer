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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _FILTEREDITDIALOG_H
#define _FILTEREDITDIALOG_H

#include <kdialogbase.h>
#include "filteredit_base.h"

class FilterEdit;
template<class type> class QPtrList;

namespace KPIM { class CategorySelectDialog; }
namespace KCal { class CalFilter; }

using namespace KCal;

/**
  This is the class to add/edit a calendar filter.

  @short Creates a dialog box to create/edit a calendar filter
  @author Cornelius Schumacher, Thomas Zander
*/
class FilterEditDialog : public KDialogBase
{
    Q_OBJECT
  public:
    FilterEditDialog(QPtrList<CalFilter> *,QWidget *parent=0, const char *name=0);
    virtual ~FilterEditDialog();

  signals:
    void filterChanged();
    void editCategories();

  public slots:
    void updateFilterList();
    void updateCategoryConfig();
    void setDialogConsistent(bool consistent);

  private:
    FilterEdit *mFilterEdit;

  protected slots:
    void slotApply();
    void slotOk();
};

class FilterEdit : public FilterEdit_base
{
    Q_OBJECT
  public:
    FilterEdit(QPtrList<CalFilter> *filters, QWidget *parent);
    ~FilterEdit();

    void updateFilterList();
    void saveChanges();
    void updateCategoryConfig();

  signals:
    void dataConsistent(bool);
    void filterChanged();
    void editCategories();

  private slots:
    void filterSelected();
    void bNewPressed();
    void bDeletePressed();
    void updateSelectedName(const QString &);
    void updateCategorySelection(const QStringList &categories);
    void editCategorySelection();

  private:
    void filterSelected(CalFilter *f);

    QPtrList<CalFilter> *mFilters;
    CalFilter *current;
    KPIM::CategorySelectDialog *mCategorySelectDialog;
};

#endif
