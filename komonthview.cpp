/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qpopupmenu.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qkeycode.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kwordwrap.h>

#include <kcalendarsystem.h>
#include <libkcal/calfilter.h>
#include <libkcal/calendar.h>
#include <libkcal/incidenceformatter.h>
#include <libkcal/calendarresources.h>

#include "koprefs.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif
#include "koglobals.h"
#include "koincidencetooltip.h"
#include "koeventpopupmenu.h"

#include "komonthview.h"
#include "komonthview.moc"

//--------------------------------------------------------------------------

KOMonthCellToolTip::KOMonthCellToolTip( QWidget *parent,
                                        KNoScrollListBox *lv )
  : QToolTip( parent )
{
  eventlist = lv;
}

void KOMonthCellToolTip::maybeTip( const QPoint & pos )
{
  QRect r;
  QListBoxItem *it = eventlist->itemAt( pos );
  MonthViewItem *i = static_cast<MonthViewItem*>( it );

  if( i && KOPrefs::instance()->mEnableToolTips ) {
    /* Calculate the rectangle. */
    r=eventlist->itemRect( it );
    /* Show the tip */
    QString tipText( IncidenceFormatter::toolTipString( i->incidence() ) );
    if ( !tipText.isEmpty() ) {
      tip( r, tipText );
    }
  }
}

KNoScrollListBox::KNoScrollListBox( QWidget *parent, const char *name )
  : QListBox( parent, name ),
    mSqueezing( false )
{
  QPalette pal = palette();
  pal.setColor( QColorGroup::Foreground, KOPrefs::instance()->agendaBgColor().dark( 150 ) );
  pal.setColor( QColorGroup::Base, KOPrefs::instance()->agendaBgColor() );
  setPalette( pal );
}

void KNoScrollListBox::setBackground( bool primary, bool workDay )
{
  QColor color;
  if ( workDay ) {
    color = KOPrefs::instance()->workingHoursColor();
  } else {
    color = KOPrefs::instance()->agendaBgColor();
  }

  QPalette pal = palette();
  if ( primary ) {
    pal.setColor( QColorGroup::Base, color );
  } else {
    pal.setColor( QColorGroup::Base, color.dark( 115 ) );
  }
  setPalette( pal );
}

void KNoScrollListBox::keyPressEvent( QKeyEvent *e )
{
  switch( e->key() ) {
    case Key_Right:
      scrollBy( 4, 0 );
      break;
    case Key_Left:
      scrollBy( -4, 0 );
      break;
    case Key_Up:
      if ( !count() ) break;
      setCurrentItem( ( currentItem() + count() - 1 ) % count() );
      if ( !itemVisible( currentItem() ) ) {
        if ( (unsigned int)currentItem() == ( count() - 1 ) ) {
          setTopItem( currentItem() - numItemsVisible() + 1 );
        } else {
          setTopItem( topItem() - 1 );
        }
      }
      break;
    case Key_Down:
      if ( !count() ) break;
      setCurrentItem( ( currentItem() + 1 ) % count() );
      if( !itemVisible( currentItem() ) ) {
        if( currentItem() == 0 ) {
          setTopItem( 0 );
        } else {
          setTopItem( topItem() + 1 );
        }
      }
    case Key_Shift:
      emit shiftDown();
      break;
    default:
      break;
  }
}

void KNoScrollListBox::keyReleaseEvent( QKeyEvent *e )
{
  switch( e->key() ) {
    case Key_Shift:
      emit shiftUp();
      break;
    default:
      break;
  }
}

void KNoScrollListBox::mousePressEvent( QMouseEvent *e )
{
  QListBox::mousePressEvent( e );

  if ( e->button() == RightButton ) {
    emit rightClick();
  }
}

void KNoScrollListBox::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  QListBox::contentsMouseDoubleClickEvent( e );
  QListBoxItem *item = itemAt( e->pos() );
  if ( !item ) {
    emit doubleClicked( item );
  }
}

void KNoScrollListBox::resizeEvent( QResizeEvent *e )
{
  bool s = count() && ( maxItemWidth() > e->size().width() );
  if ( mSqueezing || s )
    triggerUpdate( false );

  mSqueezing = s;
  QListBox::resizeEvent( e );
}

