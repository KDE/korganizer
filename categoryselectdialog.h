#ifndef CATEGORYSELECTDIALOG_H
#define CATEGORYSELECTDIALOG_H
// $Id$

#include "categoryselectdialog_base.h"

class CategorySelectDialog : public CategorySelectDialog_base
{ 
    Q_OBJECT
  public:
    CategorySelectDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CategorySelectDialog();

    void setCategories();
    void setSelected(const QStringList &selList);
    
  public slots:
    void slotOk();
    void slotApply();
    void clear();
    void updateCategoryConfig();
    
  signals:
    void categoriesSelected(const QString &);
    void categoriesSelected(const QStringList &);
    void editCategories();
};

#endif // CATEGORYSELECTDIALOG_H
