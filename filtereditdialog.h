/*
    This file is part of KOrganizer.
    Copyright (c) 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _FILTEREDITDIALOG_H
#define _FILTEREDITDIALOG_H

#include <kdialogbase.h>

class QComboBox;
class QPushButton;
class FilterEdit_base;
template<class type> class QPtrList;

namespace KPIM { class CategorySelectDialog; }
namespace KCal { class CalFilter; }

using namespace KCal;

/**
  This is the class to add/edit a calendar filter.
  
  @short Creates a dialog box to create/edit a calendar filter
  @author Cornelius Schumacher
*/
class FilterEditDialog : public KDialogBase
{
    Q_OBJECT
  public:
    FilterEditDialog(QPtrList<CalFilter> *,QWidget *parent=0, const char *name=0);
    virtual ~FilterEditDialog();

  public slots:
    void updateFilterList();
    void updateCategoryConfig();

  signals:
    void filterChanged();
    void editCategories();

  protected slots:
    void slotDefault();
    void slotApply();
    void slotOk();

    void slotAdd();
    void slotRemove();
    
    void filterSelected();
    void editCategorySelection();
    void updateCategorySelection(const QStringList &categories);

  protected:
    void readFilter(CalFilter *);
    void writeFilter(CalFilter *);

  private:
    QPtrList<CalFilter> *mFilters;

    QComboBox *mSelectionCombo;
    QPushButton *mRemoveButton;
    FilterEdit_base *mEditor;

    QStringList mCategories;

    KPIM::CategorySelectDialog *mCategorySelectDialog;
};

#endif
