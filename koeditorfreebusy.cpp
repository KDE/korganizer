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
#include <qwhatsthis.h>

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
      KDGanttViewTaskItem( parent ), mAttendee( attendee ), mTimerID( 0 ),
      mIsDownloading( false )
    {
      Q_ASSERT( attendee );
      updateItem();
      setFreeBusyPeriods( 0 );
      setDisplaySubitemsAsGroup( true );
      if ( listView () )
          listView ()->setRootIsDecorated( false );
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

    void setUpdateTimerID( int id ) { mTimerID = id; }
    int updateTimerID() const { return mTimerID; }

    void startDownload() {
      mIsDownloading = true;
      FreeBusyManager *m = KOGroupware::instance()->freeBusyManager();
      m->retrieveFreeBusy( attendee()->email() );
    }
    void setIsDownloading( bool d ) { mIsDownloading = d; }
    bool isDownloading() const { return mIsDownloading; }

  private:
    Attendee *mAttendee;
    KCal::FreeBusy *mFreeBusy;

    QMap<int,QString> mKeyMap;

    // This is used for the update timer
    int mTimerID;

    // Only run one download job at a time
    bool mIsDownloading;
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
      //debug only start
      //   int ii ;
      //       QDateTime cur = QDateTime::currentDateTime();
      //       for( ii = 0; ii < 10 ;++ii ) {
      //           KDGanttViewTaskItem* newSubItem = new KDGanttViewTaskItem( this );
      //           cur = cur.addSecs( 7200 );
      //           newSubItem->setStartTime( cur );
      //           cur = cur.addSecs( 7200 );
      //           newSubItem->setEndTime( cur );
      //           newSubItem->setColors( Qt::red, Qt::red, Qt::red );
      //       }
      //debug only end
      setFreeBusy( 0 );
      setShowNoInformation( true );
  }

  // We are no longer downloading
  mIsDownloading = false;
}


