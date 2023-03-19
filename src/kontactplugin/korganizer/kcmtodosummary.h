/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include "kcmutils_version.h"
#include "ui_todosummaryconfig_base.h"
#include <KCModule>

class KCMTodoSummary : public KCModule, public Ui::TodoSummaryConfig_Base
{
    Q_OBJECT

public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KCMTodoSummary(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
#else
    explicit KCMTodoSummary(QObject *parent, const KPluginMetaData &data, const QVariantList &args = QVariantList());
#endif
    ~KCMTodoSummary() override;

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void modified();
    void customDaysChanged(int value);
};
