/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KORG_INTERFACES_CALENDARVIEWBASE_H
#define KORG_INTERFACES_CALENDARVIEWBASE_H

#include "baseview.h"
#include <Akonadi/Calendar/ETMCalendar>

namespace KOrg
{
/**
  @short interface for main calendar view widget
  @author Cornelius Schumacher
*/
class CalendarViewBase : public QWidget
{
public:
    explicit CalendarViewBase(QWidget *parent)
        : QWidget(parent)
    {
    }

    ~CalendarViewBase() override
    {
    }

    virtual Akonadi::ETMCalendar::Ptr calendar() const = 0;
    virtual Akonadi::IncidenceChanger *incidenceChanger() const = 0;

    virtual QDate startDate() = 0;
    virtual QDate endDate() = 0;

    virtual Akonadi::Item currentSelection() = 0;

    virtual void addView(KOrg::BaseView *) = 0;

    /** changes the view to be the currently selected view */
    virtual void showView(KOrg::BaseView *) = 0;

    virtual bool editIncidence(const Akonadi::Item &item, bool isCounter = false) = 0;

public Q_SLOTS:
    virtual void updateView() = 0;

Q_SIGNALS:
    virtual void newIncidenceChanger(Akonadi::IncidenceChanger *) = 0;
};
}

#endif
