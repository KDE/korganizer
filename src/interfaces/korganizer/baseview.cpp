/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 1999, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "baseview.h"

#include <KRandom>

using namespace KOrg;

class KOrg::BaseViewPrivate
{
    BaseView *const q;

public:
    explicit BaseViewPrivate(BaseView *qq)
        : q(qq)
        , mChanges(EventViews::EventView::IncidencesAdded | EventViews::EventView::DatesChanged)
    {
        QByteArray cname = q->metaObject()->className();
        cname.replace(':', '_');
        identifier = cname + '_' + KRandom::randomString(8).toLatin1();
    }

    ~BaseViewPrivate() = default;

    EventViews::EventView::Changes mChanges;
    QAbstractItemModel *model = nullptr;
    Akonadi::EntityTreeModel *etm = nullptr;
    QList<Akonadi::CollectionCalendar::Ptr> calendars;
    QByteArray identifier;
    QDateTime startDateTime;
    QDateTime endDateTime;
    QDateTime actualStartDateTime;
    QDateTime actualEndDateTime;
};

BaseView::BaseView(QWidget *parent)
    : QWidget(parent)
    , d(new BaseViewPrivate(this))
{
}

BaseView::~BaseView() = default;

void BaseView::setModel(QAbstractItemModel *model)
{
    d->model = model;
}

QAbstractItemModel *BaseView::model() const
{
    return d->model;
}

CalendarSupport::CalPrinterBase::PrintType BaseView::printType() const
{
    return CalendarSupport::CalPrinterBase::Month;
}
QDateTime BaseView::selectionStart()
{
    return {};
}

QDateTime BaseView::selectionEnd()
{
    return {};
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

void BaseView::calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    d->calendars.push_back(calendar);
}

void BaseView::calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    d->calendars.removeOne(calendar);
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

Akonadi::CollectionCalendar::Ptr BaseView::calendarForCollection(Akonadi::Collection::Id collectionId) const
{
    const auto cal = std::find_if(d->calendars.cbegin(), d->calendars.cend(), [collectionId](const auto &cal) {
        return cal->collection().id() == collectionId;
    });
    return cal == d->calendars.cend() ? Akonadi::CollectionCalendar::Ptr{} : *cal;
}

Akonadi::CollectionCalendar::Ptr BaseView::calendarForIncidence(const KCalendarCore::Incidence::Ptr &incidence) const
{
    bool ok = false;
    const auto collectionId = incidence->customProperty("VOLATILE", "COLLECTION-ID").toLongLong(&ok);
    if (!ok || collectionId < 0) {
        return {};
    }

    return calendarForCollection(collectionId);
}
#include "moc_baseview.cpp"
