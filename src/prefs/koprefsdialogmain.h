/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KOPREFSDIALOGMAIN_H
#define KOPREFSDIALOGMAIN_H

#include <KCModule>
class QCheckBox;
class QLineEdit;

class KOPrefsDialogMain : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogMain(QWidget *parent);

    void save() override;
    void load() override;

protected Q_SLOTS:
    void toggleEmailSettings(bool on);
private:
    void slotConfigChanged();
    QWidget *mUserEmailSettings = nullptr;
    QCheckBox *mEmailControlCenterCheckBox = nullptr;
    QLineEdit *mUserName = nullptr;
    QLineEdit *mUserEmail = nullptr;
    QCheckBox *mConfirmCheckBox = nullptr;
    QCheckBox *mDestinationCheckBox = nullptr;
    QCheckBox *mShowReminderDaemonCheckBox = nullptr;
};

#endif // KOPREFSDIALOGMAIN_H
