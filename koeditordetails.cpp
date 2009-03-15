/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koeditordetails.h"
#include "koglobals.h"
#include "koprefs.h"

#include <libkdepim/distributionlist.h>
#include <libkdepim/kvcarddrag.h>

#include <KABC/StdAddressBook>
#include <KCal/Incidence>
#include <KPIMUtils/Email>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include <QBoxLayout>
#include <QCheckBox>
#include <QMimeData>
#include <QPushButton>
#include <QVBoxLayout>

#ifndef KORG_NODND
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#endif

template <>
CustomListViewItem<KCal::Attendee *>::~CustomListViewItem()
{
  delete mData;
}

template <>
void CustomListViewItem<KCal::Attendee *>::updateItem()
{
  setText( 0, mData->name() );
  setText( 1, mData->email() );
  setText( 2, mData->roleStr() );
  setText( 3, mData->statusStr() );
  if ( mData->RSVP() && !mData->email().isEmpty() ) {
    setPixmap( 4, KOGlobals::self()->smallIcon( "mail-flag" ) );
  } else {
    setPixmap( 4, KOGlobals::self()->smallIcon( "mail-queue" ) );
  }
  setText( 5, mData->delegate() );
  setText( 6, mData->delegator() );
}

KOAttendeeListView::KOAttendeeListView ( QWidget *parent )
  : K3ListView( parent )
{
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
  setSorting( -1 );
}

/** KOAttendeeListView is a child class of K3ListView  which supports
 *  dropping of attendees (e.g. from kaddressbook) onto it. If an attendee
 *  was dropped, the signal dropped(Attendee*)  is emitted.
 */
KOAttendeeListView::~KOAttendeeListView()
{
}

#ifndef KORG_NODND
void KOAttendeeListView::contentsDragEnterEvent( QDragEnterEvent *e )
{
  dragEnterEvent( e );
}
#endif

#ifndef KORG_NODND
void KOAttendeeListView::contentsDragMoveEvent( QDragMoveEvent *e )
{
  const QMimeData *md = e->mimeData();
  if ( KPIM::KVCardDrag::canDecode( md ) || md->hasText() ) {
    e->accept();
  } else {
    e->ignore();
  }
}
#endif

#ifndef KORG_NODND
void KOAttendeeListView::dragEnterEvent( QDragEnterEvent *e )
{
  const QMimeData *md = e->mimeData();
  if ( KPIM::KVCardDrag::canDecode( md ) || md->hasText() ) {
    e->accept();
  } else {
    e->ignore();
  }
}
#endif

void KOAttendeeListView::addAttendee( const QString &newAttendee )
{
  kDebug() << " Email:" << newAttendee;
  QString name;
  QString email;
  KPIMUtils::extractEmailAddressAndName( newAttendee, email, name );
#ifndef KORG_NODND
  emit dropped( new Attendee( name, email, true ) );
#endif
}

#ifndef KORG_NODND
void KOAttendeeListView::contentsDropEvent( QDropEvent *e )
{
  dropEvent( e );
}
#endif

#ifndef KORG_NODND
void KOAttendeeListView::dropEvent( QDropEvent *e )
{
  const QMimeData *md = e->mimeData();

  if ( KPIM::KVCardDrag::canDecode( md ) ) {
    KABC::Addressee::List list;
    KPIM::KVCardDrag::fromMimeData( md, list );

    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      QString em( (*it).fullEmail() );
      if ( em.isEmpty() ) {
        em = (*it).realName();
      }
      addAttendee( em );
    }
  }

  if ( md->hasText() ) {
    QString text = md->text();
    kDebug() << "Dropped :" << text;
    QStringList emails = text.split( ',', QString::SkipEmptyParts );
    for ( QStringList::ConstIterator it = emails.constBegin(); it != emails.constEnd(); ++it ) {
      addAttendee( *it );
    }
  }
}
#endif

KOEditorDetails::KOEditorDetails( int spacing, QWidget *parent )
  : KOAttendeeEditor( parent ), mDisableItemUpdate( false )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  initOrganizerWidgets( this, topLayout );

  mListView = new KOAttendeeListView( this );
  mListView->setObjectName( "mListView" );
  mListView->setWhatsThis( i18nc( "@info:whatsthis",
                                  "Displays information about current attendees. "
                                  "To edit an attendee, select it in this list "
                                  "and modify the values in the area below. "
                                  "Clicking on a column title will sort the list "
                                  "according to that column. The RSVP column "
                                  "indicates whether or not a response is "
                                  "requested from the attendee." ) );
  mListView->addColumn( i18nc( "@title:column attendee name", "Name" ), 200 );
  mListView->addColumn( i18nc( "@title:column attendee email", "Email" ), 200 );
  mListView->addColumn( i18nc( "@title:column attendee role", "Role" ), 80 );
  mListView->addColumn( i18nc( "@title:column attendee status", "Status" ), 100 );
  mListView->addColumn( i18nc( "@title:column attendee has RSVPed?", "RSVP" ), 55 );
  mListView->addColumn( i18nc( "@title:column attendee delegated to", "Delegated To" ), 120 );
  mListView->addColumn( i18nc( "@title:column attendee delegated from", "Delegated From" ), 120 );
  mListView->setResizeMode( Q3ListView::LastColumn );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mListView->setFixedHeight( 78 );
  }

  connect( mListView, SIGNAL(selectionChanged(Q3ListViewItem*)),
           SLOT(updateAttendeeInput()) );
#ifndef KORG_NODND
  connect( mListView, SIGNAL( dropped( Attendee * ) ),
           SLOT( slotInsertAttendee( Attendee * ) ) );
