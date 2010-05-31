/*
    This file is part of KOrganizer.

    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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

#include <qlistview.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qstyle.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>

#include <libkcal/calendar.h>
#include <libkcal/incidenceformatter.h>

#include "koglobals.h"
#include "koprefs.h"
#include "koincidencetooltip.h"
#include "koeventpopupmenu.h"

#include "kolistview.h"
#include "kolistview.moc"

enum {
  Summary_Column = 0,
  Reminder_Column,
  Recurs_Column,
  StartDateTime_Column,
  EndDateTime_Column,
  Categories_Column
};


KOListViewToolTip::KOListViewToolTip( QWidget* parent,
                                      Calendar *calendar,
                                      KListView *lv )
  :QToolTip( parent ), mCalendar( calendar )
{
  eventlist = lv;
}

void KOListViewToolTip::maybeTip( const QPoint &pos )
{
  QRect r;
  QListViewItem *it = eventlist->itemAt( pos );
  KOListViewItem *i = static_cast<KOListViewItem*>( it );

  if ( i && KOPrefs::instance()->mEnableToolTips ) {
    /* Calculate the rectangle. */
    r = eventlist->itemRect( it );
    /* Show the tip */
    QString tipText( IncidenceFormatter::toolTipStr( mCalendar, i->data() ) );
    if ( !tipText.isEmpty() ) {
      tip( r, tipText );
    }
  }
}

/**
  This class provides the initialization of a KOListViewItem for calendar
  components using the Incidence::Visitor.
*/
class KOListView::ListItemVisitor : public IncidenceBase::Visitor
{
  public:
    ListItemVisitor( KOListViewItem *item ) : mItem( item ) {}
    ~ListItemVisitor() {}

    bool visit( Event * );
    bool visit( Todo * );
    bool visit( Journal * );

  private:
    KOListViewItem *mItem;
};

bool KOListView::ListItemVisitor::visit( Event *e )
{
  mItem->setText( Summary_Column, e->summary() );
  if ( e->isAlarmEnabled() ) {
    static const QPixmap alarmPxmp = KOGlobals::self()->smallIcon( "bell" );
    mItem->setPixmap( Reminder_Column, alarmPxmp );
    mItem->setSortKey( Reminder_Column, "1" );
  } else {
    mItem->setSortKey( Reminder_Column, "0" );
  }

  if ( e->doesRecur() ) {
    static const QPixmap recurPxmp = KOGlobals::self()->smallIcon( "recur" );
    mItem->setPixmap( Recurs_Column, recurPxmp );
    mItem->setSortKey( Recurs_Column, "1" );
  } else {
    mItem->setSortKey( Recurs_Column, "0" );
  }

  QPixmap eventPxmp;
  if ( e->customProperty( "KABC", "BIRTHDAY" ) == "YES" ) {
    if ( e->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
      eventPxmp = KOGlobals::self()->smallIcon( "calendaranniversary" );
    } else {
      eventPxmp = KOGlobals::self()->smallIcon( "calendarbirthday" );
    }
  } else {
    eventPxmp = KOGlobals::self()->smallIcon( "appointment" );
  }

  mItem->setPixmap( Summary_Column, eventPxmp );

  QString startDateTime;
  QString endDateTime;

  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString( e->dtStart(), e->doesFloat() ) );
  mItem->setSortKey( StartDateTime_Column, e->dtStart().toString( Qt::ISODate ) );
  mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString( e->dtEnd(), e->doesFloat() ) );
  mItem->setSortKey( EndDateTime_Column, e->dtEnd().toString( Qt::ISODate ) );
  mItem->setText( Categories_Column, e->categoriesStr() );

  return true;
}

bool KOListView::ListItemVisitor::visit( Todo *t )
{
  static const QPixmap todoPxmp = KOGlobals::self()->smallIcon( "todo" );
  static const QPixmap todoDonePxmp = KOGlobals::self()->smallIcon( "checkedbox" );
  mItem->setPixmap(Summary_Column, t->isCompleted() ? todoDonePxmp : todoPxmp );
  mItem->setText(Summary_Column, t->summary());
  if ( t->isAlarmEnabled() ) {
    static const QPixmap alarmPxmp = KOGlobals::self()->smallIcon( "bell" );
    mItem->setPixmap( Reminder_Column, alarmPxmp );
    mItem->setSortKey( Reminder_Column, "1" );
  } else {
    mItem->setSortKey( Reminder_Column, "0" );
  }

  if ( t->doesRecur() ) {
    static const QPixmap recurPxmp = KOGlobals::self()->smallIcon( "recur" );
    mItem->setPixmap( Recurs_Column, recurPxmp );
    mItem->setSortKey( Recurs_Column, "1" );
  } else {
    mItem->setSortKey( Recurs_Column, "0" );
  }

  if ( t->hasStartDate() ) {
    mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString( t->dtStart(), t->doesFloat() ) );
    mItem->setSortKey( StartDateTime_Column, t->dtStart().toString( Qt::ISODate ) );
  } else {
    mItem->setText( StartDateTime_Column, "---" );
  }

  if ( t->hasDueDate() ) {
    mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString( t->dtDue(), t->doesFloat() ) );
    mItem->setSortKey( EndDateTime_Column, t->dtDue().toString( Qt::ISODate ) );
  } else {
    mItem->setText( EndDateTime_Column, "---" );
  }
  mItem->setText( Categories_Column, t->categoriesStr() );

  return true;
}

