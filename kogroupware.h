/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002 Klarälvdalens Datakonsult AB

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KOGROUPWARE_H
#define KOGROUPWARE_H

#include <libkcal/icalformat.h>
#include <libkcal/freebusy.h>
#include <qstring.h>
#include <qobject.h>

#include <kio/job.h>

using namespace KCal;

namespace KCal {
class Calendar;
class Event;
}
class CalendarView;
class KOGroupware;

/**
 * Class for downloading FreeBusy Lists
 */
class FBDownloadJob : public QObject {
  Q_OBJECT
public:
  FBDownloadJob( const QString& email, const KURL& url, KOGroupware* kogroupware, const char* name = 0 );

  virtual ~FBDownloadJob();
protected slots:
  void slotResult( KIO::Job* );
  void slotData(  KIO::Job*, const QByteArray &data );
signals:
  void fbDownloaded( const QString&, FreeBusy*);

private:
  KOGroupware* mKogroupware;
  QString  mEmail;

  QCString mFBData;
};

class KOGroupware : public QObject
{
  Q_OBJECT
public:
    static KOGroupware* create( CalendarView*, KCal::Calendar* );
    static KOGroupware* instance();

    /** Send iCal messages after asking the user
         Returns false if the user cancels the dialog, and true if the
         user presses Yes og or No.
    */
    bool sendICalMessage( QWidget* parent, KCal::Scheduler::Method method,
                          Incidence* incidence, bool isDeleting = false );

    // THIS IS THE ACTUAL KM/KO API
    enum EventState { Accepted, ConditionallyAccepted, Declined, Request };

    // Event initiated by somebody else, coming into KO from KM, returning
    // resulting state
    bool incomingEventRequest( const QString& request,
                               const QCString& receiver,
                               const QString& vCalIn );
    void incomingResourceRequest( const QValueList<QPair<QDateTime, QDateTime> >& busy,
                                  const QCString& resource,
                                  const QString& vCalIn,
                                  bool& vCalInOK,
                                  QString& vCalOut,
                                  bool& vCalOutOK,
                                  bool& isFree,
                                  QDateTime& start, QDateTime& end );

    // ANswer to invitation
    bool incidenceAnswer( const QCString& sender, const QString& vCalIn, QString& vCalOut );

    // END OF THE ACTUAL KM/KO API

    // KOrganizer publishes the free/busy list
    void publishFreeBusy();

    // Get the free/busy list as a string
    QString getFreeBusyString();

    /** KOrganizer downloads somebody else's free/busy list
        The call is asynchronous, and upon download, the
        receivers slot specified by member will be called.
        The slot should be of type "member(const QString&, KCal::FreeBusy*)"

        Return true if a download is initiated, and false otherwise
    */
  /*KCal::FreeBusy**/ bool downloadFreeBusyData( const QString& email, QObject* receiver, const char* member );
    KCal::FreeBusy* parseFreeBusy( const QCString& data );

protected:
    KOGroupware( CalendarView*, KCal::Calendar* );

private:
    static KOGroupware* mInstance;
    KCal::ICalFormat mFormat;
    CalendarView* mView;
    KCal::Calendar* mCalendar;
};

#endif
