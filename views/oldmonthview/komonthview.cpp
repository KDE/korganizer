/*
  This file is part of KOrganizer.
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2008      Thomas Thrainer <tom_t@gmx.at>

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
#include "komonthview.h"

#include "koprefs.h"
#include "koglobals.h"
#include "koeventpopupmenu.h"
#include "kohelper.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#include "kodecorationlabel.h"
#endif

#include <kcal/calfilter.h>
#include <kcal/calendar.h>
#include <kcal/incidenceformatter.h>
#include <kcal/calendarresources.h>

#include <kcalendarsystem.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kwordwrap.h>

#include <QFont>
#include <QFontMetrics>
#include <QPushButton>
#include <QPainter>
#include <QCursor>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QKeyEvent>
#include <QFrame>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

#include "komonthview.moc"

using namespace KOrg;

//--------------------------------------------------------------------------
//-------- CaptureClickListBox ---------------------------------------------
//--------------------------------------------------------------------------

CaptureClickListBox::CaptureClickListBox( QWidget *parent )
  : QListWidget( parent )
{}

void CaptureClickListBox::mousePressEvent( QMouseEvent *event )
{
  emit listBoxPressed();
  QListWidget::mousePressEvent( event );
}

//--------------------------------------------------------------------------
//-------- MonthViewItem --------------------------------------------------
//--------------------------------------------------------------------------

MonthViewItem::MonthViewItem( Incidence *incidence,
                              const KDateTime &dt, const QString & s )
  : QListWidgetItem( 0, QListWidgetItem::UserType )
{
  setText( s );

  mIncidence = incidence;
  mDateTime = dt;

  mEventPixmap     = KOGlobals::self()->smallIcon( "view-calendar-day" );
  mTodoPixmap      = KOGlobals::self()->smallIcon( "view-calendar-tasks" );
  mTodoDonePixmap  = KOGlobals::self()->smallIcon( "task-complete" );
  mJournalPixmap   = KOGlobals::self()->smallIcon( "view-pim-journal" );
  mAlarmPixmap     = KOGlobals::self()->smallIcon( "appointment-reminder" );
  mRecurPixmap     = KOGlobals::self()->smallIcon( "appointment-recurring" );
  mReplyPixmap     = KOGlobals::self()->smallIcon( "mail-reply-sender" );
  mHolidayPixmap   = KOGlobals::self()->smallIcon( "emblem-favorite" );

  mResourceColor = QColor();
  mEvent = false;
  mTodo = false;
  mTodoDone = false;
  mJournal = false;
  mRecur = false;
  mAlarm = false;
  mReply = false;
  mHoliday = false;

  QString tipText;
  if ( incidence ) {
    tipText = IncidenceFormatter::toolTipString( incidence );
  } else {
    tipText = s;
  }
  if ( !tipText.isEmpty() ) {
    setToolTip( tipText );
  }
}

void MonthViewItem::drawIt()
{
  if ( KOPrefs::instance()->enableMonthItemIcons() ) {
    // Icon
    if ( mEvent ) {
      setIcon( mEventPixmap );
    }
    if ( mTodo ) {
      setIcon( mTodoPixmap );
    }
    if ( mTodoDone ) {
      setIcon( mTodoDonePixmap );
    }
    if ( mRecur ) {
      setIcon( mRecurPixmap );
    }
    if ( mAlarm ) {
      setIcon( mAlarmPixmap );
    }
    if ( mReply ) {
      setIcon( mReplyPixmap );
    }
    if ( mHoliday ) {
      setIcon( mHolidayPixmap );
    }
  }

  // Background color
  QColor bgColor;

  if ( KOPrefs::instance()->monthViewUsesResourceColor() ) {
    bgColor = mResourceColor;
  }
  if ( !bgColor.isValid() ) {
    // the palette is set to whatever color should be used
    // (category or default color)
    bgColor = mPalette.color( QPalette::Normal,
                              isSelected() ? QPalette::Highlight : QPalette::Background );
  }

  setBackground( QBrush( bgColor ) );
  setForeground( QBrush( getTextColor( bgColor ) ) );
}

int MonthViewItem::height( const QListWidget *lw ) const
{
  return qMax( qMax( mRecurPixmap.height(), mReplyPixmap.height() ),
               qMax( mAlarmPixmap.height(), lw->fontMetrics().lineSpacing() + 1 ) );
}

int MonthViewItem::width( const QListWidget *lw ) const
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

  return x + lw->fontMetrics().boundingRect( text() ).width() + 1;
}

//--------------------------------------------------------------------------
//-------- MonthViewCell --------------------------------------------------
//--------------------------------------------------------------------------

MonthViewCell::MonthViewCell( KOMonthView *parent )
  : QWidget( parent ),
    mMonthView( parent ),
    mEvenMonth( false ), mSelected( false )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 );

/* TODO: Add code for the loading of the cell decorations around here? */
  mLabel = new QLabel( this );
  mLabel->setFrameStyle( QFrame::Box | QFrame::Raised );
  mLabel->setLineWidth( 1 );
  mLabel->setAlignment( Qt::AlignCenter );

  mItemList = new CaptureClickListBox( this );
  mItemList->setMinimumSize( 10, 10 );
  mItemList->setFrameStyle( QFrame::Panel | QFrame::Plain );
  mItemList->setLineWidth( 1 );
  mItemList->setContextMenuPolicy( Qt::CustomContextMenu );
  topLayout->addWidget( mItemList, 0, 0 );

  mLabel->raise();

  enableScrollBars( false );

  updateConfig();

  connect( mItemList, SIGNAL(itemDoubleClicked( QListWidgetItem *)),
           SLOT(defaultAction(QListWidgetItem *)) );
  connect( mItemList, SIGNAL(customContextMenuRequested(const QPoint &)),
           SLOT(contextMenu(const QPoint &)) );
  connect( mItemList, SIGNAL(listBoxPressed()), SLOT(select()) );
}