bool KOListView::ListItemVisitor::visit( Journal *j )
{
  static const QPixmap jornalPxmp = KOGlobals::self()->smallIcon( "journal" );
  mItem->setPixmap( Summary_Column, jornalPxmp );
  // Just use the first line
  mItem->setText( Summary_Column, j->description().section( "\n", 0, 0 ) );
  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString( j->dtStart(), j->doesFloat() ) );
  mItem->setSortKey( StartDateTime_Column, j->dtStart().toString( Qt::ISODate ) );

  return true;
}

KOListView::KOListView( Calendar *calendar,
                        QWidget *parent,
                        const char *name,
                        bool nonInteractive )
  : KOEventView( calendar, parent, name )
{
  mActiveItem = 0;
  mIsNonInteractive = nonInteractive;

  mListView = new KListView( this );
  mListView->addColumn( i18n("Summary") );
  mListView->addColumn( i18n("Reminder") ); // alarm set?
  mListView->setColumnAlignment( Reminder_Column, AlignHCenter );

  mListView->addColumn( i18n("Recurs") ); // recurs?
  mListView->setColumnAlignment( Recurs_Column, AlignHCenter );

  mListView->addColumn( i18n("Start Date/Time") );
  mListView->setColumnAlignment( StartDateTime_Column, AlignHCenter );

  mListView->addColumn( i18n("End Date/Time") );
  mListView->setColumnAlignment( EndDateTime_Column, AlignHCenter );

  mListView->addColumn( i18n("Categories") );

  QBoxLayout *layoutTop = new QVBoxLayout( this );
  layoutTop->addWidget( mListView );

  mPopupMenu = eventPopup();
/*
  mPopupMenu->insertSeparator();
  mPopupMenu->insertItem(i18n("Show Dates"), this,
                      SLOT(showDates()));
  mPopupMenu->insertItem(i18n("Hide Dates"), this,
                      SLOT(hideDates()));
*/

  QObject::connect( mListView, SIGNAL( doubleClicked( QListViewItem * ) ),
                    SLOT( defaultItemAction( QListViewItem * ) ) );
  QObject::connect( mListView, SIGNAL( returnPressed( QListViewItem * ) ),
                    SLOT( defaultItemAction( QListViewItem * ) ) );
  QObject::connect( mListView, SIGNAL( rightButtonClicked ( QListViewItem *,
                                                            const QPoint &,
                                                            int ) ),
                    SLOT( popupMenu( QListViewItem *, const QPoint &, int ) ) );
  QObject::connect( mListView, SIGNAL( selectionChanged() ),
                    SLOT( processSelectionChange() ) );

//  setMinimumSize(100,100);
  mListView->restoreLayout( KOGlobals::self()->config(), "KOListView Layout" );

  new KOListViewToolTip( mListView->viewport(), calendar, mListView );

  mSelectedDates.append( QDate::currentDate() );
}

KOListView::~KOListView()
{
  delete mPopupMenu;
}

int KOListView::maxDatesHint()
{
  return 0;
}

int KOListView::currentDateCount()
{
  return mSelectedDates.count();
}

Incidence::List KOListView::selectedIncidences()
{
  Incidence::List eventList;

  QListViewItem *item = mListView->selectedItem();
  if ( item ) {
    eventList.append( static_cast<KOListViewItem *>( item )->data() );
  }

  return eventList;
}

DateList KOListView::selectedIncidenceDates()
{
  return mSelectedDates;
}

void KOListView::showDates( bool show )
{
  // Shouldn't we set it to a value greater 0? When showDates is called with
  // show == true at first, then the columnwidths are set to zero.
  static int oldColWidth1 = 0;
  static int oldColWidth3 = 0;

  if ( !show ) {
    oldColWidth1 = mListView->columnWidth( 1 );
    oldColWidth3 = mListView->columnWidth( 3 );
    mListView->setColumnWidth( 1, 0 );
    mListView->setColumnWidth( 3, 0 );
  } else {
    mListView->setColumnWidth( 1, oldColWidth1 );
    mListView->setColumnWidth( 3, oldColWidth3 );
  }
  mListView->repaint();
}

void KOListView::showDates()
{
  showDates( true );
}

void KOListView::hideDates()
{
  showDates( false );
}

void KOListView::updateView()
{
  kdDebug(5850) << "KOListView::updateView() does nothing" << endl;
}

void KOListView::showDates( const QDate &start, const QDate &end )
{
  clear();

  QDate date = start;
  while( date <= end ) {
    addIncidences( calendar()->incidences( date ), date );
    mSelectedDates.append( date );
    date = date.addDays( 1 );
  }

  emit incidenceSelected( 0, QDate() );
}

