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

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

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

template <>
CustomListViewItem<class Attendee *>::~CustomListViewItem()
{
  delete mData;
}

template <>
void CustomListViewItem<class Attendee *>::updateItem()
{
  setText(0,mData->name());
  setText(1,mData->email());
  setText(2,mData->roleStr());
  setText(3,mData->statusStr());
  if (mData->RSVP() && !mData->email().isEmpty())
    setPixmap(4,SmallIcon("mailappt"));
  else
    setPixmap(4,SmallIcon("nomailappt"));
}


KOEditorDetails::KOEditorDetails (int spacing,QWidget* parent,const char* name)
  : QWidget( parent, name), mDisableItemUpdate( false )
{
  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setSpacing(spacing);

  QString organizer = KOPrefs::instance()->email();
  mOrganizerLabel = new QLabel(i18n("Organizer: %1").arg(organizer),this);

  mListView = new KListView(this,"mListView");
  mListView->addColumn(i18n("Name"),180);
  mListView->addColumn(i18n("Email"),180);
  mListView->addColumn(i18n("Role"),60);
  mListView->addColumn(i18n("Status"),100);
  mListView->addColumn(i18n("RSVP"),35);
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mListView->setFixedHeight(78);
  }

  connect(mListView,SIGNAL(selectionChanged(QListViewItem *)),
          SLOT(updateAttendeeInput()));

  QLabel *attendeeLabel = new QLabel(this);
  attendeeLabel->setText(i18n("Name:"));

  mNameEdit = new QLineEdit(this);
  connect(mNameEdit,SIGNAL(textChanged(const QString &)),
          SLOT(updateAttendeeItem()));

  mUidEdit = new QLineEdit(0);
  mUidEdit->setText("");

  QLabel *emailLabel = new QLabel(this);
  emailLabel->setText(i18n("Email:"));

  mEmailEdit = new QLineEdit(this);
  connect(mEmailEdit,SIGNAL(textChanged(const QString &)),
          SLOT(updateAttendeeItem()));

  QLabel *attendeeRoleLabel = new QLabel(this);
  attendeeRoleLabel->setText(i18n("Role:"));

  mRoleCombo = new QComboBox(false,this);
  mRoleCombo->insertStringList(Attendee::roleList());
  connect(mRoleCombo,SIGNAL(activated(int)),SLOT(updateAttendeeItem()));

  QLabel *statusLabel = new QLabel(this);
  statusLabel->setText( i18n("Status:") );

  mStatusCombo = new QComboBox(false,this);
  mStatusCombo->insertStringList(Attendee::statusList());
  connect(mStatusCombo,SIGNAL(activated(int)),SLOT(updateAttendeeItem()));

  mRsvpButton = new QCheckBox(this);
  mRsvpButton->setText(i18n("Request response"));
  connect(mRsvpButton,SIGNAL(clicked()),SLOT(updateAttendeeItem()));

  QWidget *buttonBox = new QWidget(this);
  QVBoxLayout *buttonLayout = new QVBoxLayout(buttonBox);

  QPushButton *newButton = new QPushButton(i18n("&New"),buttonBox);
  buttonLayout->addWidget(newButton);
  connect(newButton,SIGNAL(clicked()),SLOT(addNewAttendee()));

  mRemoveButton = new QPushButton(i18n("&Remove"),buttonBox);
  buttonLayout->addWidget(mRemoveButton);
  connect(mRemoveButton, SIGNAL(clicked()),SLOT(removeAttendee()));

  mAddressBookButton = new QPushButton(i18n("Address &Book..."),buttonBox);
  buttonLayout->addWidget(mAddressBookButton);
  connect(mAddressBookButton,SIGNAL(clicked()),SLOT(openAddressBook()));

  topLayout->addMultiCellWidget(mOrganizerLabel,0,0,0,5);
  topLayout->addMultiCellWidget(mListView,1,1,0,5);
  topLayout->addWidget(attendeeLabel,2,0);
  topLayout->addMultiCellWidget(mNameEdit,2,2,1,1);
  topLayout->addWidget(emailLabel,3,0);
  topLayout->addMultiCellWidget(mEmailEdit,3,3,1,1);
  topLayout->addWidget(attendeeRoleLabel,4,0);
  topLayout->addWidget(mRoleCombo,4,1);
#if 0
  topLayout->setColStretch(2,1);
  topLayout->addWidget(statusLabel,3,3);
  topLayout->addWidget(mStatusCombo,3,4);
#else
  topLayout->addWidget(statusLabel,5,0);
  topLayout->addWidget(mStatusCombo,5,1);
#endif
  topLayout->addMultiCellWidget(mRsvpButton,6,6,0,1);
  topLayout->addMultiCellWidget(buttonBox,2,5,5,5);

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

  Attendee *delA = new Attendee(aItem->data()->name(),aItem->data()->email(),
    aItem->data()->RSVP(),aItem->data()->status(),aItem->data()->role(),
    aItem->data()->uid());
  mdelAttendees.append(delA);

  delete aItem;

  updateAttendeeInput();
}


void KOEditorDetails::openAddressBook()
{
#ifndef KORG_NOKABC
  KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
  if (!a.isEmpty()) {
    insertAttendee( new Attendee( a.realName(), a.preferredEmail(),false,KCal::Attendee::NeedsAction,KCal::Attendee::ReqParticipant,a.uid()) );
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
	kdDebug(5850) << "positive match" << endl;
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
  mListView->clear();
  mdelAttendees.clear();
  Attendee::List al = event->attendees();
  Attendee::List::ConstIterator it;
  for( it = al.begin(); it != al.end(); ++it )
    insertAttendee( new Attendee( **it ) );

  mListView->setSelected( mListView->firstChild(), true );
  mOrganizerLabel->setText(i18n("Organizer: %1").arg(event->organizer()));
}

void KOEditorDetails::writeEvent(Incidence *event)
{
  event->clearAttendees();
  QListViewItem *item;
  AttendeeListItem *a;
  for (item = mListView->firstChild(); item;
       item = item->nextSibling()) {
    a = (AttendeeListItem *)item;
    event->addAttendee(new Attendee(*(a->data())));
  }
}

void KOEditorDetails::cancelAttendeeEvent(Incidence *event)
{
  event->clearAttendees();
  Attendee * att;
  for (att=mdelAttendees.first();att;att=mdelAttendees.next()) {
    event->addAttendee(new Attendee(*att));
  }
  mdelAttendees.clear();
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
  mUidEdit->setText("");
  mEmailEdit->setText("");
  mRoleCombo->setCurrentItem(0);
  mStatusCombo->setCurrentItem(0);
  mRsvpButton->setChecked(true);
  setEnabledAttendeeInput( false );
}

void KOEditorDetails::fillAttendeeInput( AttendeeListItem *aItem )
{
  Attendee *a = aItem->data();
  mDisableItemUpdate = true;
  mNameEdit->setText(a->name());
  mUidEdit->setText(a->uid());
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

  Attendee *a = aItem->data();

  a->setName( mNameEdit->text() );
  a->setUid( mUidEdit->text() );
  a->setEmail( mEmailEdit->text() );
  a->setRole( Attendee::Role( mRoleCombo->currentItem() ) );
  a->setStatus( Attendee::PartStat( mStatusCombo->currentItem() ) );
  a->setRSVP( mRsvpButton->isChecked() );
  aItem->updateItem();
}
