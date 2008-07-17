/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Barry D Benowitz <b.benowitz@telesciences.com>
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOMAILCLIENT_H
#define KOMAILCLIENT_H

#include <QString>
#include <QByteArray>

class KUrl;
namespace KCal {
class IncidenceBase;
}
using namespace KCal;

class KOMailClient
{
  public:
    KOMailClient();
    virtual ~KOMailClient();

    bool mailAttendees(IncidenceBase *,const QString &attachment=QString());
    bool mailOrganizer(IncidenceBase *, const QString &attachment = QString(), const QString &sub = QString() );
    bool mailTo(IncidenceBase *,const QString &recipients,const QString &attachment=QString());

  protected:
    /** Send mail with specified from, to and subject field and body as text. If
     * bcc is set, send a blind carbon copy to the sender from
         @param from sender of the mail invitation
         @param to receiver of the invitation
         @param subject subject of the invitation (not relevant for
                        scheduling purposes)
         @param body Body of the message
         @param bcc optional bcc for the message
         @param attachment optional attachment (raw data)
     */
    bool send(const QString &from,const QString &to,const QString &subject,
              const QString &body,bool bcc=false,
              const QString &attachment=QString());
};

#endif
