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

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KORG_PLUGINS_THISDAYINHISTORY_THISDAYINHISTORY_H
#define KORG_PLUGINS_THISDAYINHISTORY_THISDAYINHISTORY_H

#include <calendarviews/agenda/calendardecoration.h>

using namespace EventViews::CalendarDecoration;

class ThisDayInHistory : public Decoration
{
public:
    ThisDayInHistory();
    ~ThisDayInHistory() {}

    Element::List createDayElements(const QDate &);
    Element::List createMonthElements(const QDate &);

//    void configure( QWidget *parent );

    QString info() const;
};

class ThisDayInHistoryFactory : public DecorationFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.korganizer.ThisDayInHistory");
public:
    Decoration *createPluginFactory()
    {
        return new ThisDayInHistory;
    }
};

#endif
