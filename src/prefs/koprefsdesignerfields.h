/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include "kcmdesignerfields.h"

class KOPrefsDesignerFields : public KCMDesignerFields
{
public:
    explicit KOPrefsDesignerFields(QObject *parent, const KPluginMetaData &data);

protected:
    [[nodiscard]] QString localUiDir() override;
    [[nodiscard]] QString uiPath() override;
    void writeActivePages(const QStringList &) override;
    [[nodiscard]] QStringList readActivePages() override;
    [[nodiscard]] QString applicationName() override;
};
