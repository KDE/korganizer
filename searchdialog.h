#ifndef _SEARCHDIALOG_H
#define _SEARCHDIALOG_H

#include <qlabel.h>
#include <qlined.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qregexp.h>

//#include "baselistview.h"
#include "kolistview.h"
#include "calobject.h"

class SearchDialog : public QDialog
{
  Q_OBJECT

public:
  SearchDialog(CalObject *_cal);
  virtual ~SearchDialog();
  void updateView();

public slots:
/** cancel is public so TopWidget can call it to close the dialog. */
  void cancel();
  void changeEventDisplay(KOEvent *, int) { updateView(); } 

protected slots:
  void doSearch();

signals:
    void closed(QWidget *);
    void editEventSignal(KOEvent *);
    void deleteEventSignal(KOEvent *);

private:
  void closeEvent(QCloseEvent *);

  CalObject *cal;
  QPushButton *searchButton;
  QLabel *searchLabel;
  QLineEdit *searchEdit;
  KOListView *listView;
};

#endif
