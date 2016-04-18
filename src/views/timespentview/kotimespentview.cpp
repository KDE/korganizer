/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>

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

#include "kotimespentview.h"

#include <EventViews/TimeSpentView>
#include <Akonadi/Calendar/ETMCalendar>
#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <KCalCore/Event>

#include <QVBoxLayout>

KOTimeSpentView::KOTimeSpentView(QWidget *parent)
    : KOrg::BaseView(parent)
{
    mView = new EventViews::TimeSpentView(this);
    QBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->addWidget(mView);

    connect(mView, &EventViews::EventView::incidenceSelected,
            this, &KOrg::BaseView::incidenceSelected);

    connect(mView, &EventViews::EventView::showIncidenceSignal,
            this, &KOrg::BaseView::showIncidenceSignal);

    connect(mView, &EventViews::EventView::editIncidenceSignal,
            this, &KOrg::BaseView::editIncidenceSignal);

    connect(mView, &EventViews::EventView::deleteIncidenceSignal,
            this, &KOrg::BaseView::deleteIncidenceSignal);

    connect(mView, &EventViews::EventView::cutIncidenceSignal,
            this, &KOrg::BaseView::cutIncidenceSignal);

    connect(mView, &EventViews::EventView::copyIncidenceSignal,
            this, &KOrg::BaseView::copyIncidenceSignal);

    connect(mView, &EventViews::EventView::pasteIncidenceSignal,
            this, &KOrg::BaseView::pasteIncidenceSignal);

    connect(mView, &EventViews::EventView::toggleAlarmSignal,
            this, &KOrg::BaseView::toggleAlarmSignal);

    connect(mView, &EventViews::EventView::toggleTodoCompletedSignal,
            this, &KOrg::BaseView::toggleTodoCompletedSignal);

    connect(mView, &EventViews::EventView::copyIncidenceToResourceSignal,
            this, &KOrg::BaseView::copyIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::moveIncidenceToResourceSignal,
            this, &KOrg::BaseView::moveIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::dissociateOccurrencesSignal,
            this, &KOrg::BaseView::dissociateOccurrencesSignal);

    connect(mView, SIGNAL(newEventSignal()),
            SIGNAL(newEventSignal()));

    connect(mView, SIGNAL(newEventSignal(QDate)),
            SIGNAL(newEventSignal(QDate)));

    connect(mView, SIGNAL(newEventSignal(QDateTime)),
            SIGNAL(newEventSignal(QDateTime)));

    connect(mView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
            SIGNAL(newEventSignal(QDateTime,QDateTime)));

    connect(mView, &EventViews::EventView::newTodoSignal,
            this, &KOrg::BaseView::newTodoSignal);

    connect(mView, &EventViews::EventView::newSubTodoSignal,
            this, &KOrg::BaseView::newSubTodoSignal);

    connect(mView, &EventViews::EventView::newJournalSignal,
            this, &KOrg::BaseView::newJournalSignal);
}

KOTimeSpentView::~KOTimeSpentView()
{
}

int KOTimeSpentView::currentDateCount() const
{
    return mView->currentDateCount();
}

void KOTimeSpentView::showDates(const QDate &start, const QDate &end,
                                const QDate &dummy)
{
    return mView->showDates(start, end, dummy);
}

void KOTimeSpentView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    Q_UNUSED(incidenceList);
    Q_UNUSED(date);
}

void KOTimeSpentView::changeIncidenceDisplay(const Akonadi::Item &,
        Akonadi::IncidenceChanger::ChangeType)
{
    mView->updateView();
}

void KOTimeSpentView::updateView()
{
    mView->updateView();
}

CalendarSupport::CalPrinterBase::PrintType KOTimeSpentView::printType() const
{
    // If up to three days are selected, use day style, otherwise week
    if (currentDateCount() <= 3) {
        return CalendarSupport::CalPrinterBase::Day;
    } else {
        return CalendarSupport::CalPrinterBase::Week;
    }
}

void KOTimeSpentView::setCalendar(const Akonadi::ETMCalendar::Ptr &cal)
{
    KOrg::BaseView::setCalendar(cal);
    mView->setCalendar(cal);
}

