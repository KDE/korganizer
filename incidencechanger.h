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

namespace KOrg {
  class AkonadiCalendar;
}

class KORGANIZERPRIVATE_EXPORT IncidenceChanger : public KOrg::IncidenceChangerBase
{
  Q_OBJECT
  public:
    explicit IncidenceChanger( KOrg::AkonadiCalendar *cal, QObject *parent=0 );
    ~IncidenceChanger();

    bool beginChange( const Akonadi::Item & incidence );
    bool sendGroupwareMessage( const Akonadi::Item &incidence, KCal::iTIPMethod method,
                               bool deleting = false );
    bool endChange( const Akonadi::Item &incidence );

    bool addIncidence( const KCal::Incidence::Ptr &incidence, QWidget *parent = 0 );
    bool changeIncidence( const KCal::Incidence::Ptr &oldinc, const Akonadi::Item &newItem, int action = -1 );
    bool deleteIncidence( const Akonadi::Item &incidence );

    bool cutIncidence( const Akonadi::Item &incidence );
    static bool incidencesEqual( KCal::Incidence *inc1, KCal::Incidence *inc2 );
    static bool assignIncidence( KCal::Incidence *inc1, KCal::Incidence *inc2 );

  public slots:
    void cancelAttendees( const Akonadi::Item &incidence );

  protected:
    bool myAttendeeStatusChanged( const KCal::Incidence *newInc, const KCal::Incidence *oldInc );

  private Q_SLOTS:
    void addIncidenceFinished( KJob* job );
    void deleteIncidenceFinished( KJob* job );

  private:
    class ComparisonVisitor;
};

#endif
