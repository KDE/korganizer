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
*/
#ifndef KOEVENTVIEWER_H
#define KOEVENTVIEWER_H

#include <qtextbrowser.h>

#include <libkcal/event.h>

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

    void setSource( const QString & );
    void setEvent( Event *event );
    void addEvent( Event *event );
    void setTodo( Todo *event );

    void appendEvent( Event *event );
    void appendTodo( Todo *event );
    
    void clearEvents( bool now = false );
    
    void addText( const QString &text );

  protected:
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
