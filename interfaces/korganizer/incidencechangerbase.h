/*
    This file is part of the KOrganizer interfaces.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "korganizer/koglobals.h"
#include <libkcal/scheduler.h>
#include <libkcal/incidence.h>
#include <qobject.h>

class QWidget;
namespace KCal {
  class Calendar;
  class Incidence;
  class ResourceCalendar;
}
using namespace KCal;

namespace KOrg {

class IncidenceChangerBase : public QObject
{
Q_OBJECT
public:
  IncidenceChangerBase( Calendar*cal, QObject *parent = 0 ) :
        QObject( parent ), mCalendar( cal ) {}
  virtual ~IncidenceChangerBase() {}

  virtual bool sendGroupwareMessage( Incidence *incidence,
                                     KCal::Scheduler::Method method,
                                     KOGlobals::HowChanged action,
                                     QWidget *parent ) = 0;

  virtual bool beginChange( Incidence *incidence,
                            ResourceCalendar *res, const QString &subRes ) = 0;
  virtual bool endChange( Incidence *incidence,
                          ResourceCalendar *res, const QString &subRes ) = 0;

  virtual bool addIncidence( Incidence *incidence,
                             ResourceCalendar *res, const QString &subRes,
                             QWidget *parent ) = 0;

  virtual bool changeIncidence( Incidence *oldinc, Incidence *newinc,
                                KOGlobals::WhatChanged, QWidget *parent ) = 0;
  virtual bool deleteIncidence( Incidence *incidence, QWidget *parent ) = 0;

  virtual bool cutIncidences( const Incidence::List &incidences, QWidget *parent ) = 0;
  virtual bool cutIncidence( Incidence *incidence, QWidget *parent ) = 0;

signals:
  void incidenceAdded( Incidence * );
  void incidenceChanged( Incidence *oldInc, Incidence *newInc, KOGlobals::WhatChanged );
  void incidenceToBeDeleted( Incidence * );
  void incidenceDeleted( Incidence * );

  void schedule( Scheduler::Method method, Incidence *incidence );
protected:
  Calendar *mCalendar;
};




}

#endif
