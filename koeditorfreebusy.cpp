/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>

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
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkcal/event.h>
#include <libkcal/freebusy.h>

#include <kdgantt/KDGanttView.h>
#include <kdgantt/KDGanttViewTaskItem.h>

#include "koprefs.h"
#include "koglobals.h"
#include "kogroupware.h"
#include "freebusymanager.h"
#include "freebusyurldialog.h"

#include "koeditorfreebusy.h"


// We can't use the CustomListViewItem base class, since we need a
// different inheritance hierarchy for supporting the Gantt view.
class FreeBusyItem : public KDGanttViewTaskItem
{
  public:
    FreeBusyItem( Attendee *attendee, KDGanttView *parent ) :
      KDGanttViewTaskItem( parent ), mAttendee( attendee )
    {
      Q_ASSERT( attendee );
      updateItem();
      setFreeBusyPeriods( 0 );
    }
    ~FreeBusyItem() {}

    void updateItem();

    Attendee *attendee() const { return mAttendee; }
    void setFreeBusy( KCal::FreeBusy *fb ) { mFreeBusy = fb; }
    KCal::FreeBusy* freeBusy() const { return mFreeBusy; }

    void setFreeBusyPeriods( FreeBusy *fb );

    QString key( int column, bool ) const
    {
      QMap<int,QString>::ConstIterator it = mKeyMap.find( column );
      if ( it == mKeyMap.end() ) return listViewText( column );
      else return *it;
    }

    void setSortKey( int column, const QString &key )
    {
      mKeyMap.insert( column, key );
    }

    QString email() const { return mAttendee->email(); }

  private:
    Attendee *mAttendee;
    KCal::FreeBusy *mFreeBusy;

    QMap<int,QString> mKeyMap;
};

void FreeBusyItem::updateItem()
{
  setListViewText( 0, mAttendee->name() );
  setListViewText( 1, mAttendee->email() );
  setListViewText( 2, mAttendee->roleStr() );
  setListViewText( 3, mAttendee->statusStr() );
  if ( mAttendee->RSVP() && !mAttendee->email().isEmpty() )
    setPixmap( 4, KOGlobals::self()->smallIcon( "mailappt" ) );
  else
    setPixmap( 4, KOGlobals::self()->smallIcon( "nomailappt" ) );
}


// Set the free/busy periods for this attendee
void FreeBusyItem::setFreeBusyPeriods( FreeBusy* fb )
{
  if( fb ) {
    // Clean out the old entries
    for( KDGanttViewItem* it = firstChild(); it; it = firstChild() )
      delete it;

    // Evaluate free/busy information
    QValueList<KCal::Period> busyPeriods = fb->busyPeriods();
    for( QValueList<KCal::Period>::Iterator it = busyPeriods.begin();
	 it != busyPeriods.end(); ++it ) {
      KDGanttViewTaskItem* newSubItem = new KDGanttViewTaskItem( this );
      newSubItem->setStartTime( (*it).start() );
      newSubItem->setEndTime( (*it).end() );
      newSubItem->setColors( Qt::red, Qt::red, Qt::red );
    }
    setFreeBusy( fb );
    setShowNoInformation( false );
  } else {
    // No free/busy information
    setFreeBusy( 0 );
    setShowNoInformation( true );
  }
}


