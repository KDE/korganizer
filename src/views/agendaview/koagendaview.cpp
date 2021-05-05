/*
  This file is part of KOrganizer.
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koagendaview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"

#include <EventViews/AgendaView>

#include <QHBoxLayout>

class KOAgendaView::Private
{
public:
    Private(bool isSideBySide, KOAgendaView *parent)
        : q(parent)
    {
        mAgendaView =
            new EventViews::AgendaView(KOPrefs::instance()->eventViewsPreferences(), QDate::currentDate(), QDate::currentDate(), true, isSideBySide, parent);
        mPopup = q->eventPopup();
    }

    ~Private()
    {
        delete mAgendaView;
        delete mPopup;
    }

    EventViews::AgendaView *mAgendaView = nullptr;
    KOEventPopupMenu *mPopup = nullptr;

private:
    KOAgendaView *const q;
};

KOAgendaView::KOAgendaView(QWidget *parent, bool isSideBySide)
    : KOEventView(parent)
    , d(new Private(isSideBySide, this))
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(d->mAgendaView);

    connect(d->mAgendaView, &EventViews::AgendaView::zoomViewHorizontally, this, &KOAgendaView::zoomViewHorizontally);

    connect(d->mAgendaView, &EventViews::AgendaView::timeSpanSelectionChanged, this, &KOAgendaView::timeSpanSelectionChanged);

    connect(d->mAgendaView, &EventViews::AgendaView::showIncidencePopupSignal, d->mPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(d->mAgendaView, &EventViews::AgendaView::showNewEventPopupSignal, this, &KOAgendaView::showNewEventPopup);

    connect(d->mAgendaView, &EventViews::EventView::datesSelected, this, &KOEventView::datesSelected);

    connect(d->mAgendaView, &EventViews::EventView::shiftedEvent, this, &KOEventView::shiftedEvent);

    connect(d->mAgendaView, &EventViews::EventView::incidenceSelected, this, &KOrg::BaseView::incidenceSelected);

    connect(d->mAgendaView, &EventViews::EventView::showIncidenceSignal, this, &KOrg::BaseView::showIncidenceSignal);

    connect(d->mAgendaView, &EventViews::EventView::editIncidenceSignal, this, &KOrg::BaseView::editIncidenceSignal);

    connect(d->mAgendaView, &EventViews::EventView::deleteIncidenceSignal, this, &KOrg::BaseView::deleteIncidenceSignal);

    connect(d->mAgendaView, &EventViews::EventView::cutIncidenceSignal, this, &KOrg::BaseView::cutIncidenceSignal);

    connect(d->mAgendaView, &EventViews::EventView::copyIncidenceSignal, this, &KOrg::BaseView::copyIncidenceSignal);

    connect(d->mAgendaView, &EventViews::EventView::pasteIncidenceSignal, this, &KOrg::BaseView::pasteIncidenceSignal);

    connect(d->mAgendaView, &EventViews::EventView::toggleAlarmSignal, this, &KOrg::BaseView::toggleAlarmSignal);

    connect(d->mAgendaView, &EventViews::EventView::toggleTodoCompletedSignal, this, &KOrg::BaseView::toggleTodoCompletedSignal);

    connect(d->mAgendaView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &KOrg::BaseView::copyIncidenceToResourceSignal);

    connect(d->mAgendaView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &KOrg::BaseView::moveIncidenceToResourceSignal);

    connect(d->mAgendaView, &EventViews::EventView::dissociateOccurrencesSignal, this, &KOrg::BaseView::dissociateOccurrencesSignal);

    connect(d->mAgendaView, qOverload<>(&EventViews::AgendaView::newEventSignal), this, qOverload<>(&KOAgendaView::newEventSignal));

    connect(d->mAgendaView, qOverload<const QDate &>(&EventViews::AgendaView::newEventSignal),
            this, qOverload<const QDate &>(&KOAgendaView::newEventSignal));

    connect(d->mAgendaView, qOverload<const QDateTime &>(&EventViews::AgendaView::newEventSignal),
            this, qOverload<const QDateTime &>(&KOAgendaView::newEventSignal));

    connect(d->mAgendaView, qOverload<const QDateTime &, const QDateTime &>(&EventViews::AgendaView::newEventSignal),
            this, qOverload<const QDateTime &, const QDateTime &>(&KOAgendaView::newEventSignal));

    connect(d->mAgendaView, &EventViews::EventView::newTodoSignal, this, &KOrg::BaseView::newTodoSignal);

    connect(d->mAgendaView, &EventViews::EventView::newSubTodoSignal, this, &KOrg::BaseView::newSubTodoSignal);

    connect(d->mAgendaView, &EventViews::EventView::newJournalSignal, this, &KOrg::BaseView::newJournalSignal);

    d->mAgendaView->show();
}

KOAgendaView::~KOAgendaView()
{
    delete d;
}

void KOAgendaView::setCalendar(const Akonadi::ETMCalendar::Ptr &cal)
{
    KOEventView::setCalendar(cal);
    d->mPopup->setCalendar(cal);
    d->mAgendaView->setCalendar(cal);
}

void KOAgendaView::zoomInVertically()
{
    d->mAgendaView->zoomInVertically();
}

void KOAgendaView::zoomOutVertically()
{
    d->mAgendaView->zoomOutVertically();
}

void KOAgendaView::zoomInHorizontally(QDate date)
{
    d->mAgendaView->zoomInHorizontally(date);
}

void KOAgendaView::zoomOutHorizontally(QDate date)
{
    d->mAgendaView->zoomOutHorizontally(date);
}

void KOAgendaView::zoomView(const int delta, const QPoint &pos, const Qt::Orientation orient)
{
    d->mAgendaView->zoomView(delta, pos, orient);
}

void KOAgendaView::enableAgendaUpdate(bool enable)
{
    d->mAgendaView->enableAgendaUpdate(enable);
}

int KOAgendaView::maxDatesHint() const
{
    // Not sure about the max number of events, so return 0 for now.
    return 0;
}

int KOAgendaView::currentDateCount() const
{
    return d->mAgendaView->currentDateCount();
}

Akonadi::Item::List KOAgendaView::selectedIncidences()
{
    return d->mAgendaView->selectedIncidences();
}

KCalendarCore::DateList KOAgendaView::selectedIncidenceDates()
{
    return d->mAgendaView->selectedIncidenceDates();
}

bool KOAgendaView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    return d->mAgendaView->eventDurationHint(startDt, endDt, allDay);
}

/** returns if only a single cell is selected, or a range of cells */
bool KOAgendaView::selectedIsSingleCell()
{
    return d->mAgendaView->selectedIsSingleCell();
}

