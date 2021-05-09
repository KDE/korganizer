/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "kprefsdialog.h"

class KItemIconCheckCombo;

class KOPrefsDialogViews : public Korganizer::KPrefsModule
{
public:
    explicit KOPrefsDialogViews(QWidget *parent, const QVariantList &args = {});

protected:
    void usrReadConfig() override;

private:
    KItemIconCheckCombo *mMonthIconComboBox;
    KItemIconCheckCombo *mAgendaIconComboBox;
};

