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

#include "whatsnextprint.h"

class WhatsNextPrintFactory : public KOrg::PrintPluginFactory
{
  public:
    KOrg::PrintPlugin *createPluginFactory() { return new CalPrintWhatsNext; }
};

K_EXPORT_PLUGIN( WhatsNextPrintFactory )

/**************************************************************
 *           Print What's Next
 **************************************************************/

QWidget *CalPrintWhatsNext::createConfigWidget( QWidget *w )
{
  return new CalPrintWhatsNextConfig( w );
}

void CalPrintWhatsNext::readSettingsWidget()
{
  CalPrintWhatsNextConfig *cfg =
      dynamic_cast<CalPrintWhatsNextConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();
  }
}

void CalPrintWhatsNext::setSettingsWidget()
{
  CalPrintWhatsNextConfig *cfg =
      dynamic_cast<CalPrintWhatsNextConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );
  }
}

void CalPrintWhatsNext::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Whatsnextprint" );
    //TODO: Read in settings
  }
  setSettingsWidget();
}

void CalPrintWhatsNext::saveConfig()
{
  kDebug();

  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Whatsnextprint" );
    //TODO: Write out settings
  }
}

void CalPrintWhatsNext::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintWhatsNextConfig *cfg =
      dynamic_cast<CalPrintWhatsNextConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintWhatsNext::print( QPainter &p, int width, int height )
{
  Q_UNUSED( p );
  Q_UNUSED( width );
  Q_UNUSED( height );
  //TODO: Print something!
}
