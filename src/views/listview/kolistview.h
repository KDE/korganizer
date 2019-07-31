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

#include <KCalendarCore/Incidence> //for KCalendarCore::DateList typedef

namespace EventViews {
class ListView;
}

namespace Akonadi {
class IncidenceChanger;
}

class QModelIndex;

class KOListView : public KOEventView
{
    Q_OBJECT
public:
    explicit KOListView(const Akonadi::ETMCalendar::Ptr &calendar, QWidget *parent = nullptr, bool nonInteractive = false);
    ~KOListView() override;

    int maxDatesHint() const override;
    int currentDateCount() const override;
    Akonadi::Item::List selectedIncidences() override;
    KCalendarCore::DateList selectedIncidenceDates() override;

    // Shows all incidences of the calendar
    void showAll();

    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);

    void clear();
    QSize sizeHint() const override;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) override;
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

    CalendarSupport::CalPrinterBase::PrintType printType() const override;

public Q_SLOTS:
    void updateView() override;
    virtual void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void clearSelection() override;

    void changeIncidenceDisplay(const Akonadi::Item &,
                                Akonadi::IncidenceChanger::ChangeType) override;

    void defaultItemAction(const QModelIndex &);
    void defaultItemAction(const Akonadi::Item::Id id);

    void popupMenu(const QPoint &);

private:
    KOEventPopupMenu *mPopupMenu = nullptr;
    EventViews::ListView *mListView = nullptr;
};

#endif
