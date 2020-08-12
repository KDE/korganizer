/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Thomas Zander <zander@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

namespace Akonadi {
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

    Q_REQUIRED_RESULT bool correctName(const QString &newText);
    void filterSelected(KCalendarCore::CalFilter *f);

    QString mNegativeBackground;
    QList<KCalendarCore::CalFilter *> *mFilters = nullptr;
    KCalendarCore::CalFilter *mCurrent = nullptr;
    Akonadi::TagSelectionDialog *mCategorySelectDialog = nullptr;
};

#endif