void MonthViewCell::setDate( const QDate &date )
{
  mDate = date;

  if ( mSelected ) {
    setSelected( false );
  }

  QString text;
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  mEvenMonth = ( calSys->month( date ) % 2 == 0 );
  mToday = ( date == QDate::currentDate() );
  mWorkday = KOGlobals::self()->isWorkDay( date );

  if ( calSys->day( date ) == 1 ) {
    mFirstDay = true;

    text = i18nc( "'Month day' for month view cells", "%1 %2",
                  calSys->monthName( date, KCalendarSystem::ShortName ),
                  calSys->day( mDate ) );
    QFontMetrics fm( mLabel->font() );
    mLabel->resize( mLabelSize + QSize( fm.width( text ), 0 ) );
  } else {
    mFirstDay = false;

    text = QString::number( calSys->day( mDate ) );
    mLabel->resize( mLabelSize );
  }
  mLabel->setText( text );
/* TODO: Add code for the loading of the decorations around here */

  resizeEvent( 0 );
}

void MonthViewCell::setHoliday( bool holiday )
{
  mHoliday = holiday;
}

void MonthViewCell::setHolidayString( const QString &holiday )
{
  mHolidayString = holiday;
}

void MonthViewCell::updateCell()
{
  mItemList->clear();

  if ( !mHolidayString.isEmpty() ) {
    MonthViewItem *item =
      new MonthViewItem( 0, KDateTime( mDate, KOPrefs::instance()->timeSpec() ), mHolidayString );
    item->setHoliday( true );
    item->setPalette(
      QPalette ( KOPrefs::instance()->monthHolidaysBackgroundColor(),
                 KOPrefs::instance()->monthHolidaysBackgroundColor() ) );
    item->drawIt();
    mItemList->addItem( item );
  }
}

