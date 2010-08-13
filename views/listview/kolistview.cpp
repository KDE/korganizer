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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kolistview.h"
#include "koeventpopupmenu.h"
#include "koglobals.h"
#include "kohelper.h"
#include "koprefs.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/utils.h>

#include <kcalutils/incidenceformatter.h>
#include <kcalcore/todo.h>
#include <kcalcore/visitor.h>
#include <kcalcore/journal.h>

#include <kcalprefs.h>

#include <QBoxLayout>
#include <QStyle>

using namespace Akonadi;
using namespace KOrg;
using namespace KCalUtils;

enum {
  Summary_Column = 0,
  Reminder_Column,
  Recurs_Column,
  StartDateTime_Column,
  EndDateTime_Column,
  Categories_Column
};

#ifdef __GNUC__
#warning Port me!
#endif
#if 0
KOListViewToolTip::KOListViewToolTip( QWidget *parent, K3ListView *lv )
  :QToolTip( parent )
{
  eventlist = lv;
}

void KOListViewToolTip::maybeTip( const QPoint &pos )
{
  QRect r;
  Q3ListViewItem *it = eventlist->itemAt( pos );
  KOListViewItem *i = static_cast<KOListViewItem*>( it );

  if ( i && KOPrefs::instance()->mEnableToolTips ) {
    /* Calculate the rectangle. */
    r = eventlist->itemRect( it );

    /* Show the tip */
    QString tipText( IncidenceFormatter::toolTipString( i->data() ) );
    if ( !tipText.isEmpty() ) {
      tip( r, tipText );
    }
  }
}
#endif

/**
  This class provides the initialization of a KOListViewItem for calendar
  components using the Incidence::Visitor.
*/
class KOListView::ListItemVisitor : public Visitor
{
  public:
    ListItemVisitor( KOListViewItem *item ) : mItem( item ) {}
    ~ListItemVisitor() {}

    bool visit( Event::Ptr  );
    bool visit( Todo::Ptr  );
    bool visit( Journal::Ptr  );
    bool visit( FreeBusy::Ptr  ) { // to inhibit hidden virtual compile warning
      return true;
    };
  private:
    KOListViewItem *mItem;
};

bool KOListView::ListItemVisitor::visit( Event::Ptr e )
{
  mItem->setText( Summary_Column, e->summary() );
  if ( e->hasEnabledAlarms() ) {
    static const QPixmap alarmPxmp = KOGlobals::self()->smallIcon( "appointment-reminder" );
    mItem->setPixmap( Reminder_Column, alarmPxmp );
    mItem->setSortKey( Reminder_Column, "1" );
  } else {
    mItem->setSortKey( Reminder_Column, "0" );
  }

  if ( e->recurs() ) {
    static const QPixmap recurPxmp = KOGlobals::self()->smallIcon( "appointment-recurring" );
    mItem->setPixmap( Recurs_Column, recurPxmp );
    mItem->setSortKey( Recurs_Column, "1" );
  } else {
    mItem->setSortKey( Recurs_Column, "0" );
  }

  QPixmap eventPxmp;

  if ( e->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
    eventPxmp = KOGlobals::self()->smallIcon( "view-calendar-wedding-anniversary" );
  } else if ( e->customProperty( "KABC", "BIRTHDAY" ) == "YES" ) {
    eventPxmp = KOGlobals::self()->smallIcon( "view-calendar-birthday" );
  } else {
    eventPxmp = KOGlobals::self()->smallIcon( "view-calendar-day" );
  }

  mItem->setPixmap( Summary_Column, eventPxmp );

  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                    e->dtStart(), e->allDay(), true, KCalPrefs::instance()->timeSpec() ) );

  mItem->setSortKey( StartDateTime_Column, e->dtStart().toTimeSpec(
                       KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );

  mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString(
                    e->dtEnd(), e->allDay(), true, KCalPrefs::instance()->timeSpec() ) );

  mItem->setSortKey( EndDateTime_Column, e->dtEnd().toTimeSpec(
                       KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );

  mItem->setText( Categories_Column, e->categoriesStr() );

  return true;
}

