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
#ifndef FREEBUSYMANAGER_H
#define FREEBUSYMANAGER_H

#include <kurl.h>
#include <libkcal/icalformat.h>
#include <libkcal/freebusycache.h>
#include <qstring.h>
#include <qobject.h>

namespace KIO { class Job; }
namespace KCal {
class Calendar;
class FreeBusy;
}
class FreeBusyManager;

/**
 * Class for downloading FreeBusy Lists
 */
class FreeBusyDownloadJob : public QObject
{
    Q_OBJECT
  public:
    FreeBusyDownloadJob( const QString &email, const KURL &url,
                   FreeBusyManager *manager, const char *name = 0 );

    virtual ~FreeBusyDownloadJob();

  protected slots:
    void slotResult( KIO::Job * );
    void slotData(  KIO::Job *, const QByteArray &data );

  signals:
    void freeBusyDownloaded( KCal::FreeBusy *, const QString& );

  private:
    FreeBusyManager *mManager;
    QString mEmail;

    QCString mFreeBusyData;
};

class FreeBusyManager : public QObject, public KCal::FreeBusyCache
{
    Q_OBJECT
  public:
    FreeBusyManager( QObject *parent, const char *name );

    void setCalendar( KCal::Calendar * );

    /// KOrganizer publishes the free/busy list
    void publishFreeBusy();

    /**
      KOrganizer downloads somebody else's free/busy list
      The call is asynchronous, and upon download, the
      receivers slot specified by member will be called.
      The slot should be of type "member(const QString&, KCal::FreeBusy*)"

      Return true if a download is initiated, and false otherwise
    */
    bool retrieveFreeBusy( const QString &email );

    void cancelRetrieval();

    KCal::FreeBusy *iCalToFreeBusy( const QCString &data );

    /**
      Load freebusy information belonging to email.
    */
    KCal::FreeBusy *loadFreeBusy( const QString &email );
    /**
      Store freebusy information belonging to email.
    */
    bool saveFreeBusy( KCal::FreeBusy *freebusy, const KCal::Person &person );
//    bool saveFreeBusy( KCal::FreeBusy *, const QString &email );

    /**
      Return URL of freeBusy information for given email address.
    */
    KURL freeBusyUrl( const QString &email );

    /**
      Return directory used for stroing free/busy information.
    */
    QString freeBusyDir();

  public slots:
    // When something changed in the calendar, we get this called
    void slotPerhapsUploadFB();

  signals:
    /**
      This signal is emitted to return results of free/busy requests.
    */
    void freeBusyRetrieved( KCal::FreeBusy *, const QString &email );

  protected:
    void timerEvent( QTimerEvent* );

    /**
      Return free/busy list of calendar owner as iCalendar string.
    */
    QString ownerFreeBusyAsString();

    /**
      Return free/busy list of calendar owner.
    */
    KCal::FreeBusy *ownerFreeBusy();

    /**
      Convert free/busy object to iCalendar string.
    */
    QString freeBusyToIcal( KCal::FreeBusy * );

  protected slots:
    bool processRetrieveQueue();

  private slots:
    void slotUploadFreeBusyResult( KIO::Job * );

  private:
    KCal::Calendar *mCalendar;
    KCal::ICalFormat mFormat;

    QStringList mRetrieveQueue;

    // Free/Busy uploading
    QDateTime mNextUploadTime;
    int mTimerID;
    bool mUploadingFreeBusy;
};

#endif
