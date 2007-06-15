/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>

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
*/
#ifndef KORG_HEBREW_H
#define KORG_HEBREW_H

#include <QString>
#include <QStringList>
#include <calendar/oldcalendardecoration.h>

using namespace KOrg;

class Hebrew:public OldCalendarDecoration
{
  public:
    Hebrew() {}
    ~Hebrew() {}
    void configure( QWidget *parent );
    QString shortText( const QDate &qd ) const;

    QString info();
    static bool IsraelP;
};

#endif
