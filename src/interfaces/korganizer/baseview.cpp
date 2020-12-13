/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 1999, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "baseview.h"

#include <KRandom>

using namespace KOrg;

class Q_DECL_HIDDEN BaseView::Private
{
    BaseView *const q;

public:
    explicit Private(BaseView *qq)
        : q(qq)
        , mChanges(EventViews::EventView::IncidencesAdded
                   |EventViews::EventView::DatesChanged)
        , calendar(nullptr)
    {
        QByteArray cname = q->metaObject()->className();
        cname.replace(':', '_');
        identifier = cname + '_' + KRandom::randomString(8).toLatin1();
    }

    ~Private()
    {
    }

    EventViews::EventView::Changes mChanges;
    Akonadi::ETMCalendar::Ptr calendar;
    QByteArray identifier;
    QDateTime startDateTime;
    QDateTime endDateTime;
    QDateTime actualStartDateTime;
    QDateTime actualEndDateTime;
};

BaseView::BaseView(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
}

BaseView::~BaseView()
{
    delete d;
}

void BaseView::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    d->calendar = calendar;
}

CalendarSupport::CalPrinterBase::PrintType BaseView::printType() const
{
    return CalendarSupport::CalPrinterBase::Month;
}

Akonadi::ETMCalendar::Ptr BaseView::calendar()
{
    return d->calendar;
}

QDateTime BaseView::selectionStart()
{
    return QDateTime();
}

QDateTime BaseView::selectionEnd()
{
    return QDateTime();
}

bool BaseView::isEventView()
{
    return false;
}

void BaseView::dayPassed(const QDate &)
{
    updateView();
}

void BaseView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    mChanger = changer;
}

void BaseView::flushView()
{
}

BaseView *BaseView::viewAt(const QPoint &)
{
    return this;
}

void BaseView::updateConfig()
{
}

bool BaseView::hasConfigurationDialog() const
{
    return false;
}

void BaseView::setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth)
{
    d->startDateTime = start;
    d->endDateTime = end;
    showDates(start.date(), end.date(), preferredMonth);
    const QPair<QDateTime, QDateTime> adjusted = actualDateRange(start, end, preferredMonth);
    d->actualStartDateTime = adjusted.first;
    d->actualEndDateTime = adjusted.second;
}

QDateTime BaseView::startDateTime() const
{
    return d->startDateTime;
}

QDateTime BaseView::endDateTime() const
{
    return d->endDateTime;
}

QDateTime BaseView::actualStartDateTime() const
{
    return d->actualStartDateTime;
}

QDateTime BaseView::actualEndDateTime() const
{
    return d->actualEndDateTime;
}

void BaseView::showConfigurationDialog(QWidget *)
{
}

QByteArray BaseView::identifier() const
{
    return d->identifier;
}

void BaseView::setIdentifier(const QByteArray &identifier)
{
    d->identifier = identifier;
}

void BaseView::restoreConfig(const KConfigGroup &configGroup)
{
    doRestoreConfig(configGroup);
}

void BaseView::saveConfig(KConfigGroup &configGroup)
{
    doSaveConfig(configGroup);
}

void BaseView::doRestoreConfig(const KConfigGroup &)
{
}

void BaseView::doSaveConfig(KConfigGroup &)
{
}

void BaseView::clearSelection()
{
}

bool BaseView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    Q_UNUSED(startDt)
    Q_UNUSED(endDt)
    Q_UNUSED(allDay)
    return false;
}

void BaseView::getHighlightMode(bool &highlightEvents, bool &highlightTodos, bool &highlightJournals)
{
    highlightEvents = true;
    highlightTodos = false;
    highlightJournals = false;
}

bool BaseView::usesFullWindow()
{
    return false;
}

bool BaseView::supportsZoom()
{
    return false;
}

bool BaseView::supportsDateRangeSelection()
{
    return true;
}

void BaseView::calendarReset()
{
}

QPair<QDateTime, QDateTime> BaseView::actualDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth) const
{
    Q_UNUSED(preferredMonth)
    return qMakePair(start, end);
}

void BaseView::setChanges(EventViews::EventView::Changes changes)
{
    d->mChanges = changes;
}

EventViews::EventView::Changes BaseView::changes() const
{
    return d->mChanges;
}
