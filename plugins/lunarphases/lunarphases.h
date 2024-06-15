/*
  SPDX-FileCopyrightText: 2018 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <EventViews/CalendarDecoration>
#include <KHolidays/LunarPhase>
#include <QIcon>

using namespace EventViews::CalendarDecoration;

class Lunarphases : public Decoration
{
public:
    Lunarphases(QObject *parent = nullptr, const QVariantList &args = {});

    [[nodiscard]] Element::List createDayElements(const QDate &) override;

    [[nodiscard]] QString info() const override;
};

class LunarphasesElement : public Element
{
    Q_OBJECT
public:
    explicit LunarphasesElement(KHolidays::LunarPhase::Phase phase);

    [[nodiscard]] QString shortText() const override;
    [[nodiscard]] QString longText() const override;
    [[nodiscard]] QPixmap newPixmap(const QSize &size) override;

private:
    QString mName;
    QIcon mIcon;
};
