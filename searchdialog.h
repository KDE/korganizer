#ifndef _SEARCHDIALOG_H
#define _SEARCHDIALOG_H

#include <qlabel.h>
#include <qlineedit.h>
#include <qregexp.h>

#include <kdialogbase.h>

#include "kolistview.h"
#include "calobject.h"

class KDateEdit;

class SearchDialog : public KDialogBase
{
    Q_OBJECT
  public:
    SearchDialog(CalObject *calendar);
    virtual ~SearchDialog();

    void updateView();

  public slots:
    void changeEventDisplay(KOEvent *, int) { updateView(); } 

  protected slots:
    void doSearch();

  signals:
    void showEventSignal(KOEvent *);
    void editEventSignal(KOEvent *);
    void deleteEventSignal(KOEvent *);

  private:
    void search(const QRegExp &);

    CalObject *mCalendar;
    
    QList<KOEvent> mMatchedEvents;
    
    QPushButton *searchButton;
    QLabel *searchLabel;
    QLineEdit *searchEdit;
    KOListView *listView;
    
    KDateEdit *mStartDate;
    KDateEdit *mEndDate;
};

#endif
