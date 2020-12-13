/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef TODO_SUMMARYWIDGET_H
#define TODO_SUMMARYWIDGET_H

#include <AkonadiCore/Item>

#include <KCalendarCore/Todo>

#include <KontactInterface/Summary>
#include <Akonadi/Calendar/ETMCalendar>

class TodoPlugin;

namespace Akonadi {
class IncidenceChanger;
}

class QGridLayout;
class QLabel;

class TodoSummaryWidget : public KontactInterface::Summary
{
    Q_OBJECT

public:
    TodoSummaryWidget(TodoPlugin *plugin, QWidget *parent);
    ~TodoSummaryWidget() override;

    int summaryHeight() const override
    {
        return 3;
    }

    Q_REQUIRED_RESULT QStringList configModules() const override;

public Q_SLOTS:
    void updateSummary(bool force = false) override
    {
        Q_UNUSED(force)
        updateView();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private Q_SLOTS:
    void updateView();
    void popupMenu(const QString &uid);
    void viewTodo(const QString &uid);
    void removeTodo(const Akonadi::Item &item);
    void completeTodo(Akonadi::Item::Id id);

private:
    TodoPlugin *mPlugin = nullptr;
    QGridLayout *mLayout = nullptr;

    bool mDaysToGo = false;
    bool mHideInProgress = false;
    bool mHideOverdue = false;
    bool mHideCompleted = false;
    bool mHideOpenEnded = false;
    bool mHideNotStarted = false;
    bool mShowMineOnly = false;

    QList<QLabel *> mLabels;
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::IncidenceChanger *mChanger = nullptr;

    /**
      Test if the To-do starts today.
      @param todo is a pointer to a To-do object to test.
      @return if the To-do starts on the current date.
    */
    bool startsToday(const KCalendarCore::Todo::Ptr &todo);

    /**
      Create a text string containing the states of the To-do.
      @param todo is a pointer to a To-do object to test.
      @return a QString containing a comma-separated list of To-do states.
    */
    const QString stateStr(const KCalendarCore::Todo::Ptr &todo);
};

#endif