void KOListView::showAll()
{
  Incidence::List incidenceList = calendar()->incidences();

  Incidence::List::ConstIterator it;
  for( it = incidenceList.begin(); it != incidenceList.end(); ++it ) {
    // we don't need the date, using showAll in non interactive mode for now
    addIncidence( *it, QDate() );
  }
}

void KOListView::addIncidences( const Incidence::List &incidenceList, const QDate &date )
{
  Incidence::List::ConstIterator it;
  for( it = incidenceList.begin(); it != incidenceList.end(); ++it ) {
    addIncidence( *it, date );
  }
}

void KOListView::addIncidence( Incidence *incidence, const QDate &date )
{
  if ( mUidDict.find( incidence->uid() ) ) {
    return;
  }

  mDateList[incidence->uid()] = date;
  mUidDict.insert( incidence->uid(), incidence );

  KOListViewItem *item = new KOListViewItem( incidence, mListView );
  ListItemVisitor v( item );
  if (incidence->accept( v ) ) {
    return;
  } else {
    delete item;
  }
}

void KOListView::showIncidences( const Incidence::List &incidenceList, const QDate &date )
{
  clear();

  addIncidences( incidenceList, date );

  // After new creation of list view no events are selected.
  emit incidenceSelected( 0, date );
}

void KOListView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  KOListViewItem *item;
  QDate f = mSelectedDates.first();
  QDate l = mSelectedDates.last();

  QDate date;
  if ( incidence->type() == "Todo" ) {
    date = static_cast<Todo *>( incidence )->dtDue().date();
  } else {
    date = incidence->dtStart().date();
  }

  switch( action ) {
    case KOGlobals::INCIDENCEADDED: {
      if ( date >= f && date <= l )
        addIncidence( incidence, date );
      break;
    }
    case KOGlobals::INCIDENCEEDITED: {
      item = getItemForIncidence( incidence );
      if ( item ) {
        delete item;
        mUidDict.remove( incidence->uid() );
        mDateList.remove( incidence->uid() );
      }
      if ( date >= f && date <= l ) {
        addIncidence( incidence, date );
      }
    }
    break;
    case KOGlobals::INCIDENCEDELETED: {
      item = getItemForIncidence( incidence );
      if ( item ) {
        delete item;
      }
      break;
    }
    default:
      kdDebug(5850) << "KOListView::changeIncidenceDisplay(): Illegal action " << action << endl;
  }
}

KOListViewItem *KOListView::getItemForIncidence( Incidence *incidence )
{
  KOListViewItem *item = static_cast<KOListViewItem *>( mListView->firstChild() );
  while ( item ) {
//    kdDebug(5850) << "Item " << item->text(0) << " found" << endl;
    if ( item->data() == incidence ) {
      return item;
    }
    item = static_cast<KOListViewItem *>( item->nextSibling() );
  }
  return 0;
}

void KOListView::defaultItemAction( QListViewItem *i )
{
  if ( !mIsNonInteractive ) {
    KOListViewItem *item = static_cast<KOListViewItem *>( i );
    if ( item ) {
      defaultAction( item->data() );
    }
  }
}

void KOListView::popupMenu( QListViewItem *item,const QPoint &, int )
{
  if ( !mIsNonInteractive ) {
    mActiveItem = static_cast<KOListViewItem *>( item );
    if ( mActiveItem ) {
      Incidence *incidence = mActiveItem->data();
      // FIXME: For recurring incidences we don't know the date of this
      // occurrence, there's no reference to it at all!
      mPopupMenu->showIncidencePopup( calendar(), incidence, QDate() );
    } else {
      showNewEventPopup();
    }
  }
}

void KOListView::readSettings( KConfig *config )
{
  mListView->restoreLayout( config,"KOListView Layout" );
}

void KOListView::writeSettings( KConfig *config )
{
  mListView->saveLayout( config, "KOListView Layout" );
}

void KOListView::processSelectionChange()
{
  if ( !mIsNonInteractive ) {
    kdDebug(5850) << "KOListView::processSelectionChange()" << endl;

    KOListViewItem *item =
      static_cast<KOListViewItem *>( mListView->selectedItem() );

    if ( !item ) {
      emit incidenceSelected( 0, QDate() );
    } else {
      Incidence *incidence = static_cast<Incidence *>( item->data() );
      emit incidenceSelected( incidence, mDateList[incidence->uid()] );
    }
  }
}

void KOListView::clearSelection()
{
  mListView->selectAll( false );
}

void KOListView::clear()
{
  mSelectedDates.clear();
  mListView->clear();
  mUidDict.clear();
  mDateList.clear();
}

QSize KOListView::sizeHint() const
{
  const QSize s = KOEventView::sizeHint();
  return QSize( s.width() + style().pixelMetric( QStyle::PM_ScrollBarExtent ) + 1, s.height() );
}
