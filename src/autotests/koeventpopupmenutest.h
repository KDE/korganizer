/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef KOEVENTPOPUPMENUTESTTEST_H
#define KOEVENTPOPUPMENUTESTTEST_H

#include <AkonadiCore/Collection>
#include <QObject>

namespace Akonadi {
class Collection;
}

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
    void createNoteFromEvent();
    void createNoteFromTodo();
    void defaultMenuEventVisible();
    void defaultMenuTodoVisible();
};

#endif // KOEVENTPOPUPMENUTESTTEST_H
