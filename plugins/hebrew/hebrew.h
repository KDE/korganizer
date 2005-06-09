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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef KORG_HEBREW_H
#define KORG_HEBREW_H

#include <qstring.h>
#include <qstringlist.h>
#include <calendar/calendardecoration.h>

using namespace KOrg;

class Hebrew:public CalendarDecoration
{
public:
  Hebrew()
  {
  }
  ~Hebrew()
  {
  }
  void configure(QWidget * parent);
  QString shortText(const QDate &);

  QString info();
  static bool IsraelP;

private:

};

#endif
