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

#include <qfile.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <kurl.h>
#include <kdebug.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>

#include "korganizer/korganizer.h"
#include "korganizer/calendarview.h"

#include <exchangeclient.h>
#include <exchangeaccount.h>

#include "exchange.h"
#include "exchangedialog.h"
#include "exchangeconfig.h"

using namespace KCal; // Needed for connecting slots

class ExchangeFactory : public KOrg::PartFactory {
  public:
    KOrg::Part *create(KOrg::MainWindow *parent, const char *name)
    {
      return new Exchange(parent,name);
    }
};

extern "C" { 
  void *init_libkorg_exchange()
  {
    kdDebug() << "Registering Exchange Plugin...\n";
    return (new ExchangeFactory);
  }
}

Exchange::Exchange(KOrg::MainWindow *parent, const char *name) :
  KOrg::Part(parent,name)
{
  kdDebug() << "Creating Exchange Plugin...\n";

  mAccount = new KPIM::ExchangeAccount( "Calendar/Exchange Plugin" );
  mClient = new KPIM::ExchangeClient( mAccount );
  mClient->setWindow( parent );
  
  setXMLFile("plugins/exchangeui.rc");

  new KAction(i18n("Download..."), 0, this, SLOT(download()),
              actionCollection(), "exchange_download");

//  new KAction(i18n("Test"), 0, this, SLOT(test()),
//              actionCollection(), "exchange_test");

  KAction *action = new KAction(i18n("Upload Event..."), 0, this, SLOT(upload()),
                                actionCollection(), "exchange_upload");
  QObject::connect(mainWindow()->view(),SIGNAL(incidenceSelected(Incidence *)),
            this, SLOT(slotIncidenceSelected(Incidence *)));
  action->setEnabled( false );
  QObject::connect(this,SIGNAL(enableIncidenceActions(bool)),
          action,SLOT(setEnabled(bool)));
//          action,SLOT(setEnabled(bool)));

  action = new KAction(i18n("Delete Event..."), 0, this, SLOT(remove()),
                                actionCollection(), "exchange_delete");
  QObject::connect(this,SIGNAL(enableIncidenceActions(bool)),
          action,SLOT(setEnabled(bool)));
  action->setEnabled( false );

  new KAction(i18n("Configure..."), 0, this, SLOT(configure()),
              actionCollection(), "exchange_configure");

  connect( this, SIGNAL( calendarChanged() ), mainWindow()->view(), SLOT( updateView() ) );
  connect( this, SIGNAL( calendarChanged(const QDate &, const QDate &)), 
    mainWindow()->view(), SLOT(updateView(const QDate &, const QDate &)) );
}

Exchange::~Exchange()
{
  kdDebug() << "Exchange Plugin destructor" << endl;
}

QString Exchange::info()
{
  return i18n("This plugin imports and export calendar events from/to a Microsoft Exchange 2000 Server.");
}

void  Exchange::slotIncidenceSelected( Incidence *incidence )
{
  emit enableIncidenceActions( incidence != 0 );
}

void Exchange::download()
{
  ExchangeDialog dialog( mainWindow()->view()->startDate(), mainWindow()->view()->endDate() );
  
  if (dialog.exec() != QDialog::Accepted ) 
    return;

  QDate start = dialog.m_start->date();
  QDate end = dialog.m_end->date();

  KCal::Calendar* calendar = mainWindow()->view()->calendar();

  mClient->downloadSynchronous(calendar, start, end, true );

  emit calendarChanged();
}

void Exchange::upload()
{
  kdDebug() << "Called Exchange::upload()" << endl;

  Event* event = static_cast<Event *> ( mainWindow()->view()->currentSelection() );
  if ( ! event )
  {
    KMessageBox::information( 0L, i18n("Please select an appointment"), i18n("Exchange Plugin") );
    return;
  }
  if ( KMessageBox::warningContinueCancel( 0L, "Exchange Upload is EXPERIMENTAL, you may lose data on this appointment!", i18n("Exchange Plugin") )
       == KMessageBox::Continue ) {
    kdDebug() << "Trying to add appointment " << event->summary() << endl;
    mClient->uploadSynchronous( event );
  }
}

void Exchange::remove()
{
  kdDebug() << "Called Exchange::remove()" << endl;

  Event* event = static_cast<Event *> ( mainWindow()->view()->currentSelection() );
  if ( ! event )
  {
    KMessageBox::information( 0L, i18n("Please select an appointment"), i18n("Exchange Plugin") );
    return;
  }

  if ( KMessageBox::warningContinueCancel( 0L, "Exchange Delete is EXPERIMENTAL, if this is a recurring event it will delete all instances!", i18n("Exchange Plugin") )
       == KMessageBox::Continue ) {
    kdDebug() << "Trying to delete appointment " << event->summary() << endl;
    mClient->removeSynchronous( event );
    mainWindow()->view()->calendar()->deleteEvent( event );
    emit calendarChanged();
  }
}

void Exchange::configure()
{
  kdDebug() << "Exchange::configure" << endl;
  ExchangeConfig dialog( mAccount );
  
  if (dialog.exec() == QDialog::Accepted ) 
    mAccount->save( "Calendar/Exchange Plugin" );
}

void Exchange::test()
{
  kdDebug() << "Entering test()" << endl;
  mClient->test();
}

void Exchange::test2()
{
  kdDebug() << "Entering test2()" << endl;
}
