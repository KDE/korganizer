/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include "exchangeconfig.h"

ExchangeConfig::ExchangeConfig( QWidget* parent )
  : KDialogBase(Plain,i18n("Exchange Plugin"),Ok|Cancel,Ok,parent)
{
  QFrame *topFrame = plainPage();
  QGridLayout *topLayout = new QGridLayout( topFrame, 2, 2, 3 );

  m_host = new KLineEdit( "", topFrame );
  topLayout->addWidget( new QLabel( i18n( "Exchange server" ), topFrame ), 0, 0 );
  topLayout->addWidget( m_host, 0, 1 );

  m_user = new KLineEdit( "", topFrame );
  topLayout->addWidget( new QLabel( i18n( "User" ), topFrame ), 1, 0 );
  topLayout->addWidget( m_user, 1, 1 );

  load();
}

ExchangeConfig::~ExchangeConfig()
{
}

void ExchangeConfig::load()
{
  kapp->config()->setGroup("Calendar/Exchange Plugin");

  QString host = kapp->config()->readEntry( "host" );
  if ( ! host.isNull() ) {
    m_host->setText( host );
  }

  QString user = kapp->config()->readEntry( "user" );
  if ( ! user.isNull() ) {
    m_user->setText( user );
  }
}

void ExchangeConfig::save()
{
  kapp->config()->setGroup( "Calendar/Exchange Plugin" );
  kapp->config()->writeEntry( "host", m_host->text() );
  kapp->config()->writeEntry( "user", m_user->text() );
  kapp->config()->sync();
}

void ExchangeConfig::slotOk()
{
  save();

  accept();
}
