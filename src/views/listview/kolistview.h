/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "koeventview.h"

#include <KCalendarCore/Incidence> //for KCalendarCore::DateList typedef

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
    explicit KOListView(QWidget *parent = nullptr, bool nonInteractive = false);
    ~KOListView() override;

    Q_REQUIRED_RESULT int maxDatesHint() const override;
    Q_REQUIRED_RESULT int currentDateCount() const override;
    Q_REQUIRED_RESULT Akonadi::Item::List selectedIncidences() override;
    Q_REQUIRED_RESULT KCalendarCore::DateList selectedIncidenceDates() override;

    // Shows all incidences of the calendar
    void showAll();

    void readSettings(KConfig *config);
    void writeSettings(KConfig *config);

    void clear();
    QSize sizeHint() const override;

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

    Q_REQUIRED_RESULT CalendarSupport::CalPrinterBase::PrintType printType() const override;

public Q_SLOTS:
    void updateView() override;
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void clearSelection() override;

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) override;

    void defaultItemAction(const QModelIndex &);
    void defaultItemAction(const Akonadi::Item::Id id);

    void popupMenu(const QPoint &);

    void calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar) override;
    void calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar) override;

private:
    KOEventPopupMenu *mPopupMenu = nullptr;
    EventViews::ListView *mListView = nullptr;
};
