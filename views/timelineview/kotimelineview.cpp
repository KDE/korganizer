/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Till Adam <adam@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#include "kotimelineview.h"
#include "koeventpopupmenu.h"
#include "koglobals.h"
#include <kcalprefs.h>
#include "timelineitem.h"
#include "kohelper.h"

#include "kdgantt2/kdganttgraphicsview.h"
#include "kdgantt2/kdganttabstractrowcontroller.h"
#include <kdgantt2/kdganttdatetimegrid.h>
#include "kdgantt2/kdganttitemdelegate.h"
#include "kdgantt2/kdganttstyleoptionganttitem.h"

// #include <kdgantt1/KDGanttViewTaskItem.h>
// #include <kdgantt1/KDGanttViewSubwidgets.h>

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/utils.h>

#include <QApplication>
#include <QPainter>
#include <QLayout>
#include <QStandardItemModel>
#include <QResizeEvent>
#include <QSplitter>
#include <QTreeWidget>
#include <QHeaderView>

using namespace Akonadi;
using namespace KOrg;
using namespace KCal;

namespace KOrg {
class RowController : public KDGantt::AbstractRowController {
private:
    static const int ROW_HEIGHT ;
    QPointer<QAbstractItemModel> m_model;

public:
    RowController()
    {
      mRowHeight = 20;
    }

    void setModel( QAbstractItemModel* model )
    {
        m_model = model;
    }

    /*reimp*/int headerHeight() const { return 2*mRowHeight + 10; }

    /*reimp*/ bool isRowVisible( const QModelIndex& ) const { return true;}
    /*reimp*/ bool isRowExpanded( const QModelIndex& ) const { return false; }
    /*reimp*/ KDGantt::Span rowGeometry( const QModelIndex& idx ) const
    {
        return KDGantt::Span( idx.row()*mRowHeight, mRowHeight );
    }
    /*reimp*/ int maximumItemHeight() const {
        return mRowHeight/2;
    }
    /*reimp*/int totalHeight() const {
        return m_model->rowCount()* mRowHeight;
    }

    /*reimp*/ QModelIndex indexAt( int height ) const {
        return m_model->index( height/mRowHeight, 0 );
    }

    /*reimp*/ QModelIndex indexBelow( const QModelIndex& idx ) const {
        if ( !idx.isValid() )return QModelIndex();
        return idx.model()->index( idx.row()+1, idx.column(), idx.parent() );
    }
    /*reimp*/ QModelIndex indexAbove( const QModelIndex& idx ) const {
        if ( !idx.isValid() )return QModelIndex();
        return idx.model()->index( idx.row()-1, idx.column(), idx.parent() );
    }

    void setRowHeight( int height ) { mRowHeight = height; }

private:
    int mRowHeight;

};

class GanttHeaderView : public QHeaderView {
public:
    explicit GanttHeaderView( QWidget* parent=0 ) : QHeaderView( Qt::Horizontal, parent ) {
    }

