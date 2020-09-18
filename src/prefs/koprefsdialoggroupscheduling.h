/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KOPREFSDIALOGGROUPSCHEDULING_H
#define KOPREFSDIALOGGROUPSCHEDULING_H

#include "kprefsdialog.h"
class QCheckBox;
class KOPrefsDialogGroupScheduling : public Korganizer::KPrefsModule
{
    Q_OBJECT
public:
    KOPrefsDialogGroupScheduling(QWidget *parent);

protected:
    void usrWriteConfig() override;
    void usrReadConfig() override;
};
#endif // KOPREFSDIALOGGROUPSCHEDULING_H
