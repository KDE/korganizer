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

#include "engine.h"
#include "entry.h"

#include "downloaddialog.h"
#include "downloaddialog.moc"

using namespace KNS;

class KNewStuffItem : public KListViewItem
{
  public:
    KNewStuffItem( KListView *parent, Entry *entry ) :
      KListViewItem( parent ), mEntry( entry )
    {
      setText( 0, entry->name() );
      setText( 1, entry->summary() );
      setText( 2, entry->version() );
      setText( 3, QString::number( entry->release() ) );
    }
      
    Entry *entry() { return mEntry; }
    
  private:
    Entry *mEntry;
};

DownloadDialog::DownloadDialog( Engine *newStuff, QWidget *parent ) :
  KDialogBase( Plain, i18n("Get Hot New Stuff"), Ok | Apply | Cancel, Cancel,
               parent, 0, false, true ),
  mNewStuff( newStuff )
{
  QFrame *topPage = plainPage();
  
  QBoxLayout *topLayout = new QVBoxLayout( topPage );
  
  mListView = new KListView( topPage );
  mListView->addColumn( i18n("Name") );
  mListView->addColumn( i18n("Summary") );
  mListView->addColumn( i18n("Version") );
  mListView->addColumn( i18n("Release") );
  topLayout->addWidget( mListView );
}

void DownloadDialog::addEntry( Entry *entry )
{
  new KNewStuffItem( mListView, entry );
}

void DownloadDialog::clear()
{
  mListView->clear();
}

void DownloadDialog::slotApply()
{
  KNewStuffItem *item =
    static_cast<KNewStuffItem *>( mListView->selectedItem() );
    
  if ( !item ) {
    return;
  }

  mNewStuff->download( item->entry() );
}

void DownloadDialog::slotOk()
{
  slotApply();
  
  accept();
}
