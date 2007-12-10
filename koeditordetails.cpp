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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koeditordetails.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qdragobject.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>
#include <qwidgetstack.h>
#include <qvaluevector.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#include <kabc/vcardconverter.h>
#include <libkdepim/addressesdialog.h>
#include <libkdepim/addresseelineedit.h>
#include <libkdepim/distributionlist.h>
#include <kabc/stdaddressbook.h>
#endif
#include <libkdepim/kvcarddrag.h>
#include <libemailfunctions/email.h>

#include <libkcal/incidence.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorfreebusy.h"

#include "kocore.h"

template <>
CustomListViewItem<KCal::Attendee *>::~CustomListViewItem()
{
  delete mData;
}

template <>
void CustomListViewItem<KCal::Attendee *>::updateItem()
{
  setText(0,mData->name());
  setText(1,mData->email());
  setText(2,mData->roleStr());
  setText(3,mData->statusStr());
  if (mData->RSVP() && !mData->email().isEmpty())
    setPixmap(4,KOGlobals::self()->smallIcon("mailappt"));
  else
    setPixmap(4,KOGlobals::self()->smallIcon("nomailappt"));
  setText(5, mData->delegate());
  setText(6, mData->delegator());
}

KOAttendeeListView::KOAttendeeListView ( QWidget *parent, const char *name )
  : KListView(parent, name)
{
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
  setSorting( -1 );
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

void KOAttendeeListView::contentsDragMoveEvent( QDragMoveEvent *e )
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

void KOAttendeeListView::addAttendee( const QString &newAttendee )
{
  kdDebug(5850) << " Email: " << newAttendee << endl;
  QString name;
  QString email;
  KPIM::getNameAndMail( newAttendee, name, email );
  emit dropped( new Attendee( name, email, true ) );
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


KOEditorDetails::KOEditorDetails( int spacing, QWidget *parent,
                                  const char *name )
  : KOAttendeeEditor( parent, name), mDisableItemUpdate( false )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  initOrganizerWidgets( this, topLayout );

  mListView = new KOAttendeeListView( this, "mListView" );
  QWhatsThis::add( mListView,
		   i18n("Displays information about current attendees. "
		   	"To edit an attendee, select it in this list "
			"and modify the values in the area below. "
		   	"Clicking on a column title will sort the list "
			"according to that column. The RSVP column "
			"indicates whether or not a response is requested "
			"from the attendee.") );
  mListView->addColumn( i18n("Name"), 200 );
  mListView->addColumn( i18n("Email"), 200 );
  mListView->addColumn( i18n("Role"), 80 );
  mListView->addColumn( i18n("Status"), 100 );
  mListView->addColumn( i18n("RSVP"), 55 );
  mListView->addColumn( i18n("Delegated to"), 120 );
  mListView->addColumn( i18n("Delegated from" ), 120 );
  mListView->setResizeMode( QListView::LastColumn );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mListView->setFixedHeight( 78 );
  }

  connect( mListView, SIGNAL( selectionChanged( QListViewItem * ) ),
           SLOT( updateAttendeeInput() ) );
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
  AttendeeListItem *aItem =
      static_cast<AttendeeListItem *>( mListView->selectedItem() );
  if ( !aItem ) return;

  Attendee *delA = new Attendee( aItem->data()->name(), aItem->data()->email(),
                                 aItem->data()->RSVP(), aItem->data()->status(),
                                 aItem->data()->role(), aItem->data()->uid() );
  mdelAttendees.append( delA );

  delete aItem;

  updateAttendeeInput();
  emit updateAttendeeSummary( mListView->childCount() );
}


void KOEditorDetails::insertAttendee( Attendee *a, bool goodEmailAddress )
{
  Q_UNUSED( goodEmailAddress );

  // lastItem() is O(n), but for n very small that should be fine
  AttendeeListItem *item = new AttendeeListItem( a, mListView,
      static_cast<KListViewItem*>( mListView->lastItem() ) );
  mListView->setSelected( item, true );
  emit updateAttendeeSummary( mListView->childCount() );
}

void KOEditorDetails::setDefaults()
{
  mRsvpButton->setChecked( true );
}

void KOEditorDetails::readEvent( Incidence *event )
{
  mListView->clear();
  KOAttendeeEditor::readEvent( event );

  mListView->setSelected( mListView->firstChild(), true );

  emit updateAttendeeSummary( mListView->childCount() );
}

void KOEditorDetails::writeEvent(Incidence *event)
{
  event->clearAttendees();
  QValueVector<QListViewItem*> toBeDeleted;
  QListViewItem *item;
  AttendeeListItem *a;
  for (item = mListView->firstChild(); item;
       item = item->nextSibling()) {
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
      if ( attendee->email().endsWith( "example.net" ) ) {
        if ( KMessageBox::warningYesNo( this, i18n("%1 does not look like a valid email address. "
                "Are you sure you want to invite this participant?").arg( attendee->email() ),
              i18n("Invalid email address") ) != KMessageBox::Yes ) {
          skip = true;
        }
      }
      if ( !skip ) {
        event->addAttendee( new Attendee( *attendee ) );
      }
    }
  }

  KOAttendeeEditor::writeEvent( event );

  // cleanup
  QValueVector<QListViewItem*>::iterator it;
  for( it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it ) {
    delete *it;
  }
}

bool KOEditorDetails::validateInput()
{
  return true;
}

KCal::Attendee * KOEditorDetails::currentAttendee() const
{
  QListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if ( !aItem )
    return 0;
  return aItem->data();
}

void KOEditorDetails::updateCurrentItem()
{
  AttendeeListItem *item = static_cast<AttendeeListItem*>( mListView->selectedItem() );
  if ( item )
    item->updateItem();
}

void KOEditorDetails::slotInsertAttendee(Attendee * a)
{
  insertAttendee( a );
}

void KOEditorDetails::changeStatusForMe(Attendee::PartStat status)
{
  const QStringList myEmails = KOPrefs::instance()->allEmails();
  for ( QListViewItemIterator it( mListView ); it.current(); ++it ) {
    AttendeeListItem *item = static_cast<AttendeeListItem*>( it.current() );
    for ( QStringList::ConstIterator it2( myEmails.begin() ), end( myEmails.end() ); it2 != end; ++it2 ) {
      if ( item->data()->email() == *it2 ) {
        item->data()->setStatus( status );
        item->updateItem();
      }
    }
  }
}

#include "koeditordetails.moc"