MonthViewItem::MonthViewItem( Incidence *incidence, const QDateTime &qd, 
                              const QString & s ) : QListBoxItem()
{
  setText( s );

  mIncidence = incidence;
  mDateTime = qd;

  mTodoPixmap      = KOGlobals::self()->smallIcon("todo");
  mTodoDonePixmap  = KOGlobals::self()->smallIcon("checkedbox");
  mAlarmPixmap     = KOGlobals::self()->smallIcon("bell");
  mRecurPixmap     = KOGlobals::self()->smallIcon("recur");
  mReplyPixmap     = KOGlobals::self()->smallIcon("mail_reply");

  mResourceColor = QColor();
  mTodo      = false;
  mTodoDone  = false;
  mRecur     = false;
  mAlarm     = false;
  mReply     = false;
}

void MonthViewItem::paint( QPainter *p )
{
#if QT_VERSION >= 0x030000
  bool sel = isSelected();
#else
  bool sel = selected();
#endif

  QColor bgColor = palette().color( QPalette::Normal,
            sel ? QColorGroup::Highlight : QColorGroup::Background );
  int offset=0;        
  if ( KOPrefs::instance()->monthViewUsesResourceColor() && 
    mResourceColor.isValid() ) {
    p->setBackgroundColor( mResourceColor );
    p->eraseRect( 0, 0, listBox()->maxItemWidth(), height( listBox() ) );
    offset=2;
  }
  if ( KOPrefs::instance()->monthViewUsesCategoryColor() ) {
    p->setBackgroundColor( bgColor );
    p->eraseRect( offset, offset, listBox()->maxItemWidth()-2*offset, height( listBox() )-2*offset );
  }
  int x = 3;
  if ( mTodo ) {
    p->drawPixmap( x, 0, mTodoPixmap );
    x += mTodoPixmap.width() + 2;
  }
  if ( mTodoDone ) {
    p->drawPixmap( x, 0, mTodoDonePixmap );
    x += mTodoPixmap.width() + 2;
  }
  if ( mRecur ) {
    p->drawPixmap( x, 0, mRecurPixmap );
    x += mRecurPixmap.width() + 2;
  }
  if ( mAlarm ) {
    p->drawPixmap( x, 0, mAlarmPixmap );
    x += mAlarmPixmap.width() + 2;
  }
  if ( mReply ) {
    p->drawPixmap(x, 0, mReplyPixmap );
    x += mReplyPixmap.width() + 2;
  }
  QFontMetrics fm = p->fontMetrics();
  int yPos;
  int pmheight = QMAX( mRecurPixmap.height(),
                       QMAX( mAlarmPixmap.height(), mReplyPixmap.height() ) );
  if( pmheight < fm.height() )
    yPos = fm.ascent() + fm.leading()/2;
  else
    yPos = pmheight/2 - fm.height()/2  + fm.ascent();
  QColor textColor = palette().color( QPalette::Normal, sel ? \
          QColorGroup::HighlightedText : QColorGroup::Text );
  p->setPen( textColor );

  KWordWrap::drawFadeoutText( p, x, yPos, listBox()->width() - x, text() );
}

int MonthViewItem::height( const QListBox *lb ) const
{
  return QMAX( QMAX( mRecurPixmap.height(), mReplyPixmap.height() ),
               QMAX( mAlarmPixmap.height(), lb->fontMetrics().lineSpacing()+1) );
}

int MonthViewItem::width( const QListBox *lb ) const
{
  int x = 3;
  if( mRecur ) {
    x += mRecurPixmap.width()+2;
  }
  if( mAlarm ) {
    x += mAlarmPixmap.width()+2;
  }
  if( mReply ) {
    x += mReplyPixmap.width()+2;
  }

  return( x + lb->fontMetrics().boundingRect( text() ).width() + 1 );
}


