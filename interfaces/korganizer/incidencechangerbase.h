/*
  This file is part of the KOrganizer interfaces.

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
#ifndef KORG_INCIDENCECHANGERBASE_H
#define KORG_INCIDENCECHANGERBASE_H

#include "korganizer/korganizer_export.h"
#include "korganizer/koglobals.h"

#include <KCal/Incidence>
#include <KCal/Scheduler>

#include <QtCore/QObject>

#include <akonadi/kcal/groupware.h>

class QWidget;

namespace Akonadi {
  class Item;
  class Collection;
  class Calendar;
}

namespace KOrg {


class KORGANIZER_INTERFACES_EXPORT IncidenceChangerBase : public QObject
{
  Q_OBJECT
  public:

    enum HowChanged {
      INCIDENCEADDED,
      INCIDENCEEDITED,
      INCIDENCEDELETED,
      NOCHANGE
    };

    enum WhatChanged {
      PRIORITY_MODIFIED,
      COMPLETION_MODIFIED,
      CATEGORY_MODIFIED,
      DATE_MODIFIED,
      RELATION_MODIFIED,
      ALARM_MODIFIED,
      DESCRIPTION_MODIFIED,
      SUMMARY_MODIFIED,
      COMPLETION_MODIFIED_WITH_RECURRENCE,
      RECURRENCE_MODIFIED_ONE_ONLY,
      RECURRENCE_MODIFIED_ALL_FUTURE,
      UNKNOWN_MODIFIED,
      NOTHING_MODIFIED
    };

    explicit IncidenceChangerBase( Akonadi::Calendar *cal, QObject *parent = 0 );

    virtual ~IncidenceChangerBase();

    virtual bool sendGroupwareMessage( const Akonadi::Item &incidence,
                                       KCal::iTIPMethod method,
                                       Akonadi::Groupware::HowChanged action,
                                       QWidget *parent ) = 0;

    virtual bool beginChange( const Akonadi::Item &incidence ) = 0;
    virtual bool endChange( const Akonadi::Item &incidence ) = 0;
    virtual void cancelChange( const Akonadi::Item &incidence ) = 0;

    virtual bool addIncidence( const KCal::Incidence::Ptr &incidence, QWidget *parent ) = 0;
    virtual bool addIncidence( const KCal::Incidence::Ptr &incidence, const Akonadi::Collection &collection, QWidget* parent ) = 0;
    virtual bool changeIncidence( const KCal::Incidence::Ptr &oldinc, const Akonadi::Item &newinc, WhatChanged, QWidget *parent ) = 0;
    virtual bool deleteIncidence( const Akonadi::Item &incidence, QWidget *parent ) = 0;
    virtual bool cutIncidence( const Akonadi::Item &incidence, QWidget *parent ) = 0;

  Q_SIGNALS:
    void incidenceAdded( const Akonadi::Item & );
    void incidenceChanged( const Akonadi::Item &oldinc, const Akonadi::Item &newInc, WhatChanged  );
    void incidenceToBeDeleted( const Akonadi::Item & );
    void incidenceDeleted( const Akonadi::Item & );

    void schedule( iTIPMethod method, const Akonadi::Item &incidence );

  protected:
    Akonadi::Calendar *mCalendar;
};

}

#endif
