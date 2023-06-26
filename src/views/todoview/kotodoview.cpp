/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2008 Thomas Thrainer <tom_t@gmx.at>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kotodoview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"

#include <CalendarSupport/CalPrinter>

#include <Akonadi/CalendarUtils>
#include <Akonadi/EntityTreeModel>

#include <QVBoxLayout>

KOTodoView::KOTodoView(bool sidebarView, QWidget *parent)
    : BaseView(parent)
{
    auto eventPopup = new KOEventPopupMenu(this);
    mView = new EventViews::TodoView(KOPrefs::instance()->eventViewsPreferences(), sidebarView, parent);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(mView);
    connect(mView, &EventViews::TodoView::printTodo, this, [this]() {
        printTodo();
    });
    connect(mView, &EventViews::TodoView::printPreviewTodo, this, &KOTodoView::printPreviewTodo);
    connect(mView, &EventViews::TodoView::purgeCompletedSignal, this, &KOTodoView::purgeCompletedSignal);

    connect(mView, &EventViews::EventView::incidenceSelected, this, &BaseView::incidenceSelected);

    connect(mView, &EventViews::EventView::showIncidenceSignal, this, &BaseView::showIncidenceSignal);

    connect(mView, &EventViews::EventView::editIncidenceSignal, this, &BaseView::editIncidenceSignal);

    connect(mView, &EventViews::EventView::deleteIncidenceSignal, this, &BaseView::deleteIncidenceSignal);

    connect(mView, &EventViews::EventView::cutIncidenceSignal, this, &BaseView::cutIncidenceSignal);

    connect(mView, &EventViews::EventView::copyIncidenceSignal, this, &BaseView::copyIncidenceSignal);

    connect(mView, &EventViews::EventView::pasteIncidenceSignal, this, &BaseView::pasteIncidenceSignal);

    connect(mView, &EventViews::EventView::toggleAlarmSignal, this, &BaseView::toggleAlarmSignal);

    connect(mView, &EventViews::EventView::toggleTodoCompletedSignal, this, &BaseView::toggleTodoCompletedSignal);

    connect(mView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &BaseView::copyIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &BaseView::moveIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::dissociateOccurrencesSignal, this, &BaseView::dissociateOccurrencesSignal);

    connect(mView, qOverload<>(&EventViews::TodoView::newEventSignal), this, qOverload<>(&KOTodoView::newEventSignal));

    connect(mView, qOverload<const QDate &>(&EventViews::TodoView::newEventSignal), this, qOverload<const QDate &>(&KOTodoView::newEventSignal));

    connect(mView, qOverload<const QDateTime &>(&EventViews::TodoView::newEventSignal), this, qOverload<const QDateTime &>(&KOTodoView::newEventSignal));

    connect(mView,
            qOverload<const QDateTime &, const QDateTime &>(&EventViews::TodoView::newEventSignal),
            this,
            qOverload<const QDateTime &, const QDateTime &>(&KOTodoView::newEventSignal));

    connect(mView, &EventViews::EventView::newTodoSignal, this, &BaseView::newTodoSignal);

    connect(mView, &EventViews::EventView::newSubTodoSignal, this, &BaseView::newSubTodoSignal);

    connect(mView, &EventViews::TodoView::fullViewChanged, this, &KOTodoView::fullViewChanged);

    connect(mView, &EventViews::TodoView::unSubTodoSignal, this, &KOTodoView::unSubTodoSignal);
    connect(mView, &EventViews::TodoView::unAllSubTodoSignal, this, &KOTodoView::unAllSubTodoSignal);

    connect(mView,
            static_cast<void (EventViews::TodoView::*)(const Akonadi::Item &)>(&EventViews::TodoView ::createEvent),
            eventPopup,
            static_cast<void (KOEventPopupMenu::*)(const Akonadi::Item &)>(&KOEventPopupMenu::createEvent));
    connect(mView,
            static_cast<void (EventViews::TodoView::*)(const Akonadi::Item &)>(&EventViews::TodoView ::createNote),
            eventPopup,
            static_cast<void (KOEventPopupMenu::*)(const Akonadi::Item &)>(&KOEventPopupMenu::createNote));
}

KOTodoView::~KOTodoView() = default;

void KOTodoView::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    BaseView::setCalendar(calendar);
    mView->setCalendar(calendar);
}

Akonadi::Item::List KOTodoView::selectedIncidences()
{
    return mView->selectedIncidences();
}

KCalendarCore::DateList KOTodoView::selectedIncidenceDates()
{
    return {};
}

void KOTodoView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    BaseView::setIncidenceChanger(changer);
    mView->setIncidenceChanger(changer);
}

void KOTodoView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    // There is nothing to do here for the Todo View
    Q_UNUSED(start)
    Q_UNUSED(end)
}

void KOTodoView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    Q_UNUSED(incidenceList)
    Q_UNUSED(date)
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
    const Akonadi::Item::List selectedItems = mView->selectedIncidences();
    if (selectedItems.count() != 1) {
        return;
    }

    Akonadi::Item todoItem = selectedItems.first();
    KCalendarCore::Todo::Ptr todo = Akonadi::CalendarUtils::todo(todoItem);
    Q_ASSERT(todo);

    CalendarSupport::CalPrinter printer(this, calendar(), true);
    connect(this, &KOTodoView::configChanged, &printer, &CalendarSupport::CalPrinter::updateConfig);

    KCalendarCore::Incidence::List selectedIncidences;
    selectedIncidences.append(todo);

    QDate todoDate;
    if (todo->hasStartDate()) {
        todoDate = todo->dtStart().date();
    } else {
        todoDate = todo->dtDue().date();
    }

    printer.print(CalendarSupport::CalPrinterBase::Incidence, todoDate, todoDate, selectedIncidences, preview);
}

void KOTodoView::getHighlightMode(bool &highlightEvents, bool &highlightTodos, bool &highlightJournals)
{
    highlightTodos = KOPrefs::instance()->mHighlightTodos;
    highlightEvents = !highlightTodos;
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