MonthViewCell::MonthViewCell( KOMonthView *parent)
  : QWidget( parent ),
    mMonthView( parent ), mPrimary( false ), mHoliday( false )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  mLabel = new QLabel( this );
  mLabel->setFrameStyle( QFrame::Panel | QFrame::Plain );
  mLabel->setLineWidth( 1 );
  mLabel->setAlignment( AlignCenter );

  mItemList = new KNoScrollListBox( this );
  mItemList->setMinimumSize( 10, 10 );
  mItemList->setFrameStyle( QFrame::Panel | QFrame::Plain );
  mItemList->setLineWidth( 1 );

  new KOMonthCellToolTip( mItemList->viewport(),
                          static_cast<KNoScrollListBox *>( mItemList ) );

  topLayout->addWidget( mItemList );

  mLabel->raise();

  mStandardPalette = palette();

  enableScrollBars( false );

  updateConfig();

  connect( mItemList, SIGNAL( doubleClicked( QListBoxItem *) ),
           SLOT( defaultAction( QListBoxItem * ) ) );
  connect( mItemList, SIGNAL( rightButtonPressed( QListBoxItem *,
                                                  const QPoint &) ),
           SLOT( contextMenu( QListBoxItem * ) ) );
  connect( mItemList, SIGNAL( highlighted( QListBoxItem *) ),
           SLOT( selection( QListBoxItem * ) ) );
  connect( mItemList, SIGNAL( clicked( QListBoxItem * ) ),
           SLOT( cellClicked( QListBoxItem * ) ) );
}

void MonthViewCell::setDate( const QDate &date )
{
//  kdDebug(5850) << "MonthViewCell::setDate(): " << date.toString() << endl;

  mDate = date;

  QString text;
  if ( KOGlobals::self()->calendarSystem()->day( date ) == 1 ) {
    text = i18n("'Month day' for month view cells", "%1 %2")
        .arg( KOGlobals::self()->calendarSystem()->monthName( date, true ) )
        .arg( KOGlobals::self()->calendarSystem()->day(mDate) );
    QFontMetrics fm( mLabel->font() );
    mLabel->resize( mLabelSize + QSize( fm.width( text ), 0 ) );
  } else {
    mLabel->resize( mLabelSize );
    text = QString::number( KOGlobals::self()->calendarSystem()->day(mDate) );
  }
  mLabel->setText( text );

  resizeEvent( 0 );
}

QDate MonthViewCell::date() const
{
  return mDate;
}

void MonthViewCell::setPrimary( bool primary )
{
  mPrimary = primary;

  if ( mPrimary ) {
    mLabel->setBackgroundMode( PaletteBase );
  } else {
    mLabel->setBackgroundMode( PaletteBackground );
  }

  mItemList->setBackground( mPrimary, KOCore::self()->isWorkDay( mDate ) );
}

bool MonthViewCell::isPrimary() const
{
  return mPrimary;
}

void MonthViewCell::setHoliday( bool holiday )
{
  mHoliday = holiday;

  if ( holiday ) {
    setPalette( mHolidayPalette );
  } else {
    setPalette( mStandardPalette );
  }
}

void MonthViewCell::setHoliday( const QString &holiday )
{
  mHolidayString = holiday;

  if ( !holiday.isEmpty() ) {
    setHoliday( true );
  }
}

void MonthViewCell::updateCell()
{
  if ( mDate == QDate::currentDate() ) {
    setPalette( mTodayPalette );
  }
  else {
    if ( mHoliday )
      setPalette( mHolidayPalette );
    else
      setPalette( mStandardPalette );
  }

  mItemList->clear();

  if ( !mHolidayString.isEmpty() ) {
    MonthViewItem *item = new MonthViewItem( 0, QDateTime( mDate ), mHolidayString );
    item->setPalette( mHolidayPalette );
    mItemList->insertItem( item );
  }
}

