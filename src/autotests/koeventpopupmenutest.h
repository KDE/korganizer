/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class KoEventPopupMenuTest : public QObject
{
    Q_OBJECT
public:
    explicit KoEventPopupMenuTest(QObject *parent = nullptr);

private Q_SLOTS:
    void createEventFromTodo();
    void createTodoFromEvent();
    void createEventFromEvent();
    void createTodoFromTodo();
    void defaultMenuEventVisible();
    void defaultMenuTodoVisible();
};
