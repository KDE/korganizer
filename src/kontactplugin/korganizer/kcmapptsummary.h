/*
  This file is part of Kontact.
  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once
#include "ui_apptsummaryconfig_base.h"
#include <KCModule>

class KCMApptSummary : public KCModule, public Ui::ApptSummaryConfig_Base
{
    Q_OBJECT

public:
    explicit KCMApptSummary(QObject *parent, const KPluginMetaData &data);

    void load() override;
    void save() override;
    void defaults() override;

private:
    void buttonClicked(QAbstractButton *button);
    void customDaysChanged(int value);
    void modified();
    QButtonGroup *const mDaysButtonGroup;
    QButtonGroup *const mShowButtonGroup;
};