KOEditorFreeBusy::KOEditorFreeBusy( int spacing, QWidget* parent, const char* name )
  : QWidget( parent, name )
{
  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  QString organizer = KOPrefs::instance()->email();
  mOrganizerLabel = new QLabel( i18n("Organizer: %1").arg( organizer ), this );
  mIsOrganizer = true; // Will be set later. This is just valgrind silencing
  topLayout->addWidget( mOrganizerLabel );

  // Label for status summary information
  // Uses the tooltip palette to highlight it
  mStatusSummaryLabel = new QLabel( this );
  mStatusSummaryLabel->setPalette( QToolTip::palette() );
  mStatusSummaryLabel->setFrameStyle( QFrame::Plain | QFrame::Box );
  mStatusSummaryLabel->setLineWidth( 1 );
  topLayout->addWidget( mStatusSummaryLabel );

  // The control panel for the gantt widget
  QBoxLayout *controlLayout = new QHBoxLayout( topLayout );

  QLabel *label = new QLabel( i18n( "Scale: " ), this );
  controlLayout->addWidget( label );

  scaleCombo = new QComboBox( this ); 
  scaleCombo->insertItem( i18n( "Hour" ) );
  scaleCombo->insertItem( i18n( "Day" ) );
  scaleCombo->insertItem( i18n( "Week" ) );
  scaleCombo->insertItem( i18n( "Month" ) );
  scaleCombo->insertItem( i18n( "Automatic" ) );
  scaleCombo->setCurrentItem( 0 ); // start with "hour"
  connect( scaleCombo, SIGNAL( activated( int ) ),
           SLOT( slotScaleChanged( int ) ) );
  controlLayout->addWidget( scaleCombo );

  QPushButton *button = new QPushButton( i18n( "Center on Start" ), this );
  connect( button, SIGNAL( clicked() ), SLOT( slotCenterOnStart() ) );
  controlLayout->addWidget( button );

  button = new QPushButton( i18n( "Zoom to Fit" ), this );
  connect( button, SIGNAL( clicked() ), SLOT( slotZoomToTime() ) );
  controlLayout->addWidget( button );

  controlLayout->addStretch( 1 );

  button = new QPushButton( i18n( "Pick Date" ), this );
  connect( button, SIGNAL( clicked() ), SLOT( slotPickDate() ) );
  controlLayout->addWidget( button );

  controlLayout->addStretch( 1 );

  button = new QPushButton( i18n("Reload"), this );
  controlLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( reload() ) );

  mGanttView = new KDGanttView(this,"mGanttView");
  topLayout->addWidget( mGanttView );
  // Remove the predefined "Task Name" column
  mGanttView->removeColumn( 0 );
  mGanttView->addColumn(i18n("Name"),180);
  mGanttView->addColumn(i18n("Email"),180);
  mGanttView->addColumn(i18n("Role"),60);
  mGanttView->addColumn(i18n("Status"),100);
  mGanttView->addColumn(i18n("RSVP"),35);
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mGanttView->setFixedHeight(78);
  }
  mGanttView->setHeaderVisible( true );
  mGanttView->setScale( KDGanttView::Hour );
  // Initially, show 15 days back and forth
  // set start to even hours, i.e. to 12:AM 0 Min 0 Sec
  QDateTime horizonStart = QDateTime( QDateTime::currentDateTime().addDays( -15 ).date() );
  QDateTime horizonEnd = QDateTime::currentDateTime().addDays( 15 );
  mGanttView->setHorizonStart( horizonStart );
  mGanttView->setHorizonEnd( horizonEnd );
  mGanttView->setCalendarMode( true );
  mGanttView->setDisplaySubitemsAsGroup( true );
  mGanttView->setShowLegendButton( false );
  // Initially, center to current date
  mGanttView->centerTimelineAfterShow( QDateTime::currentDateTime() );
  if ( KGlobal::locale()->use12Clock() )
    mGanttView->setHourFormat( KDGanttView::Hour_12 );
  else
    mGanttView->setHourFormat( KDGanttView::Hour_24 );

//  connect( mGanttView, SIGNAL( lvItemDoubleClicked( KDGanttViewItem * ) ),
//           SLOT( updateFreeBusyData( KDGanttViewItem * ) ) );
  connect( mGanttView, SIGNAL( lvItemDoubleClicked( KDGanttViewItem * ) ),
           SLOT( editFreeBusyUrl( KDGanttViewItem * ) ) );

  FreeBusyManager *m = KOGroupware::instance()->freeBusyManager();
  connect( m, SIGNAL( freeBusyRetrieved( KCal::FreeBusy *, const QString & ) ),
           SLOT( slotInsertFreeBusy( KCal::FreeBusy *, const QString & ) ) );
}

