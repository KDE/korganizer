/*
  This file is part of KOrganizer.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KORG_KOAPP_H
#define KORG_KOAPP_H

#include <KontactInterface/PimUniqueApplication>

class KUrl;

class KOrganizerApp : public KontactInterface::PimUniqueApplication
{
    Q_OBJECT
public:
    KOrganizerApp();
    ~KOrganizerApp();

    /**
      Create new instance of KOrganizer. If there is already running a
      KOrganizer only an additional main window is opened.
    */
    int newInstance();

private:
    /**
      Process calendar from URL \arg url. If url is empty open the default
      calendar based on the resource framework.
    */
    void processCalendar(const KUrl &url);
};

#endif
