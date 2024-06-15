/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Jonathan Singer <jsinger@leeta.net>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <EventViews/CalendarDecoration>

using namespace EventViews::CalendarDecoration;

class Hebrew : public Decoration
{
public:
    Hebrew(QObject *parent = nullptr, const QVariantList &args = {});

    void configure(QWidget *parent) override;

    [[nodiscard]] Element::List createDayElements(const QDate &) override;

    [[nodiscard]] QString info() const override;

private:
    bool showParsha, showChol, showOmer;
    bool areWeInIsrael;
};
