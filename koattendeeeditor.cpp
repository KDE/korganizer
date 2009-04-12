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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "koattendeeeditor.h"
#include "koprefs.h"
#include "koglobals.h"

#include <libkdepim/addressesdialog.h>
#include <libkdepim/addresseelineedit.h>

#include <KABC/AddresseeDialog>
#include <KCal/Incidence>
#include <KPIMUtils/Email>

#include <KComboBox>
#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>

#include <QCheckBox>
#include <KHBox>
#include <QLabel>
#include <QLayout>
#include <Q3ListViewItem>
#include <QPushButton>
#include <QTimer>

using namespace KCal;

KOAttendeeEditor::KOAttendeeEditor( QWidget *parent )
  : QWidget( parent ), mDisableItemUpdate( true )
{
}

void KOAttendeeEditor::initOrganizerWidgets( QWidget *parent, QBoxLayout *layout )
{
  mOrganizerHBox = new KHBox( parent );
  layout->addWidget( mOrganizerHBox );
  // If creating a new event, then the user is the organizer -> show the
  // identity combo
  // readIncidence will delete it and set another label text instead,
  // if the user isn't the organizer.
  // Note that the i18n text below is duplicated in readIncidence
  QString whatsThis =
    i18nc( "@info:whatsthis",
           "Sets the identity corresponding to "
           "the organizer of this to-do or event. "
           "Identities can be set in the 'Personal' section "
           "of the KOrganizer configuration, or in the "
           "'Personal'->'About Me'->'Password & User Account' "
           "section of the System Settings. In addition, "
           "identities are gathered from your KMail settings "
           "and from your address book. If you choose "
           "to set it globally for KDE in the System Settings, "
           "be sure to check 'Use email settings from "
           "System Settings' in the 'Personal' section of the "
           "KOrganizer configuration." );
  mOrganizerLabel = new QLabel( i18n( "Identity as organizer:" ), mOrganizerHBox );
  mOrganizerCombo = new KComboBox( mOrganizerHBox );
  mOrganizerLabel->setWhatsThis( whatsThis );
  mOrganizerCombo->setToolTip(
    i18nc( "@info:tooltip", "Set the organizer identity" ) );
  mOrganizerCombo->setWhatsThis( whatsThis );
  fillOrganizerCombo();
  mOrganizerHBox->setStretchFactor( mOrganizerCombo, 100 );
}