#endif
  topLayout->addWidget( mListView );

  initEditWidgets( this, topLayout );

  connect( mRemoveButton, SIGNAL(clicked()), SLOT(removeAttendee()) );

  updateAttendeeInput();
}

KOEditorDetails::~KOEditorDetails()
{
}

bool KOEditorDetails::hasAttendees()
{
  return mListView->childCount() > 0;
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( mListView->selectedItem() );
  if ( !aItem ) {
    return;
  }

  AttendeeListItem *nextSelectedItem = static_cast<AttendeeListItem*>( aItem->nextSibling() );
  if ( mListView->childCount() == 1 ) {
    nextSelectedItem = 0;
  }
  if ( mListView->childCount() > 1 && aItem == mListView->lastItem() ) {
    nextSelectedItem = static_cast<AttendeeListItem*>( mListView->firstChild() );
  }

  Attendee *delA = new Attendee( aItem->data()->name(), aItem->data()->email(),
                                 aItem->data()->RSVP(), aItem->data()->status(),
                                 aItem->data()->role(), aItem->data()->uid() );
  mdelAttendees.append( delA );

  delete aItem;

  if ( nextSelectedItem ) {
    mListView->setSelected( nextSelectedItem, true );
  }
  updateAttendeeInput();
  emit updateAttendeeSummary( mListView->childCount() );
}

void KOEditorDetails::insertAttendee( Attendee *a, bool goodEmailAddress )
{
  Q_UNUSED( goodEmailAddress );

  // lastItem() is O(n), but for n very small that should be fine
  AttendeeListItem *item = new AttendeeListItem( a, mListView,
      static_cast<K3ListViewItem*>( mListView->lastItem() ) );
  mListView->setSelected( item, true );
  emit updateAttendeeSummary( mListView->childCount() );
}

void KOEditorDetails::setDefaults()
{
  mRsvpButton->setChecked( true );
}

void KOEditorDetails::readIncidence( Incidence *event )
{
  mListView->clear();
  KOAttendeeEditor::readIncidence( event );

  mListView->setSelected( mListView->firstChild(), true );

  emit updateAttendeeSummary( mListView->childCount() );
}

void KOEditorDetails::fillIncidence( Incidence *incidence )
{
  incidence->clearAttendees();
  QVector<Q3ListViewItem*> toBeDeleted;
  Q3ListViewItem *item;
  AttendeeListItem *a;
  for ( item = mListView->firstChild(); item; item = item->nextSibling() ) {
    a = (AttendeeListItem *)item;
    Attendee *attendee = a->data();
    Q_ASSERT( attendee );
    /* Check if the attendee is a distribution list and expand it */
    if ( attendee->email().isEmpty() ) {
      KPIM::DistributionList list =
        KPIM::DistributionList::findByName( KABC::StdAddressBook::self(), attendee->name() );
      if ( !list.isEmpty() ) {
        toBeDeleted.push_back( item ); // remove it once we are done expanding
        KPIM::DistributionList::Entry::List entries = list.entries( KABC::StdAddressBook::self() );
        KPIM::DistributionList::Entry::List::Iterator it( entries.begin() );
        while ( it != entries.end() ) {
          KPIM::DistributionList::Entry &e = ( *it );
          ++it;
          // this calls insertAttendee, which appends
          insertAttendeeFromAddressee( e.addressee, attendee );
          // TODO: duplicate check, in case it was already added manually
        }
      }
    } else {
      bool skip = false;
      if ( attendee->email().endsWith( QLatin1String( "example.net" ) ) ) {
        if ( KMessageBox::warningYesNo(
              this,
              i18nc( "@info",
                "%1 does not look like a valid email address. "
                "Are you sure you want to invite this participant?",
                attendee->email() ),
              i18nc( "@title", "Invalid Email Address" ) ) != KMessageBox::Yes ) {
          skip = true;
        }
      }
      if ( !skip ) {
        incidence->addAttendee( new Attendee( *attendee ) );
      }
    }
  }

  KOAttendeeEditor::fillIncidence( incidence );

  // cleanup
  qDeleteAll( toBeDeleted );
  toBeDeleted.clear();
}

bool KOEditorDetails::validateInput()
{
  return true;
}

KCal::Attendee *KOEditorDetails::currentAttendee() const
{
  Q3ListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if ( !aItem ) {
    return 0;
  }
  return aItem->data();
}

void KOEditorDetails::updateCurrentItem()
{
  AttendeeListItem *item = static_cast<AttendeeListItem*>( mListView->selectedItem() );
  if ( item ) {
    item->updateItem();
  }
}

void KOEditorDetails::slotInsertAttendee( Attendee *a )
{
  insertAttendee( a );
}

void KOEditorDetails::changeStatusForMe( Attendee::PartStat status )
{
  const QStringList myEmails = KOPrefs::instance()->allEmails();
  for ( Q3ListViewItemIterator it( mListView ); it.current(); ++it ) {
    AttendeeListItem *item = static_cast<AttendeeListItem*>( it.current() );
    for ( QStringList::ConstIterator it2( myEmails.begin() ), end( myEmails.end() );
          it2 != end; ++it2 ) {
      if ( item->data()->email() == *it2 ) {
        item->data()->setStatus( status );
        item->updateItem();
      }
    }
  }
}

Q3ListViewItem *KOEditorDetails::hasExampleAttendee() const
{
  for ( Q3ListViewItemIterator it( mListView ); it.current(); ++it ) {
    AttendeeListItem *item = static_cast<AttendeeListItem*>( it.current() );
    Attendee *attendee = item->data();
    Q_ASSERT( attendee );
    if ( isExampleAttendee( attendee ) ) {
      return item;
    }
  }
  return 0;
}

#include "koeditordetails.moc"
