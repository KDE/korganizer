/*
  This file is part of KOrganizer

  Copyright (c) 2004  Bo Thorsen <bo@sonofthor.dk>
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
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
 */

#ifndef KORG_KORGANIZERIFACEIMPL_H
#define KORG_KORGANIZERIFACEIMPL_H

#include "korganizerprivate_export.h"

#include <QObject>

class ActionManager;

class KORGANIZERPRIVATE_EXPORT KOrganizerIfaceImpl : public QObject
{
  Q_OBJECT
  public:
    explicit KOrganizerIfaceImpl( ActionManager *mActionManager,
                                  QObject *parent = 0, const char *name = 0 );
    ~KOrganizerIfaceImpl();

  public slots:
    bool openURL( const QString &url );
    bool mergeURL( const QString &url );
    bool saveURL();
    bool saveAsURL( const QString &url );
    QString getCurrentURLasString() const;

    bool editIncidence( const QString &akonadiUrl );
    /** @reimp from KOrganizerIface::deleteIncidence()
        @param akonadiUrl the akonadi Item URL of the item to delete. if no such item exists,
        nothing happens
        @return true if the item could be deleted, false otherwise
    */
    bool deleteIncidence( const QString &akonadiUrl )
    { return deleteIncidence( akonadiUrl, false ); }

    /**
      Delete the incidence with the given akonadi item URL from the active calendar.
      @param akonadiUrl The Akonadi Item URL.
      @param force If true, all recurrences and sub-todos (if applicable) will
      be deleted without prompting for confirmation.
    */
    bool deleteIncidence( const QString &akonadiUrl, bool force );

    /**
      Add an incidence to the active calendar.
      @param iCal A calendar in iCalendar format containing the incidence. The
                  calendar must consist of a VCALENDAR component which contains
                  the incidence (VEVENT, VTODO, VJOURNAL or VFREEBUSY) and
                  optionally a VTIMEZONE component. If there is more than one
                  incidence, only the first is added to KOrganizer's calendar.
    */
    bool addIncidence( const QString &iCal );

    /**
      Show a HTML representation of the incidence (the "View.." dialog).
      If no incidence with the given Akonadi Item URL exists, nothing happens.
      @param akonadiUrl The Akonadi Item URL of the incidence to be shown.
    */
    bool showIncidence( const QString &akonadiUrl );

    /**
      Show an incidence in context. This means showing the todo, agenda or
      journal view (as appropriate) and scrolling it to show the incidence.
      @param akonadiUrl the Akonadi Item URL of the incidence to show.
    */
    bool showIncidenceContext( const QString &akonadiUrl );

    /**
     * Called by KOrganizerUniqueAppHandler in the kontact plugin
     * Returns true if the command line was successfully handled
     * false otherwise.
     */
    bool handleCommandLine();

  private:
    ActionManager *mActionManager;
};

#endif // KORGANIZER_SHARED_H

