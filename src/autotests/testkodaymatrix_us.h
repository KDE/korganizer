/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTKODAYMATRIX_H
#define TESTKODAYMATRIX_H

#include <QObject>

class KODayMatrixTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMatrixLimits();
};

#endif
