/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef _KOTODOEDITOR_H
#define _KOTODOEDITOR_H

#include "koincidenceeditor.h"

class QDateTime;
class KOEditorGeneralTodo;
class KOEditorRecurrence;
/**
  This class provides a dialog for editing a Todo.
*/
class KOTodoEditor : public KOIncidenceEditor
{
    Q_OBJECT
  public:
    /**
      Constructs a new todo editor.
    */
    KOTodoEditor( Calendar *calendar, QWidget *parent );
    virtual ~KOTodoEditor();

    void init();

    void reload();

    /**
      Clear editor for new todo, and preset the dates and times with hint.
    */
    void newTodo( const QDateTime &due, Todo *relatedTodo=0, bool allDay=false);

    /**
      Edit new todo. Set summary and description from given text.
    */
    void newTodo( const QString & );
    /**
      Edit new todo.
    */
    void newTodo( const QString &summary, const QString &description,
                  const QString &attachment );
    /**
      Edit new todo.
    */
    void newTodo( const QString &summary, const QString &description,
                  const QString &attachment, const QStringList &attendees );

    /** Edit an existing todo. */
    void editIncidence(Incidence *);

    /** Set widgets to default values */
    void setDefaults( const QDateTime &due, Todo *relatedTodo, bool allDay );
    /** Read event object and setup widgets accordingly */
    void readTodo(Todo *);
    /** Write event settings to event object */
    void writeTodo(Todo *);

    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();

    /** This todo has been modified externally */
    void modified (int change=0);

  protected slots:
    void loadDefaults();
    void deleteTodo();

    void slotLoadTemplate();
    void saveTemplate( const QString & );

  protected:
    QString type() { return "ToDo"; }
    void setupGeneral();
    void setupRecurrence();
    int msgItemDelete();

  private:
    Todo *mTodo;

    Todo *mRelatedTodo;

    KOEditorGeneralTodo *mGeneral;
    KOEditorRecurrence *mRecurrence;
};

#endif
