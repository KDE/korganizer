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
#include <qdragobject.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#include <kabc/vcardconverter.h>
#endif
#include <libkdepim/kvcarddrag.h>

#include <libkcal/incidence.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorfreebusy.h"

#include "koeditordetails.h"

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
    setPixmap(4,KOGlobals::self()->smallIcon("mailappt"));
  else
    setPixmap(4,KOGlobals::self()->smallIcon("nomailappt"));
}

KOAttendeeListView::KOAttendeeListView (QWidget *parent, const char *name)
    : KListView(parent, name)
{
  setAcceptDrops(true);
  setAllColumnsShowFocus(true);
  setFullWidth(true);
}

/** KOAttendeeListView is a child class of KListView  which supports
 *  dropping of attendees (e.g. from kaddressbook) onto it. If an attendeee
 *  was dropped, the signal dropped(Attendee*)  is emitted. Valid drop classes
 *   are KVCardDrag and QTextDrag.
 */
KOAttendeeListView::~KOAttendeeListView()
{
}

void KOAttendeeListView::contentsDragEnterEvent( QDragEnterEvent *e )
{
  dragEnterEvent(e);
}

void KOAttendeeListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
#ifndef KORG_NODND
  if ( KVCardDrag::canDecode( e ) || QTextDrag::canDecode( e ) ) {
    e->accept();
  } else {
    e->ignore();
  }
#endif
}
void KOAttendeeListView::dragEnterEvent( QDragEnterEvent *e )
{
#ifndef KORG_NODND
  if ( KVCardDrag::canDecode( e ) || QTextDrag::canDecode( e ) ) {
    e->accept();
  } else {
    e->ignore();
  }
#endif
}

void KOAttendeeListView::addAttendee(const QString &newAttendee)
{
  kdDebug(5850) << " Email: " << newAttendee << endl;
  QString name;
  QString email;
  KPIM::AddresseeLineEdit::getNameAndMail(newAttendee, name, email);
  emit dropped(new Attendee(name,email));
}

void KOAttendeeListView::contentsDropEvent( QDropEvent *e )
{
  dropEvent(e);
}

void KOAttendeeListView::dropEvent( QDropEvent *e )
{
#ifndef KORG_NODND
  QString text;
  QString vcards;

#ifndef KORG_NOKABC
  if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::VCardConverter converter;

    KABC::Addressee::List list = converter.parseVCards( vcards );
    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      QString em( (*it).fullEmail() );
      if (em.isEmpty()) {
        em=(*it).realName();
      }
      addAttendee( em );
    }
  } else
#endif // KORG_NOKABC
  if (QTextDrag::decode(e,text)) {
    kdDebug(5850) << "Dropped : " << text << endl;
    QStringList emails = QStringList::split(",",text);
    for(QStringList::ConstIterator it = emails.begin();it!=emails.end();++it) {
      addAttendee(*it);
    }
  }
#endif //KORG_NODND
}


