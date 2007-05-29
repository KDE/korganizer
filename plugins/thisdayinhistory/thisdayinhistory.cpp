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

#include "thisdayinhistory.h"
#include "koglobals.h"
#include "korganizer.h"
#include <kconfig.h>
#include <kstandarddirs.h>

#include <kcalendarsystem.h>

#include <kurllabel.h>
#include <ktoolinvocation.h>

class ThisDayInHistoryFactory : public CalendarDecorationFactory {
  public:
    CalendarDecoration *create() { return new ThisDayInHistory; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_thisdayinhistory, ThisDayInHistoryFactory )


ThisDayInHistory::ThisDayInHistory()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "This Day in History Plugin");
}


QWidget* ThisDayInHistory::smallWidget( QWidget *parent, const QDate &date)
{
  KUrlLabel* l = new KUrlLabel(parent);
  l->setText(i18n("This Day in History"));
  l->setUrl(i18nc("Localized Wikipedia website", "http://en.wikipedia.org/wiki/") + date.toString(i18nc("Qt date format used by the localized Wikipedia", "MMMM_d")));
  l->setToolTip(l->url());

  // FIXME
  QObject::connect(l, SIGNAL(leftClickedUrl(const QString&)),
                   l, SLOT( invokeBrowser(const QString&)));
  return l;
}

QString ThisDayInHistory::info()
{
  return i18n("This plugin provides links to Wikipedia's 'This Day in History' pages.");
}
