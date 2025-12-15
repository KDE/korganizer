/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once
#include <QStringList>
class QColor;
class QString;
class QTime;
class QDate;
namespace KOrg
{
class CoreHelper
{
public:
    CoreHelper() = default;

    virtual ~CoreHelper() = default;

    virtual QColor categoryColor(const QStringList &cats) = 0;
    virtual QString holidayString(const QDate &dt) = 0;
    virtual QTime dayStart() = 0;
};
}
