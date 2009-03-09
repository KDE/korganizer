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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KOLISTVIEW_H
#define KOLISTVIEW_H

#include "koeventview.h"
#include "customlistviewitem.h"

#include <KCal/Incidence>

#include <QHash>

using namespace KCal;

typedef CustomListViewItem<Incidence *> KOListViewItem;

class KOListView;

#if 0
class KOListViewToolTip : public QToolTip
{
  public:
    KOListViewToolTip ( QWidget *parent, K3ListView *lv );

  protected:
    void maybeTip( const QPoint &pos );

  private:
    K3ListView *eventlist;
};
#endif

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
    explicit KOListView( Calendar *calendar, QWidget *parent = 0 );
    ~KOListView();

    virtual int maxDatesHint();
    virtual int currentDateCount();
    virtual Incidence::List selectedIncidences();
    virtual DateList selectedDates();

    void showDates( bool show );

    void readSettings( KConfig *config );
    void writeSettings( KConfig *config );

    void clear();

  public slots:
    virtual void updateView();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Incidence::List &incidenceList );

    void clearSelection();

    void showDates();
    void hideDates();

    void changeIncidenceDisplay( Incidence *, int );

    void defaultItemAction( Q3ListViewItem *item );
    void popupMenu( Q3ListViewItem *item, const QPoint &, int );

  protected slots:
    void processSelectionChange();

  protected:
    void addIncidences( const Incidence::List & );
    void addIncidence( Incidence * );
    KOListViewItem *getItemForIncidence( Incidence *incidence );

  private:
    class ListItemVisitor;
    K3ListView *mListView;
    KOEventPopupMenu *mPopupMenu;
    KOListViewItem *mActiveItem;
    QHash<QString, Incidence *> mUidHash;
    DateList mSelectedDates;
};

#endif
