/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlineedit.h>
#include <qpushbutton.h>
#include <kdebug.h>

#include <kglobal.h>
#include <klocale.h>
#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#endif

#include "koprefs.h"
#include "publishdialog.h"
#include "publishdialog_base.h"

PublishDialog::PublishDialog( QWidget* parent, const char* name,
                              bool modal )
  : KDialogBase( parent, name, modal,
    i18n("Select Addresses"), Ok|Cancel|Help, Ok, true )
{
  mWidget = new PublishDialog_base( this, "PublishFreeBusy" );
  setMainWidget( mWidget );

  mWidget->mNameLineEdit->setEnabled( false );
  mWidget->mEmailLineEdit->setEnabled( false );
  connect( mWidget->mAddressListView, SIGNAL( selectionChanged(QListViewItem *) ),
           SLOT(updateInput()));
  connect( mWidget->mNew, SIGNAL( clicked() ),
           SLOT( addItem() ) );
  connect( mWidget->mRemove, SIGNAL( clicked() ),
           SLOT( removeItem() ) );
  connect( mWidget->mSelectAddressee, SIGNAL( clicked() ),
           SLOT( openAddressbook() ) );
  connect( mWidget->mNameLineEdit, SIGNAL( textChanged(const QString&) ),
           SLOT( updateItem() ) );
  connect( mWidget->mEmailLineEdit, SIGNAL( textChanged(const QString&) ),
           SLOT( updateItem() ) );
}

PublishDialog::~PublishDialog()
{
}

void PublishDialog::addAttendee( Attendee *attendee )
{
  mWidget->mNameLineEdit->setEnabled( true );
  mWidget->mEmailLineEdit->setEnabled( true );
  QListViewItem *item = new QListViewItem( mWidget->mAddressListView );
  item->setText( 0, attendee->name() );
  item->setText( 1, attendee->email() );
  mWidget->mAddressListView->insertItem( item );
}

QString PublishDialog::addresses()
{
  QString to = "";
  QListViewItem *item;
  int i, count;
  count = mWidget->mAddressListView->childCount();
  for ( i=0; i<count; i++ ) {
    item = mWidget->mAddressListView->firstChild();
    mWidget->mAddressListView->takeItem( item );
    to += item->text( 1 );
    if ( i < count-1 ) {
      to += ", ";
    }
  }
  return to;
}

void PublishDialog::addItem()
{
  mWidget->mNameLineEdit->setEnabled( true );
  mWidget->mEmailLineEdit->setEnabled( true );
  QListViewItem *item = new QListViewItem( mWidget->mAddressListView );
  mWidget->mAddressListView->insertItem( item );
  mWidget->mAddressListView->setSelected( item, true );
  mWidget->mNameLineEdit->setText( i18n("(EmptyName)") );
  mWidget->mEmailLineEdit->setText( i18n("(EmptyEmail)") );
}

void PublishDialog::removeItem()
{
  QListViewItem *item;
  item = mWidget->mAddressListView->selectedItem();
  if (!item) return;
  mWidget->mAddressListView->takeItem( item );
  item = mWidget->mAddressListView->selectedItem();
  if ( !item ) {
    mWidget->mNameLineEdit->setText( "" );
    mWidget->mEmailLineEdit->setText( "" );
    mWidget->mNameLineEdit->setEnabled( false );
    mWidget->mEmailLineEdit->setEnabled( false );
  }
  if ( mWidget->mAddressListView->childCount() == 0 ) {
    mWidget->mNameLineEdit->setEnabled( false );
    mWidget->mEmailLineEdit->setEnabled( false );
  }
}

void PublishDialog::openAddressbook()
{
#ifndef KORG_NOKABC
  KABC::Addressee::List addressList;
  addressList = KABC::AddresseeDialog::getAddressees( this );
  //KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
  KABC::Addressee a = addressList.first();
  if ( !a.isEmpty() ) {
    uint i;
    for ( i=0; i<addressList.size(); i++ ) {
      a = addressList[i];
      mWidget->mNameLineEdit->setEnabled( true );
      mWidget->mEmailLineEdit->setEnabled( true );
      QListViewItem *item = new QListViewItem( mWidget->mAddressListView );
      mWidget->mAddressListView->setSelected( item, true );
      mWidget->mNameLineEdit->setText( a.realName() );
      mWidget->mEmailLineEdit->setText( a.preferredEmail() );
      mWidget->mAddressListView->insertItem( item );
    }
  }
#endif
}

void PublishDialog::updateItem()
{
  QListViewItem *item;
  item = mWidget->mAddressListView->selectedItem();
  if (!item) return;
  item->setText( 0, mWidget->mNameLineEdit->text() );
  item->setText( 1, mWidget->mEmailLineEdit->text() );
} 

void PublishDialog::updateInput()
{
  QListViewItem *item;
  item = mWidget->mAddressListView->selectedItem();
  if (!item) return;
  mWidget->mNameLineEdit->setEnabled( true );
  mWidget->mEmailLineEdit->setEnabled( true );
  QString mail = item->text( 1 );
  mWidget->mNameLineEdit->setText( item->text( 0 ) );
  mWidget->mEmailLineEdit->setText( mail );
}

#include "publishdialog.moc"
