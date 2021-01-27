/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_KOCOREHELPER_H
#define KORG_KOCOREHELPER_H

#include "corehelper.h"
#include "kocore.h"
#include "koglobals.h"
#include "prefs/koprefs.h"

class KOCoreHelper : public KOrg::CoreHelper
{
public:
    KOCoreHelper()
    {
    }

    ~KOCoreHelper() override
    {
    }

    Q_REQUIRED_RESULT QColor categoryColor(const QStringList &cats) override;

    Q_REQUIRED_RESULT QString holidayString(const QDate &dt) override;

    Q_REQUIRED_RESULT QTime dayStart() override
    {
        return KOPrefs::instance()->mDayBegins.time();
    }
};

#endif
