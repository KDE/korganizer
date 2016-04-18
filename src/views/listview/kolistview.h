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
                        QWidget *parent = Q_NULLPTR, bool nonInteractive = false);
    ~KOListView();

    int maxDatesHint() const Q_DECL_OVERRIDE;
    int currentDateCount() const Q_DECL_OVERRIDE;
    Akonadi::Item::List selectedIncidences() Q_DECL_OVERRIDE;
    KCalCore::DateList selectedIncidenceDates() Q_DECL_OVERRIDE;

    // Shows all incidences of the calendar
    void showAll();

    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);

    void clear();
    QSize sizeHint() const Q_DECL_OVERRIDE;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) Q_DECL_OVERRIDE;
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) Q_DECL_OVERRIDE;

    CalendarSupport::CalPrinterBase::PrintType printType() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void updateView() Q_DECL_OVERRIDE;
    virtual void showDates(const QDate &start, const QDate &end,
                           const QDate &preferredMonth = QDate()) Q_DECL_OVERRIDE;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) Q_DECL_OVERRIDE;

    void clearSelection() Q_DECL_OVERRIDE;

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) Q_DECL_OVERRIDE;

    void defaultItemAction(const QModelIndex &);
    void defaultItemAction(const Akonadi::Item::Id id);

    void popupMenu(const QPoint &);

private:
    KOEventPopupMenu *mPopupMenu;
    EventViews::ListView *mListView;
};

#endif
