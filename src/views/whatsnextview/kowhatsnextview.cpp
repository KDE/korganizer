/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kowhatsnextview.h"
#include <QVBoxLayout>

KOWhatsNextView::KOWhatsNextView(QWidget *parent)
    : KOrg::BaseView(parent)
    , mView(new EventViews::WhatsNextView(this))
{
    auto topLayout = new QVBoxLayout(this);
    topLayout->addWidget(mView);

    connect(mView, &EventViews::EventView::incidenceSelected, this, &KOrg::BaseView::incidenceSelected);

    connect(mView, &EventViews::EventView::showIncidenceSignal, this, &KOrg::BaseView::showIncidenceSignal);

    connect(mView, &EventViews::EventView::editIncidenceSignal, this, &KOrg::BaseView::editIncidenceSignal);

    connect(mView, &EventViews::EventView::deleteIncidenceSignal, this, &KOrg::BaseView::deleteIncidenceSignal);

    connect(mView, &EventViews::EventView::cutIncidenceSignal, this, &KOrg::BaseView::cutIncidenceSignal);

    connect(mView, &EventViews::EventView::copyIncidenceSignal, this, &KOrg::BaseView::copyIncidenceSignal);

    connect(mView, &EventViews::EventView::pasteIncidenceSignal, this, &KOrg::BaseView::pasteIncidenceSignal);

    connect(mView, &EventViews::EventView::toggleAlarmSignal, this, &KOrg::BaseView::toggleAlarmSignal);

    connect(mView, &EventViews::EventView::toggleTodoCompletedSignal, this, &KOrg::BaseView::toggleTodoCompletedSignal);

    connect(mView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &KOrg::BaseView::copyIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &KOrg::BaseView::moveIncidenceToResourceSignal);

    connect(mView, &EventViews::EventView::dissociateOccurrencesSignal, this, &KOrg::BaseView::dissociateOccurrencesSignal);

    connect(mView, qOverload<>(&EventViews::WhatsNextView::newEventSignal), this, qOverload<>(&KOWhatsNextView::newEventSignal));

    connect(mView, qOverload<const QDate &>(&EventViews::WhatsNextView::newEventSignal),
            this, qOverload<const QDate &>(&KOWhatsNextView::newEventSignal));

    connect(mView, qOverload<const QDateTime &>(&EventViews::WhatsNextView::newEventSignal),
            this, qOverload<const QDateTime &>(&KOWhatsNextView::newEventSignal));

    connect(mView, qOverload<const QDateTime &, const QDateTime &>(&EventViews::WhatsNextView::newEventSignal),
            this, qOverload<const QDateTime &, const QDateTime &>(&KOWhatsNextView::newEventSignal));

    connect(mView, &EventViews::EventView::newTodoSignal, this, &KOrg::BaseView::newTodoSignal);

    connect(mView, &EventViews::EventView::newSubTodoSignal, this, &KOrg::BaseView::newSubTodoSignal);

    connect(mView, &EventViews::EventView::newJournalSignal, this, &KOrg::BaseView::newJournalSignal);
}

KOWhatsNextView::~KOWhatsNextView()
{
}

int KOWhatsNextView::currentDateCount() const
{
    return mView->currentDateCount();
}

void KOWhatsNextView::updateView()
{
    mView->updateView();
}

void KOWhatsNextView::showDates(const QDate &start, const QDate &end, const QDate &dummy)
{
    mView->showDates(start, end, dummy);
}

void KOWhatsNextView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    mView->showIncidences(incidenceList, date);
}

void KOWhatsNextView::changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType)
{
    updateView();
}

CalendarSupport::CalPrinterBase::PrintType KOWhatsNextView::printType() const
{
    // If up to three days are selected, use day style, otherwise week
    if (currentDateCount() <= 3) {
        return CalendarSupport::CalPrinterBase::Day;
    } else {
        return CalendarSupport::CalPrinterBase::Week;
    }
}

void KOWhatsNextView::setCalendar(const Akonadi::ETMCalendar::Ptr &cal)
{
    KOrg::BaseView::setCalendar(cal);
    mView->setCalendar(cal);
}
