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
#include "kogroupware.h"

#include "koeditorgantt.h"


// We can't use the CustomListViewItem base class, since we need a
// different inheritance hierarchy for supporting the Gantt view.
class GanttItem : public KDGanttViewTaskItem
{
  public:
    GanttItem( Attendee* data, KDGanttView *parent ) :
      KDGanttViewTaskItem( parent ), mData( data )
    {
      Q_ASSERT( data );
      updateItem();
      setFreeBusyPeriods( 0 );
    }
    ~GanttItem();

    void updateItem();

    Attendee* data() const { return mData; }
    void setFreeBusy( KCal::FreeBusy* fb ) { mFreeBusy = fb; }
    KCal::FreeBusy* freeBusy() const { return mFreeBusy; }

    void setFreeBusyPeriods( FreeBusy* fb );

    QString key(int column, bool) const
    {
      QMap<int,QString>::ConstIterator it = mKeyMap.find(column);
      if (it == mKeyMap.end()) return listViewText(column);
      else return *it;
    }

    void setSortKey(int column,const QString &key)
    {
      mKeyMap.insert(column,key);
    }

    QString email() const { return mData->email(); }

  private:
    Attendee* mData;
    KCal::FreeBusy* mFreeBusy;

    QMap<int,QString> mKeyMap;
};



GanttItem::~GanttItem()
{
}

void GanttItem::updateItem()
{
  setListViewText(0,mData->name());
  setListViewText(1,mData->email());
  setListViewText(2,mData->roleStr());
  setListViewText(3,mData->statusStr());
  if (mData->RSVP() && !mData->email().isEmpty())
    setPixmap(4,SmallIcon("mailappt"));
  else
    setPixmap(4,SmallIcon("nomailappt"));
}


// Set the free/busy periods for this attendee
void GanttItem::setFreeBusyPeriods( FreeBusy* fb )
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


KOEditorGantt::KOEditorGantt( int spacing, QWidget* parent, const char* name )
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
  QHBox* scaleHB = new QHBox( this );
  topLayout->addWidget( scaleHB );
  QLabel* scaleLA = new QLabel( i18n( "Scale: " ), scaleHB );
  scaleLA->setAlignment( AlignRight | AlignVCenter );
  scaleCombo = new QComboBox( scaleHB ); 
  scaleCombo->insertItem( i18n( "Hour" ) );
  scaleCombo->insertItem( i18n( "Day" ) );
  scaleCombo->insertItem( i18n( "Week" ) );
  scaleCombo->insertItem( i18n( "Month" ) );
  scaleCombo->insertItem( i18n( "Automatic" ) );
  scaleCombo->setCurrentItem( 0 ); // start with "hour"
  QLabel* dummy = new QLabel( scaleHB );
  scaleHB->setStretchFactor( dummy, 2 );
  QLabel* hFormatLA = new QLabel( i18n( "Hour format:" ), scaleHB );
  hFormatLA->setAlignment( AlignRight | AlignVCenter );
  dummy = new QLabel( scaleHB );
  scaleHB->setStretchFactor( dummy, 2 );
  QPushButton* centerPB = new QPushButton( i18n( "Center on Start" ), scaleHB );
  connect( centerPB, SIGNAL( clicked() ), this, SLOT( slotCenterOnStart() ) );
  dummy = new QLabel( scaleHB );
  scaleHB->setStretchFactor( dummy, 2 );
  QPushButton* zoomPB = new QPushButton( i18n( "Zoom to Fit" ), scaleHB );
  connect( zoomPB, SIGNAL( clicked() ), this, SLOT( slotZoomToTime() ) );
  dummy = new QLabel( scaleHB );
  scaleHB->setStretchFactor( dummy, 2 );
  QPushButton* pickPB = new QPushButton( i18n( "Pick Date" ), scaleHB );
  connect( pickPB, SIGNAL( clicked() ), this, SLOT( slotPickDate() ) );
  connect( scaleCombo, SIGNAL( activated( int ) ),
           this, SLOT( slotScaleChanged( int ) ) );

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
#if 0
  // TODO: Do something about this ugly kroupware_branch hack
  if ( !strcmp(QTextCodec::codecForLocale()->locale(),"de_DE@euro") ) {
    mGanttView->setHourFormat( KDGanttView::Hour_24   );
    hFormatCombo->setCurrentItem( 0 );
  }
#endif
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

  connect( mGanttView, SIGNAL( lvItemDoubleClicked( KDGanttViewItem* ) ),
           SLOT( updateFreeBusyData() ) );
}

KOEditorGantt::~KOEditorGantt()
{
}

void KOEditorGantt::removeAttendee( Attendee* attendee )
{
  GanttItem *anItem =
    static_cast<GanttItem*>( mGanttView->firstChild() );
  while( anItem ) {
    if( anItem->data() == attendee ) {
      delete anItem;
      updateStatusSummary();
      break;
    }
    anItem = static_cast<GanttItem*>( anItem->nextSibling() );
  }
}

