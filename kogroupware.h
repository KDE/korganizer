/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA  02111-1307, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#ifndef KOGROUPWARE_H
#define KOGROUPWARE_H

#include <libkcal/icalformat.h>
#include <libkcal/scheduler.h>
#include <qstring.h>

#include <kio/job.h>

using namespace KCal;

namespace KCal {
class Calendar;
class Event;
}
class CalendarView;
class FreeBusyManager;

class KOGroupware : public QObject
{
    Q_OBJECT
  public:
    static KOGroupware* create( CalendarView*, KCal::Calendar* );
    static KOGroupware* instance();

    FreeBusyManager *freeBusyManager();

    /** Send iCal messages after asking the user
         Returns false if the user cancels the dialog, and true if the
         user presses Yes og or No.
    */
    bool sendICalMessage( QWidget* parent, KCal::Scheduler::Method method,
                          Incidence* incidence, bool isDeleting = false,
                          bool statusChanged = false );

    // THIS IS THE ACTUAL KM/KO API
    enum EventState { Accepted, ConditionallyAccepted, Declined, Request };

  private slots:
    /** Handle iCals given by KMail. */
    void incomingDirChanged( const QString& path );

  protected:
    KOGroupware( CalendarView*, KCal::Calendar* );

  private:
    static KOGroupware *mInstance;
    KCal::ICalFormat mFormat;
    CalendarView *mView;
    KCal::Calendar *mCalendar;
    static FreeBusyManager *mFreeBusyManager;
};

#endif
