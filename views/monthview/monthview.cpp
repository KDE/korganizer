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

class MonthView::Private
{
public:
    Private(MonthView *qq) : q(qq)
    {
        QVBoxLayout *layout = new QVBoxLayout(q);
        layout->setMargin(0);
        mMonthView = new EventViews::MonthView(EventViews::MonthView::Visible, q);
        mMonthView->setPreferences(KOPrefs::instance()->eventViewsPreferences());
        layout->addWidget(mMonthView);
        mPopup = q->eventPopup();
    }

    EventViews::MonthView *mMonthView;
    KOEventPopupMenu *mPopup;

private:
    MonthView *q;
};

MonthView::MonthView(QWidget *parent)
    : KOEventView(parent), d(new Private(this))
{
    connect(d->mMonthView, &EventViews::MonthView::showIncidencePopupSignal, d->mPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(d->mMonthView, &EventViews::MonthView::showNewEventPopupSignal, this, &MonthView::showNewEventPopup);

    connect(d->mMonthView, &EventViews::EventView::datesSelected,
            this, &KOEventView::datesSelected);

    connect(d->mMonthView, &EventViews::EventView::shiftedEvent,
            this, &KOEventView::shiftedEvent);

    connect(d->mMonthView, &EventViews::EventView::incidenceSelected,
            this, &BaseView::incidenceSelected);

    connect(d->mMonthView, &EventViews::EventView::showIncidenceSignal,
            this, &BaseView::showIncidenceSignal);

    connect(d->mMonthView, &EventViews::EventView::editIncidenceSignal,
            this, &BaseView::editIncidenceSignal);

    connect(d->mMonthView, &EventViews::EventView::deleteIncidenceSignal,
            this, &BaseView::deleteIncidenceSignal);

    connect(d->mMonthView, &EventViews::EventView::cutIncidenceSignal,
            this, &BaseView::cutIncidenceSignal);

    connect(d->mMonthView, &EventViews::EventView::copyIncidenceSignal,
            this, &BaseView::copyIncidenceSignal);

    connect(d->mMonthView, &EventViews::EventView::pasteIncidenceSignal,
            this, &BaseView::pasteIncidenceSignal);

    connect(d->mMonthView, &EventViews::EventView::toggleAlarmSignal,
            this, &BaseView::toggleAlarmSignal);

    connect(d->mMonthView, &EventViews::EventView::toggleTodoCompletedSignal,
            this, &BaseView::toggleTodoCompletedSignal);

    connect(d->mMonthView, &EventViews::EventView::copyIncidenceToResourceSignal,
            this, &BaseView::copyIncidenceToResourceSignal);

    connect(d->mMonthView, &EventViews::EventView::moveIncidenceToResourceSignal,
            this, &BaseView::moveIncidenceToResourceSignal);

    connect(d->mMonthView, &EventViews::EventView::dissociateOccurrencesSignal,
            this, &BaseView::dissociateOccurrencesSignal);

    connect(d->mMonthView, SIGNAL(newEventSignal()),
            SIGNAL(newEventSignal()));

    connect(d->mMonthView, SIGNAL(newEventSignal(QDate)),
            SIGNAL(newEventSignal(QDate)));

    connect(d->mMonthView, SIGNAL(newEventSignal(QDateTime)),
            SIGNAL(newEventSignal(QDateTime)));

    connect(d->mMonthView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
            SIGNAL(newEventSignal(QDateTime,QDateTime)));

    connect(d->mMonthView, &EventViews::EventView::newTodoSignal,
            this, &BaseView::newTodoSignal);

    connect(d->mMonthView, &EventViews::EventView::newSubTodoSignal,
            this, &BaseView::newSubTodoSignal);

    connect(d->mMonthView, &EventViews::EventView::newJournalSignal,
            this, &BaseView::newJournalSignal);

    connect(d->mMonthView, &EventViews::MonthView::fullViewChanged,
            this, &MonthView::fullViewChanged);
}

MonthView::~MonthView()
{
    delete d;
}

CalendarSupport::CalPrinterBase::PrintType MonthView::printType() const
{
    return CalendarSupport::CalPrinterBase::Month;
}

int MonthView::currentDateCount() const
{
    return d->mMonthView->currentDateCount();
}

int MonthView::currentMonth() const
{
    return d->mMonthView->currentMonth();
}

KCalCore::DateList MonthView::selectedIncidenceDates()
{
    return d->mMonthView->selectedIncidenceDates();
}

QDateTime MonthView::selectionStart()
{
    return d->mMonthView->selectionStart();
}

QDateTime MonthView::selectionEnd()
{
    return d->mMonthView->selectionEnd();
}

bool MonthView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    return d->mMonthView->eventDurationHint(startDt, endDt, allDay);
}

QDate MonthView::averageDate() const
{
    return d->mMonthView->averageDate();
}

bool MonthView::usesFullWindow()
{
    return d->mMonthView->usesFullWindow();
}

bool MonthView::supportsDateRangeSelection()
{
    return d->mMonthView->supportsDateRangeSelection();
}

void MonthView::updateView()
{
    d->mMonthView->updateView();
}

void MonthView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    d->mMonthView->showIncidences(incidenceList, date);
}

void MonthView::changeIncidenceDisplay(const Akonadi::Item &item,
                                       Akonadi::IncidenceChanger::ChangeType changeType)
{
    d->mMonthView->changeIncidenceDisplay(item, changeType);
}

void MonthView::updateConfig()
{
    d->mMonthView->updateConfig();
}

int MonthView::maxDatesHint() const
{
    return 6 * 7;
}

Akonadi::Item::List MonthView::selectedIncidences()
{
    return d->mMonthView->selectedIncidences();
}

void MonthView::setTypeAheadReceiver(QObject *o)
{
    d->mMonthView->setTypeAheadReceiver(o);
}

void MonthView::setDateRange(const KDateTime &start, const KDateTime &end,
                             const QDate &preferredMonth)
{
    d->mMonthView->setDateRange(start, end, preferredMonth);
}

void MonthView::setCalendar(const Akonadi::ETMCalendar::Ptr &cal)
{
    KOEventView::setCalendar(cal);
    d->mPopup->setCalendar(cal);
    d->mMonthView->setCalendar(cal);
}

void MonthView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    d->mMonthView->setIncidenceChanger(changer);
}

void MonthView::showDates(const QDate &start, const QDate &end, const QDate &preferredMonth)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(preferredMonth);
}

