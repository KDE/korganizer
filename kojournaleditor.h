/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOJOURNALEDITOR_H
#define KOJOURNALEDITOR_H

#include "koincidenceeditor.h"

#include <qdatetime.h>

class QDateTime;
namespace KCal {
class Calendar;
class Journal;
class Incidence;
}
using namespace KCal;

class KOEditorGeneralJournal;

/**
  This class provides a dialog for editing a Journal.
*/
class KOJournalEditor : public KOIncidenceEditor
{
    Q_OBJECT
  public:
    /**
      Constructs a new Journal editor.
    */
    KOJournalEditor( Calendar *calendar, QWidget *parent );
    virtual ~KOJournalEditor();

    void init();

    void reload();

    /**
      Clear editor for new Journal
    */
    void newJournal();

    /**
      Sets the given summary and description. If description is empty and the
      summary contains multiple lines, the summary will be used as description
      and only the first line of summary will be used as the summary.
    */
    void setTexts( const QString &summary, const QString &description = QString::null );
    /** Edit an existing Journal. */
    void editIncidence(Incidence *, Calendar *);

    /** Set widgets to default values */
    void setDate( const QDate &date );
    /** Read event object and setup widgets accordingly */
    void readJournal( Journal * );
    /** Write event settings to event object */
    void writeJournal( Journal * );

    int msgItemDelete();
    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();

    /** This Journal has been modified externally */
    void modified (int change=0);

  protected slots:
    void loadDefaults();
    void deleteJournal();

    void slotSaveTemplate( const QString & );

  protected:
    QString type() { return "Journal"; }
    void setupGeneral();
//    int msgItemDelete();

    void loadTemplate( /*const*/ CalendarLocal& );
    QStringList& templates() const;
  private:
    Journal *mJournal;
    KOEditorGeneralJournal *mGeneral;
};

#endif
