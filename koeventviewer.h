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
#include <kdepimmacros.h>

#include <kconfig.h>
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
class KDE_EXPORT KOEventViewer : public QTextBrowser
{
    Q_OBJECT
  public:
    KOEventViewer( QWidget *parent = 0, const char *name = 0 );
    virtual ~KOEventViewer();

    /**
      Reimplemented from QTextBrowser to handle links.
    */
    void setSource( const QString & );
    
    virtual bool appendIncidence( Incidence * );
    
    /**
      Clear viewer. If \a now is set to true delete view immediately. If set to
      false delete it with next call to appendIncidence().
    */
    void clearEvents( bool now = false );

    /**
      Add given text to currently shown content.
    */    
  
    void addText( const QString &text );
   
    /**
      Set the default text that is showed when 
      there aren't a incidence to show
    */
    void setDefaultText( const QString &text );
    
    void readSettings( KConfig *config);
    void writeSettings ( KConfig *config);
    
  public slots:
    /**
      Show given incidence in viewer. Clear all previously shown incidences.
    */
    virtual void setIncidence( Incidence * );
    void changeIncidenceDisplay( Incidence *incidence, int action );
  private:
    Incidence *mIncidence;
    QTextBrowser *mEventTextView;
    QString mDefaultText;
    QString mText;
};

#endif