class MonthViewCell::CreateItemVisitor
  : public IncidenceBase::Visitor
{
  public:
    CreateItemVisitor() : mItem(0) {}

    bool act( IncidenceBase *incidence, const QDate &date, const QPalette &stdPal, int multiDay )
    {
      mItem = 0;
      mDate = date;
      mStandardPalette = stdPal;
      mMultiDay = multiDay;
      return incidence->accept( *this );
    }
    MonthViewItem *item() const { return mItem; }

  protected:
    bool visit( Event *event ) {
      QString text;
      KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
      KDateTime dt( mDate, timeSpec );
      // take the time 0:00 into account, which is non-inclusive
      QDate dtEnd =
        event->dtEnd().toTimeSpec( timeSpec ).addSecs( event->allDay() ? 0 : -1 ).date();
      KDateTime dtStart = event->dtStart().toTimeSpec( timeSpec );
      int length = dtStart.date().daysTo( dtEnd );
      if ( event->isMultiDay() ) {
        if ( mDate == dtStart.date() ||
           ( mMultiDay == 0 && event->recursOn( mDate, timeSpec ) ) ) {
          text = "(-- " + event->summary();
          dt = event->dtStart();
        } else if ( !event->recurs() && mDate == dtEnd ||
                    // last day of a recurring multi-day event?
                    ( mMultiDay == length &&
                      event->recursOn( mDate.addDays( -length ), timeSpec ) ) ) {
          text = event->summary() + " --)";
        } else if ( !( event->dtStart().date().daysTo( mDate ) % 7 ) && length > 7 ) {
          text = "-- " + event->summary() + " --";
        } else {
          text = "----------------";
        }
      } else {
        if ( event->allDay() ) {
          text = event->summary();
        } else {
          QTime startTime = event->dtStart().toTimeSpec( timeSpec ).time();
          text = KGlobal::locale()->formatTime(startTime);
          dt.setTime( startTime );
          text += ' ' + event->summary();
        }
      }

      mItem = new MonthViewItem( event, dt, text );
      if ( KOPrefs::instance()->monthViewUsesCategoryColor() ) {
        QStringList categories = event->categories();
        QString cat;
        if ( !categories.isEmpty() ) {
          cat = categories.first();
        }
        if ( cat.isEmpty() ) {
          mItem->setPalette(
            QPalette( KOPrefs::instance()->monthCalendarItemsEventsBackgroundColor(),
                      KOPrefs::instance()->monthCalendarItemsEventsBackgroundColor() ) );
        } else {
          mItem->setPalette(
            QPalette( KOPrefs::instance()->categoryColor( cat ),
                      KOPrefs::instance()->categoryColor( cat ) ) );
        }
      } else {
        mItem->setPalette( mStandardPalette );
      }
      mItem->setEvent( true );

      Attendee *me = event->attendeeByMails( KOPrefs::instance()->allEmails() );
      if ( me != 0 ) {
        mItem->setReply( me->status() == Attendee::NeedsAction && me->RSVP() );
      } else {
        mItem->setReply( false );
      }
      return true;
    }
    bool visit( Todo *todo ) {
      QString text;
      if ( !KOPrefs::instance()->showAllDayTodo() ) {
        return false;
      }
      KDateTime dt( mDate, KOPrefs::instance()->timeSpec() );
      if ( todo->hasDueDate() && !todo->allDay() ) {
        QTime dueTime = todo->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() ).time();
        text += KGlobal::locale()->formatTime( dueTime );
        text += ' ';
        dt.setTime( dueTime );
      }
      text += todo->summary();

      mItem = new MonthViewItem( todo, dt, text );
      if ( todo->recurs() ) {
        mDate < todo->dtDue().date() ? mItem->setTodoDone( true ) : mItem->setTodo( true );
      } else {
        todo->isCompleted() ? mItem->setTodoDone( true ) : mItem->setTodo( true );
      }
      mItem->setPalette(
                QPalette(
        KOPrefs::instance()->monthCalendarItemsToDosBackgroundColor(),
        KOPrefs::instance()->monthCalendarItemsToDosBackgroundColor() ) );
      return true;
    }
    bool visit( Journal *journal ) {
      //TODO: maybe monthview should show journals?
      Q_UNUSED( journal );
      return true;
    }
    bool visit( FreeBusy *freebusy ) {
      //For completeness and to inhibit compile warnings
      Q_UNUSED( freebusy );
      return true;
    }

  protected:
    MonthViewItem *mItem;
    QDate mDate;
    QPalette mStandardPalette;
    int mMultiDay;
};

void MonthViewCell::addIncidence( Incidence *incidence, int multiDay )
{
  CreateItemVisitor v;

  if ( v.act( incidence, mDate, palette(), multiDay ) ) {
    MonthViewItem *item = v.item();
    if ( item ) {
      item->setAlarm( incidence->isAlarmEnabled() );
      item->setRecur( incidence->recurrenceType() );

      QColor resourceColor = KOHelper::resourceColor( mCalendar, incidence );
      item->setResourceColor( resourceColor ); //setting an invalid color is OK
      item->drawIt();

      int i = 0;
      int pos = -1;
      KDateTime dt( item->incidenceDateTime() );

      while ( i < mItemList->count() && pos < 0 ) {
        QListWidgetItem *item = mItemList->item( i );
        MonthViewItem *mvitem = dynamic_cast<MonthViewItem*>( item );
        if ( mvitem && mvitem->incidenceDateTime() > dt ) {
          pos = i;
        }
        ++i;
      }
      if ( pos >= 0 ) {
        // insert chronologically
        mItemList->insertItem( pos, item );
      } else {
        // append to end of list
        mItemList->addItem( item );
      }
    }
  }
}

void MonthViewCell::removeIncidence( Incidence *incidence )
{
  for ( int i = 0; i < mItemList->count(); ++i ) {
    MonthViewItem *item = static_cast<MonthViewItem *>( mItemList->item( i ) );
    if ( item && item->incidence() && item->incidence()->uid() == incidence->uid() ) {
      mItemList->removeItemWidget( mItemList->item( i ) );
      break;
    }
  }
}

void MonthViewCell::updateStyles()
{
  // force re-applying of the stylesheet
  QString tmp = styleSheet();
  setStyleSheet( QString() );
  setStyleSheet( tmp );
}

