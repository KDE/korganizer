/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORGENERALJOURNAL_H
#define KOEDITORGENERALJOURNAL_H

#include <qobject.h>
#include <qdatetime.h>

class KDateEdit;
class KTextEdit;
class QLabel;
class QBoxLayout;
class QWidget;

namespace KCal {
class Incidence;
class Journal;
}
using namespace KCal;

class KOEditorGeneralJournal : public QObject
{
    Q_OBJECT
  public:
    KOEditorGeneralJournal ( QObject* parent=0, const char* name=0 );
    virtual ~KOEditorGeneralJournal();

    void initDate( QWidget *, QBoxLayout * );
    void initDescription( QWidget *, QBoxLayout * );

    /** Set widgets to default values */
    void setDefaults( const QDate &date );
    void setDate( const QDate &date );
    /** Read journal object and setup widgets accordingly */
    void readJournal( Journal *, bool tmpl = false );
    /** Write journal settings to event object */
    void writeJournal( Journal * );

    /** Check if the input is valid. */
    bool validateInput();

    void setDescription( const QString & );
    void finishSetup();

  protected:
    KTextEdit  *mDescriptionEdit;
    QLabel     *mDateLabel;
    KDateEdit  *mDateEdit;
};

#endif
