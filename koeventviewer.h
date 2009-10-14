/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
#ifndef KOEVENTVIEWER_H
#define KOEVENTVIEWER_H

#include "korganizer_export.h"
#include <KTextBrowser>

class KConfig;
class QUrl;

namespace Akonadi {
  class Item;
}

namespace KCal {
  class Incidence;
}
using namespace KCal;

/**
  Viewer widget for events.
*/
class KORGANIZER_EVENTVIEWER_EXPORT KOEventViewer : public KTextBrowser
{
  Q_OBJECT
  public:
    explicit KOEventViewer( QWidget *parent = 0 );
    virtual ~KOEventViewer();

    /** Reimplemented from QTextBrowser to handle links. */
    void setSource( const QUrl &name );

    virtual bool appendIncidence( Incidence *incidence, const QDate &date );

    /**
      Clear viewer.
      @param now Delete view immediately if set; else delete it with the
      next call to appendIncidence().
    */
    void clearEvents( bool now = false );

    /**
      Add given text to currently shown content.
    */
    void addText( const QString &text );

    /**
      Set the default text when there isn't an incidence to display.
    */
    void setDefaultText( const QString &text );

    void readSettings( KConfig *config );
    void writeSettings ( KConfig *config );

  public slots:
    /**
      Show given incidence in viewer. Clear all previously shown incidences.
    */
    void setIncidence( Incidence *incidence, const QDate &date );
    void setIncidence( const Akonadi::Item &, const QDate &date );
    void changeIncidenceDisplay( Incidence *incidence, const QDate &date, int action );
    void editIncidence();

    /**
      Show the incidence in context. That means showing the todo, agenda or
      journal view and scrolling to show it.
    */
    virtual void showIncidenceContext();

  private:
    Incidence *mIncidence;
    QString mDefaultText;
    QString mText;
};

#endif
