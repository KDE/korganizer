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

#include <KCalCore/Incidence>

#include <QHash>
#include <QList>

using namespace KCalCore;

namespace CalendarSupport {
  class Calendar;
}


typedef CustomListViewItem<Akonadi::Item::Id> KOListViewItem;

class KOListView;
class QTreeWidget;

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
  explicit KOListView( CalendarSupport::Calendar *calendar,
                       QWidget *parent = 0,  bool nonInteractive = false );
    ~KOListView();

    virtual int maxDatesHint() const;
    virtual int currentDateCount() const;
    virtual Akonadi::Item::List selectedIncidences();
    virtual DateList selectedIncidenceDates();

    void showDates( bool show );

    // Shows all incidences of the calendar
    void showAll();

    void readSettings( KConfig *config );
    void writeSettings( KConfig *config );

    void clear();
    QSize sizeHint() const;
    virtual KOrg::CalPrinterBase::PrintType printType() const;

  public slots:
    virtual void updateView();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );

    void clearSelection();

    void showDates();
    void hideDates();

    void changeIncidenceDisplay( const Akonadi::Item &, int );

    void defaultItemAction( const QModelIndex & );
    void defaultItemAction( const Akonadi::Item::Id id );

    void popupMenu( const QPoint & );

  protected slots:
    void processSelectionChange();

  protected:
    void addIncidences( const Akonadi::Item::List &incidenceList, const QDate & );
    void addIncidence( const Akonadi::Item &, const QDate &date );

  private:
    KOListViewItem *getItemForIncidence( const Akonadi::Item & );
    KCalCore::Incidence::Ptr incidenceForId( Akonadi::Item::Id id ) const;

  private:
    class ListItemVisitor;
    QTreeWidget *mTreeWidget;
    KOEventPopupMenu *mPopupMenu;
    KOListViewItem *mActiveItem;
    QHash<Akonadi::Item::Id,Akonadi::Item> mItems;
    QHash<Akonadi::Item::Id, QDate> mDateList;
    QDate mStartDate;
    QDate mEndDate;
    DateList mSelectedDates;

    // if it's non interactive we disable context menu, and incidence editing
    bool mIsNonInteractive;
};

#endif
