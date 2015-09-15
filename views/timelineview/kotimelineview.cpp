/*
  This file is part of KOrganizer.
  Copyright (c) 2007 Till Adam <adam@kde.org>

  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Copyright (c) 2010 Sérgio Martins <sergio.martins@kdab.com>

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

#include "kotimelineview.h"
#include "koeventpopupmenu.h"

#include <EventViews/TimeLineView>

#include <QVBoxLayout>

class KOTimelineView::Private
{
public:
    Private(KOTimelineView *q) : mEventPopup(Q_NULLPTR), mParent(q)
    {
        QVBoxLayout *vbox = new QVBoxLayout(mParent);
        vbox->setMargin(0);
        mTimeLineView = new EventViews::TimelineView(mParent);
        vbox->addWidget(mTimeLineView);
        mEventPopup = q->eventPopup();
    }
    ~Private()
    {
        delete mEventPopup;
    }
    KOEventPopupMenu *mEventPopup;
    EventViews::TimelineView *mTimeLineView;

private:
    KOTimelineView *mParent;
};

KOTimelineView::KOTimelineView(QWidget *parent)
    : KOEventView(parent), d(new Private(this))
{
    connect(d->mTimeLineView, &EventViews::TimelineView::showIncidencePopupSignal,
            d->mEventPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(d->mTimeLineView, &EventViews::TimelineView::showNewEventPopupSignal,
            this, &KOTimelineView::showNewEventPopup);

    connect(d->mTimeLineView, &EventViews::EventView::datesSelected,
            this, &KOEventView::datesSelected);

    connect(d->mTimeLineView, &EventViews::EventView::shiftedEvent,
            this, &KOEventView::shiftedEvent);

    connect(d->mTimeLineView, &EventViews::EventView::incidenceSelected,
            this, &KOrg::BaseView::incidenceSelected);

    connect(d->mTimeLineView, &EventViews::EventView::showIncidenceSignal,
            this, &KOrg::BaseView::showIncidenceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::editIncidenceSignal,
            this, &KOrg::BaseView::editIncidenceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::deleteIncidenceSignal,
            this, &KOrg::BaseView::deleteIncidenceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::cutIncidenceSignal,
            this, &KOrg::BaseView::cutIncidenceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::copyIncidenceSignal,
            this, &KOrg::BaseView::copyIncidenceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::pasteIncidenceSignal,
            this, &KOrg::BaseView::pasteIncidenceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::toggleAlarmSignal,
            this, &KOrg::BaseView::toggleAlarmSignal);

    connect(d->mTimeLineView, &EventViews::EventView::toggleTodoCompletedSignal,
            this, &KOrg::BaseView::toggleTodoCompletedSignal);

    connect(d->mTimeLineView, &EventViews::EventView::copyIncidenceToResourceSignal,
            this, &KOrg::BaseView::copyIncidenceToResourceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::moveIncidenceToResourceSignal,
            this, &KOrg::BaseView::moveIncidenceToResourceSignal);

    connect(d->mTimeLineView, &EventViews::EventView::dissociateOccurrencesSignal,
            this, &KOrg::BaseView::dissociateOccurrencesSignal);

    connect(d->mTimeLineView, SIGNAL(newEventSignal()),
            SIGNAL(newEventSignal()));

    connect(d->mTimeLineView, SIGNAL(newEventSignal(QDate)),
            SIGNAL(newEventSignal(QDate)));

    connect(d->mTimeLineView, SIGNAL(newEventSignal(QDateTime)),
            SIGNAL(newEventSignal(QDateTime)));

    connect(d->mTimeLineView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
            SIGNAL(newEventSignal(QDateTime,QDateTime)));

    connect(d->mTimeLineView, &EventViews::EventView::newTodoSignal,
            this, &KOrg::BaseView::newTodoSignal);

    connect(d->mTimeLineView, &EventViews::EventView::newSubTodoSignal,
            this, &KOrg::BaseView::newSubTodoSignal);

    connect(d->mTimeLineView, &EventViews::EventView::newJournalSignal,
            this, &KOrg::BaseView::newJournalSignal);
}

KOTimelineView::~KOTimelineView()
{
    delete d;
}

Akonadi::Item::List KOTimelineView::selectedIncidences()
{
    return d->mTimeLineView->selectedIncidences();
}

KCalCore::DateList KOTimelineView::selectedIncidenceDates()
{
    return d->mTimeLineView->selectedIncidenceDates();
}

int KOTimelineView::currentDateCount() const
{
    return d->mTimeLineView->currentDateCount();
}

void KOTimelineView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    d->mTimeLineView->showDates(start, end);
}

void KOTimelineView::showIncidences(const Akonadi::Item::List &incidenceList,
                                    const QDate &date)
{
    d->mTimeLineView->showIncidences(incidenceList, date);
}

void KOTimelineView::updateView()
{
    d->mTimeLineView->updateView();
}

void KOTimelineView::changeIncidenceDisplay(const Akonadi::Item &incidence,
        Akonadi::IncidenceChanger::ChangeType changeType)
{
    d->mTimeLineView->changeIncidenceDisplay(incidence, changeType);
}

bool KOTimelineView::eventDurationHint(QDateTime &startDt, QDateTime &endDt,
                                       bool &allDay)
{
    return d->mTimeLineView->eventDurationHint(startDt, endDt, allDay);
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

void KOTimelineView::setCalendar(const Akonadi::ETMCalendar::Ptr &cal)
{
    KOEventView::setCalendar(cal);
    d->mEventPopup->setCalendar(cal);
    d->mTimeLineView->setCalendar(cal);
}

void KOTimelineView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    d->mTimeLineView->setIncidenceChanger(changer);
}