class MonthViewCell::CreateItemVisitor : 
      public IncidenceBase::Visitor
{
  public:
    CreateItemVisitor() : mItem(0) {}

    bool act( IncidenceBase *incidence, QDate date, QPalette stdPal )
    {
      mItem = 0;
      mDate = date;
      mStandardPalette = stdPal;
      return incidence->accept( *this );
    }
    MonthViewItem *item() const { return mItem; }

  protected:
    bool visit( Event *event ) {
      QString text;
      QDateTime dt( mDate );
      if ( event->isMultiDay() ) {
        if (mDate == event->dtStart().date()) {
          text = "(-- " + event->summary();
          dt = event->dtStart();
        } else if (mDate == event->dtEnd().date()) {
          text = event->summary() + " --)";
        } else if (!(event->dtStart().date().daysTo(mDate) % 7)) {
          text = "-- " + event->summary() + " --";
        } else {
          text = "----------------";
          dt = QDateTime( mDate );
        }
      } else {
        if (event->doesFloat())
          text = event->summary();
        else {
          text = KGlobal::locale()->formatTime(event->dtStart().time());
          dt.setTime( event->dtStart().time() );
          text += " " + event->summary();
        }
      }

      mItem = new MonthViewItem( event, dt, text );
      if (KOPrefs::instance()->monthViewUsesCategoryColor()) {
        QStringList categories = event->categories();
        QString cat = categories.first();
        if (cat.isEmpty()) {
          mItem->setPalette(QPalette(KOPrefs::instance()->mEventColor, KOPrefs::instance()->mEventColor));
        } else {
          mItem->setPalette(QPalette(*(KOPrefs::instance()->categoryColor(cat)), *(KOPrefs::instance()->categoryColor(cat))));
        }
      } else {
        mItem->setPalette( mStandardPalette );
      }

      Attendee *me = event->attendeeByMails( KOPrefs::instance()->allEmails() );
      if ( me != 0 ) {
        mItem->setReply( me->status() == Attendee::NeedsAction && me->RSVP() );
      } else
        mItem->setReply(false);
      return true;
    }
    bool visit( Todo *todo ) {
      QString text;
      if ( !KOPrefs::instance()->showAllDayTodo() ) 
        return false;
      QDateTime dt( mDate );
      if ( todo->hasDueDate() && !todo->doesFloat() ) {
        text += KGlobal::locale()->formatTime( todo->dtDue().time() );
        text += " ";
        dt.setTime( todo->dtDue().time() );
      }
      text += todo->summary();

      mItem = new MonthViewItem( todo, dt, text );
      if ( todo->doesRecur() ) {
        mDate < todo->dtDue().date() ?
        mItem->setTodoDone( true ) : mItem->setTodo( true );
      }
      else
        todo->isCompleted() ? mItem->setTodoDone( true ) : mItem->setTodo( true );
      mItem->setPalette( mStandardPalette );
      return true;
    }
  protected:
    MonthViewItem *mItem;
    QDate mDate;
    QPalette mStandardPalette;
};


void MonthViewCell::addIncidence( Incidence *incidence )
{
  CreateItemVisitor v;
  
  QColor resourceColor=KOPrefs::instance()->mEventColor;
  
  CalendarResources *calendarRsc = dynamic_cast<CalendarResources*>( mCalendar );
  if ( calendarRsc ) {
    ResourceCalendar *rscCalendar = calendarRsc->resource( incidence );
    resourceColor= *KOPrefs::instance()->resourceColor( rscCalendar->identifier() );
  }else{
    kdDebug(5850) << "MonthViewCell::addIncidence and mCalendar is not a CalendarResources" <<endl;
  }
  
  if ( v.act( incidence, mDate, mStandardPalette ) ) {
    MonthViewItem *item = v.item();
    if ( item ) {
      item->setAlarm( incidence->isAlarmEnabled() );
      item->setRecur( incidence->doesRecur() );
      item->setResourceColor(resourceColor);
      // FIXME: Find the correct position (time-wise) to insert the item.
      //        Currently, the items are displayed in "random" order instead of
      //        chronologically sorted.
      uint i = 0;
      int pos = -1;
      QDateTime dt( item->incidenceDateTime() );
      
      while ( i < mItemList->count() && pos<0 ) {
        QListBoxItem *item = mItemList->item( i );
        MonthViewItem *mvitem = dynamic_cast<MonthViewItem*>( item );
        if ( mvitem && mvitem->incidenceDateTime()>dt ) {
          pos = i;
        }
        ++i;
      }
      mItemList->insertItem( item, pos );
    }
  }
}

bool MonthViewCell::removeIncidence( Incidence *incidence )
{
  for ( uint i = 0; i < mItemList->count(); i++ ) {
    MonthViewItem *item = static_cast<MonthViewItem *>(mItemList->item( i ) );
    if ( item && item->incidence() &&
         item->incidence()->uid() == incidence->uid() ) {
      mItemList->removeItem( i );
      return true;
    }
  }

  return false;
}

