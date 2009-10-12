/*
  This file is part of KOrganizer.
  Copyright (c) 2009 Sebastian Sauer <sebsauer@kdab.net>

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

#ifndef KOEDITORCONFIG_H
#define KOEDITORCONFIG_H

#include "incidenceeditor_export.h"

#include <QtCore/QObject>
#include <QtCore/QDateTime>

/**
 * Configuration details. An application can inherit from this class
 * to provide application specific configurations to the editor.
 *
 * KOrganizer's ActionManager inherits from this class.
 */
class INCIDENCEEDITOR_EXPORT KOEditorConfig : public QObject
{
    Q_OBJECT
  public:
    explicit KOEditorConfig();
    virtual ~KOEditorConfig();

    static KOEditorConfig* instance();
    static void setKOEditorConfig(KOEditorConfig*);

    /// Return the own full name.
    virtual QString fullName() const;
    /// Return the own mail address.
    virtual QString email() const;
    /// Return true if the given email belongs to the user.
    virtual bool thatIsMe( const QString &email ) const;
    /// Returns all email addresses for the user.
    virtual QStringList allEmails() const;
    /// Returns all email addresses together with the full username for the user.
    virtual QStringList fullEmails() const;
    /// Show timezone selectors in the event and todo editor dialog.
    virtual bool showTimeZoneSelectorInIncidenceEditor() const;

    virtual QDateTime defaultDuration() const { return QDateTime(QDate(1752,1,1), QTime(2,0)); }
    virtual QDateTime startTime() const { return QDateTime(QDate(1752,1,1), QTime(10,0)); }
    virtual int reminderTime() const { return 15; }
    virtual int reminderTimeUnits() const { return 0; }
    virtual bool defaultTodoReminders() const { return false; }
    virtual bool defaultEventReminders() const { return false; }

  private:
    class Private;
    Private* const d;
};

#endif
