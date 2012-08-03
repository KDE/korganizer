/*
  This file is part of the KOrganizer interfaces.

  Copyright (c) 1999,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "baseview.h"

#include <KRandom>

using namespace KOrg;

class BaseView::Private
{
  BaseView *const q;

  public:
    explicit Private( BaseView *qq )
      : q( qq ),
        mChanges( EventViews::EventView::IncidencesAdded |
                  EventViews::EventView::DatesChanged ),
        calendar( 0 )
    {
      QByteArray cname = q->metaObject()->className();
      cname.replace( ':', '_' );
      identifier = cname + '_' + KRandom::randomString( 8 ).toLatin1();
    }

    ~Private()
    {
    }

    EventViews::EventView::Changes mChanges;
    Akonadi::ETMCalendar::Ptr calendar;
    QByteArray identifier;
    KDateTime startDateTime;
    KDateTime endDateTime;
    KDateTime actualStartDateTime;
    KDateTime actualEndDateTime;
};

BaseView::BaseView( QWidget *parent )
  : QWidget( parent ), mChanger( 0 ), d( new Private( this ) )
{
}

BaseView::~BaseView()
{
  delete d;
}

void BaseView::setCalendar( const Akonadi::ETMCalendar::Ptr &calendar )
{
  if ( d->calendar != calendar ) {
    d->calendar = calendar;
  }
}

CalPrinterBase::PrintType BaseView::printType() const
{
  return CalPrinterBase::Month;
}

Akonadi::ETMCalendar::Ptr BaseView::calendar()
{
  return d->calendar;
}

bool BaseView::isEventView()
{
  return false;
}

void BaseView::dayPassed( const QDate & )
{
  updateView();
}

void BaseView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  mChanger = changer;
}

void BaseView::flushView()
{
}

BaseView *BaseView::viewAt( const QPoint & )
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

void BaseView::setDateRange( const KDateTime &start, const KDateTime &end,
                             const QDate &preferredMonth )
{
  d->startDateTime = start;
  d->endDateTime = end;
  showDates( start.date(), end.date(), preferredMonth );
  const QPair<KDateTime,KDateTime> adjusted = actualDateRange( start, end, preferredMonth );
  d->actualStartDateTime = adjusted.first;
  d->actualEndDateTime = adjusted.second;
}

KDateTime BaseView::startDateTime() const
{
  return d->startDateTime;
}

KDateTime BaseView::endDateTime() const
{
  return d->endDateTime;
}

KDateTime BaseView::actualStartDateTime() const
{
  return d->actualStartDateTime;
}

KDateTime BaseView::actualEndDateTime() const
{
  return d->actualEndDateTime;
}

void BaseView::showConfigurationDialog( QWidget * )
{
}

QByteArray BaseView::identifier() const
{
  return d->identifier;
}

void BaseView::setIdentifier( const QByteArray &identifier )
{
  d->identifier = identifier;
}

void BaseView::restoreConfig( const KConfigGroup &configGroup )
{
  doRestoreConfig( configGroup );
}

void BaseView::saveConfig( KConfigGroup &configGroup )
{
  doSaveConfig( configGroup );
}

void BaseView::doRestoreConfig( const KConfigGroup & )
{
}

void BaseView::doSaveConfig( KConfigGroup & )
{
}

void BaseView::clearSelection()
{
}

bool BaseView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  Q_UNUSED( startDt );
  Q_UNUSED( endDt );
  Q_UNUSED( allDay );
  return false;
}

void BaseView::getHighlightMode( bool &highlightEvents,
                                 bool &highlightTodos,
                                 bool &highlightJournals )
{
  highlightEvents   = true;
  highlightTodos    = false;
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

QPair<KDateTime,KDateTime> BaseView::actualDateRange( const KDateTime &start,
                                                      const KDateTime &end,
                                                      const QDate &preferredMonth ) const
{
  Q_UNUSED( preferredMonth );
  return qMakePair( start, end );
}

void BaseView::setChanges( EventViews::EventView::Changes changes )
{
  d->mChanges = changes;
}

EventViews::EventView::Changes BaseView::changes() const
{
  return d->mChanges;
}

#include "baseview.moc"
