#ifndef _SEARCHDIALOG_H
#define _SEARCHDIALOG_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qregexp.h>

#include <kdialogbase.h>

#include "kolistview.h"
#include "calendar.h"

class KDateEdit;
class QCheckBox;

using namespace KCal;

class SearchDialog : public KDialogBase
{
    Q_OBJECT
  public:
    SearchDialog(Calendar *calendar);
    virtual ~SearchDialog();

    void updateView();

  public slots:
    void changeEventDisplay(Event *, int) { updateView(); } 

  protected slots:
    void doSearch();

  signals:
    void showEventSignal(Event *);
    void editEventSignal(Event *);
    void deleteEventSignal(Event *);

  private:
    void search(const QRegExp &);

    Calendar *mCalendar;
    
    QList<Event> mMatchedEvents;
    
    QLabel *searchLabel;
    QLineEdit *searchEdit;
    KOListView *listView;
    
    KDateEdit *mStartDate;
    KDateEdit *mEndDate;
    
    QCheckBox *mInclusiveCheck;
    QCheckBox *mSummaryCheck;
    QCheckBox *mDescriptionCheck;
    QCheckBox *mCategoryCheck;
};

#endif
