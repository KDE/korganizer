// $Id$

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include "calfilter.h"

#include "kofilterview.h"

/* 
 *  Constructs a KOFilterView which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
KOFilterView::KOFilterView(QList<CalFilter> *filterList,QWidget* parent,
                           const char* name,WFlags fl )
  : KOFilterView_base(parent,name,fl)
{
  mFilters = filterList;

  connect(mSelectionCombo,SIGNAL(activated(int)),SIGNAL(filterChanged()));
  connect(mEnabledCheck,SIGNAL(clicked()),SIGNAL(filterChanged()));
  connect(mEditButton,SIGNAL(clicked()),SIGNAL(editFilters()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
KOFilterView::~KOFilterView()
{
    // no need to delete child widgets, Qt does it all for us
}

bool KOFilterView::filtersEnabled()
{
  return mEnabledCheck->isChecked();
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

#include "kofilterview.moc"