KOEditorFreeBusy::~KOEditorFreeBusy()
{
}

void KOEditorFreeBusy::removeAttendee( Attendee* attendee )
{
  FreeBusyItem *anItem =
    static_cast<FreeBusyItem*>( mGanttView->firstChild() );
  while( anItem ) {
    if( anItem->attendee() == attendee ) {
      delete anItem;
      updateStatusSummary();
      break;
    }
    anItem = static_cast<FreeBusyItem*>( anItem->nextSibling() );
  }
}

void KOEditorFreeBusy::insertAttendee( Attendee* attendee )
{
  (void)new FreeBusyItem( attendee, mGanttView );
  updateFreeBusyData( attendee );
  updateStatusSummary();
}

void KOEditorFreeBusy::updateAttendee( Attendee* attendee )
{
  FreeBusyItem *anItem =
    static_cast<FreeBusyItem*>( mGanttView->firstChild() );
  while( anItem ) {
    if( anItem->attendee() == attendee ) {
      anItem->updateItem();
      updateFreeBusyData( attendee );
      updateStatusSummary();
      break;
    }
    anItem = static_cast<FreeBusyItem*>( anItem->nextSibling() );
  }
}

void KOEditorFreeBusy::clearAttendees()
{
  mGanttView->clear();
}


void KOEditorFreeBusy::setUpdateEnabled( bool enabled )
{
  mGanttView->setUpdateEnabled( enabled );
}

bool KOEditorFreeBusy::updateEnabled() const
{
  return mGanttView->getUpdateEnabled();
}


void KOEditorFreeBusy::readEvent( Event* event )
{
  setDateTimes( event->dtStart(), event->dtEnd() );
}


void KOEditorFreeBusy::setDateTimes( QDateTime start, QDateTime end )
{
  mDtStart = start;
  mDtEnd = end;

  mGanttView->centerTimelineAfterShow( start );
  mGanttView->clearBackgroundColor();
  mGanttView->setIntervalBackgroundColor( start, end, Qt::magenta );
}

void KOEditorFreeBusy::slotScaleChanged( int newScale )
{
  // The +1 is for the Minute scale which we don't offer in the combo box.
  KDGanttView::Scale scale = static_cast<KDGanttView::Scale>( newScale+1 );
  mGanttView->setScale( scale );
  slotCenterOnStart();
}

void KOEditorFreeBusy::slotCenterOnStart() 
{
  mGanttView->centerTimeline( mDtStart );
}

void KOEditorFreeBusy::slotZoomToTime() 
{
  bool block  = mGanttView->getUpdateEnabled();
  mGanttView->setUpdateEnabled( false );
  if ( scaleCombo->currentItem() != 4 ) {
    scaleCombo->setCurrentItem( 4 );// auto
    slotScaleChanged( 4 );// auto
  }
  mGanttView->setUpdateEnabled( block );
  mGanttView->zoomToSelection( mDtStart, mDtEnd );
}

void KOEditorFreeBusy::updateFreeBusyData( KDGanttViewItem *item )
{
  FreeBusyItem *g = static_cast<FreeBusyItem *>( item );
  updateFreeBusyData( g->attendee() );
}

/*!
  This slot is called when the user changes the email address of a
  participant. It downloads the free/busy data from the net and enters
  it into the Gantt view by means of the KOGroupware class.
*/
void KOEditorFreeBusy::updateFreeBusyData( Attendee *attendee )
{
  if( KOGroupware::instance() && attendee->name() != "(EmptyName)" ) {
    FreeBusyManager *m = KOGroupware::instance()->freeBusyManager();
    m->retrieveFreeBusy( attendee->email() );
  }
}

