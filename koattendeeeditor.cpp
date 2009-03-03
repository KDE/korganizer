/*
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "koattendeeeditor.h"
#include "koprefs.h"
#include "koglobals.h"

#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#include <libkdepim/addressesdialog.h>
#include <libkdepim/addresseelineedit.h>
#endif

#include <libkcal/incidence.h>

#include <libemailfunctions/email.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

using namespace KCal;

KOAttendeeEditor::KOAttendeeEditor( QWidget * parent, const char *name ) :
    QWidget( parent, name ),
    mDisableItemUpdate( true )
{
}

void KOAttendeeEditor::initOrganizerWidgets(QWidget * parent, QBoxLayout * layout)
{
  mOrganizerHBox = new QHBox( parent );
  layout->addWidget( mOrganizerHBox );
  // If creating a new event, then the user is the organizer -> show the
  // identity combo
  // readEvent will delete it and set another label text instead, if the user
  // isn't the organizer.
  // Note that the i18n text below is duplicated in readEvent
  QString whatsThis = i18n("Sets the identity corresponding to "
                           "the organizer of this to-do or event. "
                           "Identities can be set in the 'Personal' "
                           "section of the KOrganizer configuration, or in the "
                           "'Security & Privacy'->'Password & User Account' "
                           "section of the KDE Control Center. In addition, "
                           "identities are gathered from your KMail settings "
                           "and from your address book. If you choose "
                           "to set it globally for KDE in the Control Center, "
                           "be sure to check 'Use email settings from "
                           "Control Center' in the 'Personal' section of the "
                           "KOrganizer configuration.");
  mOrganizerLabel = new QLabel( i18n( "Identity as organizer:" ),
                                mOrganizerHBox );
  mOrganizerCombo = new QComboBox( mOrganizerHBox );
  QWhatsThis::add( mOrganizerLabel, whatsThis );
  QWhatsThis::add( mOrganizerCombo, whatsThis );
  fillOrganizerCombo();
  mOrganizerHBox->setStretchFactor( mOrganizerCombo, 100 );
}

void KOAttendeeEditor::initEditWidgets(QWidget * parent, QBoxLayout * layout)
{
  QGridLayout *topLayout = new QGridLayout();
  layout->addLayout( topLayout );

  QString whatsThis = i18n("Edits the name of the attendee selected in the list "
                           "above, or adds a new attendee if there are no attendees"
                           "in the list.");
  QLabel *attendeeLabel = new QLabel( parent );
  QWhatsThis::add( attendeeLabel, whatsThis );
  attendeeLabel->setText( i18n("Na&me:") );
  topLayout->addWidget( attendeeLabel, 0, 0 );

  mNameEdit = new KPIM::AddresseeLineEdit( parent );
  QWhatsThis::add( mNameEdit, whatsThis );
  mNameEdit->setClickMessage( i18n("Click to add a new attendee") );
  attendeeLabel->setBuddy( mNameEdit );
  mNameEdit->installEventFilter( this );
  connect( mNameEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( updateAttendee() ) );
  topLayout->addMultiCellWidget( mNameEdit, 0, 0, 1, 2 );

  whatsThis = i18n("Edits the role of the attendee selected "
                   "in the list above.");
  QLabel *attendeeRoleLabel = new QLabel( parent );
  QWhatsThis::add( attendeeRoleLabel, whatsThis );
  attendeeRoleLabel->setText( i18n("Ro&le:") );
  topLayout->addWidget( attendeeRoleLabel, 1, 0 );

  mRoleCombo = new QComboBox( false, parent );
  QWhatsThis::add( mRoleCombo, whatsThis );
  mRoleCombo->insertStringList( Attendee::roleList() );
  attendeeRoleLabel->setBuddy( mRoleCombo );
  connect( mRoleCombo, SIGNAL( activated( int ) ),
           SLOT( updateAttendee() ) );
  topLayout->addWidget( mRoleCombo, 1, 1 );

  mDelegateLabel = new QLabel( parent );
  topLayout->addWidget( mDelegateLabel, 1, 2 );

  whatsThis = i18n("Edits the current attendance status of the attendee "
                   "selected in the list above.");
  QLabel *statusLabel = new QLabel( parent );
  QWhatsThis::add( statusLabel, whatsThis );
  statusLabel->setText( i18n("Stat&us:") );
  topLayout->addWidget( statusLabel, 2, 0 );

  mStatusCombo = new QComboBox( false, parent );
  QWhatsThis::add( mStatusCombo, whatsThis );
//   mStatusCombo->insertStringList( Attendee::statusList() );
  mStatusCombo->insertItem( SmallIcon( "help" ), Attendee::statusName( Attendee::NeedsAction ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "ok" ), Attendee::statusName( Attendee::Accepted ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "no" ), Attendee::statusName( Attendee::Declined ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "apply" ), Attendee::statusName( Attendee::Tentative ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "mail_forward" ), Attendee::statusName( Attendee::Delegated ) );
  mStatusCombo->insertItem( Attendee::statusName( Attendee::Completed ) );
  mStatusCombo->insertItem( KOGlobals::self()->smallIcon( "help" ), Attendee::statusName( Attendee::InProcess ) );

  statusLabel->setBuddy( mStatusCombo );
  connect( mStatusCombo, SIGNAL( activated( int ) ),
           SLOT( updateAttendee() ) );
  topLayout->addWidget( mStatusCombo, 2, 1 );

  topLayout->setColStretch( 2, 1 );

  mRsvpButton = new QCheckBox( parent );
  QWhatsThis::add( mRsvpButton,
		   i18n("Edits whether to send an email to the attendee "
			"selected in the list above to request "
			"a response concerning attendance.") );
  mRsvpButton->setText( i18n("Re&quest response") );
  connect( mRsvpButton, SIGNAL( clicked() ), SLOT( updateAttendee() ) );
  topLayout->addWidget( mRsvpButton, 2, 2 );

  QWidget *buttonBox = new QWidget( parent );
  QVBoxLayout *buttonLayout = new QVBoxLayout( buttonBox );

  mAddButton = new QPushButton( i18n("&New"), buttonBox );
  QWhatsThis::add( mAddButton,
		   i18n("Adds a new attendee to the list. Once the "
		   	"attendee is added, you will be able to "
			"edit the attendee's name, role, attendance "
			"status, and whether or not the attendee is required "
			"to respond to the invitation. To select an attendee "
			"from your addressbook, click the 'Select Addressee' "
			"button instead.") );
  buttonLayout->addWidget( mAddButton );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( addNewAttendee() ) );

  mRemoveButton = new QPushButton( i18n("&Remove"), buttonBox );
  QWhatsThis::add( mRemoveButton,
		   i18n("Removes the attendee selected in "
		   	"the list above.") );
  buttonLayout->addWidget( mRemoveButton );

  mAddressBookButton = new QPushButton( i18n("Select Addressee..."),
                                        buttonBox );
  QWhatsThis::add( mAddressBookButton,
		   i18n("Opens your address book, allowing you to select "
			"new attendees from it.") );
  buttonLayout->addWidget( mAddressBookButton );
  connect( mAddressBookButton, SIGNAL( clicked() ), SLOT( openAddressBook() ) );

  topLayout->addMultiCellWidget( buttonBox, 0, 3, 3, 3 );

#ifdef KORG_NOKABC
  mAddressBookButton->hide();
#endif
}

void KOAttendeeEditor::openAddressBook()
{
#ifndef KORG_NOKABC
  KPIM::AddressesDialog *dia = new KPIM::AddressesDialog( this, "adddialog" );
  dia->setShowCC( false );
  dia->setShowBCC( false );
  if ( dia->exec() ) {
    KABC::Addressee::List aList = dia->allToAddressesNoDuplicates();
    for ( KABC::Addressee::List::iterator itr = aList.begin();
          itr != aList.end(); ++itr ) {
      insertAttendeeFromAddressee( (*itr) );
    }
  }
  delete dia;
  return;
#if 0
    // old code
    KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
    if (!a.isEmpty()) {
        // If this is myself, I don't want to get a response but instead
        // assume I will be available
        bool myself = KOPrefs::instance()->thatIsMe( a.preferredEmail() );
        KCal::Attendee::PartStat partStat =
            myself ? KCal::Attendee::Accepted : KCal::Attendee::NeedsAction;
        insertAttendee( new Attendee( a.realName(), a.preferredEmail(),
                                      !myself, partStat,
                                      KCal::Attendee::ReqParticipant, a.uid() ) );
    }
#endif
#endif
}

void KOAttendeeEditor::insertAttendeeFromAddressee(const KABC::Addressee &a, const Attendee * at)
{
  bool myself = KOPrefs::instance()->thatIsMe( a.preferredEmail() );
  bool sameAsOrganizer = mOrganizerCombo &&
  KPIM::compareEmail( a.preferredEmail(), mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat = at? at->status() : KCal::Attendee::NeedsAction;
  bool rsvp = at? at->RSVP() : true;

  if ( myself && sameAsOrganizer ) {
    partStat = KCal::Attendee::Accepted;
    rsvp = false;
  }
  Attendee *newAt = new Attendee( a.realName(),
                               a.preferredEmail(),
                               !myself, partStat,
                               at ? at->role() : Attendee::ReqParticipant,
                               a.uid() );
  newAt->setRSVP( rsvp );
  insertAttendee( newAt, true );
}

void KOAttendeeEditor::fillOrganizerCombo()
{
  Q_ASSERT( mOrganizerCombo );
  // Get all emails from KOPrefs (coming from various places),
  // and insert them - removing duplicates
  const QStringList lst = KOPrefs::instance()->fullEmails();
  QStringList uniqueList;
  for( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( uniqueList.find( *it ) == uniqueList.end() )
      uniqueList << *it;
  }
  mOrganizerCombo->insertStringList( uniqueList );
}

void KOAttendeeEditor::addNewAttendee()
{
  // check if there's still an unchanged example entry, and if so
  // suggest to edit that first
  if ( QListViewItem* item = hasExampleAttendee() ) {
      KMessageBox::information( this,
          i18n( "Please edit the example attendee, before adding more." ), QString::null,
          "EditExistingExampleAttendeeFirst" );
      // make sure the example attendee is selected
      item->setSelected( true );
      item->listView()->setCurrentItem( item );
      return;
  }
  Attendee *a = new Attendee( i18n("Firstname Lastname"),
                              i18n("name") + "@example.net", true );
  insertAttendee( a, false );
  mnewAttendees.append(a);
  updateAttendeeInput();
  // We don't want the hint again
  mNameEdit->setClickMessage( "" );
  mNameEdit->setFocus();
  QTimer::singleShot( 0, mNameEdit, SLOT( selectAll() ) );
}

void KOAttendeeEditor::readEvent(KCal::Incidence * incidence)
{
  mdelAttendees.clear();
  mnewAttendees.clear();
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) ) {
    if ( !mOrganizerCombo ) {
      mOrganizerCombo = new QComboBox( mOrganizerHBox );
      fillOrganizerCombo();
    }
    mOrganizerLabel->setText( i18n( "Identity as organizer:" ) );

    int found = -1;
    QString fullOrganizer = incidence->organizer().fullName();
    for ( int i = 0 ; i < mOrganizerCombo->count(); ++i ) {
      if ( mOrganizerCombo->text( i ) == fullOrganizer ) {
        found = i;
        mOrganizerCombo->setCurrentItem( i );
        break;
      }
    }
    if ( found < 0 ) {
      mOrganizerCombo->insertItem( fullOrganizer, 0 );
      mOrganizerCombo->setCurrentItem( 0 );
    }
  } else { // someone else is the organizer
    if ( mOrganizerCombo ) {
      delete mOrganizerCombo;
      mOrganizerCombo = 0;
    }
    mOrganizerLabel->setText( i18n( "Organizer: %1" ).arg( incidence->organizer().fullName() ) );
  }

  Attendee::List al = incidence->attendees();
  Attendee::List::ConstIterator it;
  Attendee *first = 0;
  for( it = al.begin(); it != al.end(); ++it ) {
    Attendee *a = new Attendee( **it );
    if ( !first ) {
      first = a;
    }
    insertAttendee( a, true );
  }

  // Set the initial editing values to the first attendee in the list.
  if ( first ) {
    mNameEdit->setText( first->fullName() );
    mUid = first->uid();
    mRoleCombo->setCurrentItem( first->role() );
    if ( first->status() != KCal::Attendee::None ) {
      mStatusCombo->setCurrentItem( first->status() );
    } else {
      mStatusCombo->setCurrentItem( KCal::Attendee::NeedsAction );
    }
    mRsvpButton->setChecked( first->RSVP() );
    mRsvpButton->setEnabled( true );
  }
}

void KOAttendeeEditor::writeEvent(KCal::Incidence * incidence)
{
  if ( mOrganizerCombo ) {
    // TODO: Don't take a string and split it up... Is there a better way?
    incidence->setOrganizer( mOrganizerCombo->currentText() );
  }
}

void KOAttendeeEditor::setEnableAttendeeInput(bool enabled)
{
  //mNameEdit->setEnabled( enabled );
  mRoleCombo->setEnabled( enabled );
  mStatusCombo->setEnabled( enabled );
  mRsvpButton->setEnabled( enabled );

  mRemoveButton->setEnabled( enabled );
}

void KOAttendeeEditor::clearAttendeeInput()
{
  mNameEdit->setText("");
  mUid = QString::null;
  mRoleCombo->setCurrentItem(0);
  mStatusCombo->setCurrentItem(0);
  mRsvpButton->setChecked(true);
  setEnableAttendeeInput( false );
  mDelegateLabel->setText( QString() );
}

void KOAttendeeEditor::updateAttendee()
{
  Attendee *a = currentAttendee();
  if ( !a || mDisableItemUpdate )
    return;

  QString name;
  QString email;
  KPIM::getNameAndMail(mNameEdit->text(), name, email);

  bool iAmTheOrganizer = mOrganizerCombo &&
    KOPrefs::instance()->thatIsMe( mOrganizerCombo->currentText() );
  if ( iAmTheOrganizer ) {
    bool myself =
      KPIM::compareEmail( email, mOrganizerCombo->currentText(), false );
    bool wasMyself =
      KPIM::compareEmail( a->email(), mOrganizerCombo->currentText(), false );
    if ( myself ) {
      mRsvpButton->setChecked( false );
      mRsvpButton->setEnabled( false );
    } else if ( wasMyself ) {
      // this was me, but is no longer, reset
      mStatusCombo->setCurrentItem( KCal::Attendee::NeedsAction );
      mRsvpButton->setChecked( true );
      mRsvpButton->setEnabled( true );
    }
  }
  a->setName( name );
  a->setUid( mUid );
  a->setEmail( email );
  a->setRole( Attendee::Role( mRoleCombo->currentItem() ) );
  a->setStatus( Attendee::PartStat( mStatusCombo->currentItem() ) );
  a->setRSVP( mRsvpButton->isChecked() );

  updateCurrentItem();
}

void KOAttendeeEditor::fillAttendeeInput( KCal::Attendee *a )
{
  mDisableItemUpdate = true;

  QString name = a->name();
  if (!a->email().isEmpty()) {
    name = KPIM::quoteNameIfNecessary( name );
    name += " <" + a->email() + ">";
  }

  bool myself = KOPrefs::instance()->thatIsMe( a->email() );
  bool sameAsOrganizer = mOrganizerCombo &&
          KPIM::compareEmail( a->email(),
                                   mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat = a->status();
  bool rsvp = a->RSVP();

  if ( myself && sameAsOrganizer && a->status() == KCal::Attendee::None ) {
      partStat = KCal::Attendee::Accepted;
      rsvp = false;
  }

  mNameEdit->setText(name);
  mUid = a->uid();
  mRoleCombo->setCurrentItem(a->role());
  if ( partStat != KCal::Attendee::None ) {
    mStatusCombo->setCurrentItem( partStat );
  } else {
    mStatusCombo->setCurrentItem( KCal::Attendee::NeedsAction );
  }
  mRsvpButton->setChecked( rsvp );

  mDisableItemUpdate = false;
  setEnableAttendeeInput( true );

  if ( a->status() == Attendee::Delegated ) {
    if ( !a->delegate().isEmpty() )
      mDelegateLabel->setText( i18n( "Delegated to %1" ).arg( a->delegate() ) );
    else if ( !a->delegator().isEmpty() )
      mDelegateLabel->setText( i18n( "Delegated from %1" ).arg( a->delegator() ) );
    else
      mDelegateLabel->setText( i18n( "Not delegated" ) );
  }
}

void KOAttendeeEditor::updateAttendeeInput()
{
  setEnableAttendeeInput(!mNameEdit->text().isEmpty());
  Attendee* a = currentAttendee();
  if ( a ) {
    fillAttendeeInput( a );
  } else {
    clearAttendeeInput();
  }
}

void KOAttendeeEditor::cancelAttendeeEvent( KCal::Incidence *incidence )
{
  incidence->clearAttendees();
  Attendee * att;
  for (att=mdelAttendees.first();att;att=mdelAttendees.next()) {
    bool isNewAttendee = false;
    for (Attendee *newAtt=mnewAttendees.first();newAtt;newAtt=mnewAttendees.next()) {
      if (*att==*newAtt) {
        isNewAttendee = true;
        break;
      }
    }
    if (!isNewAttendee) {
      incidence->addAttendee(new Attendee(*att));
    }
  }
  mdelAttendees.clear();
}

void KOAttendeeEditor::acceptForMe()
{
  changeStatusForMe( Attendee::Accepted );
}

void KOAttendeeEditor::declineForMe()
{
  changeStatusForMe( Attendee::Declined );
}

bool KOAttendeeEditor::eventFilter(QObject *watched, QEvent *ev)
{
  if ( watched && watched == mNameEdit && ev->type() == QEvent::FocusIn &&
       currentAttendee() == 0 ) {
    addNewAttendee();
  }

  return QWidget::eventFilter( watched, ev );
}

bool KOAttendeeEditor::isExampleAttendee( const KCal::Attendee* attendee ) const
{
    if ( !attendee ) return false;
    if ( attendee->name() == i18n( "Firstname Lastname" )
        && attendee->email().endsWith( "example.net" ) ) {
        return true;
    }
    return false;
}

#include "koattendeeeditor.moc"
