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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "publishdialog.h"
#include "koprefs.h"

#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#endif
#include <kcal/attendee.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <q3listview.h>
#include <QLineEdit>
#include <QPushButton>

PublishDialog::PublishDialog( QWidget* parent, bool modal )
  : KDialog( parent )
{
  setCaption( i18n("Select Addresses") );
  setButtons( Ok|Cancel|Help );
  setHelp(QString(), "korganizer");
  setModal( modal );
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "PublishFreeBusy" );
  mUI.setupUi( widget );
  setMainWidget( widget );
  mUI.mNameLineEdit->setEnabled( false );
  mUI.mEmailLineEdit->setEnabled( false );
  connect( mUI.mAddressListView, SIGNAL( selectionChanged(Q3ListViewItem *) ),
           SLOT(updateInput()));
  connect( mUI.mNew, SIGNAL( clicked() ),
           SLOT( addItem() ) );
  connect( mUI.mRemove, SIGNAL( clicked() ),
           SLOT( removeItem() ) );
  connect( mUI.mSelectAddressee, SIGNAL( clicked() ),
           SLOT( openAddressbook() ) );
  connect( mUI.mNameLineEdit, SIGNAL( textChanged(const QString&) ),
           SLOT( updateItem() ) );
  connect( mUI.mEmailLineEdit, SIGNAL( textChanged(const QString&) ),
           SLOT( updateItem() ) );
}

PublishDialog::~PublishDialog()
{
}

void PublishDialog::addAttendee( Attendee *attendee )
{
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  Q3ListViewItem *item = new Q3ListViewItem( mUI.mAddressListView );
  item->setText( 0, attendee->name() );
  item->setText( 1, attendee->email() );
  mUI.mAddressListView->insertItem( item );
}

QString PublishDialog::addresses()
{
  QString to = "";
  Q3ListViewItem *item;
  int i, count;
  count = mUI.mAddressListView->childCount();
  for ( i=0; i<count; i++ ) {
    item = mUI.mAddressListView->firstChild();
    mUI.mAddressListView->takeItem( item );
    if( !item->text( 1 ).isEmpty() ) {
      to += item->text( 1 );
      if ( i < count-1 ) {
        to += ", ";
      }
    }
  }
  return to;
}

void PublishDialog::addItem()
{
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  Q3ListViewItem *item = new Q3ListViewItem( mUI.mAddressListView );
  mUI.mAddressListView->insertItem( item );
  mUI.mAddressListView->setSelected( item, true );
  mUI.mNameLineEdit->setText( i18n("(EmptyName)") );
  mUI.mEmailLineEdit->setText( i18n("(EmptyEmail)") );
}

void PublishDialog::removeItem()
{
  Q3ListViewItem *item;
  item = mUI.mAddressListView->selectedItem();
  if (!item) return;
  mUI.mAddressListView->takeItem( item );
  item = mUI.mAddressListView->selectedItem();
  if ( !item ) {
    mUI.mNameLineEdit->setText( "" );
    mUI.mEmailLineEdit->setText( "" );
    mUI.mNameLineEdit->setEnabled( false );
    mUI.mEmailLineEdit->setEnabled( false );
  }
  if ( mUI.mAddressListView->childCount() == 0 ) {
    mUI.mNameLineEdit->setEnabled( false );
    mUI.mEmailLineEdit->setEnabled( false );
  }
}

void PublishDialog::openAddressbook()
{
#ifndef KORG_NOKABC
  KABC::Addressee::List addressList;
  addressList = KABC::AddresseeDialog::getAddressees( this );
  //KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
  if( addressList.isEmpty())
     return;
  KABC::Addressee a = addressList.first();
  if ( !a.isEmpty() ) {
    int i;
    for ( i=0; i<addressList.size(); i++ ) {
      a = addressList[i];
      mUI.mNameLineEdit->setEnabled( true );
      mUI.mEmailLineEdit->setEnabled( true );
      Q3ListViewItem *item = new Q3ListViewItem( mUI.mAddressListView );
      mUI.mAddressListView->setSelected( item, true );
      mUI.mNameLineEdit->setText( a.realName() );
      mUI.mEmailLineEdit->setText( a.preferredEmail() );
      mUI.mAddressListView->insertItem( item );
    }
  }
#endif
}

void PublishDialog::updateItem()
{
  Q3ListViewItem *item;
  item = mUI.mAddressListView->selectedItem();
  if (!item) return;
  item->setText( 0, mUI.mNameLineEdit->text() );
  item->setText( 1, mUI.mEmailLineEdit->text() );
}

void PublishDialog::updateInput()
{
  Q3ListViewItem *item;
  item = mUI.mAddressListView->selectedItem();
  if (!item) return;
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  mUI.mNameLineEdit->setText( item->text( 0 ) );
  mUI.mEmailLineEdit->setText( item->text( 1 ) );
}

#include "publishdialog.moc"
