/*
  SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "activitiesmanagertest.h"
#include "activities/activitiesmanager.h"
#include <QTest>
QTEST_MAIN(ActivitiesManagerTest)
ActivitiesManagerTest::ActivitiesManagerTest(QObject *parent)
    : QObject{parent}
{
}

void ActivitiesManagerTest::shouldHaveDefaultValues()
{
    const ActivitiesManager w;
    QVERIFY(!w.enabled());
    QVERIFY(w.accountActivities());
}

#include "moc_activitiesmanagertest.cpp"
