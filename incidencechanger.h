/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef INCIDENCECHANGER_H
#define INCIDENCECHANGER_H

#include "korganizer/incidencechangerbase.h"

class IncidenceChanger : public KOrg::IncidenceChangerBase
{
Q_OBJECT
public:
  IncidenceChanger( Calendar*cal, QObject *parent ) : IncidenceChangerBase( cal, parent ) {}
  ~IncidenceChanger() {}

  bool beginChange( Incidence * incidence );
  bool sendGroupwareMessage( Incidence *incidence, KCal::Scheduler::Method method, bool deleting = false );
  bool endChange( Incidence *incidence );

  bool addIncidence( Incidence *incidence, QWidget *parent = 0 );
  bool changeIncidence( Incidence *oldinc, Incidence *newinc, int action = -1 );
  bool deleteIncidence( Incidence *incidence );

  bool cutIncidence( Incidence *incidence );
  static bool incidencesEqual( Incidence *inc1, Incidence *inc2 );
  static bool assignIncidence( Incidence *inc1, Incidence *inc2 );
public slots:
  void cancelAttendees( Incidence *incidence );

protected:
  bool myAttendeeStatusChanged( Incidence *oldInc, Incidence *newInc );

private:
  class ComparisonVisitor;
  class AssignmentVisitor;
};

#endif