// Set the Free Busy list for everyone having this email address
// If fb == 0, this disabled the free busy list for them
void KOEditorFreeBusy::slotInsertFreeBusy( KCal::FreeBusy *fb,
                                           const QString &email )
{
  if( fb )
    fb->sortList();
  bool block = mGanttView->getUpdateEnabled();
  mGanttView->setUpdateEnabled(false);
  for( KDGanttViewItem* it = mGanttView->firstChild(); it;
       it = it->nextSibling() ) {
    FreeBusyItem* item = static_cast<FreeBusyItem*>( it );
    if( item->email() == email )
      item->setFreeBusyPeriods( fb );
  }
  mGanttView->setUpdateEnabled(block);
}


/*!
  Centers the Gantt view to the date/time passed in.
*/

void KOEditorFreeBusy::slotUpdateGanttView( QDateTime dtFrom, QDateTime dtTo )
{
  bool block = mGanttView->getUpdateEnabled( );
  mGanttView->setUpdateEnabled( false );
  QDateTime horizonStart = QDateTime( dtFrom.addDays( -15 ).date() );
  mGanttView->setHorizonStart( horizonStart  );
  mGanttView->setHorizonEnd( dtTo.addDays( 15 ) );
  mGanttView->clearBackgroundColor();
  mGanttView->setIntervalBackgroundColor( dtFrom, dtTo, Qt::magenta ); 
  mGanttView->setUpdateEnabled( block );
  mGanttView->centerTimelineAfterShow( dtFrom );
}


/*!
  This slot is called when the user clicks the "Pick a date" button.
*/
void KOEditorFreeBusy::slotPickDate()
{
  QDateTime start = mDtStart;
  QDateTime end = mDtEnd;
  bool success = findFreeSlot( start, end );

  if( success ) {
    if ( start == mDtStart && end == mDtEnd ) {
      KMessageBox::information( this, i18n( "The meeting has already suitable start/end times." ));
    } else {
      emit dateTimesChanged( start, end );
      slotUpdateGanttView( start, end );
      KMessageBox::information( this, i18n( "The meeting has been moved to\nStart: %1\nEnd: %2." ).arg( start.toString() ).arg( end.toString() ) );
    }
  } else
    KMessageBox::sorry( this, i18n( "No suitable date found." ) );
}


/*!
  Finds a free slot in the future which has at least the same size as
  the initial slot.
*/
bool KOEditorFreeBusy::findFreeSlot( QDateTime& dtFrom, QDateTime& dtTo )
{
  if( tryDate( dtFrom, dtTo ) )
    // Current time is acceptable
    return true;

  QDateTime tryFrom = dtFrom;
  QDateTime tryTo = dtTo;

  // Make sure that we never suggest a date in the past, even if the
  // user originally scheduled the meeting to be in the past.
  if( tryFrom < QDateTime::currentDateTime() ) {
    // The slot to look for is at least partially in the past.
    int secs = tryFrom.secsTo( tryTo );
    tryFrom = QDateTime::currentDateTime();
    tryTo = tryFrom.addSecs( secs );
  }

  bool found = false;
  while( !found ) {
    found = tryDate( tryFrom, tryTo );
    // PENDING(kalle) Make the interval configurable
    if( !found && dtFrom.daysTo( tryFrom ) > 365 )
      break; // don't look more than one year in the future
  }

  dtFrom = tryFrom;
  dtTo = tryTo;

  return found;
}


/*!
  Checks whether the slot specified by (tryFrom, tryTo) is free
  for all participants. If yes, return true. If at least one
  participant is found for which this slot is occupied, this method
  returns false, and (tryFrom, tryTo) contain the next free slot for
  that participant. In other words, the returned slot does not have to
  be free for everybody else.
*/
bool KOEditorFreeBusy::tryDate( QDateTime& tryFrom, QDateTime& tryTo )
{
  FreeBusyItem* currentItem = static_cast<FreeBusyItem*>( mGanttView->firstChild() );
  while( currentItem ) {
    if( !tryDate( currentItem, tryFrom, tryTo ) ) {
      // kdDebug(5850) << "++++date is not OK, new suggestion: " << tryFrom.toString() << " to " << tryTo.toString() << endl;
      return false;
    }

    currentItem = static_cast<FreeBusyItem*>( currentItem->nextSibling() );
  }

  return true;
}

