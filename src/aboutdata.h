/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef KORG_ABOUTDATA_H
#define KORG_ABOUTDATA_H

#include "korganizerprivate_export.h"
#include <KAboutData>

namespace KOrg {
class KORGANIZERPRIVATE_EXPORT AboutData : public KAboutData
{
public:
    AboutData();
};
}

#endif
