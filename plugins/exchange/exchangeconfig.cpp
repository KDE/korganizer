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
#include <kdebug.h>
#include <kmessagebox.h>

#include <exchangeaccount.h>

#include "exchangeconfig.h"

ExchangeConfig::ExchangeConfig( KPIM::ExchangeAccount* account, QWidget* parent )
  : KDialogBase(Plain,i18n("Exchange Plugin"),Ok|Cancel,Ok,parent)
{
  mAccount = account;

  kdDebug() << "Creating ExchangeConfig with account: " << 
      account->host() << ":" << account->account() << endl;

  QFrame *topFrame = plainPage();
  QGridLayout *topLayout = new QGridLayout( topFrame, 3, 2, 3 );

  m_host = new KLineEdit( mAccount->host(), topFrame );
  topLayout->addWidget( new QLabel( i18n( "Exchange server" ), topFrame ), 0, 0 );
  topLayout->addWidget( m_host, 0, 1 );

  m_user = new KLineEdit( mAccount->account(), topFrame );
  topLayout->addWidget( new QLabel( i18n( "User" ), topFrame ), 1, 0 );
  topLayout->addWidget( m_user, 1, 1 );

  m_password = new KLineEdit( mAccount->password(), topFrame );
  topLayout->addWidget( new QLabel( i18n( "Password" ), topFrame ), 2, 0 );
  topLayout->addWidget( m_password, 2, 1 );
  m_password->setEchoMode( QLineEdit::Password );
  m_password->selectAll();
}

ExchangeConfig::~ExchangeConfig()
{
}

void ExchangeConfig::slotOk()
{
  mAccount->setHost( m_host->text() );
  mAccount->setAccount( m_user->text() );
  mAccount->setPassword( m_password->text() );

  accept();
}
