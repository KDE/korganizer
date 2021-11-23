/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "monthview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"

#include <EventViews/MonthView>

#include <QVBoxLayout>

using namespace KOrg;

MonthView::MonthView(QWidget *parent)
    : KOEventView(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    mMonthView = new EventViews::MonthView(EventViews::MonthView::Visible, this);
    mMonthView->setPreferences(KOPrefs::instance()->eventViewsPreferences());
    layout->addWidget(mMonthView);
    mPopup = eventPopup();

    connect(mMonthView, &EventViews::MonthView::showIncidencePopupSignal, mPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(mMonthView, &EventViews::MonthView::showNewEventPopupSignal, this, &MonthView::showNewEventPopup);

    connect(mMonthView, &EventViews::EventView::datesSelected, this, &KOEventView::datesSelected);

    connect(mMonthView, &EventViews::EventView::shiftedEvent, this, &KOEventView::shiftedEvent);

    connect(mMonthView, &EventViews::EventView::incidenceSelected, this, &BaseView::incidenceSelected);

    connect(mMonthView, &EventViews::EventView::showIncidenceSignal, this, &BaseView::showIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::editIncidenceSignal, this, &BaseView::editIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::deleteIncidenceSignal, this, &BaseView::deleteIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::cutIncidenceSignal, this, &BaseView::cutIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::copyIncidenceSignal, this, &BaseView::copyIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::pasteIncidenceSignal, this, &BaseView::pasteIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::toggleAlarmSignal, this, &BaseView::toggleAlarmSignal);

    connect(mMonthView, &EventViews::EventView::toggleTodoCompletedSignal, this, &BaseView::toggleTodoCompletedSignal);

    connect(mMonthView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &BaseView::copyIncidenceToResourceSignal);

    connect(mMonthView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &BaseView::moveIncidenceToResourceSignal);

    connect(mMonthView, &EventViews::EventView::dissociateOccurrencesSignal, this, &BaseView::dissociateOccurrencesSignal);

    connect(mMonthView, qOverload<>(&EventViews::MonthView::newEventSignal), this, qOverload<>(&KOrg::MonthView::newEventSignal));

    connect(mMonthView, qOverload<const QDate &>(&EventViews::MonthView::newEventSignal),
            this, qOverload<const QDate &>(&KOrg::MonthView::newEventSignal));

    connect(mMonthView, qOverload<const QDateTime &>(&EventViews::MonthView::newEventSignal),
            this, qOverload<const QDateTime &>(&KOrg::MonthView::newEventSignal));

    connect(mMonthView, qOverload<const QDateTime &, const QDateTime &>(&EventViews::MonthView::newEventSignal),
            this, qOverload<const QDateTime &, const QDateTime &>(&KOrg::MonthView::newEventSignal));

    connect(mMonthView, &EventViews::EventView::newTodoSignal, this, &BaseView::newTodoSignal);

    connect(mMonthView, &EventViews::EventView::newSubTodoSignal, this, &BaseView::newSubTodoSignal);

    connect(mMonthView, &EventViews::EventView::newJournalSignal, this, &BaseView::newJournalSignal);

    connect(mMonthView, &EventViews::MonthView::fullViewChanged, this, &MonthView::fullViewChanged);
}

MonthView::~MonthView() = default;

CalendarSupport::CalPrinterBase::PrintType MonthView::printType() const
{
    return CalendarSupport::CalPrinterBase::Month;
}

int MonthView::currentDateCount() const
{
    return mMonthView->currentDateCount();
}

int MonthView::currentMonth() const
{
    return mMonthView->currentMonth();
}

KCalendarCore::DateList MonthView::selectedIncidenceDates()
{
    return mMonthView->selectedIncidenceDates();
}

QDateTime MonthView::selectionStart()
{
    return mMonthView->selectionStart();
}

QDateTime MonthView::selectionEnd()
{
    return mMonthView->selectionEnd();
}

bool MonthView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    return mMonthView->eventDurationHint(startDt, endDt, allDay);
}

QDate MonthView::averageDate() const
{
    return mMonthView->averageDate();
}

bool MonthView::usesFullWindow()
{
    return mMonthView->usesFullWindow();
}

bool MonthView::supportsDateRangeSelection()
{
    return mMonthView->supportsDateRangeSelection();
}

void MonthView::updateView()
{
    mMonthView->updateView();
}

void MonthView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    mMonthView->showIncidences(incidenceList, date);
}

void MonthView::changeIncidenceDisplay(const Akonadi::Item &item, Akonadi::IncidenceChanger::ChangeType changeType)
{
    mMonthView->changeIncidenceDisplay(item, changeType);
}

void MonthView::updateConfig()
{
    mMonthView->updateConfig();
}

int MonthView::maxDatesHint() const
{
    return 6 * 7;
}

Akonadi::Item::List MonthView::selectedIncidences()
{
    return mMonthView->selectedIncidences();
}

void MonthView::setTypeAheadReceiver(QObject *o)
{
    mMonthView->setTypeAheadReceiver(o);
}

void MonthView::setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth)
{
    mMonthView->setDateRange(start, end, preferredMonth);
}

void MonthView::setCalendar(const Akonadi::ETMCalendar::Ptr &cal)
{
    KOEventView::setCalendar(cal);
    mPopup->setCalendar(cal);
    mMonthView->setCalendar(cal);
}

void MonthView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mMonthView->setIncidenceChanger(changer);
}

void MonthView::showDates(const QDate &start, const QDate &end, const QDate &preferredMonth)
{
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(preferredMonth)
}
