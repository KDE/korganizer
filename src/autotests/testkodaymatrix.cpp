/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testkodaymatrix.h"

#include "../kodaymatrix.h"

#include <QTest>
QTEST_MAIN(KODayMatrixTest)

void KODayMatrixTest::testMatrixLimits()
{
    QMap<QDate, KODayMatrix::MatrixRange> dates;
    QLocale::setDefault(QLocale(QStringLiteral("de_DE"))); // week start on Monday
    dates.insert(QDate(2011, 1, 1), KODayMatrix::MatrixRange(QDate(2010, 12, 27), QDate(2011, 2, 6)));
    dates.insert(QDate(2011, 2, 1), KODayMatrix::MatrixRange(QDate(2011, 1, 31), QDate(2011, 3, 13)));
    dates.insert(QDate(2011, 3, 1), KODayMatrix::MatrixRange(QDate(2011, 2, 28), QDate(2011, 4, 10)));
    dates.insert(QDate(2011, 4, 1), KODayMatrix::MatrixRange(QDate(2011, 3, 28), QDate(2011, 5, 8)));
    dates.insert(QDate(2011, 5, 1), KODayMatrix::MatrixRange(QDate(2011, 4, 25), QDate(2011, 6, 5)));

    QMapIterator<QDate, KODayMatrix::MatrixRange> iterator(dates);
    while (iterator.hasNext()) {
        iterator.next();
        const KODayMatrix::MatrixRange range = KODayMatrix::matrixLimits(iterator.key());
        // qCDebug(KORGANIZER_LOG) << "Expected is " << iterator.value() << " and got " << range;
        QCOMPARE(range, iterator.value());
    }
}

#include "moc_testkodaymatrix.cpp"
