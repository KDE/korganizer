/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORGANIZERIFACE_H
#define KORGANIZERIFACE_H

#include <dcopobject.h>


class KOrganizerIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual bool openURL(const QString &url) = 0;
    virtual bool mergeURL(const QString &url) = 0;
    virtual void closeURL() = 0;
    virtual bool saveURL() = 0;
    virtual bool saveAsURL(const QString &url) = 0;
    virtual QString getCurrentURLasString() const = 0;
    virtual bool editIncidence(const QString &uid) = 0;
    virtual bool deleteIncidence(const QString &uid) = 0;
    /**
      Delete the incidence with the given unique ID from the active calendar.
      @param uid The incidence's unique ID.
      @param force If true, all recurrences and sub-todos (if applicable) will
                   be deleted without prompting for confirmation.
    */
    virtual bool deleteIncidence(const QString &uid, bool force) = 0;
    /**
      Add an incidence to the active calendar.
      @param iCal A calendar in iCalendar format containing the incidence. The
                  calendar must consist of a VCALENDAR component which contains
                  the incidence (VEVENT, VTODO, VJOURNAL or VFREEBUSY) and
                  optionally a VTIMEZONE component. If there is more than one
                  incidence, only the first is added to KOrganizer's calendar.
    */
    virtual bool addIncidence(const QString &iCal) = 0;

    virtual void loadProfile( const QString& path ) = 0;
    virtual void saveToProfile( const QString& path ) const = 0;
};

#endif
