// $Id$
#ifndef CATEGORYEDITDIALOG_H
#define CATEGORYEDITDIALOG_H
#include "categoryeditdialog_base.h"

class CategoryEditDialog : public CategoryEditDialog_base
{ 
    Q_OBJECT
  public:
    CategoryEditDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CategoryEditDialog();

  public slots:
    void add();
    void remove();
    void modify();

    void slotOk();
    void slotApply();
    
  signals:
    void categoryConfigChanged();
};

#endif // CATEGORYEDITDIALOG_H
