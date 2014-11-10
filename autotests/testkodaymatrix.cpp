/*
  This file is part of KOrganizer.

  Copyright (c) 2011 Sérgio Martins <iamsergio@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#include "testkodaymatrix.h"

#include "../kodaymatrix.h"

#include <qtest.h>
#include <KLocale>
QTEST_MAIN(KODayMatrixTest)

typedef QPair<QDate, QDate> DateRange;

void KODayMatrixTest::testMatrixLimits()
{
    QMap<QDate, DateRange> dates;
    KLocale::global()->setWeekStartDay(1);   // Monday
    dates.insert(QDate(2011, 1, 1), DateRange(QDate(2010, 12, 27), QDate(2011, 2, 6)));
    dates.insert(QDate(2011, 2, 1), DateRange(QDate(2011, 1, 31), QDate(2011, 3, 13)));
    dates.insert(QDate(2011, 3, 1), DateRange(QDate(2011, 2, 28), QDate(2011, 4, 10)));
    dates.insert(QDate(2011, 4, 1), DateRange(QDate(2011, 3, 28), QDate(2011, 5, 8)));
    dates.insert(QDate(2011, 5, 1), DateRange(QDate(2011, 4, 25), QDate(2011, 6, 5)));

    QMapIterator<QDate, DateRange> iterator(dates);
    while (iterator.hasNext()) {
        iterator.next();
        const DateRange range = KODayMatrix::matrixLimits(iterator.key());
        // qDebug() << "Expected is " << iterator.value() << " and got " << range;
        QVERIFY(range == iterator.value());
    }

    QMap<QDate, DateRange> dates2;
    KLocale::global()->setWeekStartDay(1);   // Monday
    KLocale::global()->setWeekStartDay(7);   // Sunday
    dates2.insert(QDate(2011, 1, 1), DateRange(QDate(2010, 12, 26), QDate(2011, 2, 5)));
    dates2.insert(QDate(2011, 2, 1), DateRange(QDate(2011, 1, 30), QDate(2011, 3, 12)));
    dates2.insert(QDate(2011, 3, 1), DateRange(QDate(2011, 2, 27), QDate(2011, 4, 9)));
    dates2.insert(QDate(2011, 4, 1), DateRange(QDate(2011, 3, 27), QDate(2011, 5, 7)));
    dates2.insert(QDate(2011, 5, 1), DateRange(QDate(2011, 4, 24), QDate(2011, 6, 4)));

    QMapIterator<QDate, DateRange> iterator2(dates2);
    while (iterator2.hasNext()) {
        iterator2.next();
        const DateRange range = KODayMatrix::matrixLimits(iterator2.key());
        // qDebug() << "Expected is " << iterator.value() << " and got " << range;
        QVERIFY(range == iterator2.value());
    }
}
