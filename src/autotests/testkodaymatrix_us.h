/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class KODayMatrixTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMatrixLimits();
};

