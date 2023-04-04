/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include "kcmutils_version.h"
#include "kprefsdialog.h"

class KOPrefsDialogMain : public Korganizer::KPrefsModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogMain(QObject *parent, const KPluginMetaData &data, const QVariantList &args = {});

protected:
    void usrWriteConfig() override;

private:
    void toggleEmailSettings(bool on);
    QWidget *mUserEmailSettings = nullptr;
};
