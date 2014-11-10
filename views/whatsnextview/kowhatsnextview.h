/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_VIEWS_KOWHATSNEXTVIEW_H
#define KORG_VIEWS_KOWHATSNEXTVIEW_H

#include "korganizer/baseview.h"
#include <calendarviews/whatsnext/whatsnextview.h>

/**
  This class provides a view of the next events and todos
*/
class KOWhatsNextView : public KOrg::BaseView
{
    Q_OBJECT
public:
    explicit KOWhatsNextView(QWidget *parent = 0);
    ~KOWhatsNextView();

    virtual int currentDateCount() const;
    virtual Akonadi::Item::List selectedIncidences()
    {
        return Akonadi::Item::List();
    }
    KCalCore::DateList selectedIncidenceDates()
    {
        return KCalCore::DateList();
    }

    bool supportsDateNavigation() const
    {
        return true;
    }
    virtual CalendarSupport::CalPrinterBase::PrintType printType() const;
    void setCalendar(const Akonadi::ETMCalendar::Ptr &);

public Q_SLOTS:
    virtual void updateView();
    virtual void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth);
    virtual void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType);

private:
    EventViews::WhatsNextView *mView;
};

#endif
