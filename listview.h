// 	$Id$	

#ifndef _LISTVIEW_H
#define _LISTVIEW_H

#include "qdatelist.h"
#include "baselistview.h"

class ListView : public BaseListView
{
  Q_OBJECT

public:
  ListView(CalObject *, QWidget *parent = 0, const char *name = 0);
  virtual ~ListView();

public slots:
  virtual void updateView();
  void selectDates(const QDateList);

protected slots:
  void doPopup(int, int);

signals:
  void datesSelected(const QDateList);

protected:
  QDate currDate;				     
  QPopupMenu *rmbMenu;
  virtual QString makeDisplayStr(KOEvent *anEvent);
};

#endif
