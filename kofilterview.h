#ifndef KOFILTERVIEW_H
#define KOFILTERVIEW_H

#include "kofilterview_base.h"

class CalFilter;

class KOFilterView : public KOFilterView_base
{
    Q_OBJECT
  public:
    KOFilterView(CalFilter *,QWidget* parent=0,const char* name=0, WFlags fl=0);
    ~KOFilterView();

  public slots:
    void updateFilter();
    
  signals:
    void filterChanged();

  private:
    CalFilter *mFilter;
};

#endif // KOFILTERVIEW_H
