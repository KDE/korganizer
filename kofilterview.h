#ifndef KOFILTERVIEW_H
#define KOFILTERVIEW_H

#include "kofilterview_base.h"

#include <calfilter.h>

using namespace KCal;

class KOFilterView : public KOFilterView_base
{
    Q_OBJECT
  public:
    KOFilterView(QPtrList<CalFilter> *filterList,QWidget* parent=0,const char* name=0, WFlags fl=0);
    ~KOFilterView();

    void updateFilters();

    bool filtersEnabled();
    CalFilter *selectedFilter();

  signals:
    void filterChanged();
    void editFilters();
    
  private:
    QPtrList<CalFilter> *mFilters;
};

#endif // KOFILTERVIEW_H
