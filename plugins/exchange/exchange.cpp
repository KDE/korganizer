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

#include <qfile.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <kurl.h>
#include <kdebug.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kglobal.h>

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
    KGlobal::locale()->insertCatalogue("libkpimexchange");
    return (new ExchangeFactory);
  }
}

Exchange::Exchange(KOrg::MainWindow *parent, const char *name) :
  KOrg::Part(parent,name)
{
  kdDebug() << "Creating Exchange Plugin...\n";

  mAccount = new KPIM::ExchangeAccount( "Calendar/Exchange Plugin" );
  mClient = new KPIM::ExchangeClient( mAccount );
  mClient->setWindow( parent->topLevelWidget() );

  setXMLFile("plugins/exchangeui.rc");

  parent->addPluginAction( new KAction(i18n("Download..."), 0, this, SLOT(download()),
              actionCollection(), "exchange_download"));

//  new KAction(i18n("Test"), 0, this, SLOT(test()),
//              actionCollection(), "exchange_test");

  KAction *action = new KAction(i18n("Upload Event..."), 0, this, SLOT(upload()),
                                actionCollection(), "exchange_upload");
  parent->addPluginAction( action );
  QObject::connect(mainWindow()->view(),SIGNAL(incidenceSelected(Incidence *)),
            this, SLOT(slotIncidenceSelected(Incidence *)));
  action->setEnabled( false );
  QObject::connect(this,SIGNAL(enableIncidenceActions(bool)),
          action,SLOT(setEnabled(bool)));
//          action,SLOT(setEnabled(bool)));

  action = new KAction(i18n("Delete Event"), 0, this, SLOT(remove()),
                                actionCollection(), "exchange_delete");
  parent->addPluginAction( action );
  QObject::connect(this,SIGNAL(enableIncidenceActions(bool)),
          action,SLOT(setEnabled(bool)));
  action->setEnabled( false );

  parent->addPluginAction( new KAction(i18n("Configure..."), 0, this, SLOT(configure()),
              actionCollection(), "exchange_configure") );

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

  int result = mClient->downloadSynchronous(calendar, start, end, true );

  if ( result == KPIM::ExchangeClient::ResultOK )
    emit calendarChanged();
  else
    showError( result, mClient->detailedErrorString() );

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
  if ( KMessageBox::warningContinueCancel( 0L, i18n("Exchange Upload is EXPERIMENTAL, you may lose data on this appointment!"), i18n("Exchange Plugin") )
       == KMessageBox::Continue ) {
    kdDebug() << "Trying to add appointment " << event->summary() << endl;
    int result = mClient->uploadSynchronous( event );
    if ( result != KPIM::ExchangeClient::ResultOK )
      showError( result, mClient->detailedErrorString() );
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

  if ( KMessageBox::warningContinueCancel( 0L, i18n("Exchange Delete is EXPERIMENTAL, if this is a recurring event it will delete all instances!"), i18n("Exchange Plugin") )
       == KMessageBox::Continue ) {
    kdDebug() << "Trying to delete appointment " << event->summary() << endl;
    int result = mClient->removeSynchronous( event );

    if ( result == KPIM::ExchangeClient::ResultOK ) {
      mainWindow()->view()->calendar()->deleteEvent( event );
      emit calendarChanged();
    } else
      showError( result, mClient->detailedErrorString() );
  }
}

void Exchange::configure()
{
  kdDebug() << "Exchange::configure" << endl;
  ExchangeConfig dialog( mAccount );

  if (dialog.exec() == QDialog::Accepted )
    mAccount->save( "Calendar/Exchange Plugin" );
}

void Exchange::showError( int error, const QString& moreInfo /* = QString::null */ )
{
  QString errorText;
  switch( error ) {
  case KPIM::ExchangeClient::ResultOK:
    errorText = i18n( "No Error" );
    break;
  case KPIM::ExchangeClient::CommunicationError:
    errorText = i18n( "The Exchange server could not be reached or returned an error." );
    break;
  case KPIM::ExchangeClient::ServerResponseError:
    errorText = i18n( "Server response could not be interpreted." );
    break;
  case KPIM::ExchangeClient::IllegalAppointmentError:
    errorText = i18n( "Appointment data could not be interpreted." );
    break;
  case KPIM::ExchangeClient::NonEventError:
    errorText = i18n( "This should not happen: trying to upload wrong type of event." );
    break;
  case KPIM::ExchangeClient::EventWriteError:
    errorText = i18n( "An error occurred trying to write an appointment to the server." );
    break;
  case KPIM::ExchangeClient::DeleteUnknownEventError:
    errorText = i18n( "Trying to delete an event that is not present on the server." );
    break;
  case KPIM::ExchangeClient::UnknownError:
  default:
    errorText = i18n( "Unknown Error" );
  }

  if ( error !=  KPIM::ExchangeClient::ResultOK ) {
    if ( moreInfo.isNull() )
      KMessageBox::error( mainWindow()->topLevelWidget(), errorText, i18n( "Exchange Plugin" ) );
    else
      KMessageBox::detailedError( mainWindow()->topLevelWidget(), errorText, moreInfo, i18n( "Exchange Plugin" ) );
  }
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
#include "exchange.moc"