void MonthViewCell::updateConfig()
{
  setFont( KOPrefs::instance()->mMonthViewFont );

  QFontMetrics fm( font() );
  mLabelSize = fm.size( 0, "30" ) +
               QSize( mLabel->frameWidth() * 2, mLabel->frameWidth() * 2 ) +
               QSize( 2, 2 );
//  mStandardPalette = mOriginalPalette;
  QColor bg = mStandardPalette.color( QPalette::Active, QColorGroup::Background );
  int h,s,v;
  bg.getHsv( &h, &s, &v );
  if ( date().month() %2 == 0 ) {
    if ( v < 128 ) {
      bg = bg.light( 125 );
    } else {
      bg = bg.dark( 125 );
    }
  }
  setPaletteBackgroundColor( bg );
//  mStandardPalette.setColor( QColorGroup::Background, bg);*/

  mHolidayPalette = mStandardPalette;
  mHolidayPalette.setColor( QColorGroup::Foreground,
                            KOPrefs::instance()->holidayColor() );
  mHolidayPalette.setColor( QColorGroup::Text,
                            KOPrefs::instance()->holidayColor() );
  mTodayPalette = mStandardPalette;
  mTodayPalette.setColor( QColorGroup::Foreground,
                          KOPrefs::instance()->highlightColor() );
  mTodayPalette.setColor( QColorGroup::Text,
                          KOPrefs::instance()->highlightColor() );
  updateCell();

  mItemList->setBackground( mPrimary, KOCore::self()->isWorkDay( mDate ) );
}

void MonthViewCell::enableScrollBars( bool enabled )
{
  if ( enabled ) {
    mItemList->setVScrollBarMode( QScrollView::Auto );
    mItemList->setHScrollBarMode( QScrollView::Auto );
  } else {
    mItemList->setVScrollBarMode( QScrollView::AlwaysOff );
    mItemList->setHScrollBarMode( QScrollView::AlwaysOff );
  }
}

Incidence *MonthViewCell::selectedIncidence()
{
  int index = mItemList->currentItem();
  if ( index < 0 ) return 0;

  MonthViewItem *item =
      static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) return 0;

  return item->incidence();
}

QDate MonthViewCell::selectedIncidenceDate()
{
  QDate qd;
  int index = mItemList->currentItem();
  if ( index < 0 ) return qd;

  MonthViewItem *item =
      static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) return qd;

  return item->incidenceDateTime().date();
}

void MonthViewCell::deselect()
{
  mItemList->clearSelection();

  enableScrollBars( false );
}

void MonthViewCell::resizeEvent ( QResizeEvent * )
{
  mLabel->move( width() - mLabel->width(), height() - mLabel->height() );
}

void MonthViewCell::defaultAction( QListBoxItem *item )
{
  if ( !item ) {
    emit newEventSignal( date() );
  } else {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) mMonthView->defaultAction( incidence );
  }
}

void MonthViewCell::cellClicked( QListBoxItem * )
{
  if( KOPrefs::instance()->enableMonthScroll() ) enableScrollBars( true );
}

void MonthViewCell::contextMenu( QListBoxItem *item )
{
  mMonthView->setSelectedCell( this );
  if ( item ) {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) mMonthView->showEventContextMenu( incidence, date() );
  }
  else {
    mMonthView->showGeneralContextMenu();
  }
}

void MonthViewCell::selection( QListBoxItem *item )
{
  if ( !item ) return;

  mMonthView->setSelectedCell( this );
}

KOMonthView::KOMonthView( Calendar *calendar, QWidget *parent, const char *name )
    : KOEventView( calendar, parent, name ),
      mDaysPerWeek( 7 ), mNumWeeks( 6 ), mNumCells( mDaysPerWeek * mNumWeeks ),
      mShortDayLabels( false ), mWidthLongDayLabel( 0 ), mSelectedCell( 0 )
{
  mCells.setAutoDelete( true );

  QGridLayout *dayLayout = new QGridLayout( this );

  // create the day of the week labels (Sun, Mon, etc) and add them to
  // the layout.
  mDayLabels.resize( mDaysPerWeek );
  QFont bfont = font();
  bfont.setBold( true );
  int i;
  for( i = 0; i < mDaysPerWeek; i++ ) {
    QLabel *label = new QLabel( this );
    label->setFont( bfont );
    label->setFrameStyle( QFrame::Panel | QFrame::Raised );
    label->setLineWidth( 1 );
    label->setAlignment( AlignCenter );

    mDayLabels.insert( i, label );

    dayLayout->addWidget( label, 0, i );
    dayLayout->addColSpacing( i, 10 );
    dayLayout->setColStretch( i, 1 );
  }

  int row, col;

  mCells.resize( mNumCells );
  for( row = 0; row < mNumWeeks; ++row ) {
    for( col = 0; col < mDaysPerWeek; ++col ) {
      MonthViewCell *cell = new MonthViewCell( this );
      cell->setCalendar(calendar);
      mCells.insert( row * mDaysPerWeek + col, cell );
      dayLayout->addWidget( cell, row + 1, col );

      connect( cell, SIGNAL( defaultAction( Incidence * ) ),
               SLOT( defaultAction( Incidence * ) ) );
      connect( cell, SIGNAL( newEventSignal( QDate ) ),
               SIGNAL( newEventSignal( QDate ) ) );
    }
    dayLayout->setRowStretch( row + 1, 1 );
  }

  mEventContextMenu = eventPopup();

  updateConfig();

  emit incidenceSelected( 0 );
}

