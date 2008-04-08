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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qtooltip.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qvaluevector.h>
#include <qwhatsthis.h>

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

#include <libkcal/event.h>
#include <libkcal/freebusy.h>

#include <libemailfunctions/email.h>

#include <kdgantt/KDGanttView.h>
#include <kdgantt/KDGanttViewTaskItem.h>
#include <kdgantt/KDGanttViewSubwidgets.h>

#include "koprefs.h"
#include "koglobals.h"
#include "kogroupware.h"
#include "freebusymanager.h"
#include "freebusyurldialog.h"

#include "koeditorfreebusy.h"

// The FreeBusyItem is the whole line for a given attendee.
// Individual "busy" periods are created as sub-items of this item.
//
// We can't use the CustomListViewItem base class, since we need a
// different inheritance hierarchy for supporting the Gantt view.
class FreeBusyItem : public KDGanttViewTaskItem
{
  public:
    FreeBusyItem( Attendee *attendee, KDGanttView *parent ) :
      KDGanttViewTaskItem( parent, parent->lastItem() ), mAttendee( attendee ), mTimerID( 0 ),
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

    void startDownload( bool forceDownload ) {
      mIsDownloading = true;
      FreeBusyManager *m = KOGroupware::instance()->freeBusyManager();
      if ( !m->retrieveFreeBusy( attendee()->email(), forceDownload ) )
        mIsDownloading = false;
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
  setListViewText( 0, mAttendee->fullName() );
  switch ( mAttendee->status() ) {
    case Attendee::Accepted:
      setPixmap( 0, KOGlobals::self()->smallIcon( "ok" ) );
      break;
    case Attendee::Declined:
      setPixmap( 0, KOGlobals::self()->smallIcon( "no" ) );
      break;
    case Attendee::NeedsAction:
    case Attendee::InProcess:
      setPixmap( 0, KOGlobals::self()->smallIcon( "help" ) );
      break;
    case Attendee::Tentative:
      setPixmap( 0, KOGlobals::self()->smallIcon( "apply" ) );
      break;
    case Attendee::Delegated:
      setPixmap( 0, KOGlobals::self()->smallIcon( "mail_forward" ) );
      break;
    default:
      setPixmap( 0, QPixmap() );
  }
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
      QString toolTip;
      if ( !(*it).summary().isEmpty() )
        toolTip += "<b>" + (*it).summary() + "</b><br/>";
      if ( !(*it).location().isEmpty() )
        toolTip += i18n( "Location: %1" ).arg( (*it).location() );
      if ( !toolTip.isEmpty() )
        newSubItem->setTooltipText( toolTip );
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

////

KOEditorFreeBusy::KOEditorFreeBusy( int spacing, QWidget *parent,
                                    const char *name )
  : KOAttendeeEditor( parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  initOrganizerWidgets( this, topLayout );

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
  connect( button, SIGNAL( clicked() ), SLOT( manualReload() ) );

  mGanttView = new KDGanttView( this, "mGanttView" );
  QWhatsThis::add( mGanttView,
		   i18n("Shows the free/busy status of all attendees. "
		   	"Double-clicking on an attendees entry in the "
			"list will allow you to enter the location of their "
			"Free/Busy Information.") );
  topLayout->addWidget( mGanttView );
  // Remove the predefined "Task Name" column
  mGanttView->removeColumn( 0 );
  mGanttView->addColumn( i18n("Attendee") );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mGanttView->setFixedHeight( 78 );
  }
  mGanttView->setHeaderVisible( true );
  mGanttView->setScale( KDGanttView::Hour );
  mGanttView->setShowHeaderPopupMenu( false, false, false, false, false, false );
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

  // mEventRectangle is the colored rectangle representing the event being modified
  mEventRectangle = new KDIntervalColorRectangle( mGanttView );
  mEventRectangle->setColor( Qt::magenta );
  mGanttView->addIntervalBackgroundColor( mEventRectangle );

  connect( mGanttView, SIGNAL ( timeIntervalSelected( const QDateTime &,
                                                      const QDateTime & ) ),
           mGanttView, SLOT( zoomToSelection( const QDateTime &,
                                              const  QDateTime & ) ) );
  connect( mGanttView, SIGNAL( lvItemDoubleClicked( KDGanttViewItem * ) ),
           SLOT( editFreeBusyUrl( KDGanttViewItem * ) ) );
  connect( mGanttView, SIGNAL( intervalColorRectangleMoved( const QDateTime&, const QDateTime& ) ),
           this, SLOT( slotIntervalColorRectangleMoved( const QDateTime&, const QDateTime& ) ) );

  connect( mGanttView, SIGNAL(lvSelectionChanged(KDGanttViewItem*)),
          this, SLOT(updateAttendeeInput()) );
  connect( mGanttView, SIGNAL(lvItemLeftClicked(KDGanttViewItem*)),
           this, SLOT(showAttendeeStatusMenu()) );
  connect( mGanttView, SIGNAL(lvItemRightClicked(KDGanttViewItem*)),
           this, SLOT(showAttendeeStatusMenu()) );
  connect( mGanttView, SIGNAL(lvMouseButtonClicked(int, KDGanttViewItem*, const QPoint&, int)),
           this, SLOT(listViewClicked(int, KDGanttViewItem*)) );

  FreeBusyManager *m = KOGroupware::instance()->freeBusyManager();
  connect( m, SIGNAL( freeBusyRetrieved( KCal::FreeBusy *, const QString & ) ),
           SLOT( slotInsertFreeBusy( KCal::FreeBusy *, const QString & ) ) );

  connect( &mReloadTimer, SIGNAL( timeout() ), SLOT( autoReload() ) );

  initEditWidgets( this, topLayout );
  connect( mRemoveButton, SIGNAL(clicked()),
           SLOT(removeAttendee()) );

  slotOrganizerChanged( mOrganizerCombo->currentText() );
  connect( mOrganizerCombo, SIGNAL( activated(const QString&) ),
           this, SLOT( slotOrganizerChanged(const QString&) ) );

  //suppress the buggy consequences of clicks on the time header widget
  mGanttView->timeHeaderWidget()->installEventFilter( this );
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
  else {
    clearSelection();
    mGanttView->setSelected( item, true );
  }
  updateStatusSummary();
  emit updateAttendeeSummary( mGanttView->childCount() );
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
  bool block = updateEnabled();
  setUpdateEnabled( false );
  clearAttendees();

  setDateTimes( event->dtStart(), event->dtEnd() );
  mIsOrganizer = KOPrefs::instance()->thatIsMe( event->organizer().email() );
  updateStatusSummary();
  clearSelection();
  KOAttendeeEditor::readEvent( event );

  setUpdateEnabled( block );
  emit updateAttendeeSummary( mGanttView->childCount() );
}

void KOEditorFreeBusy::slotIntervalColorRectangleMoved( const QDateTime& start, const QDateTime& end )
{
  kdDebug() << k_funcinfo << "slotIntervalColorRectangleMoved " << start << "," << end << endl;
  mDtStart = start;
  mDtEnd = end;
  emit dateTimesChanged( start, end );
}

void KOEditorFreeBusy::setDateTimes( const QDateTime &start, const QDateTime &end )
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
      item->startDownload( mForceDownload );
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

void KOEditorFreeBusy::slotUpdateGanttView( const QDateTime &dtFrom, const QDateTime &dtTo )
{
  mDtStart = dtFrom;
  mDtEnd = dtTo;
  bool block = mGanttView->getUpdateEnabled( );
  mGanttView->setUpdateEnabled( false );
  QDateTime horizonStart = QDateTime( dtFrom.addDays( -15 ).date() );
  mGanttView->setHorizonStart( horizonStart  );
  mGanttView->setHorizonEnd( dtTo.addDays( 15 ) );
  mEventRectangle->setDateTimes( dtFrom, dtTo );
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
          i18n( "The meeting already has suitable start/end times." ), QString::null,
          "MeetingTimeOKFreeBusy" );
    } else {
      emit dateTimesChanged( start, end );
      slotUpdateGanttView( start, end );
      KMessageBox::information( this,
          i18n( "The meeting has been moved to\nStart: %1\nEnd: %2." )
          .arg( start.toString() ).arg( end.toString() ), QString::null,
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

void KOEditorFreeBusy::manualReload()
{
  mForceDownload = true;
  reload();
}

void KOEditorFreeBusy::autoReload()
{
  mForceDownload = false;
  reload();
}

void KOEditorFreeBusy::reload()
{
  kdDebug(5850) << "KOEditorFreeBusy::reload()" << endl;

  FreeBusyItem *item = static_cast<FreeBusyItem *>( mGanttView->firstChild() );
  while( item ) {
    if (  mForceDownload )
      item->startDownload( mForceDownload );
    else
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

void KOEditorFreeBusy::writeEvent(KCal::Event * event)
{
  event->clearAttendees();
  QValueVector<FreeBusyItem*> toBeDeleted;
  for ( FreeBusyItem *item = static_cast<FreeBusyItem *>( mGanttView->firstChild() ); item;
        item = static_cast<FreeBusyItem*>( item->nextSibling() ) )
  {
    Attendee *attendee = item->attendee();
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
  QValueVector<FreeBusyItem*>::iterator it;
  for( it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it ) {
    delete *it;
  }
}

KCal::Attendee * KOEditorFreeBusy::currentAttendee() const
{
  KDGanttViewItem *item = mGanttView->selectedItem();
  FreeBusyItem *aItem = static_cast<FreeBusyItem*>( item );
  if ( !aItem )
    return 0;
  return aItem->attendee();
}

void KOEditorFreeBusy::updateCurrentItem()
{
  FreeBusyItem* item = static_cast<FreeBusyItem*>( mGanttView->selectedItem() );
  if ( item ) {
    item->updateItem();
    updateFreeBusyData( item );
    updateStatusSummary();
  }
}

void KOEditorFreeBusy::removeAttendee()
{
  FreeBusyItem *item = static_cast<FreeBusyItem*>( mGanttView->selectedItem() );
  if ( !item )
    return;

  Attendee *delA = new Attendee( item->attendee()->name(), item->attendee()->email(),
                                 item->attendee()->RSVP(), item->attendee()->status(),
                                 item->attendee()->role(), item->attendee()->uid() );
  mdelAttendees.append( delA );
  delete item;

  updateStatusSummary();
  updateAttendeeInput();
  emit updateAttendeeSummary( mGanttView->childCount() );
}

void KOEditorFreeBusy::clearSelection() const
{
  KDGanttViewItem *item = mGanttView->selectedItem();
  if ( item )
    mGanttView->setSelected( item, false );
  mGanttView->repaint();
  item->repaint();
}

void KOEditorFreeBusy::changeStatusForMe(KCal::Attendee::PartStat status)
{
  const QStringList myEmails = KOPrefs::instance()->allEmails();
  for ( FreeBusyItem *item = static_cast<FreeBusyItem *>( mGanttView->firstChild() ); item;
        item = static_cast<FreeBusyItem*>( item->nextSibling() ) )
  {
    for ( QStringList::ConstIterator it2( myEmails.begin() ), end( myEmails.end() ); it2 != end; ++it2 ) {
      if ( item->attendee()->email() == *it2 ) {
        item->attendee()->setStatus( status );
        item->updateItem();
      }
    }
  }
}

void KOEditorFreeBusy::showAttendeeStatusMenu()
{
  if ( mGanttView->mapFromGlobal( QCursor::pos() ).x() > 22 )
    return;
  QPopupMenu popup;
  popup.insertItem( SmallIcon( "help" ), Attendee::statusName( Attendee::NeedsAction ), Attendee::NeedsAction );
  popup.insertItem( KOGlobals::self()->smallIcon( "ok" ), Attendee::statusName( Attendee::Accepted ), Attendee::Accepted );
  popup.insertItem( KOGlobals::self()->smallIcon( "no" ), Attendee::statusName( Attendee::Declined ), Attendee::Declined );
  popup.insertItem( KOGlobals::self()->smallIcon( "apply" ), Attendee::statusName( Attendee::Tentative ), Attendee::Tentative );
  popup.insertItem( KOGlobals::self()->smallIcon( "mail_forward" ), Attendee::statusName( Attendee::Delegated ), Attendee::Delegated );
  popup.insertItem( Attendee::statusName( Attendee::Completed ), Attendee::Completed );
  popup.insertItem( KOGlobals::self()->smallIcon( "help" ), Attendee::statusName( Attendee::InProcess ), Attendee::InProcess );
  popup.setItemChecked( currentAttendee()->status(), true );
  int status = popup.exec( QCursor::pos() );
  if ( status >= 0 ) {
    currentAttendee()->setStatus( (Attendee::PartStat)status );
    updateCurrentItem();
    updateAttendeeInput();
  }
}

void KOEditorFreeBusy::listViewClicked(int button, KDGanttViewItem * item)
{
  if ( button == Qt::LeftButton && item == 0 )
    addNewAttendee();
}

void KOEditorFreeBusy::slotOrganizerChanged(const QString & newOrganizer)
{
  if (newOrganizer==mCurrentOrganizer) return;

  QString name;
  QString email;
  bool success = KPIM::getNameAndMail( newOrganizer, name, email );

  if (!success) return;
//

  Attendee *currentOrganizerAttendee = 0;
  Attendee *newOrganizerAttendee = 0;

  FreeBusyItem *anItem =
    static_cast<FreeBusyItem *>( mGanttView->firstChild() );
  while( anItem ) {
    Attendee *attendee = anItem->attendee();
    if( attendee->fullName() == mCurrentOrganizer )
      currentOrganizerAttendee = attendee;

    if( attendee->fullName() == newOrganizer )
      newOrganizerAttendee = attendee;

    anItem = static_cast<FreeBusyItem *>( anItem->nextSibling() );
  }

  int answer = KMessageBox::No;

  if (currentOrganizerAttendee) {
    answer = KMessageBox::questionYesNo( this, i18n("You are changing the organiser of "
                                                    "this event, who is also attending, "
                                                    "do you want to change that attendee "
                                                    "as well?") );
  } else {
    answer = KMessageBox::Yes;
  }

  if (answer==KMessageBox::Yes) {
    if (currentOrganizerAttendee) {
      removeAttendee( currentOrganizerAttendee );
    }

    if (!newOrganizerAttendee) {
      Attendee *a = new Attendee( name, email, true );
      insertAttendee( a, false );
      updateAttendee();
    }
  }

  mCurrentOrganizer = newOrganizer;
}

bool KOEditorFreeBusy::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mGanttView->timeHeaderWidget() &&
       event->type() >= QEvent::MouseButtonPress && event->type() <= QEvent::MouseMove ) {
    return true;
  } else {
    return KOAttendeeEditor::eventFilter( watched, event );
  }
}

#include "koeditorfreebusy.moc"
