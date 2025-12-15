/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once
#include "kprefsdialog.h"
class KOPrefsDialogGroupScheduling : public Korganizer::KPrefsModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogGroupScheduling(QObject *parent, const KPluginMetaData &data);

protected:
    void usrWriteConfig() override; /* cppcheck-suppress uselessOverride */
    void usrReadConfig() override; /* cppcheck-suppress uselessOverride */
};
