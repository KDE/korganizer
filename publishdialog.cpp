/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2009 Allen Winter <winter@kde.org>

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

// TODO: validate hand-entered email addresses
// TODO: don't allow duplicates; at least remove dupes when passing back
// TODO: the list in PublishDialog::addresses()

#include "publishdialog.h"

#include <kabc/addresseedialog.h>
#include <kcal/attendee.h>
#include <kcal/person.h>
#include <kpimutils/email.h>

#include <klineedit.h>
#include <klocale.h>

PublishDialog::PublishDialog( QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Select Addresses" ) );
  setButtons( Ok|Cancel|Help );
  setHelp( "group-scheduling", "korganizer" );
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "PublishFreeBusy" );
  mUI.setupUi( widget );
  setMainWidget( widget );
  mUI.mListWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  mUI.mNameLineEdit->setEnabled( false );
  mUI.mEmailLineEdit->setEnabled( false );

  setButtonToolTip( Ok, i18n( "Send email to these recipients" ) );
  setButtonWhatsThis( Ok, i18n( "Clicking the <b>Ok</b> button will cause "
                                "an email to be sent to the recipients you "
                                "have entered." ) );
  setButtonToolTip( Cancel, i18n( "Cancel recipient selection and the email" ) );
  setButtonWhatsThis( Cancel, i18n( "Clicking the <b>Cancel</b> button will "
                                    "cause the email operation to be terminated." ) );

  setButtonWhatsThis( Help, i18n( "Click the <b>Help</b> button to read "
                                  "more information about Group Scheduling." ) );

  mUI.mNew->setIcon( KIcon( "list-add" ) );
  mUI.mRemove->setIcon( KIcon( "list-remove" ) );
  mUI.mRemove->setEnabled( false );
  mUI.mSelectAddressee->setIcon( KIcon( "view-pim-contacts" ) );

  connect( mUI.mListWidget, SIGNAL(itemSelectionChanged()),
           SLOT(updateInput()) );
  connect( mUI.mNew, SIGNAL(clicked()),
           SLOT(addItem()) );
  connect( mUI.mRemove, SIGNAL(clicked()),
           SLOT(removeItem()) );
  connect( mUI.mSelectAddressee, SIGNAL(clicked()),
           SLOT(openAddressbook()) );
  connect( mUI.mNameLineEdit, SIGNAL(textChanged(const QString &)),
           SLOT(updateItem()) );
  connect( mUI.mEmailLineEdit, SIGNAL(textChanged(const QString &)),
           SLOT(updateItem()) );
}

PublishDialog::~PublishDialog()
{
}

void PublishDialog::addAttendee( Attendee *attendee )
{
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
  Person person( attendee->name(), attendee->email() );
  item->setText( person.fullName() );
  mUI.mListWidget->addItem( item );
  mUI.mRemove->setEnabled( true );
}

QString PublishDialog::addresses()
{
  QString to = "";
  QListWidgetItem *item;
  int i, count;
  count = mUI.mListWidget->count();
  for ( i=0; i<count; i++ ) {
    item = mUI.mListWidget->takeItem( i );
    to += item->text();
    if ( i < count-1 ) {
      to += ", ";
    }
  }
  return to;
}

void PublishDialog::addItem()
{
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
  mUI.mListWidget->addItem( item );
  mUI.mListWidget->setItemSelected( item, true );
  mUI.mNameLineEdit->setText( i18n( "(EmptyName)" ) );
  mUI.mEmailLineEdit->setText( i18n( "(EmptyEmail)" ) );

  mUI.mRemove->setEnabled( true );
}

void PublishDialog::removeItem()
{
  QListWidgetItem *item;
  item = mUI.mListWidget->selectedItems().first();
  if ( !item ) {
    return;
  }

  int row = mUI.mListWidget->row( item );
  mUI.mListWidget->takeItem( row );

  if ( !mUI.mListWidget->count() ) {
    mUI.mNameLineEdit->setText( QString() );
    mUI.mNameLineEdit->setEnabled( false );
    mUI.mEmailLineEdit->setText( QString() );
    mUI.mEmailLineEdit->setEnabled( false );
    mUI.mRemove->setEnabled( false );
    return;
  }
  if ( row > 0 ) {
    row--;
  }

  mUI.mListWidget->setCurrentRow( row );
}

void PublishDialog::openAddressbook()
{
  KABC::Addressee::List addressList = KABC::AddresseeDialog::getAddressees( this );
  if( addressList.isEmpty() ) {
     return;
  }

  KABC::Addressee a = addressList.first();
  if ( !a.isEmpty() ) {
    int i;
    for ( i=0; i<addressList.size(); i++ ) {
      a = addressList[i];
      mUI.mNameLineEdit->setEnabled( true );
      mUI.mEmailLineEdit->setEnabled( true );
      QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
      mUI.mListWidget->setItemSelected( item, true );
      mUI.mNameLineEdit->setText( a.realName() );
      mUI.mEmailLineEdit->setText( a.preferredEmail() );
      mUI.mListWidget->addItem( item );
    }
    mUI.mRemove->setEnabled( true );
  }
}

void PublishDialog::updateItem()
{
  if ( !mUI.mListWidget->selectedItems().count() ) {
    return;
  }

  Person person( mUI.mNameLineEdit->text(), mUI.mEmailLineEdit->text() );
  QListWidgetItem *item = mUI.mListWidget->selectedItems().first();
  item->setText( person.fullName() );
}

void PublishDialog::updateInput()
{
  if ( !mUI.mListWidget->selectedItems().count() ) {
    return;
  }

  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );

  QListWidgetItem *item = mUI.mListWidget->selectedItems().first();
  QString mail, name;
  KPIMUtils::extractEmailAddressAndName( item->text(), mail, name );
  mUI.mNameLineEdit->setText( name );
  mUI.mEmailLineEdit->setText( mail );
}

#include "publishdialog.moc"
