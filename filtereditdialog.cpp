/*
    $Id$

     Requires the Qt and KDE widget libraries, available at no cost at
     http://www.trolltech.com and http://www.kde.org respectively

     Copyright (C) 2001 Cornelius Schumacher <schumacher@kde.org>

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

     -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

     This file implements a class for displaying a dialog box for
     adding or editing calendar filters.
*/

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qinputdialog.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <kdebug.h>
#include <klocale.h>

#include "koprefs.h"
#include "filteredit_base.h"
#include "categoryselectdialog.h"

#include "filtereditdialog.h"
#include "filtereditdialog.moc"

FilterEditDialog::FilterEditDialog(QList<CalFilter> *filters,QWidget *parent,
                                   const char *name) :
  KDialogBase(parent,name,false,i18n("Edit Calendar Filters"),
              Ok|Apply|Cancel)
{
  mFilters = filters;

  QWidget *mainWidget = new QWidget(this);
  setMainWidget(mainWidget);

  mSelectionCombo = new QComboBox(mainWidget);
  connect(mSelectionCombo,SIGNAL(activated(int)),SLOT(filterSelected()));  
  
  QPushButton *addButton = new QPushButton(i18n("Add Filter"),mainWidget);
  connect(addButton,SIGNAL(clicked()),SLOT(slotAdd()));
  
  mEditor = new FilterEdit_base(mainWidget);
  
  QGridLayout *topLayout = new QGridLayout(mainWidget,2,2);
  topLayout->setSpacing(spacingHint());
  topLayout->addWidget(mSelectionCombo,0,0);
  topLayout->addWidget(addButton,0,1);
  topLayout->addMultiCellWidget(mEditor,1,1,0,1);

  connect(mEditor->mCatEditButton,SIGNAL(clicked()),
          SLOT(editCategorySelection()));

  // Clicking cancel exits the dialog without saving
  connect(this,SIGNAL(cancelClicked()),SLOT(reject()));

  updateFilterList();
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::updateFilterList()
{
  mSelectionCombo->clear();

  CalFilter *filter = mFilters->first();

  if (!filter) {
    enableButtonOK(false);
    enableButtonApply(false);
  } else {
    while(filter) {
      mSelectionCombo->insertItem(filter->name());
      filter = mFilters->next();
    }
  
    CalFilter *f = mFilters->at(mSelectionCombo->currentItem());
    if (f) readFilter(f);
    
    enableButtonOK(true);
    enableButtonApply(true);
  }
}

void FilterEditDialog::slotDefault()
{
}

void FilterEditDialog::slotApply()
{
  CalFilter *f = mFilters->at(mSelectionCombo->currentItem());
  writeFilter(f);
  emit filterChanged();
}

void FilterEditDialog::slotOk()
{
  CalFilter *f = mFilters->at(mSelectionCombo->currentItem());
  writeFilter(f);
  emit filterChanged();
  accept();
}

void FilterEditDialog::slotAdd()
{
  QString filterName = QInputDialog::getText(i18n("Add Filter"), i18n("Enter Filter Name"));
  if (!filterName.isEmpty()) {
    mFilters->append(new CalFilter(filterName));
    updateFilterList();
  }
}

void FilterEditDialog::editCategorySelection()
{
  CategorySelectDialog *dlg = new CategorySelectDialog(this,"filterCatSelect",true);
  dlg->setSelected(mCategories);

  connect(dlg,SIGNAL(categoriesSelected(const QStringList &)),
          SLOT(updateCategorySelection(const QStringList &)));

  dlg->exec();
}

void FilterEditDialog::updateCategorySelection(const QStringList &categories)
{
  mCategories = categories;

  mEditor->mCatList->clear();
  mEditor->mCatList->insertStringList(mCategories);
}

void FilterEditDialog::filterSelected()
{
  CalFilter *f = mFilters->at(mSelectionCombo->currentItem());
  kdDebug() << "Selected filter " << f->name() << endl;
  if (f) readFilter(f);
}

void FilterEditDialog::readFilter(CalFilter *filter)
{
  int c = filter->criteria();
  
  mEditor->mCompletedCheck->setChecked(c & CalFilter::HideCompleted);
  mEditor->mRecurringCheck->setChecked(c & CalFilter::HideRecurring);

  if (c & CalFilter::ShowCategories) {
    mEditor->mCatShowCheck->setChecked(true);
  } else {
    mEditor->mCatHideCheck->setChecked(true);
  }
  
  mEditor->mCatList->insertStringList(filter->categoryList());
}

void FilterEditDialog::writeFilter(CalFilter *filter)
{
  int c = 0;

  if (mEditor->mCompletedCheck->isChecked()) c |= CalFilter::HideCompleted;
  if (mEditor->mRecurringCheck->isChecked()) c |= CalFilter::HideRecurring;
  if (mEditor->mCatShowCheck->isChecked()) c |= CalFilter::ShowCategories;

  filter->setCriteria(c);

  QStringList categoryList;
  for(uint i=0;i<mEditor->mCatList->count();++i) {
    categoryList.append(mEditor->mCatList->text(i));
  }
  filter->setCategoryList(categoryList);
}
