/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Jonathan Singer <jsinger@leeta.net>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDialog>

class QCheckBox;

/**
  @author Jonathan Singer
*/
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
    QCheckBox *mOmerBox = nullptr;
    QCheckBox *mParshaBox = nullptr;
    QCheckBox *mIsraelBox = nullptr;
    QCheckBox *mCholBox = nullptr;
};