void KOAgendaView::updateView()
{
    d->mAgendaView->updateView();
}

void KOAgendaView::updateConfig()
{
    d->mAgendaView->updateConfig();
}

void KOAgendaView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    d->mAgendaView->showDates(start, end);
}

void KOAgendaView::showIncidences(const Akonadi::Item::List &incidences, const QDate &date)
{
    d->mAgendaView->showIncidences(incidences, date);
}

void KOAgendaView::changeIncidenceDisplayAdded(const Akonadi::Item &)
{
    // Do nothing, EventViews::AgendaView knows when items change
}

void KOAgendaView::changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType)
{
    // Do nothing, EventViews::AgendaView knows when items change
}

CalendarSupport::CalPrinter::PrintType KOAgendaView::printType() const
{
    // If up to three days are selected, use day style, otherwise week
    if (currentDateCount() <= 3) {
        return CalendarSupport::CalPrinter::Day;
    } else {
        return CalendarSupport::CalPrinter::Week;
    }
}

void KOAgendaView::readSettings()
{
    d->mAgendaView->readSettings();
}

void KOAgendaView::readSettings(KConfig *config)
{
    d->mAgendaView->readSettings(config);
}

void KOAgendaView::writeSettings(KConfig *config)
{
    d->mAgendaView->writeSettings(config);
}

void KOAgendaView::clearSelection()
{
    d->mAgendaView->clearSelection();
}

void KOAgendaView::deleteSelectedDateTime()
{
    d->mAgendaView->deleteSelectedDateTime();
}

void KOAgendaView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    d->mAgendaView->setIncidenceChanger(changer);
}

QDateTime KOAgendaView::selectionStart()
{
    return d->mAgendaView->selectionStart();
}

QDateTime KOAgendaView::selectionEnd()
{
    return d->mAgendaView->selectionEnd();
}

bool KOAgendaView::selectedIsAllDay()
{
    return d->mAgendaView->selectedIsAllDay();
}

void KOAgendaView::setTypeAheadReceiver(QObject *o)
{
    d->mAgendaView->setTypeAheadReceiver(o);
}

void KOAgendaView::setChanges(EventViews::EventView::Changes changes)
{
    // Only ConfigChanged and FilterChanged should go from korg->AgendaView
    // All other values are already detected inside AgendaView.
    // We could just pass "changes", but korganizer does a very bad job at
    // determining what changed, for example if you move an incidence
    // the BaseView::setDateRange(...) is called causing DatesChanged
    // flag to be on, when no dates changed.
    EventViews::EventView::Changes c;
    if (changes.testFlag(EventViews::EventView::ConfigChanged)) {
        c = EventViews::EventView::ConfigChanged;
    }

    if (changes.testFlag(EventViews::EventView::FilterChanged)) {
        c |= EventViews::EventView::FilterChanged;
    }

    d->mAgendaView->setChanges(c | d->mAgendaView->changes());
}

void KOAgendaView::setDateRange(const QDateTime &start, const QDateTime &end, const QDate &)
{
    d->mAgendaView->setDateRange(start, end);
}