KOEditorFreeBusy::KOEditorFreeBusy( int spacing, QWidget *parent,
                                    const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  // Label for status summary information
  // Uses the tooltip palette to highlight it
  mIsOrganizer = false; // Will be set later. This is just valgrind silencing
  mStatusSummaryLabel = new QLabel( this );
  mStatusSummaryLabel->setPalette( QToolTip::palette() );
  mStatusSummaryLabel->setFrameStyle( QFrame::Plain | QFrame::Box );
  mStatusSummaryLabel->setLineWidth( 1 );
  mStatusSummaryLabel->hide(); // Will be unhidden later if you are organizer
  topLayout->addWidget( mStatusSummaryLabel );

  // The control panel for the gantt widget
  QBoxLayout *controlLayout = new QHBoxLayout( topLayout );

  QString whatsThis = i18n("Sets the zoom level on the Gantt chart. "
  			   "'Hour' shows a range of several hours, "
			   "'Day' shows a range of a few days, "
			   "'Week' shows a range of a few months, "
			   "and 'Month' shows a range of a few years, "
			   "while 'Automatic' selects the range most "
			   "appropriate for the current event or to-do.");
  QLabel *label = new QLabel( i18n( "Scale: " ), this );
  QWhatsThis::add( label, whatsThis );
  controlLayout->addWidget( label );

  scaleCombo = new QComboBox( this ); 
  QWhatsThis::add( scaleCombo, whatsThis );
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
  QWhatsThis::add( button,
		   i18n("Centers the Gantt chart on the start time "
		        "and day of this event.") );
  connect( button, SIGNAL( clicked() ), SLOT( slotCenterOnStart() ) );
  controlLayout->addWidget( button );

  button = new QPushButton( i18n( "Zoom to Fit" ), this );
  QWhatsThis::add( button,
		   i18n("Zooms the Gantt chart so that you can see the "
			"entire duration of the event on it.") );
  connect( button, SIGNAL( clicked() ), SLOT( slotZoomToTime() ) );
  controlLayout->addWidget( button );

  controlLayout->addStretch( 1 );

  button = new QPushButton( i18n( "Pick Date" ), this );
  QWhatsThis::add( button,
		   i18n("Moves the event to a date and time when all the "
			"attendees are free.") );
  connect( button, SIGNAL( clicked() ), SLOT( slotPickDate() ) );
  controlLayout->addWidget( button );

  controlLayout->addStretch( 1 );

  button = new QPushButton( i18n("Reload"), this );
  QWhatsThis::add( button,
		   i18n("Reloads Free/Busy data for all attendees from "
		   	"the corresponding servers.") );
  controlLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( reload() ) );

  mGanttView = new KDGanttView( this, "mGanttView" );
  QWhatsThis::add( mGanttView,
		   i18n("Shows the free/busy status of all attendees. "
		   	"Double-clicking on an attendees entry in the "
			"list will allow you to enter the location of their "
			"Free/Busy Information.") );
  topLayout->addWidget( mGanttView );
  // Remove the predefined "Task Name" column
  mGanttView->removeColumn( 0 );
  mGanttView->addColumn( i18n("Name"), 180 );
  mGanttView->addColumn( i18n("Email"), 180 );
  mGanttView->addColumn( i18n("Role"), 60 );
  mGanttView->addColumn( i18n("Status"), 100 );
  mGanttView->addColumn( i18n("RSVP"), 35 );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mGanttView->setFixedHeight( 78 );
  }
  mGanttView->setHeaderVisible( true );
  mGanttView->setScale( KDGanttView::Hour );
  mGanttView->setShowHeaderPopupMenu( true, true, true, false, false, true );
  // Initially, show 15 days back and forth
  // set start to even hours, i.e. to 12:AM 0 Min 0 Sec
  QDateTime horizonStart = QDateTime( QDateTime::currentDateTime()
                           .addDays( -15 ).date() );
  QDateTime horizonEnd = QDateTime::currentDateTime().addDays( 15 );
  mGanttView->setHorizonStart( horizonStart );
  mGanttView->setHorizonEnd( horizonEnd );
  mGanttView->setCalendarMode( true );
  //mGanttView->setDisplaySubitemsAsGroup( true );
  mGanttView->setShowLegendButton( false );
  // Initially, center to current date
  mGanttView->centerTimelineAfterShow( QDateTime::currentDateTime() );
  if ( KGlobal::locale()->use12Clock() )
    mGanttView->setHourFormat( KDGanttView::Hour_12 );
  else
    mGanttView->setHourFormat( KDGanttView::Hour_24_FourDigit );
  connect( mGanttView, SIGNAL ( timeIntervalSelected( const QDateTime &,
                                                      const QDateTime & ) ),
           mGanttView, SLOT( zoomToSelection( const QDateTime &,
                                              const  QDateTime & ) ) );
  connect( mGanttView, SIGNAL( lvItemDoubleClicked( KDGanttViewItem * ) ),
           SLOT( editFreeBusyUrl( KDGanttViewItem * ) ) );

  FreeBusyManager *m = KOGroupware::instance()->freeBusyManager();
  connect( m, SIGNAL( freeBusyRetrieved( KCal::FreeBusy *, const QString & ) ),
           SLOT( slotInsertFreeBusy( KCal::FreeBusy *, const QString & ) ) );

  connect( &mReloadTimer, SIGNAL( timeout() ), SLOT( reload() ) );
}

KOEditorFreeBusy::~KOEditorFreeBusy()
{
}

void KOEditorFreeBusy::removeAttendee( Attendee *attendee )
{
  FreeBusyItem *anItem =
      static_cast<FreeBusyItem *>( mGanttView->firstChild() );
  while( anItem ) {
    if( anItem->attendee() == attendee ) {
      if ( anItem->updateTimerID() != 0 )
        killTimer( anItem->updateTimerID() );
      delete anItem;
      updateStatusSummary();
      break;
    }
    anItem = static_cast<FreeBusyItem *>( anItem->nextSibling() );
  }
}

void KOEditorFreeBusy::insertAttendee( Attendee *attendee, bool readFBList )
{
  FreeBusyItem* item = new FreeBusyItem( attendee, mGanttView );
  if ( readFBList )
    updateFreeBusyData( item );
  updateStatusSummary();
}

