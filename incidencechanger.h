/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef INCIDENCECHANGER_H
#define INCIDENCECHANGER_H

#include "korganizer/incidencechangerbase.h"
#include "korganizer_export.h"

class KJob;

namespace Akonadi {
  class Calendar;
}

class KORGANIZERPRIVATE_EXPORT IncidenceChanger : public KOrg::IncidenceChangerBase
{
  Q_OBJECT
  public:
    IncidenceChanger( Akonadi::Calendar *cal, QObject *parent );
    ~IncidenceChanger();

    /** Locks the incidence */
    bool beginChange( const Akonadi::Item & incidence );

    /** Commits the change by starting an akonadi modify job
     *  The lock is removed in changeIncidenceFinished()
     */
    bool endChange( const Akonadi::Item &incidence );

    /** Removes the lock in case you don't want to call endchange() anymore */
    void cancelChange( const Akonadi::Item &incidence );

    bool sendGroupwareMessage( const Akonadi::Item &incidence,
                               KCal::iTIPMethod method,
                               Akonadi::Groupware::HowChanged action,
                               QWidget *parent );

    bool addIncidence( const KCal::Incidence::Ptr &incidence, QWidget *parent );
    bool addIncidence( const KCal::Incidence::Ptr &incidence,
                       const Akonadi::Collection &collection, QWidget *parent );
    bool changeIncidence( const KCal::Incidence::Ptr &oldinc, const Akonadi::Item &newItem,
                          KOGlobals::WhatChanged, QWidget *parent );
    bool deleteIncidence( const Akonadi::Item &incidence, QWidget *parent );

    bool cutIncidence( const Akonadi::Item &incidence, QWidget *parent );
    static bool incidencesEqual( KCal::Incidence *inc1, KCal::Incidence *inc2 );
    static bool assignIncidence( KCal::Incidence *inc1, KCal::Incidence *inc2 );

  public slots:
    void cancelAttendees( const Akonadi::Item &incidence );

  protected:
    bool myAttendeeStatusChanged( const KCal::Incidence *newInc, const KCal::Incidence *oldInc );

  private Q_SLOTS:
    void addIncidenceFinished( KJob* job );
    void deleteIncidenceFinished( KJob* job );
    void changeIncidenceFinished( KJob* job );

  private:
    class Private;
    Private * const d;
};

#endif