bool KOListView::ListItemVisitor::visit( Todo::Ptr t )
{
  static const QPixmap todoPxmp = KOGlobals::self()->smallIcon( "view-calendar-tasks" );
  static const QPixmap todoDonePxmp = KOGlobals::self()->smallIcon( "task-complete" );
  mItem->setPixmap( Summary_Column, t->isCompleted() ? todoDonePxmp : todoPxmp );
  mItem->setText( Summary_Column, t->summary() );
  if ( t->hasEnabledAlarms() ) {
    static const QPixmap alarmPxmp = KOGlobals::self()->smallIcon( "appointment-reminder" );
    mItem->setPixmap( Reminder_Column, alarmPxmp );
    mItem->setSortKey( Reminder_Column, "1" );
  } else {
    mItem->setSortKey( Reminder_Column, "0" );
  }

  if ( t->recurs() ) {
    static const QPixmap recurPxmp = KOGlobals::self()->smallIcon( "appointment-recurring" );
    mItem->setPixmap( Recurs_Column, recurPxmp );
    mItem->setSortKey( Recurs_Column, "1" );
  } else {
    mItem->setSortKey( Recurs_Column, "0" );
  }

  if ( t->hasStartDate() ) {
    mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                      t->dtStart(), t->allDay(), true, KCalPrefs::instance()->timeSpec() ) );
    mItem->setSortKey( StartDateTime_Column, t->dtStart().toTimeSpec(
                       KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );
  } else {
    mItem->setText( StartDateTime_Column, "---" );
  }

  if ( t->hasDueDate() ) {
    mItem->setText( EndDateTime_Column, IncidenceFormatter::dateTimeToString(
                      t->dtDue(), t->allDay(), true, KCalPrefs::instance()->timeSpec() ) );

    mItem->setSortKey( EndDateTime_Column, t->dtDue().toTimeSpec(
                         KCalPrefs::instance()->timeSpec() ).toString( KDateTime::ISODate ) );
  } else {
    mItem->setText( EndDateTime_Column, "---" );
  }
  mItem->setText( Categories_Column, t->categoriesStr() );

  return true;
}

bool KOListView::ListItemVisitor::visit( Journal::Ptr j )
{
  static const QPixmap jrnalPxmp = KOGlobals::self()->smallIcon( "view-pim-journal" );
  mItem->setPixmap( Summary_Column, jrnalPxmp );
  if ( j->summary().isEmpty() ) {
    mItem->setText( Summary_Column, j->description().section( '\n', 0, 0 ) );
  } else {
    mItem->setText( Summary_Column, j->summary() );
  }
  mItem->setText( StartDateTime_Column, IncidenceFormatter::dateTimeToString(
                  j->dtStart(), j->allDay(), true, KCalPrefs::instance()->timeSpec() ) );

  mItem->setSortKey( StartDateTime_Column, j->dtStart().toString( KDateTime::ISODate ) );

  return true;
}

