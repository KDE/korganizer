/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2010 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kolistview.h"
#include "koeventpopupmenu.h"

#include <EventViews/ListView>

#include <KConfigGroup>

#include <QVBoxLayout>

using namespace KOrg;

KOListView::KOListView(QWidget *parent, bool nonInteractive)
    : KOEventView(parent)
{
    auto layout = new QVBoxLayout(this);
    mListView = new EventViews::ListView(this, nonInteractive);
    mPopupMenu = eventPopup();

    layout->addWidget(mListView);

    connect(mListView, &EventViews::ListView::showIncidencePopupSignal, mPopupMenu, &KOEventPopupMenu::showIncidencePopup);

    connect(mListView, &EventViews::ListView::showNewEventPopupSignal, this, &KOListView::showNewEventPopup);

    connect(mListView, &EventViews::EventView::datesSelected, this, &KOEventView::datesSelected);

    connect(mListView, &EventViews::EventView::shiftedEvent, this, &KOEventView::shiftedEvent);

    connect(mListView, &EventViews::EventView::incidenceSelected, this, &BaseView::incidenceSelected);

    connect(mListView, &EventViews::EventView::showIncidenceSignal, this, &BaseView::showIncidenceSignal);

    connect(mListView, &EventViews::EventView::editIncidenceSignal, this, &BaseView::editIncidenceSignal);

    connect(mListView, &EventViews::EventView::deleteIncidenceSignal, this, &BaseView::deleteIncidenceSignal);

    connect(mListView, &EventViews::EventView::cutIncidenceSignal, this, &BaseView::cutIncidenceSignal);

    connect(mListView, &EventViews::EventView::copyIncidenceSignal, this, &BaseView::copyIncidenceSignal);

    connect(mListView, &EventViews::EventView::pasteIncidenceSignal, this, &BaseView::pasteIncidenceSignal);

    connect(mListView, &EventViews::EventView::toggleAlarmSignal, this, &BaseView::toggleAlarmSignal);

    connect(mListView, &EventViews::EventView::toggleTodoCompletedSignal, this, &BaseView::toggleTodoCompletedSignal);

    connect(mListView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &BaseView::copyIncidenceToResourceSignal);

    connect(mListView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &BaseView::moveIncidenceToResourceSignal);

    connect(mListView, &EventViews::EventView::dissociateOccurrencesSignal, this, &BaseView::dissociateOccurrencesSignal);

    connect(mListView, qOverload<>(&EventViews::EventView::newEventSignal), this, qOverload<>(&KOListView::newEventSignal));

    connect(mListView, qOverload<const QDate &>(&EventViews::EventView::newEventSignal), this, qOverload<const QDate &>(&KOListView::newEventSignal));

    connect(mListView, qOverload<const QDateTime &>(&EventViews::EventView::newEventSignal), this, qOverload<const QDateTime &>(&KOListView::newEventSignal));

    connect(mListView,
            qOverload<const QDateTime &, const QDateTime &>(&EventViews::EventView::newEventSignal),
            this,
            qOverload<const QDateTime &, const QDateTime &>(&KOListView::newEventSignal));

    connect(mListView, &EventViews::EventView::newTodoSignal, this, &BaseView::newTodoSignal);

    connect(mListView, &EventViews::EventView::newSubTodoSignal, this, &BaseView::newSubTodoSignal);

    connect(mListView, &EventViews::EventView::newJournalSignal, this, &BaseView::newJournalSignal);
}

KOListView::~KOListView()
{
    delete mPopupMenu;
    delete mListView;
}

int KOListView::maxDatesHint() const
{
    return 0;
}

int KOListView::currentDateCount() const
{
    return mListView->currentDateCount();
}

Akonadi::Item::List KOListView::selectedIncidences()
{
    return mListView->selectedIncidences();
}

KCalendarCore::DateList KOListView::selectedIncidenceDates()
{
    return mListView->selectedIncidenceDates();
}

void KOListView::updateView()
{
    mListView->updateView();
}

void KOListView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    mListView->showDates(start, end);
}

void KOListView::showAll()
{
    mListView->showAll();
}

void KOListView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    mListView->showIncidences(incidenceList, date);
}

void KOListView::changeIncidenceDisplay(const Akonadi::Item &aitem, Akonadi::IncidenceChanger::ChangeType changeType)
{
    mListView->changeIncidenceDisplay(aitem, changeType);
}

void KOListView::defaultItemAction(const QModelIndex &index)
{
    mListView->defaultItemAction(index);
}

void KOListView::defaultItemAction(const Akonadi::Item::Id id)
{
    mListView->defaultItemAction(id);
}

void KOListView::popupMenu(const QPoint &point)
{
    mListView->popupMenu(point);
}

void KOListView::readSettings(KConfig *config)
{
    mListView->readSettings(config->group(QLatin1String("ListView Layout")));
}

void KOListView::writeSettings(KConfig *config)
{
    auto cfgGroup = config->group(QLatin1String("ListView Layout"));
    mListView->writeSettings(cfgGroup);
}

void KOListView::clearSelection()
{
    mListView->clearSelection();
}

void KOListView::clear()
{
    mListView->clear();
}

CalendarSupport::CalPrinterBase::PrintType KOListView::printType() const
{
    return CalendarSupport::CalPrinterBase::Incidence;
}

QSize KOListView::sizeHint() const
{
    return mListView->sizeHint();
}

void KOListView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mListView->setIncidenceChanger(changer);
}

void KOListView::calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mListView->addCalendar(calendar);
}

void KOListView::calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    mListView->removeCalendar(calendar);
}
#include "moc_kolistview.cpp"
