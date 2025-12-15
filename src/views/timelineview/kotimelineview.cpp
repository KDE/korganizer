/*
  This file is part of KOrganizer.
  SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileCopyrightText: 2010 Andras Mantia <andras@kdab.com>
  SPDX-FileCopyrightText: 2010 Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "kotimelineview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"

#include <EventViews/TimeLineView>

#include <QVBoxLayout>

KOTimelineView::KOTimelineView(QWidget *parent)
    : KOEventView(parent)
{
    auto vbox = new QVBoxLayout(this);
    vbox->setContentsMargins({});
    mTimeLineView = new EventViews::TimelineView(KOPrefs::instance()->eventViewsPreferences(), this);
    vbox->addWidget(mTimeLineView);
    mEventPopup = eventPopup();

    connect(mTimeLineView, &EventViews::TimelineView::showIncidencePopupSignal, mEventPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(mTimeLineView, &EventViews::TimelineView::showNewEventPopupSignal, this, &KOTimelineView::showNewEventPopup);

    connect(mTimeLineView, &EventViews::EventView::datesSelected, this, &KOEventView::datesSelected);

    connect(mTimeLineView, &EventViews::EventView::shiftedEvent, this, &KOEventView::shiftedEvent);

    connect(mTimeLineView, &EventViews::EventView::incidenceSelected, this, &KOrg::BaseView::incidenceSelected);

    connect(mTimeLineView, &EventViews::EventView::showIncidenceSignal, this, &KOrg::BaseView::showIncidenceSignal);

    connect(mTimeLineView, &EventViews::EventView::editIncidenceSignal, this, &KOrg::BaseView::editIncidenceSignal);

    connect(mTimeLineView, &EventViews::EventView::deleteIncidenceSignal, this, &KOrg::BaseView::deleteIncidenceSignal);

    connect(mTimeLineView, &EventViews::EventView::cutIncidenceSignal, this, &KOrg::BaseView::cutIncidenceSignal);

    connect(mTimeLineView, &EventViews::EventView::copyIncidenceSignal, this, &KOrg::BaseView::copyIncidenceSignal);

    connect(mTimeLineView, &EventViews::EventView::pasteIncidenceSignal, this, &KOrg::BaseView::pasteIncidenceSignal);

    connect(mTimeLineView, &EventViews::EventView::toggleAlarmSignal, this, &KOrg::BaseView::toggleAlarmSignal);

    connect(mTimeLineView, &EventViews::EventView::toggleTodoCompletedSignal, this, &KOrg::BaseView::toggleTodoCompletedSignal);

    connect(mTimeLineView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &KOrg::BaseView::copyIncidenceToResourceSignal);

    connect(mTimeLineView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &KOrg::BaseView::moveIncidenceToResourceSignal);

    connect(mTimeLineView, &EventViews::EventView::dissociateOccurrencesSignal, this, &KOrg::BaseView::dissociateOccurrencesSignal);

    connect(mTimeLineView, qOverload<>(&EventViews::TimelineView::newEventSignal), this, qOverload<>(&KOTimelineView::newEventSignal));

    connect(mTimeLineView,
            qOverload<const QDate &>(&EventViews::TimelineView::newEventSignal),
            this,
            qOverload<const QDate &>(&KOTimelineView::newEventSignal));

    connect(mTimeLineView,
            qOverload<const QDateTime &>(&EventViews::TimelineView::newEventSignal),
            this,
            qOverload<const QDateTime &>(&KOTimelineView::newEventSignal));

    connect(mTimeLineView,
            qOverload<const QDateTime &, const QDateTime &>(&EventViews::TimelineView::newEventSignal),
            this,
            qOverload<const QDateTime &, const QDateTime &>(&KOTimelineView::newEventSignal));

    connect(mTimeLineView, &EventViews::EventView::newTodoSignal, this, &KOrg::BaseView::newTodoSignal);

    connect(mTimeLineView, &EventViews::EventView::newSubTodoSignal, this, &KOrg::BaseView::newSubTodoSignal);

    connect(mTimeLineView, &EventViews::EventView::newJournalSignal, this, &KOrg::BaseView::newJournalSignal);
}

KOTimelineView::~KOTimelineView()
{
    delete mEventPopup;
}

Akonadi::Item::List KOTimelineView::selectedIncidences()
{
    return mTimeLineView->selectedIncidences();
}

KCalendarCore::DateList KOTimelineView::selectedIncidenceDates()
{
    return mTimeLineView->selectedIncidenceDates();
}

int KOTimelineView::currentDateCount() const
{
    return mTimeLineView->currentDateCount();
}

void KOTimelineView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    mTimeLineView->showDates(start, end);
}

void KOTimelineView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    mTimeLineView->showIncidences(incidenceList, date);
}

void KOTimelineView::updateView()
{
    mTimeLineView->updateView();
}

void KOTimelineView::changeIncidenceDisplay(const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType changeType)
{
    mTimeLineView->changeIncidenceDisplay(incidence, changeType);
}

bool KOTimelineView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    return mTimeLineView->eventDurationHint(startDt, endDt, allDay);
}

CalendarSupport::CalPrinterBase::PrintType KOTimelineView::printType() const
{
    // If up to three days are selected, use day style, otherwise week
    if (currentDateCount() <= 3) {
        return CalendarSupport::CalPrinterBase::Day;
    } else {
        return CalendarSupport::CalPrinterBase::Week;
    }
}

void KOTimelineView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mTimeLineView->setIncidenceChanger(changer);
}

void KOTimelineView::calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mTimeLineView->addCalendar(calendar);
}

void KOTimelineView::calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mTimeLineView->removeCalendar(calendar);
}
#include "moc_kotimelineview.cpp"