void MonthViewCell::updateConfig()
{
  QString styleSheet;

  QColor bg_even =
      KOPrefs::instance()->monthGridBackgroundColor();
  QColor bg_odd;
  QColor bg_even_workday =
      KOPrefs::instance()->monthGridWorkHoursBackgroundColor();
  QColor bg_odd_workday;
  if ( bg_even.value() < 128 ) {
    bg_odd          = bg_even.light( 120 );
    bg_odd_workday  = bg_even_workday.light( 120 );
  } else {
    bg_odd          = bg_even.dark( 120 );
    bg_odd_workday  = bg_even_workday.dark( 120 );
  }
  // general styles
  //-------------------------------

  // set the font
  QFont f = KOPrefs::instance()->mMonthViewFont;
  styleSheet +=
    "MonthViewCell, MonthViewCell > * {"
    "  font-family: \"%1\";"
    "  font-size: %2%3;"
    "  font-style: %4;"
    "  font-weight: %5;"
    "}";
  styleSheet =
    styleSheet.arg( f.family() ).
    arg( f.pixelSize() > 0 ?
         f.pixelSize() : f.pointSize() ).
    arg( f.pixelSize() > 0 ?
         "px" : "pt" ).
    arg( f.italic() ?
         "italic" : "normal" ).
    arg( f.bold() ?
         "bold" : "normal" );

  // change background color alternatively
  styleSheet +=
    "MonthViewCell[evenMonth=\"true\"],"
    "         MonthViewCell[evenMonth=\"true\"] > * {"
    "  background: %1;"
    "}"
    "MonthViewCell[evenMonth=\"false\"],"
    "         MonthViewCell[evenMonth=\"false\"] > * {"
    "  background: %2;"
    "}";
  styleSheet = styleSheet.arg( bg_even.name() ).arg( bg_odd.name() );

  // draw a border around the labels
  styleSheet +=
    "MonthViewCell > QLabel {"
    "  border: 1px solid;"
    "}";

  // draw a border around the list-widgets
  styleSheet +=
    "MonthViewCell > QListWidget {"
    "  border: 1px solid palette(dark);"
    "}";

  // styles for current day
  //-------------------------------
  QColor fg_today = KOPrefs::instance()->monthGridHighlightColor();
  styleSheet +=
    "MonthViewCell[today=\"true\"] QLabel {"
    "  color: %1;"
    "  border-color: %1;"
    "}"
    "MonthViewCell[today=\"true\"] QListWidget {"
    "  border: 2px solid %1;"
    "}";
  styleSheet = styleSheet.arg( fg_today.name() );

  // styles for workdays
  //-------------------------------
  styleSheet +=
    "MonthViewCell[workday=\"true\"][evenMonth=\"true\"] > * {"
    "  background: %1;"
    "}"
    "MonthViewCell[workday=\"true\"][evenMonth=\"false\"] > * {"
    "  background: %3;"
    "}";
  styleSheet = styleSheet.arg( bg_even_workday.name() ).arg( bg_odd_workday.name() );

  // styles for first day of month
  //-------------------------------
  styleSheet +=
    "MonthViewCell[firstDay=\"true\"][today=\"false\"] QLabel {"
    "  background: palette(highlight);"
    "  color: palette(highlightedtext);"
    "}";

  // styles for holidays
  //-------------------------------
  QColor fg_holiday = KOPrefs::instance()->monthHolidaysBackgroundColor();
  styleSheet +=
    "MonthViewCell[holiday=\"true\"] QLabel {"
    "  color: %1;"
    "  border-color: %1;"
    "}";
  styleSheet = styleSheet.arg( fg_holiday.name() );

  // styles for selected cells
  //-------------------------------
  styleSheet +=
    "MonthViewCell[selected=\"true\"] QListWidget {"
    "  border: 3px inset palette(dark);"
    "}";

  setStyleSheet( styleSheet );

  QFontMetrics fm( font() );
  mLabelSize = fm.size( 0, "30" ) +
               QSize( mLabel->frameWidth() * 2, mLabel->frameWidth() * 2 ) +
               QSize( 2, 2 );

  updateCell();
}

void MonthViewCell::enableScrollBars( bool enabled )
{
  if ( enabled ) {
    mItemList->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    mItemList->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  } else {
    mItemList->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    mItemList->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  }
  // the list has to be reset in order to display the full items
  // otherwise the item text remains cutted altough scrollbars would
  // support the display of the full entry.
  mItemList->reset();
}

Incidence *MonthViewCell::selectedIncidence()
{
  int index = mItemList->currentRow();
  if ( index < 0 ) {
    return 0;
  }

  MonthViewItem *item = static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) {
    return 0;
  }

  return item->incidence();
}

QDate MonthViewCell::selectedIncidenceDate()
{
  QDate qd;
  int index = mItemList->currentRow();
  if ( index < 0 ) {
    return qd;
  }

  MonthViewItem *item = static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) {
    return qd;
  }

  return item->incidenceDateTime().date();
}

void MonthViewCell::setSelected( bool selected )
{
  mSelected = selected;

  if ( selected ) {
    // setSelectedCell will deselect currently selected cells
    mMonthView->setSelectedCell( this );

    if( KOPrefs::instance()->enableMonthScroll() ) {
      enableScrollBars( true );
    }
  } else {
    mItemList->clearSelection();

    enableScrollBars( false );
  }

  updateStyles();
}

