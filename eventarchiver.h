/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 David Faure <faure@kde.org>

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
#ifndef EVENTARCHIVER_H
#define EVENTARCHIVER_H

#include <qobject.h>
#include <qdatetime.h>
#include <libkcal/event.h>

namespace KCal {
class Calendar;
class Event;
}
using namespace KCal;

/**
 * This class handles expiring and archiving of events.
 * It is used directly by the archivedialog, and it is also
 * triggered by actionmanager's timer for auto-archiving.
 *
 * The settings are not held in this class, but directly in KOPrefs (from korganizer.kcfg)
 * Be sure to set mArchiveAction and mArchiveFile before a manual archiving
 * mAutoArchive is used for auto archiving.
 */
class EventArchiver : public QObject
{
    Q_OBJECT

  public:
    EventArchiver( QObject* parent = 0, const char* name = 0 );
    virtual ~EventArchiver();

    /**
     * Delete or archive events once
     * @param calendar the calendar to archive
     * @param limitDate all events *before* the limitDate (not included) will be deleted/archived.
     * @param widget parent widget for message boxes
     * Confirmation and "no events to process" dialogs will be shown
     */
    void runOnce( Calendar* calendar, const QDate& limitDate, QWidget* widget  );

    /**
     * Delete or archive events. This is called regularly, when auto-archiving is enabled
     * @param calendar the calendar to archive
     * @param widget parent widget for message boxes
     * @param withGUI whether this is called from the dialog, so message boxes should be shown.
     * Note that error dialogs like "cannot save" are shown even if from this method, so widget
     * should be set in all cases.
     */
    void runAuto( Calendar* calendar, QWidget* widget, bool withGUI );

  signals:
    void eventsDeleted();

  private:
    void run( Calendar* calendar, const QDate& limitDate, QWidget* widget, bool withGUI, bool errorIfNone );

    void deleteIncidences( Calendar* calendar, const QDate& limitDate, QWidget* widget, const Incidence::List& incidences, bool withGUI );
    void archiveIncidences( Calendar* calendar, const QDate& limitDate, QWidget* widget, const Incidence::List& incidences, bool withGUI );
};

#endif /* EVENTARCHIVER_H */
