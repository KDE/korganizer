/*
    This file is part of KOrganizer.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KOWINDOWLIST_H
#define KOWINDOWLIST_H
// $Id$

#include <qobject.h>

#include "korganizer.h"

class KURL;

/**
  This class manages a list of KOrganizer instances, each associated with a
  window displaying a calendar. It acts as relay for signals between this
  windows and manages information like the active calendar, which requires
  interaction of all instances.

  @short manages a list of all KOrganizer instances
  @author Cornelius Schumacher
*/
class KOWindowList : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructs a new list of KOrganizer windows. There should only be one
     * instance of this class. The KOrganizer class takes care of this.
     */
    KOWindowList(const char *name=0);
    virtual ~KOWindowList();

    /** Is there only one instance left? */
    bool lastInstance();

    /** Is there a instance with this URL? */
    KOrganizer* findInstance(const KURL &url);

  signals:

  public slots:
    void addWindow(KOrganizer *);
    void removeWindow(KOrganizer *);
    
    /** Deactivating all calendars despite the one given in the argument*/
    void deactivateCalendars(KOrganizer *);
    
  private:
    QPtrList<KOrganizer> mWindowList; // list of all existing KOrganizer instances
};

#endif
