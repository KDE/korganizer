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
#ifndef KORG_EXCHANGE_H
#define KORG_EXCHANGE_H

#include <qstring.h>
#include <qdatetime.h>

#include <korganizer/part.h>

#include <libkcal/incidence.h>
#include <libkcal/event.h>

#include <exchangeaccount.h>
#include <exchangeclient.h>

// using namespace KOrg;
using namespace KCal;

class Exchange : public KOrg::Part {
    Q_OBJECT
  public:
    Exchange( KOrg::MainWindow *, const char * );
    ~Exchange();

    QString info();

  signals:
    void enableIncidenceActions( bool );
    void calendarChanged();
    void calendarChanged(const QDate&start,const QDate&end);

  private slots:
    void download();
    void upload();
    void remove();
    void configure();
    void test();
    void slotIncidenceSelected( Incidence * );

  private:
    void test2();
    void showError( int error, const QString& moreInfo = QString::null );

    KPIM::ExchangeClient *mClient;
    KPIM::ExchangeAccount* mAccount;
};

#endif

