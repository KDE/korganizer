/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once
#include "kprefsdialog.h"

class KItemIconCheckCombo;

class KOPrefsDialogViews : public Korganizer::KPrefsModule
{
public:
    explicit KOPrefsDialogViews(QObject *parent, const KPluginMetaData &data);

protected:
    void usrReadConfig() override;
    void usrWriteConfig() override;

private:
    KItemIconCheckCombo *const mMonthIconComboBox;
    KItemIconCheckCombo *const mAgendaIconComboBox;
};
