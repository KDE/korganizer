/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) override;
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

    Q_REQUIRED_RESULT CalendarSupport::CalPrinterBase::PrintType printType() const override;

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