KOEditorDetails::KOEditorDetails (int spacing,QWidget* parent,const char* name)
  : QWidget( parent, name), mDisableItemUpdate( false ), mFreeBusy( 0 )
{
  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setSpacing(spacing);

  QString organizer = KOPrefs::instance()->email();
  mOrganizerLabel = new QLabel(i18n("Organizer: %1").arg(organizer),this);

  mListView = new KOAttendeeListView(this,"mListView");
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
#ifndef KORG_NODND
  connect(mListView, SIGNAL(dropped( Attendee *)),
          SLOT(insertAttendee(Attendee *)));
#endif

  QLabel *attendeeLabel = new QLabel(this);
  attendeeLabel->setText(i18n("Na&me:"));

  mNameEdit = new KPIM::AddresseeLineEdit(this);
  mNameEdit->setClickMessage(i18n("Click to add a new attendee"));
  attendeeLabel->setBuddy( mNameEdit );
  mNameEdit->installEventFilter(this);
  connect(mNameEdit,SIGNAL(textChanged(const QString &)),
          SLOT(updateAttendeeItem()));

  mUidEdit = new QLineEdit(0);
  mUidEdit->setText("");

  QLabel *attendeeRoleLabel = new QLabel(this);
  attendeeRoleLabel->setText(i18n("Ro&le:"));

  mRoleCombo = new QComboBox(false,this);
  mRoleCombo->insertStringList(Attendee::roleList());
  attendeeRoleLabel->setBuddy( mRoleCombo );
  connect(mRoleCombo,SIGNAL(activated(int)),SLOT(updateAttendeeItem()));

  QLabel *statusLabel = new QLabel(this);
  statusLabel->setText( i18n("Stat&us:") );

  mStatusCombo = new QComboBox(false,this);
  mStatusCombo->insertStringList(Attendee::statusList());
  statusLabel->setBuddy( mStatusCombo );
  connect(mStatusCombo,SIGNAL(activated(int)),SLOT(updateAttendeeItem()));

  mRsvpButton = new QCheckBox(this);
  mRsvpButton->setText(i18n("Re&quest response"));
  connect(mRsvpButton,SIGNAL(clicked()),SLOT(updateAttendeeItem()));

  QWidget *buttonBox = new QWidget(this);
  QVBoxLayout *buttonLayout = new QVBoxLayout(buttonBox);

  QPushButton *newButton = new QPushButton(i18n("&New"),buttonBox);
  buttonLayout->addWidget(newButton);
  connect(newButton,SIGNAL(clicked()),SLOT(addNewAttendee()));

  mRemoveButton = new QPushButton(i18n("&Remove"),buttonBox);
  buttonLayout->addWidget(mRemoveButton);
  connect(mRemoveButton, SIGNAL(clicked()),SLOT(removeAttendee()));

  mAddressBookButton = new QPushButton(i18n("Select Addressee..."),buttonBox);
  buttonLayout->addWidget(mAddressBookButton);
  connect(mAddressBookButton,SIGNAL(clicked()),SLOT(openAddressBook()));

  topLayout->addMultiCellWidget(mOrganizerLabel,0,0,0,5);
  topLayout->addMultiCellWidget(mListView,1,1,0,5);
  topLayout->addWidget(attendeeLabel,2,0);
  topLayout->addMultiCellWidget(mNameEdit,2,2,1,1);
//  topLayout->addWidget(emailLabel,3,0);
  topLayout->addWidget(attendeeRoleLabel,3,0);
  topLayout->addWidget(mRoleCombo,3,1);
#if 0
  topLayout->setColStretch(2,1);
  topLayout->addWidget(statusLabel,3,3);
  topLayout->addWidget(mStatusCombo,3,4);
#else
  topLayout->addWidget(statusLabel,4,0);
  topLayout->addWidget(mStatusCombo,4,1);
#endif
  topLayout->addMultiCellWidget(mRsvpButton,5,5,0,1);
  topLayout->addMultiCellWidget(buttonBox,2,4,5,5);

#ifdef KORG_NOKABC
  mAddressBookButton->hide();
#endif

  updateAttendeeInput();
}

KOEditorDetails::~KOEditorDetails()
{
}

bool KOEditorDetails::eventFilter( QObject *watched, QEvent *ev)
{
  if (watched && watched == mNameEdit && ev->type() == QEvent::FocusIn && mListView->childCount() == 0 )
  {
    addNewAttendee();
  }

  return QWidget::eventFilter( watched, ev );

}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)mListView->selectedItem();
  if (!aItem) return;

  Attendee *delA = new Attendee(aItem->data()->name(),aItem->data()->email(),
    aItem->data()->RSVP(),aItem->data()->status(),aItem->data()->role(),
    aItem->data()->uid());
  mdelAttendees.append(delA);

  if ( mFreeBusy ) mFreeBusy->removeAttendee( aItem->data() );
  delete aItem;

  updateAttendeeInput();
}


