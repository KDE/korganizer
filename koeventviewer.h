/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOEVENTVIEWER_H
#define KOEVENTVIEWER_H

#include <qtextbrowser.h>

namespace KCal {
class Incidence;
class Todo;
class Event;
class Journal;
}
using namespace KCal;


/**
  Viewer widget for events.
*/
class KOEventViewer : public QTextBrowser
{
    Q_OBJECT
  public:
    KOEventViewer( QWidget *parent = 0, const char *name = 0 );
    virtual ~KOEventViewer();

    /**
      Reimplemented from QTextBrowser to handle links.
    */
    void setSource( const QString & );

    /**
      Show given event in viewer. Clear all previously shown incidences.
    */
    void setEvent( Event *event );
    /**
      Show given todo in viewer. Clear all previously shown incidences.
    */
    void setTodo( Todo *event );
    /**
      Show given journal in viewer. Clear all previously shown incidences.
    */
    void setJournal( Journal *journal );

    /**
      Show given event by appending it to already shown incidences.
    */
    void appendEvent( Event *event );
    /**
      Show given todo by appending it to already shown incidences.
    */
    void appendTodo( Todo *event );
    /**
      Show given journal by appending it to already shown incidences.
    */
    void appendJournal( Journal *journal );

    /**
      Clear viewer. If \a now is set to true delete view immediately. If set to
      false delete it with next call to appendIncidence().
    */
    void clearEvents( bool now = false );

    /**
      Add given text to currently shown content.
    */
    void addText( const QString &text );

  protected:
    void linkPerson( const QString& email, QString name, QString uid,
                     const QString& iconPath );
    void addTag( const QString &tag, const QString &text );
    void addLink( const QString &ref, const QString &text,
                  bool newline = true );

    void formatCategories( Incidence * );
    void formatAttendees( Incidence * );
    void formatReadOnly( Incidence * );
    void formatAttachments( Incidence * );

  private:
    QTextBrowser *mEventTextView;

    QString mText;
};

#endif
