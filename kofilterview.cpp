/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <libkcal/calfilter.h>

#include "kofilterview.h"
#include "kofilterview.moc"

KOFilterView::KOFilterView(QPtrList<CalFilter> *filterList,QWidget* parent,
                           const char* name,WFlags fl )
  : KOFilterView_base(parent,name,fl)
{
  mFilters = filterList;

  connect(mSelectionCombo,SIGNAL(activated(int)),SIGNAL(filterChanged()));
  connect(mEnabledCheck,SIGNAL(clicked()),SIGNAL(filterChanged()));
  connect(mEditButton,SIGNAL(clicked()),SIGNAL(editFilters()));
}

KOFilterView::~KOFilterView()
{
    // no need to delete child widgets, Qt does it all for us
}

bool KOFilterView::filtersEnabled()
{
  return mEnabledCheck->isChecked();
}

void KOFilterView::setFiltersEnabled(bool set)
{
  mEnabledCheck->setChecked(set);
  emit filterChanged();
}


void KOFilterView::updateFilters()
{
  mSelectionCombo->clear();

  CalFilter *filter = mFilters->first();
  while(filter) {
    mSelectionCombo->insertItem(filter->name());
    filter = mFilters->next();
  }
}

CalFilter *KOFilterView::selectedFilter()
{
  CalFilter *f = mFilters->at(mSelectionCombo->currentItem());
  return f;
}

void KOFilterView::setSelectedFilter(QString filterName)
{
  int filter_num = mSelectionCombo->count();
  int i;
  for (i=0;i<filter_num;i++) {
    if (mSelectionCombo->text(i)==filterName)
      mSelectionCombo->setCurrentItem(i);
  }
  emit filterChanged();
}

