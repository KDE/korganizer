/*
    This file is part of KOrganizer.

    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOLISTVIEW_H
#define _KOLISTVIEW_H

#include <qdict.h>
#include <qtooltip.h>

#include <libkcal/incidence.h>

#include "koeventview.h"
#include "customlistviewitem.h"

using namespace KCal;

typedef CustomListViewItem<Incidence *> KOListViewItem;

class KOListView;

class KOListViewToolTip : public QToolTip
{
  public:
    KOListViewToolTip (QWidget* parent, KListView* lv );

  protected:
    void maybeTip( const QPoint & pos);

  private:
    KListView* eventlist;
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
    virtual Incidence::List selectedIncidences();
    virtual DateList selectedDates();

    void showDates(bool show);

    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);

    void clear();

  public slots:
    virtual void updateView();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Incidence::List &incidenceList );

    void clearSelection();

    void showDates();
    void hideDates();

    void changeIncidenceDisplay(Incidence *, int);

    void defaultItemAction(QListViewItem *item);
    void popupMenu(QListViewItem *item,const QPoint &,int);

  protected slots:
    void processSelectionChange();

  protected:
    void addIncidences( const Incidence::List & );
    void addIncidence(Incidence *);
    KOListViewItem *getItemForIncidence(Incidence *incidence);

  private:
    class ListItemVisitor;
    KListView *mListView;
    KOEventPopupMenu *mPopupMenu;
    KOListViewItem *mActiveItem;
    QDict<Incidence> mUidDict;
    DateList mSelectedDates;
};

#endif