KOListView::KOListView( QWidget *parent,  bool nonInteractive )
  : KOEventView( parent )
{
  mActiveItem = 0;
  mIsNonInteractive = nonInteractive;

  mListView = new K3ListView( this );
  mListView->addColumn( i18n( "Summary" ) );
  mListView->addColumn( i18n( "Reminder" ) ); // alarm set?
  mListView->addColumn( i18n( "Recurs" ) ); // recurs?
  mListView->addColumn( i18n( "Start Date/Time" ) );
  mListView->setColumnAlignment( StartDateTime_Column, Qt::AlignHCenter );
  mListView->addColumn( i18n( "End Date/Time" ) );
  mListView->setColumnAlignment( EndDateTime_Column, Qt::AlignHCenter );
  mListView->addColumn( i18n( "Categories" ) );

  QBoxLayout *layoutTop = new QVBoxLayout( this );
  layoutTop->setMargin( 0 );
  layoutTop->addWidget( mListView );

  mPopupMenu = eventPopup();
/*
  mPopupMenu->addSeparator();
  mPopupMenu->insertItem(i18n("Show Dates"), this,
                      SLOT(showDates()));
  mPopupMenu->insertItem(i18n("Hide Dates"), this,
                      SLOT(hideDates()));
*/

  QObject::connect( mListView, SIGNAL(doubleClicked(Q3ListViewItem *)),
                    SLOT(defaultItemAction(Q3ListViewItem *)) );
  QObject::connect( mListView, SIGNAL(returnPressed(Q3ListViewItem *)),
                    SLOT(defaultItemAction(Q3ListViewItem *)) );
  QObject::connect( mListView,
                    SIGNAL(rightButtonClicked(Q3ListViewItem *,const QPoint &,int)),
                    SLOT(popupMenu(Q3ListViewItem *,const QPoint &,int)) );
  QObject::connect( mListView, SIGNAL(selectionChanged()),
                    SLOT(processSelectionChange()) );

  mListView->restoreLayout( KOGlobals::self()->config(), "KOListView Layout" );

#ifdef __GNUC__
#warning Port me!
#endif
//  new KOListViewToolTip( mListView->viewport(), mListView );

  mSelectedDates.append( QDate::currentDate() );
}

KOListView::~KOListView()
{
  delete mPopupMenu;
}

int KOListView::maxDatesHint() const
{
  return 0;
}

int KOListView::currentDateCount() const
{
  return mSelectedDates.count();
}

Akonadi::Item::List KOListView::selectedIncidences()
{
  Akonadi::Item::List eventList;
  Q3ListViewItem *item = mListView->selectedItem();
  if ( item ) {
    KOListViewItem *i = static_cast<KOListViewItem *>( item );
    eventList.append( mItems.value( i->data() ) );
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
  kDebug() << "not implemented yet";
}

void KOListView::showDates( const QDate &start, const QDate &end )
{
  clear();

  mStartDate = start;
  mEndDate = end;

  QDate date = start;
  while ( date <= end ) {
    addIncidences( calendar()->incidences( date ), date );
    mSelectedDates.append( date );
    date = date.addDays( 1 );
  }

  emit incidenceSelected( Item(), QDate() );
}

void KOListView::showAll()
{
  const Item::List incidenceList = calendar()->incidences();
  addIncidences( incidenceList, QDate() );
}

void KOListView::addIncidences( const Item::List &incidenceList, const QDate &date )
{
  Q_FOREACH ( const Item & i, incidenceList ) {
    addIncidence( i, date );
  }
}

void KOListView::addIncidence( const Item &aitem, const QDate &date )
{
  if ( !Akonadi::hasIncidence( aitem ) || mItems.contains( aitem.id() ) ) {
    return;
  }

  mDateList.insert( aitem.id(), date );
  mItems.insert( aitem.id(), aitem );

  Incidence::Ptr tinc = Akonadi::incidence( aitem );

  if ( tinc->customProperty( "KABC", "BIRTHDAY" ) == "YES" ||
       tinc->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
    int years = KOHelper::yearDiff( tinc->dtStart().date(), mEndDate );
    if ( years > 0 ) {
      tinc = Incidence::Ptr( tinc->clone() );
      tinc->setReadOnly( false );
      tinc->setSummary( i18np( "%2 (1 year)", "%2 (%1 years)", years, tinc->summary() ) );
      tinc->setReadOnly( true );
    }
  }
  KOListViewItem *item = new KOListViewItem( aitem.id(), mListView );
  ListItemVisitor v( item );
  if ( !tinc->accept( v, tinc ) ) {
    delete item;
  }
}

void KOListView::showIncidences( const Item::List &incidenceList, const QDate &date )
{
  clear();

  addIncidences( incidenceList, date );

  // After new creation of list view no events are selected.
  emit incidenceSelected( Item(), date );
}

void KOListView::changeIncidenceDisplay( const Item & aitem, int action )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  KOListViewItem *item;
  QDate f = mSelectedDates.first();
  QDate l = mSelectedDates.last();

  QDate date;
  if ( Akonadi::hasTodo( aitem ) ) {
    date = Akonadi::todo( aitem )->dtDue().
           toTimeSpec( KCalPrefs::instance()->timeSpec() ).date();
  } else {
    date = incidence->dtStart().
           toTimeSpec( KCalPrefs::instance()->timeSpec() ).date();
  }

  switch( action ) {
  case Akonadi::IncidenceChanger::INCIDENCEADDED:
  {
    if ( date >= f && date <= l ) {
      addIncidence( aitem, date );
    }
    break;
  }
  case Akonadi::IncidenceChanger::INCIDENCEEDITED:
  {
    item = getItemForIncidence( aitem );
    if ( item ) {
      delete item;
      mItems.remove( aitem.id() );
      mDateList.remove( aitem.id() );
    }
    if ( date >= f && date <= l ) {
      addIncidence( aitem, date );
    }
    break;
  }
  case Akonadi::IncidenceChanger::INCIDENCEDELETED:
  {
    item = getItemForIncidence( aitem );
    if ( item ) {
      delete item;
    }
    break;
  }
  default:
    kDebug() << "Illegal action" << action;
  }
}

