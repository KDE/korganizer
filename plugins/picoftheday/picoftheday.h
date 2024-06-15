/*
  This file is part of KOrganizer.
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>
  SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <EventViews/CalendarDecoration>
using namespace EventViews::CalendarDecoration;

struct ElementData;

class Picoftheday : public Decoration
{
public:
    Picoftheday(QObject *parent = nullptr, const QVariantList &args = {});

    [[nodiscard]] Element::List createDayElements(const QDate &) override;

    void configure(QWidget *parent) override;

    [[nodiscard]] QString info() const override;

    static void cacheData(QDate date, ElementData *data);

private:
    QSize mThumbSize;
};
