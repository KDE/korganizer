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
    explicit KOPrefsDesignerFields(QObject *parent, const KPluginMetaData &data, const QVariantList &args = QVariantList());

protected:
    Q_REQUIRED_RESULT QString localUiDir() override;
    Q_REQUIRED_RESULT QString uiPath() override;
    void writeActivePages(const QStringList &) override;
    Q_REQUIRED_RESULT QStringList readActivePages() override;
    Q_REQUIRED_RESULT QString applicationName() override;
};