KOListViewItem *KOListView::getItemForIncidence( const Item &aitem )
{
  KOListViewItem *item = (KOListViewItem *)mListView->firstChild();
  while ( item ) {
    if ( item->data() == aitem.id() ) {
      return item;
    }
    item = static_cast<KOListViewItem *>( item->nextSibling() );
  }
  return 0;
}

Incidence::Ptr KOListView::incidenceForId( const Item::Id &id ) const
{
  return Akonadi::incidence( mItems.value( id ) );
}

void KOListView::defaultItemAction( Q3ListViewItem *i )
{
  KOListViewItem *item = static_cast<KOListViewItem *>( i );
  if ( item && !mIsNonInteractive ) {
    defaultAction( mItems.value( item->data() ) );
  }
}

void KOListView::popupMenu( Q3ListViewItem *item, const QPoint &, int )
{
  mActiveItem = static_cast<KOListViewItem *>( item );
  if ( mActiveItem && !mIsNonInteractive ) {
    const Item aitem = mItems.value( mActiveItem->data() );
    // FIXME: For recurring incidences we don't know the date of this
    // occurrence, there's no reference to it at all!
    mPopupMenu->showIncidencePopup( aitem, Akonadi::incidence( aitem )->dtStart().date() );
  } else {
    showNewEventPopup();
  }
}

void KOListView::readSettings( KConfig *config )
{
  mListView->restoreLayout( config, "KOListView Layout" );
}

void KOListView::writeSettings( KConfig *config )
{
  mListView->saveLayout( config, "KOListView Layout" );
}

void KOListView::processSelectionChange()
{
  if ( !mIsNonInteractive ) {
    KOListViewItem *item = static_cast<KOListViewItem *>( mListView->selectedItem() );

    if ( !item ) {
      emit incidenceSelected( Item(), QDate() );
    } else {
      emit incidenceSelected( mItems.value( item->data() ), mDateList.value( item->data() ) );
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
  mDateList.clear();
  mItems.clear();
}

KOrg::CalPrinterBase::PrintType KOListView::printType()
{
  return KOrg::CalPrinterBase::Incidence;
}

QSize KOListView::sizeHint() const
{
  const QSize s = KOEventView::sizeHint();
  return QSize( s.width() + style()->pixelMetric( QStyle::PM_ScrollBarExtent ) + 1,
                s.height() );
}

#include "kolistview.moc"