void KOAttendeeEditor::initEditWidgets( QWidget *parent, QBoxLayout *layout )
{
  QGridLayout *topLayout = new QGridLayout();
  layout->addLayout( topLayout );

  QString whatsThis = i18n( "Edits the name of the attendee selected in the "
                            "list above, or adds a new attendee if there are "
                            "no attendees in the list." );
  QLabel *attendeeLabel = new QLabel( parent );
  attendeeLabel->setWhatsThis( whatsThis );
  attendeeLabel->setText( i18nc( "@label attendee's name", "Na&me:" ) );
  topLayout->addWidget( attendeeLabel, 0, 0 );

  mNameEdit = new KPIM::AddresseeLineEdit( parent );
  mNameEdit->setWhatsThis( whatsThis );
  mNameEdit->setClickMessage( i18n( "Click to add a new attendee" ) );
  attendeeLabel->setBuddy( mNameEdit );
  mNameEdit->installEventFilter( this );
  connect( mNameEdit, SIGNAL(textChanged(const QString &)), SLOT(updateAttendee()) );
  topLayout->addWidget( mNameEdit, 0, 1, 1, 2 );

  whatsThis = i18nc( "@info:whatsthis",
                     "Edits the role of the attendee selected in the list above." );
  QLabel *attendeeRoleLabel = new QLabel( parent );
  attendeeRoleLabel->setWhatsThis( whatsThis );
  attendeeRoleLabel->setText( i18n( "Ro&le:" ) );
  topLayout->addWidget( attendeeRoleLabel, 1, 0 );

  mRoleCombo = new KComboBox( parent );
  mRoleCombo->setToolTip(
    i18nc( "@info:tooltip", "Select the attendee participation role" ) );
  mRoleCombo->setWhatsThis( whatsThis );
  mRoleCombo->addItems( Attendee::roleList() );
  attendeeRoleLabel->setBuddy( mRoleCombo );
  connect( mRoleCombo, SIGNAL(activated(int)), SLOT(updateAttendee()) );
  topLayout->addWidget( mRoleCombo, 1, 1 );

  mDelegateLabel = new QLabel( parent );
  topLayout->addWidget( mDelegateLabel, 1, 2 );

  whatsThis = i18nc( "@info:whatsthis",
                     "Edits the current attendance status of the attendee "
                     "selected in the list above." );
  QLabel *statusLabel = new QLabel( parent );
  statusLabel->setWhatsThis( whatsThis );
  statusLabel->setText( i18n( "Stat&us:" ) );
  topLayout->addWidget( statusLabel, 2, 0 );

  mStatusCombo = new KComboBox( parent );
  mStatusCombo->setToolTip(
    i18nc( "@info:tooltip", "Select the attendee participation status" ) );
  mStatusCombo->setWhatsThis( whatsThis );
  //TODO: the icons below aren't exactly correct
  mStatusCombo->addItem( KOGlobals::self()->smallIcon( "help-about" ),
                         Attendee::statusName( Attendee::NeedsAction ) );
  mStatusCombo->addItem( KOGlobals::self()->smallIcon( "dialog-ok-apply" ),
                         Attendee::statusName( Attendee::Accepted ) );
  mStatusCombo->addItem( KOGlobals::self()->smallIcon( "dialog-cancel" ),
                         Attendee::statusName( Attendee::Declined ) );
  mStatusCombo->addItem( KOGlobals::self()->smallIcon( "dialog-ok" ),
                         Attendee::statusName( Attendee::Tentative ) );
  mStatusCombo->addItem( KOGlobals::self()->smallIcon( "mail-forward" ),
                         Attendee::statusName( Attendee::Delegated ) );
  mStatusCombo->addItem( KOGlobals::self()->smallIcon( "mail-mark-read" ),
                         Attendee::statusName( Attendee::Completed ) ),
  mStatusCombo->addItem( KOGlobals::self()->smallIcon( "help-about" ),
                         Attendee::statusName( Attendee::InProcess ) );

  statusLabel->setBuddy( mStatusCombo );
  connect( mStatusCombo, SIGNAL(activated(int)), SLOT(updateAttendee()) );
  topLayout->addWidget( mStatusCombo, 2, 1 );

  topLayout->setColumnStretch( 2, 1 );

  mRsvpButton = new QCheckBox( parent );
  mRsvpButton->setToolTip(
    i18nc( "@info:tooltip", "Request a response from the attendee" ) );
  mRsvpButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Edits whether to send an email to the "
           "attendee selected in the list above to "
           "request a response concerning attendance." ) );
  mRsvpButton->setText( i18n( "Re&quest response" ) );
  connect( mRsvpButton, SIGNAL(clicked()), SLOT(updateAttendee()) );
  topLayout->addWidget( mRsvpButton, 2, 2 );

  QWidget *buttonBox = new QWidget( parent );
  QVBoxLayout *buttonLayout = new QVBoxLayout( buttonBox );

  mAddButton = new QPushButton( i18nc( "@action:button new attendee", "&New" ), buttonBox );
  mAddButton->setToolTip(
    i18nc( "@info:tooltip", "Add an attendee" ) );
  mAddButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Adds a new attendee to the list. Once the "
           "attendee is added, you will be able to "
           "edit the attendee's name, role, attendance "
           "status, and whether or not the attendee is "
           "required to respond to the invitation. To "
           "select an attendee from your address book, "
           "click the 'Select Addressee' button instead." ) );
  buttonLayout->addWidget( mAddButton );
  connect( mAddButton, SIGNAL(clicked()), SLOT(addNewAttendee()) );

  mRemoveButton = new QPushButton( i18n( "&Remove" ), buttonBox );
  mRemoveButton->setToolTip(
    i18nc( "@info:tooltip", "Remove the selected attendee" ) );
  mRemoveButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Removes the attendee selected in the list above." ) );
  buttonLayout->addWidget( mRemoveButton );

  mAddressBookButton = new QPushButton( i18n( "Select Addressee..." ), buttonBox );
  mAddressBookButton->setToolTip(
    i18nc( "@info:tooltip",
           "Open your address book" ) );
  mAddressBookButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Opens your address book, allowing you to select new attendees from it." ) );
  buttonLayout->addWidget( mAddressBookButton );
  connect( mAddressBookButton, SIGNAL(clicked()), SLOT(openAddressBook()) );

  topLayout->addWidget( buttonBox, 0, 4, 3, 1 );
}

void KOAttendeeEditor::openAddressBook()
{
  QPointer<KPIM::AddressesDialog> dia = new KPIM::AddressesDialog( this );
  dia->setShowCC( false );
  dia->setShowBCC( false );
  if ( dia->exec() == QDialog::Accepted ) {
    KABC::Addressee::List aList = dia->allToAddressesNoDuplicates();
    for ( KABC::Addressee::List::iterator itr = aList.begin();
          itr != aList.end(); ++itr ) {
      insertAttendeeFromAddressee( (*itr) );
    }
  }
  delete dia;
  return;
}