void MonthViewCell::select()
{
  setSelected( true );
}

void MonthViewCell::resizeEvent ( QResizeEvent * )
{
  mLabel->move( width() - mLabel->width(), height() - mLabel->height() );
/* TODO: Add code to move cell decorations around here */
}

void MonthViewCell::mousePressEvent( QMouseEvent * )
{
  select();
  /* TODO: add support for creating a range of selected dates */
}

void MonthViewCell::paintEvent( QPaintEvent * )
{
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive( QStyle::PE_Widget, &opt, &p, this );
}

void MonthViewCell::defaultAction( QListWidgetItem *item )
{
  select();

  if ( !item ) {
    emit newEventSignal( date() );
  } else {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) {
      mMonthView->defaultAction( incidence );
    }
  }
}

void MonthViewCell::contextMenu( const QPoint &pos )
{
  select();

  QListWidgetItem *item = mItemList->itemAt( pos );
  if ( item ) {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) {
      mMonthView->showEventContextMenu( incidence, date() );
    }
  } else {
    mMonthView->showGeneralContextMenu();
  }
}

//--------------------------------------------------------------------------
//-------- KOMonthView ----------------------------------------------------
//--------------------------------------------------------------------------

KOMonthView::KOMonthView( Calendar *calendar, QWidget *parent )
  : KOEventView( calendar, parent ),
    mDaysPerWeek( 7 ), mNumWeeks( 6 ), mNumCells( mDaysPerWeek * mNumWeeks ),
    mShortDayLabels( false ), mWidthLongDayLabel( 0 ), mSelectedDate()
{
  mCalendar = calendar;
  QHBoxLayout *mainLayout = new QHBoxLayout( this );

  QWidget *mainWidget = new QWidget( this );
  mDayLayout = new QGridLayout( mainWidget );
  mDayLayout->setSpacing( 0 );
  mDayLayout->setMargin( 0 );

  mainLayout->addWidget( mainWidget );

  QVBoxLayout *rightLayout = new QVBoxLayout( this );
  rightLayout->setSpacing( 0 );
  rightLayout->setMargin( 0 );

  // push buttons to the bottom
  rightLayout->addStretch( 1 );

  QToolButton *minusMonth = new QToolButton( this );
  minusMonth->setIcon( KIcon( "arrow-up-double" ) );
  minusMonth->setToolTip( i18n( "Go back one month" ) );
  minusMonth->setAutoRaise( true );
  connect( minusMonth, SIGNAL( clicked() ),
           this, SLOT( moveBackMonth() ) );

  QToolButton *minusWeek = new QToolButton( this );
  minusWeek->setIcon( KIcon( "arrow-up" ) );
  minusWeek->setToolTip( i18n( "Go back one week" ) );
  minusWeek->setAutoRaise( true );
  connect( minusWeek, SIGNAL( clicked() ),
           this, SLOT( moveBackWeek() ) );

  QToolButton *plusWeek = new QToolButton( this );
  plusWeek->setIcon( KIcon( "arrow-down" ) );
  plusWeek->setToolTip( i18n( "Go forward one week" ) );
  plusWeek->setAutoRaise( true );
  connect( plusWeek, SIGNAL( clicked() ),
           this, SLOT( moveFwdWeek() ) );

  QToolButton *plusMonth = new QToolButton( this );
  plusMonth->setIcon( KIcon( "arrow-down-double" ) );
  plusMonth->setToolTip( i18n( "Go forward one month" ) );
  plusMonth->setAutoRaise( true );
  connect( plusMonth, SIGNAL( clicked() ),
           this, SLOT( moveFwdMonth() ) );

  rightLayout->addWidget( minusMonth );
  rightLayout->addWidget( minusWeek );
  rightLayout->addWidget( plusWeek );
  rightLayout->addWidget( plusMonth );

  mainLayout->addLayout( rightLayout );

  QFont bfont = font();
  bfont.setBold( true );

  QFont mfont = bfont;
  mfont.setPointSize( 20 );

  // Top box with month name and decorations
  mTopBox = new KHBox( this );
  mTitle = new QLabel( mTopBox );
  mTitle->setFont( mfont );
  mTitle->setLineWidth( 0 );
  mTitle->setFrameStyle( QFrame::Plain );
  mDecorationsFrame = 0;

  mDayLayout->addWidget( mTopBox, 0, 0, 1, -1, Qt::AlignCenter );

  // Create the day of the week labels (Sun, Mon, etc)
  mDayLabels.resize( mDaysPerWeek );
  int i;
  for ( i = 0; i < mDaysPerWeek; i++ ) {
    QLabel *label = new QLabel( this );
    label->setFont( bfont );
    label->setFrameStyle( QFrame::Panel | QFrame::Raised );
    label->setLineWidth( 1 );
    label->setAlignment( Qt::AlignCenter );

    mDayLabels[i] = label;

    mDayLayout->addWidget( label, 1, i );
    mDayLayout->addItem( new QSpacerItem( 10, 0 ), 0, i );
    mDayLayout->setColumnStretch( i, 1 );
  }

  mCells.resize( mNumCells );
  int row, col;
  for ( row = 0; row < mNumWeeks; ++row ) {
    for ( col = 0; col < mDaysPerWeek; ++col ) {
      MonthViewCell *cell = new MonthViewCell( this );
      cell->setCalendar( calendar );
      mCells[row * mDaysPerWeek + col] = cell;
      mDayLayout->addWidget( cell, row + 2, col );

      connect( cell, SIGNAL( defaultAction( Incidence * ) ),
               SLOT( defaultAction( Incidence * ) ) );
      connect( cell, SIGNAL( newEventSignal( const QDate & ) ),
               SIGNAL( newEventSignal( const QDate & ) ) );
    }
    mDayLayout->setRowStretch( row + 2, 1 );
  }

  mEventContextMenu = eventPopup();

  updateConfig();

  emit incidenceSelected( 0 );
}

