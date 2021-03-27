/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
class QColor;
class QString;
class QTime;
class QStringList;
class QDate;
namespace KOrg
{
class CoreHelper
{
public:
    CoreHelper()
    {
    }

    virtual ~CoreHelper()
    {
    }

    virtual QColor categoryColor(const QStringList &cats) = 0;
    virtual QString holidayString(const QDate &dt) = 0;
    virtual QTime dayStart() = 0;
};
}
