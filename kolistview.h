/*
 * KOListView provides a view of events in a list.
 * This file is part of the KOrganizer project.
 * (c) 1999 Preston Brown <pbrown@kde.org>
 */

#ifndef _KOLISTVIEW_H
#define _KOLISTVIEW_H

#include <qlistview.h>

#include "koeventview.h"
#include "incidencevisitor.h"

using namespace KCal;

/**
 * This class provides a way of displaying a single Event in a QListView.
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
  KOListViewItem(QListView *parent, Incidence *ev);
  virtual ~KOListViewItem() {}

  Incidence *event() { return mEvent; }

  QString key(int, bool) const;

  void setSortKey(int column,const QString &key);

private:
  Incidence *mEvent;

  QMap<int,QString> mKeyMap;
};

/**
  This class provides the initialisation of a KOListViewItem for calendar
  components using the IncidenceVisitor.
*/
class ListItemVisitor : public IncidenceVisitor
{
  public:
    ListItemVisitor(KOListViewItem *);
    ~ListItemVisitor();
    
    bool visit(Event *);
    bool visit(Todo *);
    bool visit(Journal *);

  private:
    KOListViewItem *mItem;
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
class KOListView : public KOEventView
{
    Q_OBJECT
  public:
    KOListView(Calendar *calendar, QWidget *parent = 0, 
	       const char *name = 0);
    ~KOListView();

    virtual int maxDatesHint();
    virtual int currentDateCount();
    virtual QPtrList<Incidence> getSelected();

    void showDates(bool show);

    virtual void printPreview(CalPrinter *calPrinter,
                              const QDate &, const QDate &);
  
  public slots:
    virtual void updateView();
    virtual void selectDates(const QDateList dateList);
    virtual void selectEvents(QPtrList<Event> eventList);

    void showDates();
    void hideDates();

    void changeEventDisplay(Event *, int);
  
    void defaultItemAction(QListViewItem *item);
    void popupMenu(QListViewItem *item,const QPoint &,int);

  protected slots:
    void processSelectionChange();

  protected:
    void addEvents(QPtrList<Event> eventList);
    void addTodos(QPtrList<Todo> eventList);
    void addIncidence(Incidence *);
    KOListViewItem *getItemForEvent(Event *event);

  private:
    QListView *mListView;
    KOEventPopupMenu *mPopupMenu;
    KOListViewItem *mActiveItem;
};

#endif