void KOEditorFreeBusy::updateAttendee( Attendee *attendee )
{
  FreeBusyItem *anItem =
      static_cast<FreeBusyItem *>( mGanttView->firstChild() );
  while( anItem ) {
    if( anItem->attendee() == attendee ) {
      anItem->updateItem();
      updateFreeBusyData( anItem );
      updateStatusSummary();
      break;
    }
    anItem = static_cast<FreeBusyItem *>( anItem->nextSibling() );
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


void KOEditorFreeBusy::readEvent( Event *event )
{
  setDateTimes( event->dtStart(), event->dtEnd() );
  mIsOrganizer = KOPrefs::instance()->thatIsMe( event->organizer().email() );
  updateStatusSummary();
}


void KOEditorFreeBusy::setDateTimes( QDateTime start, QDateTime end )
{
  slotUpdateGanttView( start, end );
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
  mGanttView->zoomToFit();
}

void KOEditorFreeBusy::updateFreeBusyData( FreeBusyItem* item )
{
  if ( item->isDownloading() )
    // This item is already in the process of fetching the FB list
    return;

  if ( item->updateTimerID() != 0 )
    // An update timer is already running. Reset it
    killTimer( item->updateTimerID() );

  // This item does not have a download running, and no timer is set
  // Do the download in five seconds
  item->setUpdateTimerID( startTimer( 5000 ) );
}

void KOEditorFreeBusy::timerEvent( QTimerEvent* event )
{
  killTimer( event->timerId() );
  FreeBusyItem *item = static_cast<FreeBusyItem *>( mGanttView->firstChild() );
  while( item ) {
    if( item->updateTimerID() == event->timerId() ) {
      item->setUpdateTimerID( 0 );
      item->startDownload();
      return;
    }
    item = static_cast<FreeBusyItem *>( item->nextSibling() );
  }
}

// Set the Free Busy list for everyone having this email address
// If fb == 0, this disabled the free busy list for them
void KOEditorFreeBusy::slotInsertFreeBusy( KCal::FreeBusy *fb,
                                           const QString &email )
{
  kdDebug(5850) << "KOEditorFreeBusy::slotInsertFreeBusy() " << email << endl;

  if( fb )
    fb->sortList();
  bool block = mGanttView->getUpdateEnabled();
  mGanttView->setUpdateEnabled( false );
  for( KDGanttViewItem *it = mGanttView->firstChild(); it;
       it = it->nextSibling() ) {
    FreeBusyItem *item = static_cast<FreeBusyItem *>( it );
    if( item->email() == email )
      item->setFreeBusyPeriods( fb );
  }
  mGanttView->setUpdateEnabled( block );
}


/*!
  Centers the Gantt view to the date/time passed in.
*/

void KOEditorFreeBusy::slotUpdateGanttView( QDateTime dtFrom, QDateTime dtTo )
{
  mDtStart = dtFrom;
  mDtEnd = dtTo;
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
      KMessageBox::information( this,
          i18n( "The meeting already has suitable start/end times." ),
          "MeetinTimeOKFreeBusy" );
    } else {
      emit dateTimesChanged( start, end );
      slotUpdateGanttView( start, end );
      KMessageBox::information( this,
          i18n( "The meeting has been moved to\nStart: %1\nEnd: %2." )
          .arg( start.toString() ).arg( end.toString() ),
          "MeetingMovedFreeBusy" );
    }
  } else
    KMessageBox::sorry( this, i18n( "No suitable date found." ) );
}


/*!
  Finds a free slot in the future which has at least the same size as
  the initial slot.
*/
bool KOEditorFreeBusy::findFreeSlot( QDateTime &dtFrom, QDateTime &dtTo )
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
bool KOEditorFreeBusy::tryDate( FreeBusyItem *attendee,
                                QDateTime &tryFrom, QDateTime &tryTo )
{
  // If we don't have any free/busy information, assume the
  // participant is free. Otherwise a participant without available
  // information would block the whole allocation.
  KCal::FreeBusy *fb = attendee->freeBusy();
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
    static_cast<FreeBusyItem *>( mGanttView->firstChild() );
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
    aItem = static_cast<FreeBusyItem *>( aItem->nextSibling() );
  }
  if( total > 1 && mIsOrganizer ) {
    mStatusSummaryLabel->show();
    mStatusSummaryLabel->setText(
        i18n( "Of the %1 participants, %2 have accepted, %3"
              " have tentatively accepted, and %4 have declined.")
        .arg( total ).arg( accepted ).arg( tentative ).arg( declined ) );
  } else {
    mStatusSummaryLabel->hide();
  }
  mStatusSummaryLabel->adjustSize();
}

void KOEditorFreeBusy::triggerReload()
{
  mReloadTimer.start( 1000, true );
}

void KOEditorFreeBusy::cancelReload()
{
  mReloadTimer.stop();
}

void KOEditorFreeBusy::reload()
{
  kdDebug(5850) << "KOEditorFreeBusy::reload()" << endl;

  FreeBusyItem *item = static_cast<FreeBusyItem *>( mGanttView->firstChild() );
  while( item ) {
    updateFreeBusyData( item );
    item = static_cast<FreeBusyItem *>( item->nextSibling() );
  }
}

void KOEditorFreeBusy::editFreeBusyUrl( KDGanttViewItem *i )
{
  FreeBusyItem *item = static_cast<FreeBusyItem *>( i );
  if ( !item ) return;

  Attendee *attendee = item->attendee();

  FreeBusyUrlDialog dialog( attendee, this );
  dialog.exec();
}

#include "koeditorfreebusy.moc"
