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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <kapplication.h>
#include <kconfig.h>
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
  QGridLayout *topLayout = new QGridLayout( topFrame, 5, 3, 3 );

  m_host = new KLineEdit( mAccount->host(), topFrame );
  topLayout->addWidget( new QLabel( i18n( "Exchange server:" ), topFrame ), 0, 0 );
  topLayout->addWidget( m_host, 0, 1 );

  m_user = new KLineEdit( mAccount->account(), topFrame );
  topLayout->addWidget( new QLabel( i18n( "User:" ), topFrame ), 1, 0 );
  topLayout->addWidget( m_user, 1, 1 );
  connect( m_user, SIGNAL(textChanged(const QString&)), this, SLOT(slotUserChanged(const QString&)) );

  m_password = new KLineEdit( mAccount->password(), topFrame );
  topLayout->addWidget( new QLabel( i18n( "Password:" ), topFrame ), 2, 0 );
  topLayout->addWidget( m_password, 2, 1 );
  m_password->setEchoMode( QLineEdit::Password );

  m_autoMailbox = new QCheckBox( i18n( "Determine mailbox automatically" ), topFrame );
  topLayout->addMultiCellWidget( m_autoMailbox, 3, 3, 0, 1 );
  connect( m_autoMailbox, SIGNAL(toggled(bool)), this, SLOT(slotToggleAuto(bool)) );

  m_mailbox= new KLineEdit( mAccount->mailbox(), topFrame );
  topLayout->addWidget( new QLabel( i18n( "Mailbox URL:" ), topFrame ), 4, 0 );
  topLayout->addWidget( m_mailbox, 4, 1 );

  m_tryFindMailbox = new QPushButton( "&Find", topFrame );
  topLayout->addWidget( m_tryFindMailbox, 4, 2 );
  connect( m_tryFindMailbox, SIGNAL(clicked()), this, SLOT(slotFindClicked()) );

  kapp->config()->setGroup( "Calendar/Exchange Plugin" );
  bool autoChecked = kapp->config()->readBoolEntry( "auto-mailbox", true );
  m_autoMailbox->setChecked( autoChecked );
  // m_mailboxEqualsUser->setChecked( mAccount->mailbox() == ("webdav://"+mAccount->host()+"/exchange/"+mAccount->account() ) );
}

ExchangeConfig::~ExchangeConfig()
{
}

void ExchangeConfig::slotToggleAuto( bool on )
{
  m_mailbox->setEnabled( ! on );
//  m_tryFindMailbox->setEnabled( ! on );
//  if ( on ) {
//    m_mailbox->setText( "webdav://" + m_host->text() + "/exchange/" + m_user->text() );
//  }
}

void ExchangeConfig::slotUserChanged( const QString& text )
{
//  if ( m_mailboxEqualsUser->isChecked() ) {
//    m_mailbox->setText( "webdav://" + m_host->text() + "/exchange/" + text );
//  }
}

void ExchangeConfig::slotOk()
{
  if ( m_autoMailbox->isChecked() ) {
    QString mailbox = mAccount->tryFindMailbox( m_host->text(), m_user->text(), m_password->text() );
    if ( mailbox.isNull() ) {
      kdWarning() << "Could not find Exchange mailbox URL, incomplete settings!"<< endl;
      KMessageBox::sorry( this, "Could not determine mailbox URL" );
      return; // Do not accept
    } else {
      mAccount->setMailbox( mailbox );
    }
  } else {
    mAccount->setMailbox( m_mailbox->text() );
  }
  mAccount->setHost( m_host->text() );
  mAccount->setAccount( m_user->text() );
  mAccount->setPassword( m_password->text() );

  kapp->config()->setGroup( "Calendar/Exchange Plugin" );
  kapp->config()->writeEntry( "auto-mailbox", m_autoMailbox->isChecked() );
  
  accept();
}

void ExchangeConfig::slotFindClicked()
{
  QString mailbox = mAccount->tryFindMailbox( m_host->text(), m_user->text(), m_password->text() );
  if ( mailbox.isNull() ) {
    KMessageBox::sorry( this, "Could not determine mailbox URL" );
  } else {
    m_mailbox->setText( mailbox );
  }
}

#include "exchangeconfig.moc"
