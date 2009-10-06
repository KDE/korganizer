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
#include <kcal/scheduler.h>
#include <QtCore/QObject>

class QWidget;
namespace KCal {
  class Incidence;
}
using namespace KCal;

namespace KOrg {

class CalendarBase;

class KORGANIZER_INTERFACES_EXPORT IncidenceChangerBase : public QObject
{
  Q_OBJECT
  public:
    explicit IncidenceChangerBase( CalendarBase *cal, QObject *parent = 0 );

    virtual ~IncidenceChangerBase();

    virtual bool sendGroupwareMessage( Incidence *incidence,
                                       iTIPMethod method, bool deleting = false ) = 0;

    virtual bool beginChange( Incidence * incidence ) = 0;
    virtual bool endChange( Incidence *incidence ) = 0;

    virtual bool addIncidence( Incidence *incidence, QWidget *parent = 0 ) = 0;
    virtual bool changeIncidence( Incidence *oldinc, Incidence *newinc,
                                  int action = -1 ) = 0;
    virtual bool deleteIncidence( Incidence *incidence ) = 0;
    virtual bool cutIncidence( Incidence *incidence ) = 0;

/*
    static bool incidencesEqual( Incidence *inc1, Incidence *inc2 );
    static bool assignIncidence( Incidence *inc1, Incidence *inc2 );
*/

  Q_SIGNALS:
    void incidenceAdded( Incidence * );
    void incidenceChanged( Incidence *oldInc, Incidence *newInc, int );
    void incidenceChanged( Incidence *oldInc, Incidence *newInc );
    void incidenceToBeDeleted( Incidence * );
    void incidenceDeleted( Incidence * );

    void schedule( iTIPMethod method, Incidence *incidence );

  protected:
    CalendarBase *mCalendar;
};

}

#endif