    QSize sizeHint() const { QSize s = QHeaderView::sizeHint(); s.rheight() *= 2; return s; }
};

class GanttItemDelegate : public KDGantt::ItemDelegate {
    void paintGanttItem( QPainter* painter,
                         const KDGantt::StyleOptionGanttItem& opt,
                         const QModelIndex& idx )
    {
        painter->setRenderHints( QPainter::Antialiasing );
        if ( !idx.isValid() ) return;
        KDGantt::ItemType type = static_cast<KDGantt::ItemType>(
                            idx.model()->data( idx, KDGantt::ItemTypeRole ).toInt() );
        QString txt = idx.model()->data( idx, Qt::DisplayRole ).toString();
        QRectF itemRect = opt.itemRect;
        QRectF boundingRect = opt.boundingRect;
        boundingRect.setY( itemRect.y() );
        boundingRect.setHeight( itemRect.height() );

        QBrush brush = defaultBrush( type );
        if ( opt.state & QStyle::State_Selected ) {
            QLinearGradient selectedGrad( 0., 0., 0.,
                                          QApplication::fontMetrics().height() );
            selectedGrad.setColorAt( 0., Qt::red );
            selectedGrad.setColorAt( 1., Qt::darkRed );

            brush = QBrush( selectedGrad );
            painter->setBrush( brush );
        } else
          painter->setBrush( idx.model()->data( idx, Qt::DecorationRole ).value<QColor>() );

        painter->setPen( defaultPen( type ) );
        painter->setBrushOrigin( itemRect.topLeft() );

        switch( type ) {
        case KDGantt::TypeTask:
            if ( itemRect.isValid() ) {
                QRectF r = itemRect;
                painter->drawRect( r );

                Qt::Alignment ta;
                switch( opt.displayPosition ) {
                case KDGantt::StyleOptionGanttItem::Left: ta = Qt::AlignLeft; break;
                case KDGantt::StyleOptionGanttItem::Right: ta = Qt::AlignRight; break;
                case KDGantt::StyleOptionGanttItem::Center: ta = Qt::AlignCenter; break;
                }
                painter->drawText( boundingRect, ta, txt );
            }
            break;
        default:
            KDGantt::ItemDelegate::paintGanttItem( painter, opt, idx );
            break;
        }
    }
};

}

KOTimelineView::KOTimelineView( QWidget *parent )
  : KOEventView( parent ), mEventPopup( 0 )
{
  QVBoxLayout *vbox = new QVBoxLayout( this );
  QSplitter *splitter = new QSplitter( Qt::Horizontal, this );
  mLeftView = new QTreeWidget;
  mLeftView->setHeader( new GanttHeaderView );
  mLeftView->setHeaderLabel( i18n("Calendar") );
  mLeftView->setRootIsDecorated( false );
  mLeftView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );  
  
  mGantt = new KDGantt::GraphicsView();
  splitter->addWidget( mLeftView );
  splitter->addWidget( mGantt );
  connect( splitter, SIGNAL( splitterMoved(int,int) ), SLOT( splitterMoved() ) );
  QStandardItemModel *model = new QStandardItemModel( this );

  mRowController = new RowController; //TODO: delete
  mRowController->setRowHeight( fontMetrics().height() ); //TODO: detect
  
  mRowController->setModel( model );
  mGantt->setRowController( mRowController );

  KDGantt::DateTimeGrid *grid = new KDGantt::DateTimeGrid;
  grid->setScale( KDGantt::DateTimeGrid::ScaleHour );
  grid->setDayWidth( width() );
  grid->setRowSeparators( true );  
  mGantt->setGrid( grid );
  mGantt->setModel( model );


#if 0
  mGantt->setCalendarMode( true );
  mGantt->setShowLegendButton( false );
  mGantt->setFixedHorizon( true );
  mGantt->removeColumn( 0 );
  mGantt->addColumn( i18n( "Calendar" ) );
  mGantt->setHeaderVisible( true );
  if ( KGlobal::locale()->use12Clock() ) {
    mGantt->setHourFormat( KDGanttView::Hour_12 );
  } else {
    mGantt->setHourFormat( KDGanttView::Hour_24_FourDigit );
  }
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif
 mGantt->setItemDelegate( new GanttItemDelegate);


  vbox->addWidget( splitter );

#if 0
  connect( mGantt, SIGNAL(rescaling(KDGanttView::Scale)),
           SLOT(overscale(KDGanttView::Scale)) );
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif
  connect( model, SIGNAL(itemChanged(QStandardItem*)),
           SLOT(itemChanged(QStandardItem*)) );
  connect( mGantt, SIGNAL(doubleClicked(QModelIndex)),
           SLOT(itemDoubleClicked(QModelIndex)) );
  connect( mGantt, SIGNAL(activated(QModelIndex)),
           SLOT(itemSelected(QModelIndex)) );
  mGantt->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mGantt, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenuRequested(QPoint)) );

  connect( mGantt, SIGNAL(dateTimeDoubleClicked(const QDateTime &)),
           SLOT(newEventWithHint(const QDateTime &)) );
}

