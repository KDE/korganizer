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
#ifndef EXCHANGECONFIG_H
#define EXCHANGECONFIG_H

#include <qcheckbox.h>
#include <kdialogbase.h>
#include <klineedit.h>
//#include <kpassdlg.h>

#include <exchangeaccount.h>

class ExchangeConfig : public KDialogBase
{
    Q_OBJECT
  public:
    ExchangeConfig(KPIM::ExchangeAccount* account, QWidget *parent=0);
    virtual ~ExchangeConfig();

//  protected:
//    void load();
//    void save();

  protected slots:
    void slotToggleEquals( bool on );
    void slotUserChanged( const QString& text );
    void slotOk();

  private:
  public:
    KPIM::ExchangeAccount* mAccount;
    KLineEdit *m_host;
    KLineEdit *m_user;
    QCheckBox *m_mailboxEqualsUser;
    KLineEdit *m_mailbox;
    KLineEdit *m_password;
};

#endif

