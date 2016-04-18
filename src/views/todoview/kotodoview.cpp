/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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

#include "kotodoview.h"
#include "prefs/koprefs.h"
#include "koeventpopupmenu.h"

#include <CalendarSupport/CalPrinter>

#include <EventViews/TodoView>

#include <AkonadiCore/EntityTreeModel>

#include <QVBoxLayout>

KOTodoView::KOTodoView(bool sidebarView, QWidget *parent)
    : BaseView(parent)
{
    KOEventPopupMenu *eventPopup = new KOEventPopupMenu(calendar().data(), this);
    mView = new EventViews::TodoView(KOPrefs::instance()->eventViewsPreferences(),
                                     sidebarView, parent);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mView);
    connect(mView, SIGNAL(printTodo()), SLOT(printTodo()));
    connect(mView, &EventViews::TodoView::printPreviewTodo, this, &KOTodoView::printPreviewTodo);
    connect(mView, &EventViews::TodoView::purgeCompletedSignal, this, &KOTodoView::purgeCompletedSignal);

    connect(mView, &EventViews::EventView::incidenceSelected,
            this, &BaseView::incidenceSelected);

    connect(mView, &EventViews::EventView::showIncidenceSignal,
            this, &BaseView::showIncidenceSignal);

    connect(mView, &EventViews::EventView::editIncidenceSignal,
            this, &BaseView::editIncidenceSignal);

    connect(mView, &EventViews::EventView::deleteIncidenceSignal,
            this, &BaseView::deleteIncidenceSignal);

    connect(mView, &EventViews::EventView::cutIncidenceSignal,
            this, &BaseView::cutIncidenceSignal);

    connect(mView, &EventViews::EventView::copyIncidenceSignal,
            this, &BaseView::copyIncidenceSignal);

    connect(mView, &EventViews::EventView::pasteIncidenceSignal,
            this, &BaseView::pasteIncidenceSignal);

    connect(mView, &EventViews::EventView::toggleAlarmSignal,
            this, &BaseView::toggleAlarmSignal);

    connect(mView, &EventViews::EventView::toggleTodoCompletedSignal,
            this, &BaseView::toggleTodoCompletedSignal);

    connect(mView, &EventViews::EventView::copyIncidenceToResourceSignal,
            this, &BaseView::copyIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::moveIncidenceToResourceSignal,
            this, &BaseView::moveIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::dissociateOccurrencesSignal,
            this, &BaseView::dissociateOccurrencesSignal);

    connect(mView, SIGNAL(newEventSignal()),
            SIGNAL(newEventSignal()));

    connect(mView, SIGNAL(newEventSignal(QDate)),
            SIGNAL(newEventSignal(QDate)));

    connect(mView, SIGNAL(newEventSignal(QDateTime)),
            SIGNAL(newEventSignal(QDateTime)));

    connect(mView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
            SIGNAL(newEventSignal(QDateTime,QDateTime)));

    connect(mView, &EventViews::EventView::newTodoSignal,
            this, &BaseView::newTodoSignal);

    connect(mView, &EventViews::EventView::newSubTodoSignal,
            this, &BaseView::newSubTodoSignal);

    connect(mView, &EventViews::TodoView::fullViewChanged,
            this, &KOTodoView::fullViewChanged);

    connect(mView, &EventViews::TodoView::unSubTodoSignal,
            this, &KOTodoView::unSubTodoSignal);
    connect(mView, &EventViews::TodoView::unAllSubTodoSignal,
            this, &KOTodoView::unAllSubTodoSignal);

    connect(mView, static_cast<void (EventViews::TodoView::*)(const Akonadi::Item &)>(&EventViews::TodoView::createEvent),
            eventPopup, static_cast<void (KOEventPopupMenu::*)(const Akonadi::Item &)>(&KOEventPopupMenu::createEvent));
    connect(mView, static_cast<void (EventViews::TodoView::*)(const Akonadi::Item &)>(&EventViews::TodoView::createNote),
            eventPopup, static_cast<void (KOEventPopupMenu::*)(const Akonadi::Item &)>(&KOEventPopupMenu::createNote));
}

KOTodoView::~KOTodoView()
{
}

void KOTodoView::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    BaseView::setCalendar(calendar);
    mView->setCalendar(calendar);
}

Akonadi::Item::List KOTodoView::selectedIncidences()
{
    return mView->selectedIncidences();
}

KCalCore::DateList KOTodoView::selectedIncidenceDates()
{
    return KCalCore::DateList();
}

void KOTodoView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    BaseView::setIncidenceChanger(changer);
    mView->setIncidenceChanger(changer);
}

void KOTodoView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    // There is nothing to do here for the Todo View
    Q_UNUSED(start);
    Q_UNUSED(end);
}

void KOTodoView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    Q_UNUSED(incidenceList);
    Q_UNUSED(date);
}

void KOTodoView::updateView()
{
    // View is always updated, it's connected to ETM.
}

void KOTodoView::changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType)
{
    // Don't do anything, model is connected to ETM, it's up to date
}

void KOTodoView::updateConfig()
{
    mView->updateConfig();
}

void KOTodoView::clearSelection()
{
    mView->clearSelection();
}

void KOTodoView::printTodo()
{
    printTodo(false);
}

void KOTodoView::printPreviewTodo()
{
    printTodo(true);
}

void KOTodoView::printTodo(bool preview)
{
    Akonadi::Item::List selectedItems = mView->selectedIncidences();
    if (selectedItems.count() != 1) {
        return;
    }

    Akonadi::Item todoItem = selectedItems.first();
    KCalCore::Todo::Ptr todo = CalendarSupport::todo(todoItem);
    Q_ASSERT(todo);

    CalendarSupport::CalPrinter printer(this, calendar(), true);
    connect(this, &KOTodoView::configChanged, &printer, &CalendarSupport::CalPrinter::updateConfig);

    KCalCore::Incidence::List selectedIncidences;
    selectedIncidences.append(todo);

    KDateTime todoDate;
    if (todo->hasStartDate()) {
        todoDate = todo->dtStart();
    } else {
        todoDate = todo->dtDue();
    }

    printer.print(CalendarSupport::CalPrinterBase::Incidence,
                  todoDate.date(), todoDate.date(), selectedIncidences, preview);

}

void KOTodoView::getHighlightMode(bool &highlightEvents,
                                  bool &highlightTodos,
                                  bool &highlightJournals)
{
    highlightTodos    = KOPrefs::instance()->mHighlightTodos;
    highlightEvents   = !highlightTodos;
    highlightJournals = false;
}

void KOTodoView::saveViewState()
{
    mView->saveViewState();
}

void KOTodoView::restoreViewState()
{
    mView->restoreViewState();
}

void KOTodoView::restoreLayout(KConfig *config, const QString &group, bool minimalDefaults)
{
    mView->restoreLayout(config, group, minimalDefaults);
}

void KOTodoView::saveLayout(KConfig *config, const QString &group) const
{
    mView->saveLayout(config, group);
}

bool KOTodoView::usesFullWindow()
{
    return mView->usesFullWindow();
}

CalendarSupport::CalPrinterBase::PrintType KOTodoView::printType() const
{
    return CalendarSupport::CalPrinterBase::Todolist;
}