KOMonthView::~KOMonthView()
{
  qDeleteAll( mCells );
  mCells.clear();
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

  MonthViewCell *selectedCell = getCell( mSelectedDate );

  if ( selectedCell ) {
    Incidence *incidence = selectedCell->selectedIncidence();
    if ( incidence ) {
      selected.append( incidence );
    }
  }

  return selected;
}

DateList KOMonthView::selectedDates()
{
  DateList selected;

  MonthViewCell *selectedCell = getCell( mSelectedDate );

  if ( selectedCell ) {
    QDate qd = selectedCell->selectedIncidenceDate();
    if ( qd.isValid() ) {
      selected.append( qd );
    }
  }

  return selected;
}

bool KOMonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( !mSelectedDate.isNull() ) {
    startDt.setDate( mSelectedDate );
    endDt.setDate( mSelectedDate );
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

  for ( int i = 0; i < 7; ++i ) {
    int width = fontmetric.width( KOGlobals::self()->calendarSystem()->weekDayName( i + 1 ) );
    if ( width > mWidthLongDayLabel ) {
      mWidthLongDayLabel = width;
    }
  }

  updateDayLabels();

  for ( int i = 0; i < mCells.count(); ++i ) {
    mCells[i]->updateConfig();
  }
}

void KOMonthView::updateDayLabels()
{
  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int currDay;
  for ( int i = 0; i < 7; i++ ) {
    currDay = i + mWeekStartDay;
    if ( currDay > 7 ) {
      currDay -= 7;
    }
    if ( mShortDayLabels ) {
      mDayLabels[i]->setText( calsys->weekDayName( currDay, KCalendarSystem::ShortDayName ) );
    } else {
      mDayLabels[i]->setText( calsys->weekDayName( currDay, KCalendarSystem::LongDayName ) );
    }
  }
}

void KOMonthView::showDates( const QDate &start, const QDate &end )
{
  Q_UNUSED( end );
  // show first day of month on top for readability issues
  QDate tmp = start.addDays( -start.day() + 1 );
  setStartDate( tmp );
}

void KOMonthView::wheelEvent( QWheelEvent *event )
{
  // invert direction to get scroll-like behaviour
  if ( event->delta() > 0 ) {
    moveStartDate( -1, 0 );
  } else if ( event->delta() < 0 ) {
    moveStartDate( 1, 0 );
  }

  // call accept in every case, we do not want anybody else to react
  event->accept();
}

void KOMonthView::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_PageUp ) {
    moveStartDate( 0, -1 );
    event->accept();
  } else if ( event->key() == Qt::Key_PageDown ) {
    moveStartDate( 0, 1 );
    event->accept();
  } else {
    event->ignore();
  }
}

void KOMonthView::moveBackMonth()
{
  moveStartDate( 0, -1 );
}

void KOMonthView::moveBackWeek()
{
  moveStartDate( -1, 0 );
}

void KOMonthView::moveFwdWeek()
{
  moveStartDate( 1, 0 );
}

void KOMonthView::moveFwdMonth()
{
  moveStartDate( 0, 1 );
}

void KOMonthView::swapCells( int srcRow, int srcCol, int dstRow, int dstCol )
{
  int src = srcRow * mDaysPerWeek + srcCol;
  int dst = dstRow * mDaysPerWeek + dstCol;

  MonthViewCell *srcCell = mCells[src];
  MonthViewCell *dstCell = mCells[dst];

  mDayLayout->removeWidget( srcCell );
  mDayLayout->removeWidget( dstCell );

  mDayLayout->addWidget( dstCell, srcRow + 2, srcCol );
  mDayLayout->addWidget( srcCell, dstRow + 2, dstCol );

  mCells[dst] = srcCell;
  mCells[src] = dstCell;
}