void KOEditorGantt::insertAttendee( Attendee* attendee )
{
  (void)new GanttItem( attendee, mGanttView );
  updateFreeBusyData( attendee );
  updateStatusSummary();
}

void KOEditorGantt::updateAttendee( Attendee* attendee )
{
  GanttItem *anItem =
    static_cast<GanttItem*>( mGanttView->firstChild() );
  while( anItem ) {
    if( anItem->data() == attendee ) {
      anItem->updateItem();
      updateFreeBusyData( attendee );
      updateStatusSummary();
      break;
    }
    anItem = static_cast<GanttItem*>( anItem->nextSibling() );
  }
}

void KOEditorGantt::clearAttendees()
{
  mGanttView->clear();
}


void KOEditorGantt::setUpdateEnabled( bool enabled )
{
  mGanttView->setUpdateEnabled( enabled );
}

bool KOEditorGantt::updateEnabled() const
{
  return mGanttView->getUpdateEnabled();
}


void KOEditorGantt::readEvent( Event* event )
{
  setDateTimes( event->dtStart(), event->dtEnd() );
}


void KOEditorGantt::setDateTimes( QDateTime start, QDateTime end )
{
  mDtStart = start;
  mDtEnd = end;

  mGanttView->centerTimelineAfterShow( start );
  mGanttView->clearBackgroundColor();
  mGanttView->setIntervalBackgroundColor( start, end, Qt::magenta );
}

void KOEditorGantt::slotScaleChanged( int newScale )
{
  // The +1 is for the Minute scale which we don't offer in the combo box.
  KDGanttView::Scale scale = static_cast<KDGanttView::Scale>( newScale+1 );
  mGanttView->setScale( scale );
  slotCenterOnStart();
}

void KOEditorGantt::slotCenterOnStart() 
{
  mGanttView->centerTimeline( mDtStart );
}

void KOEditorGantt::slotZoomToTime() 
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


/*!
  This slot is called when the user changes the email address of a
  participant. It downloads the free/busy data from the net and enters
  it into the Gantt view by means of the KOGroupware class.
*/
void KOEditorGantt::updateFreeBusyData( Attendee* attendee )
{
  if( KOGroupware::instance() && attendee->name() != "(EmptyName)" ) {
    if( attendee->email() == KOPrefs::instance()->email() ) {
      // Don't download our own free-busy list from the net
      QCString fbText = KOGroupware::instance()->getFreeBusyString().utf8();
      slotInsertFreeBusy( attendee->email(),
			  KOGroupware::instance()->parseFreeBusy( fbText ) );
    } else
      KOGroupware::instance()->downloadFreeBusyData( attendee->email(), this,
						     SLOT( slotInsertFreeBusy( const QString&, FreeBusy* ) ) );
  }
}

// Set the Free Busy list for everyone having this email address
// If fb == 0, this disabled the free busy list for them
void KOEditorGantt::slotInsertFreeBusy( const QString& email, FreeBusy* fb )
{
  if( fb )
    fb->sortList();
  bool block = mGanttView->getUpdateEnabled();
  mGanttView->setUpdateEnabled(false);
  for( KDGanttViewItem* it = mGanttView->firstChild(); it;
       it = it->nextSibling() ) {
    GanttItem* item = static_cast<GanttItem*>( it );
    if( item->email() == email )
      item->setFreeBusyPeriods( fb );
  }
  mGanttView->setUpdateEnabled(block);
}


/*!
  Centers the Gantt view to the date/time passed in.
*/

void KOEditorGantt::slotUpdateGanttView( QDateTime dtFrom, QDateTime dtTo )
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
void KOEditorGantt::slotPickDate()
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
bool KOEditorGantt::findFreeSlot( QDateTime& dtFrom, QDateTime& dtTo )
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
bool KOEditorGantt::tryDate( QDateTime& tryFrom, QDateTime& tryTo )
{
  GanttItem* currentItem = static_cast<GanttItem*>( mGanttView->firstChild() );
  while( currentItem ) {
    if( !tryDate( currentItem, tryFrom, tryTo ) ) {
      // kdDebug(5850) << "++++date is not OK, new suggestion: " << tryFrom.toString() << " to " << tryTo.toString() << endl;
      return false;
    }

    currentItem = static_cast<GanttItem*>( currentItem->nextSibling() );
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
bool KOEditorGantt::tryDate( GanttItem* attendee,
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

void KOEditorGantt::updateStatusSummary()
{
  GanttItem *aItem =
    static_cast<GanttItem*>(mGanttView->firstChild());
  int total = 0;
  int accepted = 0;
  int tentative = 0;
  int declined = 0;
  while( aItem ) {
    ++total;
    switch( aItem->data()->status() ) {
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
    aItem = static_cast<GanttItem*>(aItem->nextSibling());
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

#include "koeditorgantt.moc"
