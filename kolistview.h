/*
 * KOListView provides a view of events in a list.
 * This file is part of the KOrganizer project.
 * (c) 1999 Preston Brown <pbrown@kde.org>
 */

#ifndef _KOLISTVIEW_H
#define _KOLISTVIEW_H

#include <qlistview.h>

#include "kobaseview.h"

/**
 * This class provides a way of displaying a single KOEvent in a QListView.
 *
 * @author Preston Brown <pbrown@kde.org>
 * @see KOListView
 */
class KOListViewItem : public QListViewItem
{
public:
  /**
   * Constructor.
   *
   * @param parent is the list view to which this item belongs.
   * @param ev is the event to have the item display information for.
   */
  KOListViewItem(QListView *parent, KOEvent *ev);
  virtual ~KOListViewItem() {}

  KOEvent *event() { return mEvent; }

private:
  KOEvent *mEvent;
};

/**
 * This class provides a multi-column list view of events.  It can
 * display events from one particular day or several days, it doesn't 
 * matter.  To use a view that only handles one day at a time, use
 * KODayListView.
 *
 * @short multi-column list view of various events.
 * @author Preston Brown <pbrown@kde.org>
 * @see KOBaseView, KODayListView
 */
class KOListView : public KOBaseView
{
  Q_OBJECT

public:
  KOListView(CalObject *calendar, QWidget *parent = 0, 
	     const char *name = 0);
  ~KOListView();

  virtual int maxDatesHint();
  virtual QList<KOEvent> getSelected();

  void showDates(bool show);

  virtual void printPreview(CalPrinter *calPrinter,
                            const QDate &, const QDate &);
  
public slots:
  virtual void updateView();
  virtual void selectDates(const QDateList dateList);
  virtual void selectEvents(QList<KOEvent> eventList);

  void showDates();
  void hideDates();

  void changeEventDisplay(KOEvent *, int);
  
  void defaultItemAction(QListViewItem *item);
  void popupMenu(QListViewItem *item,const QPoint &,int);

protected:
  void addEvents(QList<KOEvent> eventList);
  KOListViewItem *getItemForEvent(KOEvent *event);

  QListView *mListView;
  QPopupMenu *mPopupMenu;
  KOListViewItem *mActiveItem;
};

#endif
