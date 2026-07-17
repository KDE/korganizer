/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "komonthview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"

#include <EventViews/MonthView>

#include <QVBoxLayout>

KOMonthView::KOMonthView(QWidget *parent)
    : KOEventView(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    mMonthView = new EventViews::MonthView(EventViews::MonthView::Hidden, this);
    mMonthView->enableMonthYearHeader(false); // the month year header is redundant and takes space
    mMonthView->setPreferences(KOPrefs::instance()->eventViewsPreferences());
    layout->addWidget(mMonthView);
    mPopup = eventPopup();

    connect(mMonthView, &EventViews::MonthView::showIncidencePopupSignal, mPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(mMonthView, &EventViews::MonthView::showNewEventPopupSignal, this, &KOMonthView::showNewEventPopup);

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

    connect(mMonthView, qOverload<>(&EventViews::MonthView::newEventSignal), this, qOverload<>(&KOMonthView::newEventSignal));

    connect(mMonthView, qOverload<const QDate &>(&EventViews::MonthView::newEventSignal), this, qOverload<const QDate &>(&KOMonthView::newEventSignal));

    connect(mMonthView, qOverload<const QDateTime &>(&EventViews::MonthView::newEventSignal), this, qOverload<const QDateTime &>(&KOMonthView::newEventSignal));

    connect(mMonthView,
            qOverload<const QDateTime &, const QDateTime &>(&EventViews::MonthView::newEventSignal),
            this,
            qOverload<const QDateTime &, const QDateTime &>(&KOMonthView::newEventSignal));

    connect(mMonthView, &EventViews::EventView::newTodoSignal, this, &BaseView::newTodoSignal);

    connect(mMonthView, &EventViews::EventView::newSubTodoSignal, this, &BaseView::newSubTodoSignal);

    connect(mMonthView, &EventViews::EventView::newJournalSignal, this, &BaseView::newJournalSignal);
}

KOMonthView::~KOMonthView() = default;

CalendarSupport::CalPrinterBase::PrintType KOMonthView::printType() const
{
    return CalendarSupport::CalPrinterBase::Month;
}

int KOMonthView::currentDateCount() const
{
    return mMonthView->currentDateCount();
}

int KOMonthView::currentMonth() const
{
    return mMonthView->currentMonth();
}

KCalendarCore::DateList KOMonthView::selectedIncidenceDates()
{
    return mMonthView->selectedIncidenceDates();
}

QDateTime KOMonthView::selectionStart()
{
    return mMonthView->selectionStart();
}

QDateTime KOMonthView::selectionEnd()
{
    return mMonthView->selectionEnd();
}

bool KOMonthView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    return mMonthView->eventDurationHint(startDt, endDt, allDay);
}

QDate KOMonthView::averageDate() const
{
    return mMonthView->averageDate();
}

bool KOMonthView::showSideBar()
{
    return KOPrefs::instance()->monthViewShowSidebar();
}

void KOMonthView::setShowSideBar(bool show)
{
    KOPrefs::instance()->setMonthViewShowSidebar(show);
}

bool KOMonthView::supportsDateRangeSelection()
{
    return mMonthView->supportsDateRangeSelection();
}

void KOMonthView::updateView()
{
    mMonthView->updateView();
}

void KOMonthView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    mMonthView->showIncidences(incidenceList, date);
}

void KOMonthView::changeIncidenceDisplay(const Akonadi::Item &item, Akonadi::IncidenceChanger::ChangeType changeType)
{
    mMonthView->changeIncidenceDisplay(item, changeType);
}

void KOMonthView::updateConfig()
{
    mMonthView->updateConfig();
}

int KOMonthView::maxDatesHint() const
{
    return 6 * 7;
}

Akonadi::Item::List KOMonthView::selectedIncidences()
{
    return mMonthView->selectedIncidences();
}

void KOMonthView::setTypeAheadReceiver(QObject *o)
{
    mMonthView->setTypeAheadReceiver(o);
}

void KOMonthView::setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth)
{
    mMonthView->setDateRange(start, end, preferredMonth);
}

void KOMonthView::setModel(QAbstractItemModel *model)
{
    KOEventView::setModel(model);
    mMonthView->setModel(model);
}

void KOMonthView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mMonthView->setIncidenceChanger(changer);
}

void KOMonthView::showDates(const QDate &start, const QDate &end, const QDate &preferredMonth)
{
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(preferredMonth)
}

void KOMonthView::calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    if (calendar && calendar->collection().isValid()) {
        mMonthView->addCalendar(calendar);
    }
}

void KOMonthView::calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    if (calendar && calendar->collection().isValid()) {
        mMonthView->removeCalendar(calendar);
    }
}
#include "moc_komonthview.cpp"
