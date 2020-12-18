/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KOPREFSDIALOGGROUPWARESCHEDULING_H
#define KOPREFSDIALOGGROUPWARESCHEDULING_H
#include "kprefsdialog.h"
namespace Ui {
class KOGroupwarePrefsPage;
}

class KOPrefsDialogGroupwareScheduling : public Korganizer::KPrefsModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogGroupwareScheduling(QWidget *parent);
    ~KOPrefsDialogGroupwareScheduling() override;

protected:
    void usrWriteConfig() override;
    void usrReadConfig() override;

private:
    Ui::KOGroupwarePrefsPage *mGroupwarePage = nullptr;
};

#endif // KOPREFSDIALOGGROUPWARESCHEDULING_H