void KOAttendeeEditor::insertAttendeeFromAddressee( const KABC::Addressee &a, const Attendee *at )
{
  bool myself = KOPrefs::instance()->thatIsMe( a.preferredEmail() );
  bool sameAsOrganizer = mOrganizerCombo &&
                         KPIMUtils::compareEmail( a.preferredEmail(),
                                                  mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat = at ? at->status() : KCal::Attendee::NeedsAction;
  bool rsvp = at? at->RSVP() : true;

  if ( myself && sameAsOrganizer ) {
    partStat = KCal::Attendee::Accepted;
    rsvp = false;
  }
  Attendee *newAt = new Attendee( a.realName(), a.preferredEmail(), !myself,
                                  partStat, at ? at->role() : Attendee::ReqParticipant, a.uid() );
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
  for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( !uniqueList.contains( *it ) ) {
      uniqueList << *it;
    }
  }
  mOrganizerCombo->addItems( uniqueList );
}

void KOAttendeeEditor::addNewAttendee()
{
  // check if there's still an unchanged example entry, and if so
  // suggest to edit that first
  if ( Q3ListViewItem *item = hasExampleAttendee() ) {
      KMessageBox::information(
        this,
        i18n( "Please edit the example attendee, before adding more." ),
        QString(),
        "EditExistingExampleAttendeeFirst" );
      // make sure the example attendee is selected
      item->setSelected( true );
      item->listView()->setCurrentItem( item );
      return;
  }
  Attendee *a =
    new Attendee( i18nc( "sample attendee name", "Firstname Lastname" ),
                  i18nc( "sample attendee email name", "name" ) + "@example.net", true );
  insertAttendee( a, false );
  mnewAttendees.append( a );
  updateAttendeeInput();
  // We don't want the hint again
  mNameEdit->setClickMessage( "" );
  mNameEdit->setFocus();
  QTimer::singleShot( 0, mNameEdit, SLOT(selectAll()) );
}

void KOAttendeeEditor::readIncidence( KCal::Incidence *incidence )
{
  mdelAttendees.clear();
  mnewAttendees.clear();
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) ) {
    //TODO: make a new private method for creating the mOrganizerCombo
    //and use it here and initOrganizerWidgets() above.
    if ( !mOrganizerCombo ) {
      mOrganizerCombo = new KComboBox( mOrganizerHBox );
      fillOrganizerCombo();
    }
    mOrganizerLabel->setText( i18n( "Identity as organizer:" ) );

    int found = -1;
    QString fullOrganizer = incidence->organizer().fullName();
    for ( int i = 0; i < mOrganizerCombo->count(); ++i ) {
      if ( mOrganizerCombo->itemText( i ) == fullOrganizer ) {
        found = i;
        mOrganizerCombo->setCurrentIndex( i );
        break;
      }
    }
    if ( found < 0 ) {
      mOrganizerCombo->addItem( fullOrganizer, 0 );
      mOrganizerCombo->setCurrentIndex( 0 );
    }
  } else { // someone else is the organizer
    if ( mOrganizerCombo ) {
      delete mOrganizerCombo;
      mOrganizerCombo = 0;
    }
    mOrganizerLabel->setText( i18n( "Organizer: %1", incidence->organizer().fullName() ) );
  }

  Attendee::List al = incidence->attendees();
  Attendee::List::ConstIterator it;
  Attendee *first = 0;
  for ( it = al.constBegin(); it != al.constEnd(); ++it ) {
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
    mRoleCombo->setCurrentIndex( first->role() );
    if ( first->status() != KCal::Attendee::None ) {
      mStatusCombo->setCurrentIndex( first->status() );
    } else {
      mStatusCombo->setCurrentIndex( KCal::Attendee::NeedsAction );
    }
    mRsvpButton->setChecked( first->RSVP() );
    mRsvpButton->setEnabled( true );
  }
}

void KOAttendeeEditor::fillIncidence( KCal::Incidence *incidence )
{
  if ( mOrganizerCombo ) {
    // TODO: Don't take a string and split it up... Is there a better way?
    incidence->setOrganizer( mOrganizerCombo->currentText() );
  }
}

void KOAttendeeEditor::setEnableAttendeeInput( bool enabled )
{
  //mNameEdit->setEnabled( enabled );
  mRoleCombo->setEnabled( enabled );
  mStatusCombo->setEnabled( enabled );
  mRsvpButton->setEnabled( enabled );

  mRemoveButton->setEnabled( enabled );
}

void KOAttendeeEditor::clearAttendeeInput()
{
  mNameEdit->setText( "" );
  mUid.clear();
  mRoleCombo->setCurrentIndex( 0 );
  mStatusCombo->setCurrentIndex( 0 );
  mRsvpButton->setChecked( true );
  setEnableAttendeeInput( false );
  mDelegateLabel->setText( QString() );
}

