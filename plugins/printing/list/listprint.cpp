/*
  This file is part of KOrganizer.

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

#include "listprint.h"
#include "calprintpluginbase.h"
#include "calprinthelper.h"

#include <libkdepim/kdateedit.h>

#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/calendar.h>

#include <kconfig.h>
#include <kdebug.h>

#include <QButtonGroup>

K_PLUGIN_FACTORY(ListPrintFactory, registerPlugin<CalPrintList>();)
K_EXPORT_PLUGIN(ListPrintFactory( "korg_listprint" ))

/**************************************************************
 *           Print Day
 **************************************************************/

QWidget *CalPrintList::createConfigWidget( QWidget *w )
{
  return new CalPrintListConfig( w );
}

void CalPrintList::readSettingsWidget()
{
  CalPrintListConfig *cfg =
      dynamic_cast<CalPrintListConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();
    mUseDateRange = (cfg->mDateRangeGroup->selectedId() == 1);
  }
}

void CalPrintList::setSettingsWidget()
{
  CalPrintListConfig *cfg =
      dynamic_cast<CalPrintListConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mDateRangeGroup->setButton( (mUseDateRange)?1:0 );
  }
}

void CalPrintList::loadConfig()
{
  if ( mConfig ) {
    mUseDateRange = mConfig->readEntry( "ListsInRange", false );
  }
  setSettingsWidget();
}

void CalPrintList::saveConfig()
{
  kDebug();

  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "ListsInRange", mUseDateRange );
  }
}

void CalPrintList::setDateRange( const QDate& from, const QDate& to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintListConfig *cfg =
      dynamic_cast<CalPrintListConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintList::print( QPainter &p, int width, int height )
{
}