void KOEditorDetails::openAddressBook()
{
#ifndef KORG_NOKABC
  KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
  if (!a.isEmpty()) {
    // If this is myself, I don't want to get a response but instead
    // assume I will be available
    bool myself = a.preferredEmail() == KOPrefs::instance()->email();
    KCal::Attendee::PartStat partStat =
      myself ? KCal::Attendee::Accepted : KCal::Attendee::NeedsAction;
    insertAttendee( new Attendee( a.realName(), a.preferredEmail(),
                                  !myself, partStat,
                                  KCal::Attendee::ReqParticipant, a.uid() ) );
  }
#endif
}


void KOEditorDetails::addNewAttendee()
{
  Attendee *a = new Attendee(i18n("Firstname Lastname"),i18n("name@domain.com"));
  insertAttendee(a);
  // We don't want the hint again
  mNameEdit->setClickMessage("");
  mNameEdit->selectAll();
}


void KOEditorDetails::insertAttendee(Attendee *a)
{
  AttendeeListItem *item = new AttendeeListItem(a,mListView);
  mListView->setSelected( item, true );
  if( mFreeBusy ) mFreeBusy->insertAttendee( a );
}

void KOEditorDetails::setDefaults()
{
  mRsvpButton->setChecked(true);
}

void KOEditorDetails::readEvent(Incidence *event)
{
  // Stop flickering in the free/busy view (not sure if this is necessary)
  bool block = false;
  if( mFreeBusy ) {
    block = mFreeBusy->updateEnabled();
    mFreeBusy->setUpdateEnabled( false );
    mFreeBusy->clearAttendees();
  }

  mListView->clear();
  mdelAttendees.clear();
  Attendee::List al = event->attendees();
  Attendee::List::ConstIterator it;
  for( it = al.begin(); it != al.end(); ++it )
    insertAttendee( new Attendee( **it ) );

  mListView->setSelected( mListView->firstChild(), true );
  mOrganizerLabel->setText(i18n("Organizer: %1").arg(event->organizer()));

  // Reinstate free/busy view updates
  if( mFreeBusy ) mFreeBusy->setUpdateEnabled( block );
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

  setEnableAttendeeInput(!mNameEdit->text().isEmpty()); 
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
  mRoleCombo->setCurrentItem(0);
  mStatusCombo->setCurrentItem(0);
  mRsvpButton->setChecked(true);
  setEnableAttendeeInput( false );
}

void KOEditorDetails::fillAttendeeInput( AttendeeListItem *aItem )
{
  Attendee *a = aItem->data();
  mDisableItemUpdate = true;
  QString name = a->name();
  if (!a->email().isEmpty()) 
    name += " <" + a->email() + ">";
  mNameEdit->setText(name);
  mUidEdit->setText(a->uid());
  mRoleCombo->setCurrentItem(a->role());
  mStatusCombo->setCurrentItem(a->status());
  mRsvpButton->setChecked(a->RSVP());

  mDisableItemUpdate = false;

  setEnableAttendeeInput( true );
}

void KOEditorDetails::setEnableAttendeeInput( bool enabled )
{
  //mNameEdit->setEnabled( enabled );
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

  QString name;
  QString email;
  KPIM::AddresseeLineEdit::getNameAndMail(mNameEdit->text(), name, email);
  a->setName( name );
  a->setUid( mUidEdit->text() );
  a->setEmail( email );
  a->setRole( Attendee::Role( mRoleCombo->currentItem() ) );
  a->setStatus( Attendee::PartStat( mStatusCombo->currentItem() ) );
  a->setRSVP( mRsvpButton->isChecked() );
  aItem->updateItem();
  if ( mFreeBusy ) mFreeBusy->updateAttendee( a );
}

void KOEditorDetails::setFreeBusyWidget( KOEditorFreeBusy *v )
{
  mFreeBusy = v;
}

#include "koeditordetails.moc"