void KOMonthView::moveStartDate( int weeks, int months )
{
  setUpdatesEnabled( false );

  if ( weeks != 0 ) {
    mStartDate = mStartDate.addDays( weeks * mDaysPerWeek );
  }
  if ( months != 0 ) {
    mStartDate = mStartDate.addMonths( months );
  }

  // check if we have to updated all cells
  if ( abs( weeks ) >= mNumWeeks || abs( months ) > 0 ) {
    setStartDate( mStartDate );

  } else if ( weeks > 0 ) {
    for ( int row = weeks; row < mNumWeeks; ++row ) {
      for ( int col = 0; col < mDaysPerWeek; ++col ) {
        swapCells( row, col, row - weeks, col );
      }
    }

    updateCells( mCells.count() - weeks * mDaysPerWeek, mCells.count() - 1 );

  } else if ( weeks < 0 ) {
    weeks = -weeks;

    for ( int row = mNumWeeks-weeks-1; row >= 0; --row ) {
      for ( int col = 0; col < mDaysPerWeek; ++col ) {
        swapCells( row, col, row + weeks, col );
      }
    }

    updateCells( 0, weeks * mDaysPerWeek - 1 );
  }

  setUpdatesEnabled( true );
}

void KOMonthView::setStartDate( const QDate &start )
{
  mStartDate = start;
  // correct begin of week
  int weekdayCol=( mStartDate.dayOfWeek() + 7 - mWeekStartDay ) % 7;
  mStartDate = mStartDate.addDays( -weekdayCol );

  updateCells( 0, mCells.count() - 1 );
}

void KOMonthView::updateCells( int start, int end )
{
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  // the current month is the month of the day which is in the
  // centre of the view
  QDate avgDate = mStartDate.addDays( ( mDaysPerWeek * mNumWeeks ) / 2 - 1 );

  mTitle->setText( i18nc( "monthname year", "%1 %2",
                   calSys->monthName( avgDate ),
                   calSys->yearString( avgDate ) ) );

  delete mDecorationsFrame;
  mDecorationsFrame = new QFrame( mTopBox );
  mDecorationsFrame->setLineWidth( 0 );
  mDecorationsFrame->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
#ifndef KORG_NOPLUGINS
  // Month decoration labels
  foreach ( QString decoName, KOPrefs::instance()->decorationsAtMonthViewTop() ) {
    if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
      CalendarDecoration::Decoration *deco =
          KOCore::self()->loadCalendarDecoration( decoName );

      CalendarDecoration::Element::List elements;
      elements = deco->monthElements( mStartDate );
      if ( elements.count() > 0 ) {
        KHBox *decoHBox = new KHBox( mDecorationsFrame );
        decoHBox->setFrameShape( QFrame::StyledPanel );
        decoHBox->setMinimumWidth( 1 );

        foreach ( CalendarDecoration::Element *it, elements ) {
          kDebug() << "adding Element " << it->id()
                   << " of Decoration " << deco->info()
                   << " to the top of the month view";
          KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
          label->setAlignment( Qt::AlignBottom );
        }
      }
    }
  }
#endif

  for ( int i = start; i <= end; ++i ) {
    QDate date = mStartDate.addDays( i );

    mCells[i]->setDate( date );
    if ( date == mSelectedDate ) {
      mCells[i]->select();
    }

    /* TODO: is it really a holiday just because it's no work day?
       should we maybe display holidays in a different way than non-working-days?
    */
    bool isHoliday = ( calSys->dayOfWeek( date ) == calSys->weekDayOfPray() ||
                       !KOGlobals::self()->isWorkDay( date ) );
    mCells[i]->setHoliday( isHoliday );

    // add holiday, if present
    QStringList holidays( KOGlobals::self()->holiday( date ) );
    mCells[i]->setHolidayString( holidays.join(
                                   i18nc( "delimiter for joining holiday names", "," ) ) );

    mCells[i]->updateStyles();
  }

  updateView( start, end );
}

void KOMonthView::showIncidences( const Incidence::List & )
{
  kDebug() << "not implemented yet.";
}

class KOMonthView::GetDateVisitor : public IncidenceBase::Visitor
{
  public:
    GetDateVisitor() {}

    bool act( IncidenceBase *incidence )
    {
      return incidence->accept( *this );
    }
    KDateTime startDate() const { return mStartDate; }
    KDateTime endDate() const { return mEndDate; }

  protected:
    bool visit( Event *event ) {
      mStartDate = event->dtStart();
      mEndDate = event->dtEnd();
      return true;
    }
    bool visit( Todo *todo ) {
      if ( todo->hasDueDate() ) {
        mStartDate = todo->dtDue();
        mEndDate = todo->dtDue();
      } else {
        return false;
      }
      return true;
    }
    bool visit( Journal *journal ) {
      mStartDate = journal->dtStart();
      mEndDate = journal->dtStart();
      return true;
    }
    bool visit( FreeBusy *freebusy ) {
      //For completeness and to inhibit compile warnings
      Q_UNUSED( freebusy );
      return true;
    }

