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

#ifndef KORG_VIEWS_KOLISTVIEW_H
#define KORG_VIEWS_KOLISTVIEW_H

#include "koeventview.h"

#include <KCalCore/Incidence> //for KCalCore::DateList typedef

namespace EventViews
{
class ListView;
}

namespace Akonadi
{
class IncidenceChanger;
}

class QModelIndex;

class KOListView : public KOEventView
{
    Q_OBJECT
public:
    explicit KOListView(const Akonadi::ETMCalendar::Ptr &calendar,
                        QWidget *parent = 0, bool nonInteractive = false);
    ~KOListView();

    virtual int maxDatesHint() const;
    virtual int currentDateCount() const;
    virtual Akonadi::Item::List selectedIncidences();
    virtual KCalCore::DateList selectedIncidenceDates();

    // Shows all incidences of the calendar
    void showAll();

    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);

    void clear();
    QSize sizeHint() const;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal);
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer);

    virtual CalendarSupport::CalPrinterBase::PrintType printType() const;

public slots:
    virtual void updateView();
    virtual void showDates(const QDate &start, const QDate &end,
                           const QDate &preferredMonth = QDate());
    virtual void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);

    void clearSelection();

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType);

    void defaultItemAction(const QModelIndex &);
    void defaultItemAction(const Akonadi::Item::Id id);

    void popupMenu(const QPoint &);

private:
    KOEventPopupMenu *mPopupMenu;
    EventViews::ListView *mListView;
};

#endif
