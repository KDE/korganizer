/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef MAILSCHEDULER_H
#define MAILSCHEDULER_H
//
// Mail implementation of iTIP methods
//

#include <qptrlist.h>
#include <qmap.h>
#include <qstring.h>

#include <libkcal/imipscheduler.h>

namespace KCal {

/*
  This class implements the iTIP interface using the email interface specified
  as Mail.
*/
class MailScheduler : public IMIPScheduler {
  public:
    MailScheduler(Calendar *);
    virtual ~MailScheduler();

    bool publish (IncidenceBase *incidence,const QString &recipients);
    bool performTransaction(IncidenceBase *incidence,Method method);
    bool performTransaction(IncidenceBase *incidence,Method method,const QString &recipients);
    QPtrList<ScheduleMessage> retrieveTransactions();

    bool deleteTransaction(IncidenceBase *incidence);

  private:
    QMap<IncidenceBase*, QString> mEventMap;
};

}

#endif
