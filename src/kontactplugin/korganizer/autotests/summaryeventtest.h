/* SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>

class SummaryEventTester : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_Multiday();

    void test_eventsForRange_data();
    void test_eventsForRange();
};
