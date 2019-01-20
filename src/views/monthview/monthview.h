/*
  This file is part of KOrganizer.

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins <sergio.martins@kdab.com>

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

#ifndef KORG_VIEWS_MONTHVIEW_H
#define KORG_VIEWS_MONTHVIEW_H

#include "koeventview.h"

namespace EventViews {
class MonthView;
}
namespace KOrg {
class MonthView : public KOEventView
{
    Q_OBJECT
public:
    explicit MonthView(QWidget *parent = nullptr);
    ~MonthView() override;

    int currentDateCount() const override;
    int currentMonth() const;

    Akonadi::Item::List selectedIncidences() override;

    /** Returns dates of the currently selected events */
    KCalCore::DateList selectedIncidenceDates() override;

    QDateTime selectionStart() override;

    QDateTime selectionEnd() override;
    bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;

    /**
     * Returns the average date in the view
     */
    QDate averageDate() const;

    bool usesFullWindow() override;

    bool supportsDateRangeSelection() override;

    CalendarSupport::CalPrinterBase::PrintType printType() const override;

    int maxDatesHint() const override;

    void setTypeAheadReceiver(QObject *o) override;

    void setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate()) override;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) override;

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

public Q_SLOTS:
    void updateView() override;

    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void changeIncidenceDisplay(const Akonadi::Item &,
                                Akonadi::IncidenceChanger::ChangeType) override;

    void updateConfig() override;

Q_SIGNALS:
    void fullViewChanged(bool enabled);

private:
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;

    EventViews::MonthView *mMonthView = nullptr;
    KOEventPopupMenu *mPopup = nullptr;
};
}
#endif
