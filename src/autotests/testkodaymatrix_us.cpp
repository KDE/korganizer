/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testkodaymatrix_us.h"

#include "../kodaymatrix.h"

#include <QTest>
QTEST_MAIN(KODayMatrixTest)

typedef QPair<QDate, QDate> DateRange;

void KODayMatrixTest::testMatrixLimits()
{
    QMap<QDate, DateRange> dates2;
    QLocale::setDefault(QLocale(QStringLiteral("en_US"))); // week start on Sunday
    dates2.insert(QDate(2011, 1, 1), DateRange(QDate(2010, 12, 26), QDate(2011, 2, 5)));
    dates2.insert(QDate(2011, 2, 1), DateRange(QDate(2011, 1, 30), QDate(2011, 3, 12)));
    dates2.insert(QDate(2011, 3, 1), DateRange(QDate(2011, 2, 27), QDate(2011, 4, 9)));
    dates2.insert(QDate(2011, 4, 1), DateRange(QDate(2011, 3, 27), QDate(2011, 5, 7)));
    dates2.insert(QDate(2011, 5, 1), DateRange(QDate(2011, 4, 24), QDate(2011, 6, 4)));

    QMapIterator<QDate, DateRange> iterator2(dates2);
    while (iterator2.hasNext()) {
        iterator2.next();
        const DateRange range = KODayMatrix::matrixLimits(iterator2.key());
        // qCDebug(KORGANIZER_LOG) << "Expected is " << iterator2.value() << " and got " << range;
        QVERIFY(range == iterator2.value());
    }
}
