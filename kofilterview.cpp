// $Id$

#include <qcheckbox.h>

#include "calfilter.h"

#include "kofilterview.h"

/* 
 *  Constructs a KOFilterView which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
KOFilterView::KOFilterView(QWidget* parent,const char* name,
                           WFlags fl )
  : KOFilterView_base(parent,name,fl)
{
  mFilter = new CalFilter;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KOFilterView::~KOFilterView()
{
    // no need to delete child widgets, Qt does it all for us
}

void KOFilterView::updateFilter()
{
  if (!mEnabledCheck->isChecked()) {
    // Only emit filterChanged(), when enabled state has changed.
    if (mFilter->isEnabled()) {
      mFilter->setEnabled(false);
      emit filterChanged(mFilter);
    }
    // If filter is disabled just return and don't check the filter settings.
    return;
  } else {
    mFilter->setEnabled(true);
  }

  int inclusion = 0;
  if (mInRecurringCheck->isChecked()) inclusion |= CalFilter::Recurring;
  if (mInFloatingCheck->isChecked()) inclusion |= CalFilter::Floating;
  mFilter->setInclusionCriteria(inclusion);

  int exclusion = 0;
  if (mExRecurringCheck->isChecked()) exclusion |= CalFilter::Recurring;
  if (mExFloatingCheck->isChecked()) exclusion |= CalFilter::Floating;
  mFilter->setExclusionCriteria(exclusion);
  
  emit filterChanged(mFilter);
}

#include "kofilterview.moc"
