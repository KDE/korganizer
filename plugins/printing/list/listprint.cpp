/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KORG_NOPRINTER

#include "listprint.h"

#include "calprintpluginbase.h"
#include "calprinthelper.h"
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/calendar.h>
#include <libkdepim/kdateedit.h>
#include <kconfig.h> 
#include <kdebug.h>

#include <qbuttongroup.h>

#include "calprintlistconfig_base.h"


class ListPrintFactory : public KOrg::PrintPluginFactory {
  public:
    KOrg::PrintPlugin *create() { return new CalPrintList; }
};

extern "C" {
  void *init_libkorg_listprint()
  {
    return (new ListPrintFactory);
  }
}



/**************************************************************
 *           Print Day
 **************************************************************/

QWidget *CalPrintList::createConfigWidget( QWidget *w )
{
  return new CalPrintListConfig_Base( w );
}

void CalPrintList::readSettingsWidget()
{
  CalPrintListConfig_Base *cfg =
      dynamic_cast<CalPrintListConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();
    mUseDateRange = (cfg->mDateRangeGroup->selectedId() == 1);
  }
}

void CalPrintList::setSettingsWidget()
{
  CalPrintListConfig_Base *cfg =
      dynamic_cast<CalPrintListConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );
    
    cfg->mDateRangeGroup->setButton( (mUseDateRange)?1:0 );
  }
}

void CalPrintList::loadConfig()
{
  if ( mConfig ) {
    mUseDateRange = mConfig->readBoolEntry( "ListsInRange", false );
  }
  setSettingsWidget();
}

void CalPrintList::saveConfig()
{
  kdDebug(5850) << "CalPrintList::saveConfig()" << endl;

  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "ListsInRange", mUseDateRange );
  }
}

void CalPrintList::setDateRange( const QDate& from, const QDate& to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintListConfig_Base *cfg =
      dynamic_cast<CalPrintListConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintList::print( QPainter &p, int width, int height )
{
}

#endif
