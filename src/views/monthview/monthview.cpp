/*
  This file is part of KOrganizer.

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins <sergio.martins@kdab.com>

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

#include "monthview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"

#include <EventViews/MonthView>

#include <QVBoxLayout>

using namespace KOrg;

MonthView::MonthView(QWidget *parent)
    : KOEventView(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    mMonthView = new EventViews::MonthView(EventViews::MonthView::Visible, this);
    mMonthView->setPreferences(KOPrefs::instance()->eventViewsPreferences());
    layout->addWidget(mMonthView);
    mPopup = eventPopup();

    connect(mMonthView, &EventViews::MonthView::showIncidencePopupSignal, mPopup,
            &KOEventPopupMenu::showIncidencePopup);

    connect(mMonthView, &EventViews::MonthView::showNewEventPopupSignal, this,
            &MonthView::showNewEventPopup);

    connect(mMonthView, &EventViews::EventView::datesSelected,
            this, &KOEventView::datesSelected);

    connect(mMonthView, &EventViews::EventView::shiftedEvent,
            this, &KOEventView::shiftedEvent);

    connect(mMonthView, &EventViews::EventView::incidenceSelected,
            this, &BaseView::incidenceSelected);

    connect(mMonthView, &EventViews::EventView::showIncidenceSignal,
            this, &BaseView::showIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::editIncidenceSignal,
            this, &BaseView::editIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::deleteIncidenceSignal,
            this, &BaseView::deleteIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::cutIncidenceSignal,
            this, &BaseView::cutIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::copyIncidenceSignal,
            this, &BaseView::copyIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::pasteIncidenceSignal,
            this, &BaseView::pasteIncidenceSignal);

    connect(mMonthView, &EventViews::EventView::toggleAlarmSignal,
            this, &BaseView::toggleAlarmSignal);

    connect(mMonthView, &EventViews::EventView::toggleTodoCompletedSignal,
            this, &BaseView::toggleTodoCompletedSignal);

    connect(mMonthView, &EventViews::EventView::copyIncidenceToResourceSignal,
            this, &BaseView::copyIncidenceToResourceSignal);

    connect(mMonthView, &EventViews::EventView::moveIncidenceToResourceSignal,
            this, &BaseView::moveIncidenceToResourceSignal);

    connect(mMonthView, &EventViews::EventView::dissociateOccurrencesSignal,
            this, &BaseView::dissociateOccurrencesSignal);

    connect(mMonthView, SIGNAL(newEventSignal()),
            SIGNAL(newEventSignal()));

    connect(mMonthView, SIGNAL(newEventSignal(QDate)),
            SIGNAL(newEventSignal(QDate)));

    connect(mMonthView, SIGNAL(newEventSignal(QDateTime)),
            SIGNAL(newEventSignal(QDateTime)));

    connect(mMonthView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
            SIGNAL(newEventSignal(QDateTime,QDateTime)));

    connect(mMonthView, &EventViews::EventView::newTodoSignal,
            this, &BaseView::newTodoSignal);

    connect(mMonthView, &EventViews::EventView::newSubTodoSignal,
            this, &BaseView::newSubTodoSignal);

    connect(mMonthView, &EventViews::EventView::newJournalSignal,
            this, &BaseView::newJournalSignal);

    connect(mMonthView, &EventViews::MonthView::fullViewChanged,
            this, &MonthView::fullViewChanged);
}

MonthView::~MonthView()
{
}

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

KCalCore::DateList MonthView::selectedIncidenceDates()
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

void MonthView::changeIncidenceDisplay(const Akonadi::Item &item,
                                       Akonadi::IncidenceChanger::ChangeType changeType)
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

void MonthView::setDateRange(const QDateTime &start, const QDateTime &end,
                             const QDate &preferredMonth)
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
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(preferredMonth);
}
