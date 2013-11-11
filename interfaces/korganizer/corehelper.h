/*
  This file is part of the KOrganizer interfaces.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#ifndef KORG_INTERFACES_COREHELPER_H
#define KORG_INTERFACES_COREHELPER_H

namespace KOrg {

class CoreHelper
{
  public:
    CoreHelper() {}
    virtual ~CoreHelper() {}

    virtual QColor categoryColor( const QStringList &cats ) = 0;
    virtual QString holidayString( const QDate &dt ) = 0;
    virtual QTime dayStart() = 0;
    virtual const KCalendarSystem *calendarSystem() = 0;
};

}
#endif
