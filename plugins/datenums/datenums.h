/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
#ifndef KORG_DATENUMS_H
#define KORG_DATENUMS_H

#include <QtCore/QString>

#include <calendar/calendardecoration.h>

using namespace KOrg::CalendarDecoration;

class DatenumsAgenda : public AgendaElement {
  public:
    DatenumsAgenda();
    ~DatenumsAgenda() {}

    void configure( QWidget *parent );

    QString shortText( const QDate & ) const;

  protected:
    int mDateNum;
};

class Datenums : public Decoration {
  public:
    Datenums();
    ~Datenums() {}

  protected:
    QString info();
};

#endif
