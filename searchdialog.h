#ifndef _SEARCHDIALOG_H
#define _SEARCHDIALOG_H
// $Id$

#include <qregexp.h>

#include <kdialogbase.h>

#include <libkcal/calendar.h>

#include "kolistview.h"

class KDateEdit;
class QCheckBox;
class KLineEdit;
class QLabel;

using namespace KCal;

class SearchDialog : public KDialogBase
{
    Q_OBJECT
  public:
    SearchDialog(Calendar *calendar,QWidget *parent=0);
    virtual ~SearchDialog();

    void updateView();

  public slots:
    void changeEventDisplay(Event *, int) { updateView(); } 
  
  protected slots:
    void doSearch();
  void searchTextChanged( const QString &_text );	
  signals:
    void showEventSignal(Event *);
    void editEventSignal(Event *);
    void deleteEventSignal(Event *);

  private:
    void search(const QRegExp &);

    Calendar *mCalendar;
    
    QPtrList<Event> mMatchedEvents;
    
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
