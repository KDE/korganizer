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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#endif

#include <libkcal/incidence.h>

#include "koprefs.h"

#include "koeditordetails.h"
#include "koeditordetails.moc"


AttendeeListItem::AttendeeListItem(Attendee *a, QListView *parent) :
  QListViewItem(parent)
{
  mAttendee = a;
  updateItem();
}

AttendeeListItem::~AttendeeListItem()
{
  delete mAttendee;
}

void AttendeeListItem::updateItem()
{
  setText(0,mAttendee->name());
  setText(1,mAttendee->email());
  setText(2,mAttendee->roleStr());
  setText(3,mAttendee->statusStr());
  if (mAttendee->RSVP() && !mAttendee->email().isEmpty())
    setPixmap(4,SmallIcon("mailappt"));
  else
    setPixmap(4,SmallIcon("nomailappt"));
}


KOEditorDetails::KOEditorDetails (int spacing,QWidget* parent,const char* name)
  : QWidget( parent, name), mDisableItemUpdate( false )
{
  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setSpacing(spacing);

  mListView = new QListView(this,"mListView");
  mListView->addColumn(i18n("Name"),180);
  mListView->addColumn(i18n("Email"),180);
  mListView->addColumn(i18n("Role"),60);
  mListView->addColumn(i18n("Status"),100);
  mListView->addColumn(i18n("RSVP"),35);
  topLayout->addMultiCellWidget(mListView,0,0,0,5);

  connect(mListView,SIGNAL(selectionChanged(QListViewItem *)),
          SLOT(updateAttendeeInput()));

  QLabel *attendeeLabel = new QLabel(this);
  attendeeLabel->setText(i18n("Attendee Name:"));
  topLayout->addWidget(attendeeLabel,1,0);

  mNameEdit = new QLineEdit(this);
  mNameEdit->setText("");
  topLayout->addMultiCellWidget(mNameEdit,1,1,1,4);
  connect(mNameEdit,SIGNAL(textChanged(const QString &)),
          SLOT(updateAttendeeItem()));

  QLabel *emailLabel = new QLabel(this);
  emailLabel->setText(i18n("Email Address:"));
  topLayout->addWidget(emailLabel,2,0);

  mEmailEdit = new QLineEdit(this);
  mEmailEdit->setText("");
  topLayout->addMultiCellWidget(mEmailEdit,2,2,1,4);
  connect(mEmailEdit,SIGNAL(textChanged(const QString &)),
          SLOT(updateAttendeeItem()));

  QLabel *attendeeRoleLabel = new QLabel(this);
  attendeeRoleLabel->setText(i18n("Role:"));
  topLayout->addWidget(attendeeRoleLabel,3,0);

  mRoleCombo = new QComboBox(false,this);
  mRoleCombo->insertStringList(Attendee::roleList());
  topLayout->addWidget(mRoleCombo,3,1);
  connect(mRoleCombo,SIGNAL(activated(int)),SLOT(updateAttendeeItem()));

  topLayout->setColStretch(2,1);

  QLabel *statusLabel = new QLabel(this);
  statusLabel->setText( i18n("Status:") );
  topLayout->addWidget(statusLabel,3,3);

  mStatusCombo = new QComboBox(false,this);
  mStatusCombo->insertStringList(Attendee::statusList());
  topLayout->addWidget(mStatusCombo,3,4);
  connect(mStatusCombo,SIGNAL(activated(int)),SLOT(updateAttendeeItem()));

  mRsvpButton = new QCheckBox(this);
  mRsvpButton->setText(i18n("Request Response"));
  topLayout->addMultiCellWidget(mRsvpButton,4,4,0,4);
  connect(mRsvpButton,SIGNAL(clicked()),SLOT(updateAttendeeItem()));

  QWidget *buttonBox = new QWidget(this);
  QVBoxLayout *buttonLayout = new QVBoxLayout(buttonBox);

  topLayout->addMultiCellWidget(buttonBox,1,4,5,5);

  QPushButton *newButton = new QPushButton(i18n("&New"),buttonBox);
  buttonLayout->addWidget(newButton);
  connect(newButton,SIGNAL(clicked()),SLOT(addNewAttendee()));

  mRemoveButton = new QPushButton(i18n("&Remove"),buttonBox);
  buttonLayout->addWidget(mRemoveButton);
  connect(mRemoveButton, SIGNAL(clicked()),SLOT(removeAttendee()));

  mAddressBookButton = new QPushButton(i18n("Address &Book..."),buttonBox);
  buttonLayout->addWidget(mAddressBookButton);
  connect(mAddressBookButton,SIGNAL(clicked()),SLOT(openAddressBook()));

#ifdef KORG_NOKABC
  mAddressBookButton->hide();
#endif

  updateAttendeeInput();
}