KOMonthView::~KOMonthView()
{
  delete mEventContextMenu;
}

int KOMonthView::maxDatesHint()
{
  return mNumCells;
}

int KOMonthView::currentDateCount()
{
  return mNumCells;
}

Incidence::List KOMonthView::selectedIncidences()
{
  Incidence::List selected;

  if ( mSelectedCell ) {
    Incidence *incidence = mSelectedCell->selectedIncidence();
    if ( incidence ) selected.append( incidence );
  }

  return selected;
}

DateList KOMonthView::selectedDates()
{
  DateList selected;

  if ( mSelectedCell ) {
    QDate qd = mSelectedCell->selectedIncidenceDate();
    if ( qd.isValid() ) selected.append( qd );
  }

  return selected;
}

bool KOMonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( mSelectedCell ) {
    startDt.setDate( mSelectedCell->date() );
    endDt.setDate( mSelectedCell->date() );
    allDay = true;
    return true;
  }
  return false;
}

void KOMonthView::updateConfig()
{
  mWeekStartDay = KGlobal::locale()->weekStartDay();

  QFontMetrics fontmetric( mDayLabels[0]->font() );
  mWidthLongDayLabel = 0;

  for (int i = 0; i < 7; i++) {
    int width =
        fontmetric.width( KOGlobals::self()->calendarSystem()->weekDayName( i + 1 ) );
    if ( width > mWidthLongDayLabel ) mWidthLongDayLabel = width;
  }

  updateDayLabels();

  for ( uint i = 0; i < mCells.count(); ++i ) {
    mCells[i]->updateConfig();
  }
}

void KOMonthView::updateDayLabels()
{
  kdDebug(5850) << "KOMonthView::updateDayLabels()" << endl;

  const KCalendarSystem*calsys=KOGlobals::self()->calendarSystem();
  int currDay;
  for ( int i = 0; i < 7; i++ ) {
    currDay = i+mWeekStartDay;
    if ( currDay > 7 ) currDay -= 7;
    mDayLabels[i]->setText( calsys->weekDayName( currDay, mShortDayLabels ) );
  }
}

void KOMonthView::showDates( const QDate &start, const QDate & )
{
//  kdDebug(5850) << "KOMonthView::showDates(): " << start.toString() << endl;

  mStartDate = start;

  // correct begin of week
  int weekdayCol=( mStartDate.dayOfWeek() + 7 - mWeekStartDay ) % 7;
  mStartDate = mStartDate.addDays( -weekdayCol );

  bool primary = false;
  uint i;
  for( i = 0; i < mCells.size(); ++i ) {
    QDate date = mStartDate.addDays( i );
    if ( KOGlobals::self()->calendarSystem()->day( date ) == 1 ) {
      primary = !primary;
    }
    mCells[i]->setDate( date );

    mCells[i]->setPrimary( primary );

    if ( KOGlobals::self()->calendarSystem()->dayOfWeek( date ) ==
         KOGlobals::self()->calendarSystem()->weekDayOfPray() ) {
      mCells[i]->setHoliday( true );
    } else {
      mCells[i]->setHoliday( false );
    }

#ifndef KORG_NOPLUGINS
    // add holiday, if present
    QString hstring( KOCore::self()->holiday( date ) );
    mCells[i]->setHoliday( hstring );
#endif
  }

  updateView();
}

void KOMonthView::showIncidences( const Incidence::List & )
{
  kdDebug(5850) << "KOMonthView::showIncidences( const Incidence::List & ) is not implemented yet." << endl;
}

class KOMonthView::GetDateVisitor : public IncidenceBase::Visitor
{
  public:
    GetDateVisitor() {}

    bool act( IncidenceBase *incidence )
    {
      return incidence->accept( *this );
    }
    QDate date() const { return mDate; }

