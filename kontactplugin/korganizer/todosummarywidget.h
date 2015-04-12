/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2009 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef TODO_SUMMARYWIDGET_H
#define TODO_SUMMARYWIDGET_H

#include <AkonadiCore/Item>

#include <KCalCore/Todo>

#include <KontactInterface/Summary>
#include <Akonadi/Calendar/ETMCalendar>

class TodoPlugin;

namespace Akonadi
{
class IncidenceChanger;
}

class QGridLayout;
class QLabel;

class TodoSummaryWidget : public KontactInterface::Summary
{
    Q_OBJECT

public:
    TodoSummaryWidget(TodoPlugin *plugin, QWidget *parent);
    ~TodoSummaryWidget();

    int summaryHeight() const Q_DECL_OVERRIDE
    {
        return 3;
    }
    QStringList configModules() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void updateSummary(bool force = false) Q_DECL_OVERRIDE {
        Q_UNUSED(force);
        updateView();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void updateView();
    void popupMenu(const QString &uid);
    void viewTodo(const QString &uid);
    void removeTodo(const Akonadi::Item &item);
    void completeTodo(Akonadi::Item::Id id);

private:
    TodoPlugin *mPlugin;
    QGridLayout *mLayout;

    bool mDaysToGo;
    bool mHideInProgress;
    bool mHideOverdue;
    bool mHideCompleted;
    bool mHideOpenEnded;
    bool mHideNotStarted;
    bool mShowMineOnly;

    QList<QLabel *> mLabels;
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::IncidenceChanger *mChanger;

    /**
      Test if the To-do starts today.
      @param todo is a pointer to a To-do object to test.
      @return if the To-do starts on the current date.
    */
    bool startsToday(const KCalCore::Todo::Ptr &todo);

    /**
      Create a text string containing the states of the To-do.
      @param todo is a pointer to a To-do object to test.
      @return a QString containing a comma-separated list of To-do states.
    */
    const QString stateStr(const KCalCore::Todo::Ptr &todo);
};

#endif