/*!
  Checks whether the slot specified by (tryFrom, tryTo) is available
  for the participant specified with attendee. If yes, return true. If
  not, return false and change (tryFrom, tryTo) to contain the next
  possible slot for this participant (not necessarily a slot that is
  available for all participants).
*/
bool KOEditorFreeBusy::tryDate( FreeBusyItem* attendee,
                               QDateTime& tryFrom, QDateTime& tryTo )
{
  // If we don't have any free/busy information, assume the
  // participant is free. Otherwise a participant without available
  // information would block the whole allocation.
  KCal::FreeBusy* fb = attendee->freeBusy();
  if( !fb )
    return true;

  QValueList<KCal::Period> busyPeriods = fb->busyPeriods();
  for( QValueList<KCal::Period>::Iterator it = busyPeriods.begin();
       it != busyPeriods.end(); ++it ) {
    if( (*it).end() <= tryFrom || // busy period ends before try period
	(*it).start() >= tryTo )  // busy period starts after try period
      continue;
    else {
      // the current busy period blocks the try period, try
      // after the end of the current busy period
      int secsDuration = tryFrom.secsTo( tryTo );
      tryFrom = (*it).end();
      tryTo = tryFrom.addSecs( secsDuration );
      // try again with the new try period
      tryDate( attendee, tryFrom, tryTo );
      // we had to change the date at least once
      return false;
    }
  }

  return true;
}

void KOEditorFreeBusy::updateStatusSummary()
{
  FreeBusyItem *aItem =
    static_cast<FreeBusyItem*>(mGanttView->firstChild());
  int total = 0;
  int accepted = 0;
  int tentative = 0;
  int declined = 0;
  while( aItem ) {
    ++total;
    switch( aItem->attendee()->status() ) {
    case Attendee::Accepted:
      ++accepted;
      break;
    case Attendee::Tentative:
      ++tentative;
      break;
    case Attendee::Declined:
      ++declined;
      break;
    case Attendee::NeedsAction:
    case Attendee::Delegated:
    case Attendee::Completed:
    case Attendee::InProcess:
      /* just to shut up the compiler */
      break;
    }
    aItem = static_cast<FreeBusyItem*>(aItem->nextSibling());
  }
  if( total > 1 && mIsOrganizer ) {
    mStatusSummaryLabel->show();
    mStatusSummaryLabel->setText( i18n( "Of the %1 participants, %2 have accepted, %3"
					" have tentatively accepted, and %4 have declined.")
				  .arg(total).arg(accepted).arg(tentative).arg(declined));
  } else {
    mStatusSummaryLabel->hide();
    mStatusSummaryLabel->setText("");
  }
  mStatusSummaryLabel->adjustSize();
}

void KOEditorFreeBusy::reload()
{
  kdDebug() << "KOEditorFreeBusy::reload()" << endl;

  FreeBusyItem *item = static_cast<FreeBusyItem *>( mGanttView->firstChild() );
  while( item ) {
    updateFreeBusyData( item->attendee() );
    item = static_cast<FreeBusyItem *>( item->nextSibling() );
  }
}

void KOEditorFreeBusy::editFreeBusyUrl( KDGanttViewItem *i )
{
  FreeBusyItem *item = static_cast<FreeBusyItem *>( i );
  Attendee *attendee = item->attendee();
  
  FreeBusyUrlDialog dialog( this );
  dialog.exec();
}

#include "koeditorfreebusy.moc"
