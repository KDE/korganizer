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

#include <libkcal/icalformat.h>
#include <libkcal/freebusy.h>
#include <qstring.h>
#include <qobject.h>

#include <kio/job.h>

namespace KCal {
class Calendar;
}
class FreeBusyManager;

/**
 * Class for downloading FreeBusy Lists
 */
class FBDownloadJob : public QObject
{
    Q_OBJECT
  public:
    FBDownloadJob( const QString &email, const KURL &url,
                   FreeBusyManager *manager, const char *name = 0 );

    virtual ~FBDownloadJob();

  protected slots:
    void slotResult( KIO::Job * );
    void slotData(  KIO::Job *, const QByteArray &data );

  signals:
    void fbDownloaded( const QString&, KCal::FreeBusy * );

  private:
    FreeBusyManager *mManager;
    QString mEmail;

    QCString mFBData;
};

class FreeBusyManager : public QObject
{
    Q_OBJECT
  public:
    FreeBusyManager( QObject *parent, const char *name );

    void setCalendar( KCal::Calendar * );

    /// KOrganizer publishes the free/busy list
    void publishFreeBusy();

    /// Get the free/busy list as a string
    QString getFreeBusyString();

    /**
      KOrganizer downloads somebody else's free/busy list
      The call is asynchronous, and upon download, the
      receivers slot specified by member will be called.
      The slot should be of type "member(const QString&, KCal::FreeBusy*)"

      Return true if a download is initiated, and false otherwise
    */
    bool downloadFreeBusyData( const QString& email, QObject *receiver,
                               const char *member );
    KCal::FreeBusy *parseFreeBusy( const QCString &data );

  public slots:
    // When something changed in the calendar, we get this called
    void slotPerhapsUploadFB();

  private slots:
    void slotUploadFreeBusyResult( KIO::Job * );

  protected:
    void timerEvent( QTimerEvent* );

  private:
    KCal::Calendar *mCalendar;
    KCal::ICalFormat mFormat;

    // Free/Busy uploading
    QDateTime mNextUploadTime;
    int mTimerID;
    bool mUploadingFreeBusy;
};

#endif
