/*
    This file is part of KOrganizer.
    Copyright (c) 1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _KOLISTVIEW_H
#define _KOLISTVIEW_H
// $Id$

#include <qlistview.h>

#include <libkcal/incidence.h>

#include "koeventview.h"

using namespace KCal;

/**
  This class provides a way of displaying a single Event in a QListView.
 
  @author Preston Brown <pbrown@kde.org>
  @see KOListView
*/
class KOListViewItem : public QListViewItem
{
  public:
    /**
      Constructor.
     
      @param parent is the list view to which this item belongs.
      @param ev is the event to have the item display information for.
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
  components using the Incidence::Visitor.
*/
class ListItemVisitor : public Incidence::Visitor
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
  This class provides a multi-column list view of events.  It can
  display events from one particular day or several days, it doesn't 
  matter.  To use a view that only handles one day at a time, use
  KODayListView.

  @short multi-column list view of various events.
  @author Preston Brown <pbrown@kde.org>
  @see KOBaseView, KODayListView
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
