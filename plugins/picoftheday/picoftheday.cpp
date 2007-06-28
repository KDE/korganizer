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
#include "configdialog.h"

#include "koglobals.h"

#include <kconfig.h>
#include <kstandarddirs.h>
#include <khtmlview.h>
#include <khtml_part.h>

using namespace KOrg::CalendarDecoration;

class PicofthedayFactory : public DecorationFactory {
  public:
    Decoration *create() { return new Picoftheday; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_picoftheday, PicofthedayFactory )


Picoftheday::Picoftheday()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "Calendar/Picoftheday Plugin");

  PicofthedayAgenda* a = new PicofthedayAgenda();
  agendaElements += a;
}

void Picoftheday::configure(QWidget *parent)
{
  ConfigDialog dlg(parent);
}

QString Picoftheday::info()
{
  return i18n("This plugin provides the Wikipedia <i>Picture of the Day</i>.");
}


PicofthedayAgenda::PicofthedayAgenda()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "Calendar/Picoftheday Plugin/Agenda");
  mThumbnailSize = config.readEntry( "ThumbnailSize", 120 );
  mAspectRatioMode = (Qt::AspectRatioMode)config.readEntry( "AspectRatioMode", int(Qt::KeepAspectRatio) );
  
  // TODO: read the position from the config 
  mPosition = DayBottomC;
}

QWidget* PicofthedayAgenda::widget( QWidget *parent, const QDate &date ) const
{
  POTDWidget *w = new POTDWidget(parent);
  w->setThumbnailSize( mThumbnailSize );
  w->loadPOTD( date );
  w->setAspectRatioMode( mAspectRatioMode );

  return w;
}

