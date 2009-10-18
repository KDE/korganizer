/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef MAILSCHEDULER_H
#define MAILSCHEDULER_H

#include <QMap>
#include <KCal/Scheduler>

namespace KCal {
  class IncidenceBase;
  class Incidence;
  class ICalFormat;
}

namespace KOrg {
  class AkonadiCalendar;
  class CalendarBase;

  /*
    This class implements the iTIP interface using the email interface specified
    as Mail.
  */
  class MailScheduler //: public Scheduler
  {
    public:
      explicit MailScheduler( KOrg::AkonadiCalendar *calendar );
      virtual ~MailScheduler();

      bool publish ( KCal::IncidenceBase *incidence, const QString &recipients );
      bool performTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method );
      bool performTransaction( KCal::IncidenceBase *incidence, KCal::iTIPMethod method, const QString &recipients );

#ifdef AKONADI_PORT_DISABLED
      QList<ScheduleMessage*> retrieveTransactions();
      bool deleteTransaction( IncidenceBase *incidence );
      /** Returns the directory where the free-busy information is stored */
      virtual QString freeBusyDir();
#endif

      /** Accepts a counter proposal */
      bool acceptCounterProposal( KCal::Incidence *incidence );

    private:
      KOrg::AkonadiCalendar *mCalendar;
      KCal::ICalFormat *mFormat;
      QMap<KCal::IncidenceBase *, QString> mEventMap;
  };

}

#endif
