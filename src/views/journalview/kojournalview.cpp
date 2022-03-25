/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

// View of Journal entries

#include "kojournalview.h"
#include "prefs/koprefs.h"

#include <EventViews/JournalView>

#include <CalendarSupport/CalPrintDefaultPlugins>
#include <CalendarSupport/CalPrinter>
#include <CalendarSupport/Utils>

#include <Akonadi/ETMCalendar>

#include <QVBoxLayout>

using namespace KOrg;

KOJournalView::KOJournalView(QWidget *parent)
    : KOrg::BaseView(parent)
    , mJournalView(new EventViews::JournalView(this))
{
    auto layout = new QVBoxLayout(this);

    layout->addWidget(mJournalView);

    connect(mJournalView, &EventViews::JournalView::printJournal, this, &KOJournalView::printJournal);

    connect(mJournalView, &EventViews::EventView::incidenceSelected, this, &BaseView::incidenceSelected);

    connect(mJournalView, &EventViews::EventView::showIncidenceSignal, this, &BaseView::showIncidenceSignal);

    connect(mJournalView, &EventViews::EventView::editIncidenceSignal, this, &BaseView::editIncidenceSignal);

    connect(mJournalView, &EventViews::EventView::deleteIncidenceSignal, this, &BaseView::deleteIncidenceSignal);

    connect(mJournalView, &EventViews::EventView::cutIncidenceSignal, this, &BaseView::cutIncidenceSignal);

    connect(mJournalView, &EventViews::EventView::copyIncidenceSignal, this, &BaseView::copyIncidenceSignal);

    connect(mJournalView, &EventViews::EventView::pasteIncidenceSignal, this, &BaseView::pasteIncidenceSignal);

    connect(mJournalView, &EventViews::EventView::toggleAlarmSignal, this, &BaseView::toggleAlarmSignal);

    connect(mJournalView, &EventViews::EventView::toggleTodoCompletedSignal, this, &BaseView::toggleTodoCompletedSignal);

    connect(mJournalView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &BaseView::copyIncidenceToResourceSignal);

    connect(mJournalView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &BaseView::moveIncidenceToResourceSignal);

    connect(mJournalView, &EventViews::EventView::dissociateOccurrencesSignal, this, &BaseView::dissociateOccurrencesSignal);

    connect(mJournalView, qOverload<>(&EventViews::JournalView::newEventSignal), this, qOverload<>(&KOJournalView::newEventSignal));

    connect(mJournalView, qOverload<const QDate &>(&EventViews::JournalView::newEventSignal), this, qOverload<const QDate &>(&KOJournalView::newEventSignal));

    connect(mJournalView,
            qOverload<const QDateTime &>(&EventViews::JournalView::newEventSignal),
            this,
            qOverload<const QDateTime &>(&KOJournalView::newEventSignal));

    connect(mJournalView,
            qOverload<const QDateTime &, const QDateTime &>(&EventViews::JournalView::newEventSignal),
            this,
            qOverload<const QDateTime &, const QDateTime &>(&KOJournalView::newEventSignal));

    connect(mJournalView, &EventViews::EventView::newTodoSignal, this, &BaseView::newTodoSignal);

    connect(mJournalView, &EventViews::EventView::newSubTodoSignal, this, &BaseView::newSubTodoSignal);

    connect(mJournalView, &EventViews::EventView::newJournalSignal, this, &BaseView::newJournalSignal);
}

KOJournalView::~KOJournalView() = default;

int KOJournalView::currentDateCount() const
{
    return mJournalView->currentDateCount();
}

Akonadi::Item::List KOJournalView::selectedIncidences()
{
    return mJournalView->selectedIncidences();
}

void KOJournalView::updateView()
{
    mJournalView->updateView();
}

void KOJournalView::flushView()
{
    mJournalView->flushView();
}

void KOJournalView::showDates(const QDate &start, const QDate &end, const QDate &dummy)
{
    mJournalView->showDates(start, end, dummy);
}

void KOJournalView::showIncidences(const Akonadi::Item::List &incidences, const QDate &date)
{
    mJournalView->showIncidences(incidences, date);
}

void KOJournalView::changeIncidenceDisplay(const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType changeType)
{
    mJournalView->changeIncidenceDisplay(incidence, changeType);
}

void KOJournalView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mJournalView->setIncidenceChanger(changer);
}

void KOJournalView::getHighlightMode(bool &highlightEvents, bool &highlightTodos, bool &highlightJournals)
{
    highlightJournals = KOPrefs::instance()->mHighlightJournals;
    highlightTodos = false;
    highlightEvents = !highlightJournals;
}

CalendarSupport::CalPrinterBase::PrintType KOJournalView::printType() const
{
    return CalendarSupport::CalPrinterBase::Journallist;
}

void KOJournalView::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    BaseView::setCalendar(calendar);
    mJournalView->setCalendar(calendar);
}

void KOJournalView::printJournal(const KCalendarCore::Journal::Ptr &journal, bool preview)
{
    if (journal) {
        CalendarSupport::CalPrinter printer(this, calendar(), true);
        KCalendarCore::Incidence::List selectedIncidences;
        selectedIncidences.append(journal);

        const QDate dtStart = journal->dtStart().date();

        // make sure to clear and then restore the view stylesheet, else the view
        // stylesheet is propagated to the child print dialog. see bug 303902
        const QString css = styleSheet();
        setStyleSheet(QString());
        printer.print(CalendarSupport::CalPrinterBase::Incidence, dtStart, dtStart, selectedIncidences, preview);
        setStyleSheet(css);
    }
}