KOTimelineView::~KOTimelineView()
{
  delete mRowController;
  delete mEventPopup;
}

void KOTimelineView::resizeEvent(QResizeEvent* event)
{
  static_cast<KDGantt::DateTimeGrid*>(mGantt->grid())->setDayWidth( event->size().width() );
  KOEventView::resizeEvent( event );
}

void KOTimelineView::splitterMoved()
{
  mLeftView->setColumnWidth( 0, mLeftView->width() );
}


/*virtual*/
Akonadi::Item::List KOTimelineView::selectedIncidences()
{
  return mSelectedItemList;
}

/*virtual*/
KCal::DateList KOTimelineView::selectedIncidenceDates()
{
  return KCal::DateList();
}

/*virtual*/
int KOTimelineView::currentDateCount()
{
  return 0;
}

/*virtual*/
void KOTimelineView::showDates( const QDate &start, const QDate &end )
{
  kDebug() << "start=" << start << "end=" << end;

  mStartDate = start;
  mEndDate = end;
  mHintDate = QDateTime();
  KDGantt::DateTimeGrid *grid = static_cast<KDGantt::DateTimeGrid*>(mGantt->grid());
  grid->setStartDateTime( QDateTime( start ) );
#if 0
  mGantt->setHorizonStart( QDateTime( start ) );
  mGantt->setHorizonEnd( QDateTime( end.addDays( 1 ) ) );
  mGantt->setMinorScaleCount( 1 );
  mGantt->setScale( KDGanttView::Hour );
  mGantt->setMinimumScale( KDGanttView::Hour );
  mGantt->setMaximumScale( KDGanttView::Hour );
  mGantt->zoomToFit();

  mGantt->setUpdateEnabled( false );
  mGantt->clear();
#else
 kDebug() << "Disabled code, port to KDGantt2";
#endif

  mLeftView->clear();
  uint index = 0;
  // item for every calendar
  TimelineItem *item = 0;
  Akonadi::Calendar *calres = calendar();
  if ( !calres ) {
    item = new TimelineItem( calendar(), index++, static_cast<QStandardItemModel*>( mGantt->model() ), mGantt );
    mLeftView->addTopLevelItem( new QTreeWidgetItem( QStringList() << i18n( "Calendar" ) ) );
    mCalendarItemMap.insert( -1, item );
    
  } else {
    const CollectionSelection *colSel = collectionSelection();
    const Collection::List collections = colSel->selectedCollections();

    Q_FOREACH ( const Collection &collection, collections ) {
      if ( collection.contentMimeTypes().contains(
             QLatin1String( "application/x-vnd.akonadi.calendar.event" ) ) ) {
        item = new TimelineItem( calendar(), index++, static_cast<QStandardItemModel*>( mGantt->model() ), mGantt );
        mLeftView->addTopLevelItem(new QTreeWidgetItem( QStringList() << Akonadi::displayName( collection ) ) );
        const QColor resourceColor = KOHelper::resourceColor( collection );
        if ( resourceColor.isValid() ) {
          item->setColor( resourceColor );
        }
        kDebug() << "Created item " << item << " ( " <<  Akonadi::displayName( collection ) << " ) with index" <<  index - 1 << " from collection " << collection.id();
        mCalendarItemMap.insert( collection.id(), item );
      }
    }
  }
  
  // add incidences
  Item::List events;
  KDateTime::Spec timeSpec = KCalPrefs::instance()->timeSpec();
  for ( QDate day = start; day <= end; day = day.addDays( 1 ) ) {
    events = calendar()->events( day, timeSpec, EventSortStartDate, SortDirectionAscending );
    Q_FOREACH ( const Item &i, events ) {
      insertIncidence( i, day );
    }
  }
  splitterMoved();
}

