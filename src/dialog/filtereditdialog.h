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

#include <QDialog>

class FilterEdit;
class QPushButton;
namespace KCalendarCore {
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
class FilterEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterEditDialog(QList<KCalendarCore::CalFilter *> *, QWidget *parent = nullptr);
    ~FilterEditDialog() override;

Q_SIGNALS:
    void filterChanged();
    void editCategories();

public Q_SLOTS:
    void updateFilterList();
    void setDialogConsistent(bool consistent);

private:
    FilterEdit *mFilterEdit = nullptr;
    QPushButton *mOkButton = nullptr;
    QPushButton *mApplyButton = nullptr;

protected Q_SLOTS:
    void slotApply();
    void slotOk();
};

class FilterEdit : public QWidget, Ui::FilterEdit_base
{
    Q_OBJECT
public:
    explicit FilterEdit(QList<KCalendarCore::CalFilter *> *filters, QWidget *parent);
    ~FilterEdit();

    void updateFilterList();
    void saveChanges();

Q_SIGNALS:
    void dataConsistent(bool);
    void filterChanged();
    void editCategories();

private:
    void filterSelected();
    void bNewPressed();
    void bDeletePressed();
    void updateSelectedName(const QString &);
    void updateCategorySelection();
    void editCategorySelection();
    void slotHelp();

    bool correctName(const QString &newText);
    void filterSelected(KCalendarCore::CalFilter *f);

    QString mNegativeBackground;
    QList<KCalendarCore::CalFilter *> *mFilters = nullptr;
    KCalendarCore::CalFilter *mCurrent = nullptr;
    KPIM::TagSelectionDialog *mCategorySelectDialog = nullptr;
};

#endif