  protected:
    KDateTime mStartDate;
    KDateTime mEndDate;
};

void KOMonthView::changeIncidenceDisplayAdded( Incidence *incidence,
                                               int start, int end )
{
  if ( start == -1 || end == -1 ) {
    start = 0;
    end = mCells.count() - 1;
  }

  GetDateVisitor gdv;

  if ( !gdv.act( incidence ) ) {
    return;
  }

  bool allDay = incidence->allDay();

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  if ( incidence->recurs() ) {
    for ( int i = start; i <= end; ++i ) {
      if ( incidence->recursOn( mCells[i]->date(), timeSpec ) ) {
        // handle multiday events
        int length =
          gdv.startDate().date().daysTo( gdv.endDate().addSecs( allDay ? 0 : -1 ).date() );
        for ( int j = 0; j <= length && i+j <= end; ++j ) {
          mCells[i + j]->addIncidence( incidence, j );
        }
      }
    }
  } else {
    QDate startDate;
    QDate endDate;

    if ( gdv.endDate().isDateOnly() ) {
      // the incidence is probably a todo with no time associated
      // it makes no sense to adapt the time in this case
      startDate = gdv.startDate().date();
      endDate = gdv.endDate().date();
    } else {
      startDate = gdv.startDate().toTimeSpec( timeSpec ).date();
      // addSecs(-1) is added to handle 0:00 cases for events
      // (because it's non-inclusive according to rfc)
      endDate = gdv.endDate().toTimeSpec( timeSpec ).addSecs(
                                              allDay ? 0 : -1 ).date();
    }

    // ensure that the dates to iterate over are within the area to update
    if ( startDate < mCells[start]->date() ) {
      startDate = mCells[start]->date();
    }
    if ( endDate > mCells[end]->date() ) {
      endDate = mCells[end]->date();
    }

    for ( QDate date = startDate; date <= endDate; date = date.addDays( 1 ) ) {
      MonthViewCell *mvc = getCell( date );
      // mvc is always not null, we cannot iterate over invalid dates
      mvc->addIncidence( incidence );
    }
  }
}

void KOMonthView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  switch ( action ) {
  case KOGlobals::INCIDENCEADDED:
    changeIncidenceDisplayAdded( incidence );
    break;
  case KOGlobals::INCIDENCEEDITED:
    for ( int i = 0; i < mCells.count(); i++ ) {
      mCells[i]->removeIncidence( incidence );
    }
    changeIncidenceDisplayAdded( incidence );
    break;
  case KOGlobals::INCIDENCEDELETED:
    for ( int i = 0; i < mCells.count(); i++ ) {
      mCells[i]->removeIncidence( incidence );
    }
    break;
  default:
    return;
  }
  updateView();
}

void KOMonthView::updateView()
{
  updateView( 0, mCells.count() - 1 );
}

void KOMonthView::updateView( int start, int end )
{
  for ( int i = start; i <= end; ++i ) {
    mCells[i]->updateCell();
  }

  Incidence::List incidences = calendar()->incidences();
  Incidence::List::ConstIterator it;

  for ( it = incidences.begin(); it != incidences.end(); ++it ) {
    changeIncidenceDisplayAdded( *it, start, end );
  }

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

void KOMonthView::showEventContextMenu( Incidence *incidence, const QDate &qd )
{
  mEventContextMenu->showIncidencePopup( mCalendar, incidence, qd );
}

void KOMonthView::showGeneralContextMenu()
{
  showNewEventPopup();
}

void KOMonthView::setSelectedCell( MonthViewCell *cell )
{
  MonthViewCell *selectedCell = getCell( mSelectedDate );

  if ( selectedCell && selectedCell != cell ) {
    selectedCell->setSelected( false );
  }

  mSelectedDate = cell->date();

  if ( !cell ) {
    emit incidenceSelected( 0 );
  } else {
    emit incidenceSelected( cell->selectedIncidence() );
  }
}

void KOMonthView::clearSelection()
{
  MonthViewCell *selectedCell = getCell( mSelectedDate );
  mSelectedDate = QDate();

  if ( selectedCell ) {
    selectedCell->setSelected( false );
  }
}

void KOMonthView::processSelectionChange()
{
  Incidence::List incidences = selectedIncidences();
  if ( incidences.count() > 0 ) {
    emit incidenceSelected( incidences.first() );
  } else {
    emit incidenceSelected( 0 );
  }
}

MonthViewCell *KOMonthView::getCell( const QDate &date )
{
  int offset = mStartDate.daysTo( date );

  if ( offset < 0 || offset >= mCells.size() ) {
    return 0;
  }

  return mCells[offset];
}