/*virtual*/
void KOTimelineView::showIncidences( const Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

/*virtual*/
void KOTimelineView::updateView()
{
  if ( mStartDate.isValid() && mEndDate.isValid() ) {
    showDates( mStartDate, mEndDate );
  }
}

/*virtual*/
void KOTimelineView::changeIncidenceDisplay( const Item &incidence, int mode )
{
  switch ( mode ) {
  case Akonadi::IncidenceChanger::INCIDENCEADDED:
    insertIncidence( incidence );
    break;
  case Akonadi::IncidenceChanger::INCIDENCEEDITED:
    removeIncidence( incidence );
    insertIncidence( incidence );
    break;
  case Akonadi::IncidenceChanger::INCIDENCEDELETED:
    removeIncidence( incidence );
    break;
  default:
    updateView();
  }
}

void KOTimelineView::itemSelected( const QModelIndex &index )
{
  TimelineSubItem *tlitem = dynamic_cast<TimelineSubItem *>( static_cast<QStandardItemModel*>( mGantt->model() )->item( index.row(), index.column() ) );
  if ( tlitem ) {
    emit incidenceSelected( tlitem->incidence(), tlitem->originalStart().date() );
  }
}

void KOTimelineView::itemDoubleClicked( const QModelIndex &index )
{
  TimelineSubItem *tlitem = dynamic_cast<TimelineSubItem *>( static_cast<QStandardItemModel*>( mGantt->model() )->item( index.row(), index.column() ) );
  if ( tlitem ) {
    emit editIncidenceSignal( tlitem->incidence() );
  }
}

void KOTimelineView::contextMenuRequested(const QPoint& point)
{
   QPersistentModelIndex index = mGantt->indexAt( point );
//   mHintDate = QDateTime( mGantt->getDateTimeForCoordX( QCursor::pos().x(), true ) );
  TimelineSubItem *tlitem = dynamic_cast<TimelineSubItem *>( static_cast<QStandardItemModel*>( mGantt->model() )->item( index.row(), index.column() ) );
  if ( !tlitem ) {
    showNewEventPopup();
    mSelectedItemList = Akonadi::Item::List();
    return;
  }
  if ( !mEventPopup ) {
    mEventPopup = eventPopup();
  }
  mEventPopup->showIncidencePopup( tlitem->incidence(),
                                   Akonadi::incidence( tlitem->incidence() )->dtStart().date() );
  mSelectedItemList << tlitem->incidence();
}

bool KOTimelineView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  startDt = QDateTime( mHintDate );
  endDt = QDateTime( mHintDate.addSecs( 2 * 60 * 60 ) );
  allDay = false;
  return mHintDate.isValid();
}

//slot
void KOTimelineView::newEventWithHint( const QDateTime &dt )
{
  mHintDate = dt;
  emit newEventSignal( collectionSelection()->selectedCollections(), dt );
}

TimelineItem *KOTimelineView::calendarItemForIncidence( const Item &incidence )
{
  Akonadi::Calendar *calres = calendar();
  TimelineItem *item = 0;
  if ( !calres ) {
    item = mCalendarItemMap.value( -1 );
  } else {
    item = mCalendarItemMap.value( incidence.parentCollection().id() );
  }
  return item;
}

void KOTimelineView::insertIncidence( const Item &aitem, const QDate &day )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  kDebug() << "Item " << aitem.id() << " parentcollection: " << aitem.parentCollection().id();
  TimelineItem *item = calendarItemForIncidence( aitem );
  if ( !item ) {
    kWarning() << "Help! Something is really wrong here!";
    return;
  }

  if ( incidence->recurs() ) {
    QList<KDateTime> l = incidence->startDateTimesForDate( day );
    if ( l.isEmpty() ) {
      // strange, but seems to happen for some recurring events...
      item->insertIncidence( aitem, KDateTime( day, incidence->dtStart().time() ),
                              KDateTime( day, incidence->dtEnd().time() ) );
    } else {
      for ( QList<KDateTime>::ConstIterator it = l.constBegin(); it != l.constEnd(); ++it ) {
        item->insertIncidence( aitem, *it, incidence->endDateForStart( *it ) );
      }
    }
  } else {
    if ( incidence->dtStart().date() == day ||
         incidence->dtStart().date() < mStartDate ) {
      item->insertIncidence( aitem );
    }
  }
}

