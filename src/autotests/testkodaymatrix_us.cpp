/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testkodaymatrix_us.h"

#include "../kodaymatrix.h"

#include <QTest>
QTEST_MAIN(KODayMatrixTestUS)

void KODayMatrixTestUS::testMatrixLimits()
{
    QMap<QDate, KODayMatrix::MatrixRange> dates2;
    QLocale::setDefault(QLocale(QStringLiteral("en_US"))); // week start on Sunday
    dates2.insert(QDate(2011, 1, 1), KODayMatrix::MatrixRange(QDate(2010, 12, 26), QDate(2011, 2, 5)));
    dates2.insert(QDate(2011, 2, 1), KODayMatrix::MatrixRange(QDate(2011, 1, 30), QDate(2011, 3, 12)));
    dates2.insert(QDate(2011, 3, 1), KODayMatrix::MatrixRange(QDate(2011, 2, 27), QDate(2011, 4, 9)));
    dates2.insert(QDate(2011, 4, 1), KODayMatrix::MatrixRange(QDate(2011, 3, 27), QDate(2011, 5, 7)));
    dates2.insert(QDate(2011, 5, 1), KODayMatrix::MatrixRange(QDate(2011, 4, 24), QDate(2011, 6, 4)));

    for (auto i = dates2.cbegin(), end = dates2.cend(); i != end; ++i) {
        const KODayMatrix::MatrixRange range = KODayMatrix::matrixLimits(i.key());
        // qCDebug(KORGANIZER_LOG) << "Expected is " << i.value() << " and got " << range;
        QCOMPARE(range, i.value());
    }
}

#include "moc_testkodaymatrix_us.cpp"
