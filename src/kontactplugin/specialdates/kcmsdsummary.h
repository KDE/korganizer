/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004-2006, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "ui_sdsummaryconfig_base.h"
#include <KCModule>

class KCMSDSummary : public KCModule, public Ui::SDSummaryConfig_Base
{
    Q_OBJECT

public:
    explicit KCMSDSummary(QWidget *parent = nullptr, const QVariantList &args = {});

    void load() override;
    void save() override;
    void defaults() override;
    const KAboutData *aboutData() const override;

private Q_SLOTS:
    void modified();
    void buttonClicked(int id);
    void customDaysChanged(int value);
};

