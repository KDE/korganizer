#ifndef _FILTEREDITDIALOG_H
#define _FILTEREDITDIALOG_H
// 	$Id$	

#include <qptrlist.h>

#include <kdialogbase.h>

#include <calfilter.h>

class QComboBox;
class FilterEdit_base;

using namespace KCal;

/**
  * This is the class to add/edit a calendar filter.
  *
  * @short Creates a dialog box to create/edit a calendar filter
  * @author Cornelius Schumacher
  * @version $Revision$
  */
class FilterEditDialog : public KDialogBase
{
    Q_OBJECT
  public:
    FilterEditDialog(QPtrList<CalFilter> *,QWidget *parent=0, const char *name=0);
    virtual ~FilterEditDialog();

  public slots:
    void updateFilterList();

  signals:
    void filterChanged();

  protected slots:
    void slotDefault();
    void slotApply();
    void slotOk();

    void slotAdd();
    void filterSelected();
    void editCategorySelection();
    void updateCategorySelection(const QStringList &categories);

  protected:
    void readFilter(CalFilter *);
    void writeFilter(CalFilter *);

  private:
    QPtrList<CalFilter> *mFilters;

    QComboBox *mSelectionCombo;
    FilterEdit_base *mEditor;
    
    QStringList mCategories;
};

#endif
