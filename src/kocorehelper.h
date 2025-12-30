/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "corehelper.h"
#include <CalendarSupport/KCalPrefs>

class KOCoreHelper : public KOrg::CoreHelper
{
public:
    KOCoreHelper() = default;

    ~KOCoreHelper() override = default;

    [[nodiscard]] QColor categoryColor(const QStringList &categories) override;

    [[nodiscard]] QString holidayString(const QDate &dt) override;

    [[nodiscard]] QTime dayStart() override
    {
        return CalendarSupport::KCalPrefs::instance()->mDayBegins.time();
    }
};
