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

#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KCalCore/Attendee>
#include <KCalCore/Person>

#include <KPIMUtils/Email>

#include <QIcon>
#include <KLocalizedString>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

PublishDialog::PublishDialog( QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( i18n( "Select Addresses" ) );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  //QT5 setHelp( QLatin1String("group-scheduling"), QLatin1String("korganizer") );
  QWidget *widget = new QWidget( this );
  widget->setObjectName( QLatin1String("PublishFreeBusy") );
  mUI.setupUi( widget );
  mainLayout->addWidget(widget);
  mainLayout->addWidget(buttonBox);

  mUI.mListWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  mUI.mNameLineEdit->setEnabled( false );
  mUI.mEmailLineEdit->setEnabled( false );

  okButton->setToolTip(i18n( "Send email to these recipients"  ));
  okButton->setWhatsThis(i18n( "Clicking the <b>Ok</b> button will cause "
                                "an email to be sent to the recipients you "
                                "have entered." ) );
  buttonBox->button(QDialogButtonBox::Cancel)->setToolTip(i18n( "Cancel recipient selection and the email"  ));
  buttonBox->button(QDialogButtonBox::Cancel)->setWhatsThis(i18n( "Clicking the <b>Cancel</b> button will cause the email operation to be terminated."  ));

  buttonBox->button(QDialogButtonBox::Help)->setWhatsThis(i18n( "Click the <b>Help</b> button to read more information about Group Scheduling."  ));

  mUI.mAdd->setIcon( QIcon::fromTheme( QLatin1String("list-add") ) );
  mUI.mRemove->setIcon( QIcon::fromTheme( QLatin1String("list-remove") ) );
  mUI.mRemove->setEnabled( false );
  mUI.mSelectAddressee->setIcon( QIcon::fromTheme( QLatin1String("view-pim-contacts") ) );

  connect( mUI.mListWidget, SIGNAL(itemSelectionChanged()),
           SLOT(updateInput()) );
  connect( mUI.mAdd, SIGNAL(clicked()),
           SLOT(addItem()) );
  connect( mUI.mRemove, SIGNAL(clicked()),
           SLOT(removeItem()) );
  connect( mUI.mSelectAddressee, SIGNAL(clicked()),
           SLOT(openAddressbook()) );
  connect( mUI.mNameLineEdit, SIGNAL(textChanged(QString)),
           SLOT(updateItem()) );
  connect( mUI.mEmailLineEdit, SIGNAL(textChanged(QString)),
           SLOT(updateItem()) );
}

PublishDialog::~PublishDialog()
{
}

void PublishDialog::addAttendee( const KCalCore::Attendee::Ptr &attendee )
{
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
  KCalCore::Person person( attendee->name(), attendee->email() );
  item->setText( person.fullName() );
  mUI.mListWidget->addItem( item );
  mUI.mRemove->setEnabled( !mUI.mListWidget->selectedItems().isEmpty() );
}

QString PublishDialog::addresses()
{
  QString to;
  const int count = mUI.mListWidget->count();
  for ( int i=0; i<count; ++i ) {
    const QListWidgetItem *item = mUI.mListWidget->item( i );
    if( !item->text().isEmpty() ) {
      to += item->text();
      if ( i < count-1 ) {
        to += QLatin1String(", ");
      }
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
  if ( mUI.mListWidget->selectedItems().isEmpty() ) {
    return;
  }
  QListWidgetItem *item;
  item = mUI.mListWidget->selectedItems().first();

  int row = mUI.mListWidget->row( item );
  delete mUI.mListWidget->takeItem( row );

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
  Akonadi::EmailAddressSelectionDialog dlg( this );
  if ( !dlg.exec() ) {
    return;
  }

  const Akonadi::EmailAddressSelection::List selections = dlg.selectedAddresses();
  if ( !selections.isEmpty() ) {
    foreach ( const Akonadi::EmailAddressSelection &selection, selections ) {
      mUI.mNameLineEdit->setEnabled( true );
      mUI.mEmailLineEdit->setEnabled( true );
      QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
      mUI.mListWidget->setItemSelected( item, true );
      mUI.mNameLineEdit->setText( selection.name() );
      mUI.mEmailLineEdit->setText( selection.email() );
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

  KCalCore::Person person( mUI.mNameLineEdit->text(), mUI.mEmailLineEdit->text() );
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

