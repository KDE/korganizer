/*
  This file is part of the kcal library.

  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KORG_HTMLEXPORTJOB_H
#define KORG_HTMLEXPORTJOB_H

#include <kjob.h>

#include <KCalCore/Event>
#include <KCalCore/Todo>

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QTextStream>

class QTextStream;

namespace KCalCore {
  class Calendar;
}

namespace CalendarSupport {
  class Calendar;
}

namespace KOrg {

class HTMLExportSettings;

/**
  This class provides the functions to export a calendar as a HTML page.
*/
class HtmlExportJob : public KJob
{
  Q_OBJECT

  public:
    /**
      Create new HTML exporter for calendar.
    */
    HtmlExportJob( CalendarSupport::Calendar *calendar, HTMLExportSettings *settings, QWidget *parent = 0 );
    virtual ~HtmlExportJob();

    void addHoliday( const QDate &date, const QString &name );

    virtual void start();
    HTMLExportSettings* settings() const;

  protected:
    void createWeekView( QTextStream *ts );
    void createMonthView( QTextStream *ts );
    void createEventList( QTextStream *ts );
    void createTodoList( QTextStream *ts );
    void createJournalView( QTextStream *ts );
    void createFreeBusyView( QTextStream *ts );

    void createTodo( QTextStream *ts, const KCalCore::Todo::Ptr &todo );
    void createEvent( QTextStream *ts, const KCalCore::Event::Ptr &event, QDate date,
                      bool withDescription = true );
    void createFooter( QTextStream *ts );

    bool checkSecrecy( const KCalCore::Incidence::Ptr &incidence );

    void formatLocation( QTextStream *ts, const KCalCore::Incidence::Ptr &incidence );
    void formatCategories( QTextStream *ts, const KCalCore::Incidence::Ptr &incidence );
    void formatAttendees( QTextStream *ts, const KCalCore::Incidence::Ptr &incidence );

    QString breakString( const QString &text );

    QDate fromDate() const;
    QDate toDate() const;
    QString styleSheet() const;

  private Q_SLOTS:
    void receivedOrganizerInfo( KJob* );

  private:
    /**
      Writes out the calendar in HTML format.
    */
    bool save( const QString &fileName = QString() );

    /**
      Writes out calendar to text stream.
    */
    bool save( QTextStream *ts );

    void finishExport();

    //@cond PRIVATE
    Q_DISABLE_COPY( HtmlExportJob )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
