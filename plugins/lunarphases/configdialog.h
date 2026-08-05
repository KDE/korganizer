/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: Allen Winter <winter@kde.org>
  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDialog>

class QButtonGroup;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog() override;

protected:
    void load();
    void save();

protected Q_SLOTS:
    void slotOk();

private:
    QButtonGroup *const mLunarPhaseGroup;
};
