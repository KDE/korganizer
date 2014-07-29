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
#ifndef KORG_PLUGINS_DATENUMS_DATENUMS_H
#define KORG_PLUGINS_DATENUMS_DATENUMS_H

#include <calendarviews/agenda/calendardecoration.h>

using namespace EventViews::CalendarDecoration;

class Datenums : public Decoration
{
  public:
    Datenums();
    ~Datenums() {}

    void configure( QWidget *parent );

    Element::List createDayElements( const QDate & );
    Element::List createWeekElements( const QDate & );

    enum DayNumber {
      DayOfYear = 1,
      DaysRemaining = 2
    };
    Q_DECLARE_FLAGS( DayNumbers, DayNumber )

    QString info() const;

  private:
    DayNumbers mDisplayedInfo;
};

class DatenumsFactory : public DecorationFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.korganizer.Datenums");
  public:
    Decoration *createPluginFactory() { return new Datenums; }
};


Q_DECLARE_OPERATORS_FOR_FLAGS( Datenums::DayNumbers )

#endif
