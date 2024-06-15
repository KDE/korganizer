/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <EventViews/CalendarDecoration>

using namespace EventViews::CalendarDecoration;

class Datenums : public Decoration
{
public:
    Datenums(QObject *parent = nullptr, const QVariantList &args = {});

    void configure(QWidget *parent) override;

    Element::List createDayElements(const QDate &) override;
    Element::List createWeekElements(const QDate &) override;

    enum DayNumber {
        DayOfYear = 1,
        DaysRemaining = 2,
    };
    Q_DECLARE_FLAGS(DayNumbers, DayNumber)

    [[nodiscard]] QString info() const override;

private:
    DayNumbers mDisplayedInfo;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Datenums::DayNumbers)
