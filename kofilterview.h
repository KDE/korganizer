#ifndef KOFILTERVIEW_H
#define KOFILTERVIEW_H

#include "kofilterview_base.h"

class CalFilter;

class KOFilterView : public KOFilterView_base
{
    Q_OBJECT
  public:
    KOFilterView(QList<CalFilter> *filterList,QWidget* parent=0,const char* name=0, WFlags fl=0);
    ~KOFilterView();

    void updateFilters();

    bool filtersEnabled();
    CalFilter *selectedFilter();

  signals:
    void filterChanged();
    void editFilters();
    
  private:
    QList<CalFilter> *mFilters;
};

#endif // KOFILTERVIEW_H