void KOAttendeeEditor::updateAttendee()
{
  Attendee *a = currentAttendee();
  if ( !a || mDisableItemUpdate ) {
    return;
  }

  QString name;
  QString email;
  KPIMUtils::extractEmailAddressAndName( mNameEdit->text(), email, name );

  bool iAmTheOrganizer = mOrganizerCombo &&
                         KOPrefs::instance()->thatIsMe( mOrganizerCombo->currentText() );
  if ( iAmTheOrganizer ) {
    bool myself = KPIMUtils::compareEmail( email, mOrganizerCombo->currentText(), false );
    bool wasMyself =
      KPIMUtils::compareEmail( a->email(), mOrganizerCombo->currentText(), false );
    if ( myself ) {
      mRsvpButton->setChecked( false );
      mRsvpButton->setEnabled( false );
    } else if ( wasMyself ) {
      // this was me, but is no longer, reset
      mStatusCombo->setCurrentIndex( KCal::Attendee::NeedsAction );
      mRsvpButton->setChecked( true );
      mRsvpButton->setEnabled( true );
    }
  }
  a->setName( name );
  a->setUid( mUid );
  a->setEmail( email );
  a->setRole( Attendee::Role( mRoleCombo->currentIndex() ) );
  a->setStatus( Attendee::PartStat( mStatusCombo->currentIndex() ) );
  a->setRSVP( mRsvpButton->isChecked() );

  updateCurrentItem();
}

void KOAttendeeEditor::fillAttendeeInput( KCal::Attendee *a )
{
  mDisableItemUpdate = true;

  QString name = a->name();
  if ( !a->email().isEmpty() ) {
    name = KPIMUtils::quoteNameIfNecessary( name );
    name += " <" + a->email() + '>';
  }

  bool myself = KOPrefs::instance()->thatIsMe( a->email() );
  bool sameAsOrganizer = mOrganizerCombo &&
                         KPIMUtils::compareEmail( a->email(),
                                                  mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat = a->status();
  bool rsvp = a->RSVP();

  if ( myself && sameAsOrganizer && a->status() == KCal::Attendee::None ) {
    partStat = KCal::Attendee::Accepted;
    rsvp = false;
  }

  mNameEdit->setText( name );
  mUid = a->uid();
  mRoleCombo->setCurrentIndex( a->role() );
  if ( partStat != KCal::Attendee::None ) {
    mStatusCombo->setCurrentIndex( partStat );
  } else {
    mStatusCombo->setCurrentIndex( KCal::Attendee::NeedsAction );
  }
  mRsvpButton->setChecked( rsvp );

  mDisableItemUpdate = false;
  setEnableAttendeeInput( true );

  if ( a->status() == Attendee::Delegated ) {
    if ( !a->delegate().isEmpty() ) {
      mDelegateLabel->setText( i18n( "Delegated to %1", a->delegate() ) );
    } else if ( !a->delegator().isEmpty() ) {
      mDelegateLabel->setText( i18n( "Delegated from %1", a->delegator() ) );
    } else {
      mDelegateLabel->setText( i18n( "Not delegated" ) );
    }
  }
  if ( myself ) {
    mRsvpButton->setEnabled( false );
  }
}

void KOAttendeeEditor::updateAttendeeInput()
{
  setEnableAttendeeInput( !mNameEdit->text().isEmpty() );
  Attendee *a = currentAttendee();
  if ( a ) {
    fillAttendeeInput( a );
  } else {
    clearAttendeeInput();
  }
}

void KOAttendeeEditor::cancelAttendeeIncidence( KCal::Incidence *incidence )
{
  incidence->clearAttendees();
  foreach ( Attendee *att, mdelAttendees ) {
    bool isNewAttendee = false;
    foreach ( Attendee *newAtt, mnewAttendees ) {
      if ( *att == *newAtt ) {
        isNewAttendee = true;
        break;
      }
    }
    if ( !isNewAttendee ) {
      incidence->addAttendee( new Attendee( *att ), false );
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

bool KOAttendeeEditor::eventFilter( QObject *watched, QEvent *ev )
{
  if ( watched && watched == mNameEdit &&
       ev->type() == QEvent::FocusIn && currentAttendee() == 0 ) {
    addNewAttendee();
  }

  return QWidget::eventFilter( watched, ev );
}

bool KOAttendeeEditor::isExampleAttendee( const KCal::Attendee *attendee ) const
{
  if ( !attendee ) {
    return false;
  }

  if ( attendee->name() == i18nc( "sample attendee name", "Firstname Lastname" ) &&
       attendee->email().endsWith( QLatin1String( "example.net" ) ) ) {
    return true;
  }
  return false;
}

#include "koattendeeeditor.moc"
