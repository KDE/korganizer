/*
  This file is part of Kontact.
  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KCMAPPTSUMMARY_H
#define KCMAPPTSUMMARY_H

#include "ui_apptsummaryconfig_base.h"
#include <KCModule>

extern "C" {
Q_DECL_EXPORT KCModule *create_apptsummary(QWidget *parent, const char *);
}

class KCMApptSummary : public KCModule, public Ui::ApptSummaryConfig_Base
{
    Q_OBJECT

public:
    explicit KCMApptSummary(QWidget *parent = nullptr);

    void load() override;
    void save() override;
    void defaults() override;
    const KAboutData *aboutData() const override;

private Q_SLOTS:
    void modified();
    void buttonClicked(QAbstractButton *button);
    void customDaysChanged(int value);

private:
    QButtonGroup *mDaysButtonGroup = nullptr;
    QButtonGroup *mShowButtonGroup = nullptr;
    QButtonGroup *mGroupwareButtonGroup = nullptr;
};

#endif
