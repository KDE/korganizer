#ifndef _FILTEREDITDIALOG_H
#define _FILTEREDITDIALOG_H
// 	$Id$	

#include <qlist.h>

#include <kdialogbase.h>

#include "calfilter.h"

class QComboBox;

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
    FilterEditDialog(QList<CalFilter> *,QWidget *parent=0, const char *name=0);
    virtual ~FilterEditDialog();

  public slots:
    void updateFilterList();

  signals:

  protected slots:
    void slotDefault();
    void slotApply();
    void slotOk();

  protected:

  private:
    QList<CalFilter> *mFilters;

    QComboBox *mSelectionCombo;
};

#endif
