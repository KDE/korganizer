/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include "kprefsdialog.h"
class QSpinBox;
class KComboBox;
class QCheckBox;
namespace KPIM
{
class KCheckComboBox;
}

class KOPrefsDialogTime : public Korganizer::KPrefsModule
{
public:
    explicit KOPrefsDialogTime(QObject *parent, const KPluginMetaData &data);

protected:
    void usrReadConfig() override;

    void usrWriteConfig() override;

private:
    QStringList tzonenames;
    KPIM::KCheckComboBox *mHolidayCheckCombo = nullptr;
    QSpinBox *mReminderTimeSpin = nullptr;
    KComboBox *mReminderUnitsCombo = nullptr;
    QCheckBox *mWorkDays[7];
};