  protected:
    bool visit( Event *event ) {
      mDate = event->dtStart().date();
      return true;
    }
    bool visit( Todo *todo ) {
      if ( todo->hasDueDate() ) {
        mDate = todo->dtDue().date();
        return true;
      } else 
        return false;
    }
    bool visit( Journal *journal ) {
      mDate = journal->dtStart().date();
      return true;
    }
  protected:
    QDate mDate;
};

void KOMonthView::changeIncidenceDisplayAdded( Incidence *incidence )
{
  MonthViewCell *mvc;
  Event *event = 0;
  Todo *todo = 0;
  QDate date;
  
  // FIXME: use a visitor here
  if ( incidence->type() == "Event" ) {
    event = static_cast<Event *>( incidence );
    date = event->dtStart().date();
  }
  if ( incidence->type() == "Todo" ) {
    todo = static_cast<Todo *>( incidence );
    if ( !todo->hasDueDate() ) return;
    date = todo->dtDue().date();
  }

  if ( incidence->doesRecur() ) {
     for ( uint i = 0; i < mCells.count(); i++ ) {
       if ( incidence->recursOn( mCells[i]->date() ) ) {
         mCells[i]->addIncidence( incidence );
       }
     }
  } else if ( event ) {
      for ( QDateTime _date = date;
            _date <= event->dtEnd(); _date = _date.addDays( 1 ) ) {
        mvc = lookupCellByDate( _date.date() );
        if ( mvc ) mvc->addIncidence( event );
      }
    } else if ( todo ) {
        mvc = lookupCellByDate( date );
        if ( mvc ) mvc->addIncidence( todo );
      }
}

void KOMonthView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  switch ( action ) {
    case KOGlobals::INCIDENCEADDED:
      changeIncidenceDisplayAdded( incidence );
      break;
    case KOGlobals::INCIDENCEEDITED:
      for( uint i = 0; i < mCells.count(); i++ )
        mCells[i]->removeIncidence( incidence );
      changeIncidenceDisplayAdded( incidence );
      break;
    case KOGlobals::INCIDENCEDELETED:
      for( uint i = 0; i < mCells.count(); i++ )
        mCells[i]->removeIncidence( incidence );
      break;
    default:
      return;
  }
}

void KOMonthView::updateView()
{
  for( uint i = 0; i < mCells.count(); ++i ) {
    mCells[i]->updateCell();
  }

  Incidence::List incidences = calendar()->incidences();
  Incidence::List::ConstIterator it;

  for ( it = incidences.begin(); it != incidences.end(); ++it )
    changeIncidenceDisplayAdded( *it );

  processSelectionChange();
}

void KOMonthView::resizeEvent( QResizeEvent * )
{
  // select the appropriate heading string size. E.g. "Wednesday" or "Wed".
  // note this only changes the text if the requested size crosses the
  // threshold between big enough to support the full name and not big
  // enough.
  if( mDayLabels[0]->width() < mWidthLongDayLabel ) {
    if ( !mShortDayLabels ) {
      mShortDayLabels = true;
      updateDayLabels();
    }
  } else {
    if ( mShortDayLabels ) {
      mShortDayLabels = false;
      updateDayLabels();
    }
  }
}

void KOMonthView::showEventContextMenu( Incidence *incidence, QDate qd )
{
  mEventContextMenu->showIncidencePopup( incidence, qd );
}

void KOMonthView::showGeneralContextMenu()
{
  showNewEventPopup();
}

void KOMonthView::setSelectedCell( MonthViewCell *cell )
{
  if ( cell == mSelectedCell ) return;

  if ( mSelectedCell ) mSelectedCell->deselect();

  mSelectedCell = cell;

  if ( !mSelectedCell )
    emit incidenceSelected( 0 );
  else
    emit incidenceSelected( mSelectedCell->selectedIncidence() );
}

void KOMonthView::processSelectionChange()
{
  Incidence::List incidences = selectedIncidences();
  if (incidences.count() > 0) {
    emit incidenceSelected( incidences.first() );
  } else {
    emit incidenceSelected( 0 );
  }
}

void KOMonthView::clearSelection()
{
  if ( mSelectedCell ) {
    mSelectedCell->deselect();
    mSelectedCell = 0;
  }
}

MonthViewCell *KOMonthView::lookupCellByDate ( const QDate &date )
{
  for( uint i = 0; i < mCells.count(); i++ ) {
    if ( mCells[i]->date() == date )
      return mCells[i];
  }
  return 0;
}