void KOTimelineView::insertIncidence( const Item &incidence )
{
  const Event::Ptr event = Akonadi::event( incidence );
  if ( !event ) {
    return;
  }

  if ( event->recurs() ) {
    insertIncidence( incidence, QDate() );
  }

  KDateTime::Spec timeSpec = KCalPrefs::instance()->timeSpec();
  for ( QDate day = mStartDate; day <= mEndDate; day = day.addDays( 1 ) ) {
    Item::List events = calendar()->events(
      day, timeSpec, EventSortStartDate, SortDirectionAscending );
    if ( events.contains( incidence ) ) //PENDING(AKONADI_PORT) check if correct. also check the original if, was inside the for loop (unnecessarily)
      for ( Item::List::ConstIterator it = events.constBegin(); it != events.constEnd(); ++it ) {
        insertIncidence( *it, day );
      }
  }
}

void KOTimelineView::removeIncidence( const Item &incidence )
{
  TimelineItem *item = calendarItemForIncidence( incidence );
  if ( item ) {
    item->removeIncidence( incidence );
  } else {
#if 0 //AKONADI_PORT_DISABLED
    // try harder, the incidence might already be removed from the resource
    typedef QMap<QString, KOrg::TimelineItem *> M2_t;
    typedef QMap<KCal::ResourceCalendar *, M2_t> M1_t;
    for ( M1_t::ConstIterator it1 = mCalendarItemMap.constBegin();
          it1 != mCalendarItemMap.constEnd(); ++it1 ) {
      for ( M2_t::ConstIterator it2 = it1.value().constBegin();
            it2 != it1.value().constEnd(); ++it2 ) {
        if ( it2.value() ) {
          it2.value()->removeIncidence( incidence );
        }
      }
    }
#endif
  }
}

 void KOTimelineView::itemChanged(QStandardItem* item)
{
  TimelineSubItem *tlit = dynamic_cast<TimelineSubItem *>( item );
  if ( !tlit ) {
    return;
  }

  const Item i = tlit->incidence();
  const Incidence::Ptr inc = Akonadi::incidence( i );

  KDateTime newStart( tlit->startTime() );
  if ( inc->allDay() ) {
    newStart = KDateTime( newStart.date() );
  }

  int delta = tlit->originalStart().secsTo( newStart );
  inc->setDtStart( inc->dtStart().addSecs( delta ) );
  int duration = tlit->startTime().secsTo( tlit->endTime() );
  int allDayOffset = 0;
  if ( inc->allDay() ) {
    int secsPerDay = 60 * 60 * 24;
    duration /= secsPerDay;
    duration *= secsPerDay;
    allDayOffset = secsPerDay;
    duration -= allDayOffset;
    if ( duration < 0 ) {
      duration = 0;
    }
  }
  inc->setDuration( duration );
  TimelineItem *parent = tlit->parent();
  parent->moveItems( i, tlit->originalStart().secsTo( newStart ), duration + allDayOffset );
}

// void KOTimelineView::overscale( KDGanttView::Scale scale )
// {
//   Q_UNUSED( scale );
//   /* Disabled, looks *really* bogus:
//      this triggers and endless rescaling loop; we want to set
//      a fixed scale, the Gantt view doesn't like it and rescales
//      (emitting a rescaling signal that leads here) and so on...
//   //set a relative zoom factor of 1 (?!)
//   mGantt->setZoomFactor( 1, false );
//   mGantt->setScale( KDGanttView::Hour );
//   mGantt->setMinorScaleCount( 12 );
//   */
// }

KOrg::CalPrinterBase::PrintType KOTimelineView::printType()
{
  // If up to three days are selected, use day style, otherwise week
  if ( currentDateCount() <= 3 ) {
    return KOrg::CalPrinterBase::Day;
  } else {
    return KOrg::CalPrinterBase::Week;
  }
}


#include "kotimelineview.moc"
