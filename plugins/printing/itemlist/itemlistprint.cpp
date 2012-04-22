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

#include "itemlistprint.h"

class ItemListPrintFactory : public KOrg::PrintPluginFactory
{
  public:
    KOrg::PrintPlugin *createPluginFactory() { return new CalPrintItemList; }
};

K_EXPORT_PLUGIN( ItemListPrintFactory )

/**************************************************************
 *           Print Item List
 **************************************************************/

QWidget *CalPrintItemList::createConfigWidget( QWidget *w )
{
  return new CalPrintItemListConfig( w );
}

void CalPrintItemList::readSettingsWidget()
{
  CalPrintItemListConfig *cfg =
      dynamic_cast<CalPrintItemListConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();
  }
}

void CalPrintItemList::setSettingsWidget()
{
  CalPrintItemListConfig *cfg =
      dynamic_cast<CalPrintItemListConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );
  }
}

void CalPrintItemList::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Itemlistprint" );
    //TODO: Read in settings
  }
  setSettingsWidget();
}

void CalPrintItemList::saveConfig()
{
  kDebug();

  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Itemlistprint" );
    //TODO: Write out settings
  }
}

void CalPrintItemList::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintItemListConfig *cfg =
      dynamic_cast<CalPrintItemListConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintItemList::print( QPainter &p, int width, int height )
{
  Q_UNUSED( p );
  Q_UNUSED( width );
  Q_UNUSED( height );
  //TODO: Print something!
}