KOEditorDetails::~KOEditorDetails()
{
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)mListView->selectedItem();
  if (!aItem) return;

  delete aItem;

  updateAttendeeInput();
}


void KOEditorDetails::openAddressBook()
{
#ifndef KORG_NOKABC
  KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
  if (!a.isEmpty()) {
    insertAttendee( new Attendee( a.realName(), a.preferredEmail() ) );
  }
#endif
}


void KOEditorDetails::addNewAttendee()
{
#if 0
  // this is cool.  If they didn't enter an email address,
  // try to look it up in the address book and fill it in for them.
  if (QString(mEmailEdit->text()).stripWhiteSpace().isEmpty()) {
    KabAPI addrBook;
    QString name;
    std::list<AddressBook::Entry> entries;
    name = mNameEdit->text();
    if (addrBook.init() == AddressBook::NoError) {
      if (addrBook.getEntryByName(name, entries, 1) == AddressBook::NoError) {
	kdDebug() << "positive match" << endl;
	// take first email address
	if (!entries.front().emails.isEmpty() &&
	    entries.front().emails.first().length()>0)
	  mEmailEdit->setText(entries.front().emails.first());
      }
    }
  }
#endif

  Attendee *a = new Attendee(i18n("(EmptyName)"),i18n("(EmptyEmail)"));
  insertAttendee(a);
}


void KOEditorDetails::insertAttendee(Attendee *a)
{
  AttendeeListItem *item = new AttendeeListItem(a,mListView);
  mListView->setSelected( item, true );
}

void KOEditorDetails::setDefaults()
{
  mRsvpButton->setChecked(true);
}

void KOEditorDetails::readEvent(Incidence *event)
{
  QPtrList<Attendee> tmpAList = event->attendees();
  Attendee *a;
  for (a = tmpAList.first(); a; a = tmpAList.next())
    insertAttendee(new Attendee(*a));

  mListView->setSelected( mListView->firstChild(), true );
}

void KOEditorDetails::writeEvent(Incidence *event)
{
  event->clearAttendees();
  QListViewItem *item;
  AttendeeListItem *a;
  for (item = mListView->firstChild(); item;
       item = item->nextSibling()) {
    a = (AttendeeListItem *)item;
    event->addAttendee(new Attendee(*(a->attendee())));
  }
}

bool KOEditorDetails::validateInput()
{
  return true;
}

void KOEditorDetails::updateAttendeeInput()
{
  QListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if (aItem) {
    fillAttendeeInput( aItem );
  } else {
    clearAttendeeInput();    
  }
}

void KOEditorDetails::clearAttendeeInput()
{
  mNameEdit->setText("");
  mEmailEdit->setText("");
  mRoleCombo->setCurrentItem(0);
  mStatusCombo->setCurrentItem(0);
  mRsvpButton->setChecked(true);

  setEnabledAttendeeInput( false );
}

void KOEditorDetails::fillAttendeeInput( AttendeeListItem *aItem )
{
  Attendee *a = aItem->attendee();

  mDisableItemUpdate = true;

  mNameEdit->setText(a->name());
  mEmailEdit->setText(a->email());
  mRoleCombo->setCurrentItem(a->role());
  mStatusCombo->setCurrentItem(a->status());
  mRsvpButton->setChecked(a->RSVP());

  mDisableItemUpdate = false;

  setEnabledAttendeeInput( true );
}

void KOEditorDetails::setEnabledAttendeeInput( bool enabled )
{
  mNameEdit->setEnabled( enabled );
  mEmailEdit->setEnabled( enabled );
  mRoleCombo->setEnabled( enabled );
  mStatusCombo->setEnabled( enabled );
  mRsvpButton->setEnabled( enabled );

  mRemoveButton->setEnabled( enabled );
}

void KOEditorDetails::updateAttendeeItem()
{
  if (mDisableItemUpdate) return;

  QListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if ( !aItem ) return;

  Attendee *a = aItem->attendee();

  a->setName( mNameEdit->text() );
  a->setEmail( mEmailEdit->text() );
  a->setRole( Attendee::Role( mRoleCombo->currentItem() ) );
  a->setStatus( Attendee::PartStat( mStatusCombo->currentItem() ) );
  a->setRSVP( mRsvpButton->isChecked() );

  aItem->updateItem();
}
