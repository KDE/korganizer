/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlayout.h>
#include <qstring.h>

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "engine.h"
#include "provider.h"

#include "providerdialog.h"
#include "providerdialog.moc"

using namespace KNS;

class ProviderItem : public KListViewItem
{
  public:
    ProviderItem( KListView *parent, Provider *provider ) :
      KListViewItem( parent ), mProvider( provider )
    {
      setText( 0, provider->name() );
    }
      
    Provider *provider() { return mProvider; }
    
  private:
    Provider *mProvider;
};

ProviderDialog::ProviderDialog( Engine *engine, QWidget *parent ) :
  KDialogBase( Plain, i18n("Hot New Stuff Providers"), Ok | Cancel, Cancel,
               parent, 0, false, true ),
  mEngine( engine )
{
  QFrame *topPage = plainPage();
  
  QBoxLayout *topLayout = new QVBoxLayout( topPage );
  
  mListView = new KListView( topPage );
  mListView->addColumn( i18n("Name") );
  topLayout->addWidget( mListView );
}

void ProviderDialog::addProvider( Provider *provider )
{
  new ProviderItem( mListView, provider );
}

void ProviderDialog::slotOk()
{
  ProviderItem *item = static_cast<ProviderItem *>( mListView->selectedItem() );
  if ( !item ) {
    KMessageBox::error( this, i18n("No provider selected.") );
    return;
  }

  mEngine->requestMetaInformation( item->provider() );

  accept();
}
