#ifndef _BASE_LIST_VIEW_H
#define _BASE_LIST_VIEW_H

#include <ktablistbox.h>
#include <kapp.h>
#include <qlist.h>
#include <qpopmenu.h>

#include "qdatelist.h"
#include "calobject.h"
#include "koevent.h"

class BaseListView : public KTabListBox
{
  Q_OBJECT

public:
  BaseListView(CalObject *calendar, QWidget *parent=0, const char *name=0);
  virtual ~BaseListView();

  KOEvent *getSelected();
  enum { EVENTADDED, EVENTEDITED, EVENTDELETED };

public slots:
  virtual void updateView();
 /** used to re-read configuration when it has been modified by the
  *  options dialog. */
  virtual void updateConfig();
  void selectEvents(QList<KOEvent>);
  virtual void changeEventDisplay(KOEvent *, int);
  int getSelectedId();
  void showDates(bool show);
  inline void showDates();
  inline void hideDates();
  inline void setEventList(QList<KOEvent> el) { currEvents = el; updateView(); };

protected slots:
  void editEvent();
  void deleteEvent();

signals:
  void editEventSignal(KOEvent *);
  void deleteEventSignal(KOEvent *);
  void addEventSignal(KOEvent *);

protected:
  /* overloaded for drag support */
  bool prepareForDrag(int, int, char **, int *, int *);

  QStrList makeEntries();      // make a list of rows from currEvents.
  virtual QString makeDisplayStr(KOEvent *anEvent);
  CalObject *calendar;          // pointer to calendar object
  QList<int> currIds; // ID list for currEvents.
  QList<KOEvent> currEvents;  // list of current events
  QList<KOEvent> selectedEvents;
  QStrList entries;            // list of entries for current events
  QList<QRect> toolTipList;     // list of tooltip locations.
  bool timeAmPm;
};

#endif
