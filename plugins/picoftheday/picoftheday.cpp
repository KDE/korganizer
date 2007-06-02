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

#include "picoftheday.h"
#include "potdwidget.h"

#include "koglobals.h"

#include <kconfig.h>
#include <kstandarddirs.h>
#include <khtmlview.h>
#include <khtml_part.h>


class PicofthedayFactory : public OldCalendarDecorationFactory {
  public:
    OldCalendarDecoration *create() { return new Picoftheday; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_picoftheday, PicofthedayFactory )


Picoftheday::Picoftheday()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "Calendar/PicOfTheDay Plugin");
}


QWidget* Picoftheday::smallWidget( QWidget *parent, const QDate &date)
{
  POTDWidget *w = new POTDWidget(parent);
  w->loadPOTD(date);

  return w;
}


QString Picoftheday::info()
{
  return i18n("This plugin provides the Wikipedia <i>Picture of the Day</i>.");
}
