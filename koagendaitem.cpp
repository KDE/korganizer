/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
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

#include "koagendaitem.h"
#include "koprefs.h"
#include "koglobals.h"

#include <libkdepim/kvcarddrag.h>

#include <kcal/icaldrag.h>
#include <kcal/incidence.h>
#include <kcal/todo.h>
#include <kcal/incidenceformatter.h>
#include <kcal/vcaldrag.h>

#include <kpimutils/email.h>

#ifndef KORG_NOKABC
#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#endif

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kwordwrap.h>
#include <kmessagebox.h>

#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QList>
#include <QDropEvent>
#include <QDragEnterEvent>

#include "koagendaitem.moc"

//-----------------------------------------------------------------------------

QPixmap *KOAgendaItem::alarmPxmp = 0;
QPixmap *KOAgendaItem::recurPxmp = 0;
QPixmap *KOAgendaItem::readonlyPxmp = 0;
QPixmap *KOAgendaItem::replyPxmp = 0;
QPixmap *KOAgendaItem::groupPxmp = 0;
QPixmap *KOAgendaItem::groupPxmpTentative = 0;
QPixmap *KOAgendaItem::organizerPxmp = 0;

//-----------------------------------------------------------------------------

KOAgendaItem::KOAgendaItem( Incidence *incidence, const QDate &qd,
                            QWidget *parent )
  : QWidget( parent ), mIncidence( incidence ), mDate( qd ),
    mLabelText( mIncidence->summary() ), mIconAlarm( false ),
    mIconRecur( false ), mIconReadonly( false ), mIconReply( false ),
    mIconGroup( false ), mIconGroupTentative( false ), mIconOrganizer( false ),
    mMultiItemInfo( 0 ), mStartMoveInfo( 0 )
{

  QPalette pal = palette();
  pal.setColor(QPalette::Window, Qt::transparent);
  setPalette(pal);

  setCellXY( 0, 0, 1 );
  setCellXRight( 0 );
  setMouseTracking( true );
  mResourceColor = QColor();
  updateIcons();

  // select() does nothing if the state hasn't changed, so preset mSelected.
  mSelected = true;
  select( false );

  setToolTip(IncidenceFormatter::toolTipString( incidence ));
  setAcceptDrops( true );
}

void KOAgendaItem::updateIcons()
{
  if ( !mIncidence ) return;
  mIconReadonly = mIncidence->isReadOnly();
  mIconRecur = mIncidence->recurs();
  mIconAlarm = mIncidence->isAlarmEnabled();
  if ( mIncidence->attendeeCount() > 0 ) {
    if ( KOPrefs::instance()->thatIsMe( mIncidence->organizer().email() ) ) {
      mIconReply = false;
      mIconGroup = false;
      mIconGroupTentative = false;
      mIconOrganizer = true;
    } else {
      Attendee *me = mIncidence->attendeeByMails( KOPrefs::instance()->allEmails() );
      if ( me ) {
        if ( me->status() == Attendee::NeedsAction && me->RSVP() ) {
          mIconReply = true;
          mIconGroup = false;
          mIconGroupTentative = false;
          mIconOrganizer = false;
        } else if ( me->status() == Attendee::Tentative ) {
          mIconReply = false;
          mIconGroup = false;
          mIconGroupTentative = true;
          mIconOrganizer = false;
        } else {
          mIconReply = false;
          mIconGroup = true;
          mIconGroupTentative = false;
          mIconOrganizer = false;
        }
      } else {
        mIconReply = false;
        mIconGroup = true;
        mIconGroupTentative = false;
        mIconOrganizer = false;
      }
    }
  }
  update();
}


void KOAgendaItem::select( bool selected )
{
  if ( mSelected == selected ) return;
  mSelected = selected;

  update();
}

bool KOAgendaItem::dissociateFromMultiItem()
{
  if ( !isMultiItem() ) return false;
  KOAgendaItem *firstItem = firstMultiItem();
  if ( firstItem == this ) firstItem = nextMultiItem();
  KOAgendaItem *lastItem = lastMultiItem();
  if ( lastItem == this ) lastItem = prevMultiItem();

  KOAgendaItem *prevItem = prevMultiItem();
  KOAgendaItem *nextItem = nextMultiItem();

  if ( prevItem ) {
    prevItem->setMultiItem( firstItem,
                            prevItem->prevMultiItem(),
                            nextItem, lastItem );
  }
  if ( nextItem ) {
    nextItem->setMultiItem( firstItem, prevItem,
                            nextItem->prevMultiItem(),
                            lastItem );
  }
  delete mMultiItemInfo;
  mMultiItemInfo = 0;
  return true;
}

bool KOAgendaItem::setIncidence( Incidence *i )
{
  mIncidence = i;
  updateIcons();
  return true;
}


/*
  Return height of item in units of agenda cells
*/
int KOAgendaItem::cellHeight() const
{
  return mCellYBottom - mCellYTop + 1;
}

/*
  Return height of item in units of agenda cells
*/
int KOAgendaItem::cellWidth() const
{
  return mCellXRight - mCellXLeft + 1;
}

void KOAgendaItem::setItemDate( const QDate &qd )
{
  mDate = qd;
}

void KOAgendaItem::setCellXY( int X, int YTop, int YBottom )
{
  mCellXLeft = X;
  mCellYTop = YTop;
  mCellYBottom = YBottom;
}

void KOAgendaItem::setCellXRight( int XRight )
{
  mCellXRight = XRight;
}

void KOAgendaItem::setCellX( int XLeft, int XRight )
{
  mCellXLeft = XLeft;
  mCellXRight = XRight;
}

void KOAgendaItem::setCellY( int YTop, int YBottom )
{
  mCellYTop = YTop;
  mCellYBottom = YBottom;
}

void KOAgendaItem::setMultiItem(KOAgendaItem *first, KOAgendaItem *prev,
                                KOAgendaItem *next, KOAgendaItem *last)
{
  if (!mMultiItemInfo) mMultiItemInfo=new MultiItemInfo;
  mMultiItemInfo->mFirstMultiItem = first;
  mMultiItemInfo->mPrevMultiItem = prev;
  mMultiItemInfo->mNextMultiItem = next;
  mMultiItemInfo->mLastMultiItem = last;
}
bool KOAgendaItem::isMultiItem()
{
  return mMultiItemInfo;
}
KOAgendaItem* KOAgendaItem::prependMoveItem(KOAgendaItem* e)
{
  if (!e) return e;

  KOAgendaItem*first=0, *last=0;
  if (isMultiItem()) {
    first=mMultiItemInfo->mFirstMultiItem;
    last=mMultiItemInfo->mLastMultiItem;
  }
  if (!first) first=this;
  if (!last) last=this;

  e->setMultiItem(0, 0, first, last);
  first->setMultiItem(e, e, first->nextMultiItem(), first->lastMultiItem() );

  KOAgendaItem*tmp=first->nextMultiItem();
  while (tmp) {
    tmp->setMultiItem( e, tmp->prevMultiItem(), tmp->nextMultiItem(), tmp->lastMultiItem() );
    tmp = tmp->nextMultiItem();
  }

  if ( mStartMoveInfo && !e->moveInfo() ) {
    e->mStartMoveInfo=new MultiItemInfo( *mStartMoveInfo );
//    e->moveInfo()->mFirstMultiItem = moveInfo()->mFirstMultiItem;
//    e->moveInfo()->mLastMultiItem = moveInfo()->mLastMultiItem;
    e->moveInfo()->mPrevMultiItem = 0;
    e->moveInfo()->mNextMultiItem = first;
  }

  if (first && first->moveInfo()) {
    first->moveInfo()->mPrevMultiItem = e;
  }
  return e;
}

KOAgendaItem* KOAgendaItem::appendMoveItem(KOAgendaItem* e)
{
  if (!e) return e;

  KOAgendaItem*first=0, *last=0;
  if (isMultiItem()) {
    first=mMultiItemInfo->mFirstMultiItem;
    last=mMultiItemInfo->mLastMultiItem;
  }
  if (!first) first=this;
  if (!last) last=this;

  e->setMultiItem( first, last, 0, 0 );
  KOAgendaItem*tmp=first;

  while (tmp) {
    tmp->setMultiItem(tmp->firstMultiItem(), tmp->prevMultiItem(), tmp->nextMultiItem(), e);
    tmp = tmp->nextMultiItem();
  }
  last->setMultiItem( last->firstMultiItem(), last->prevMultiItem(), e, e);

  if ( mStartMoveInfo && !e->moveInfo() ) {
    e->mStartMoveInfo=new MultiItemInfo( *mStartMoveInfo );
//    e->moveInfo()->mFirstMultiItem = moveInfo()->mFirstMultiItem;
//    e->moveInfo()->mLastMultiItem = moveInfo()->mLastMultiItem;
    e->moveInfo()->mPrevMultiItem = last;
    e->moveInfo()->mNextMultiItem = 0;
  }
  if (last && last->moveInfo()) {
    last->moveInfo()->mNextMultiItem = e;
  }
  return e;
}

KOAgendaItem* KOAgendaItem::removeMoveItem(KOAgendaItem* e)
{
  if (isMultiItem()) {
    KOAgendaItem *first = mMultiItemInfo->mFirstMultiItem;
    KOAgendaItem *next, *prev;
    KOAgendaItem *last = mMultiItemInfo->mLastMultiItem;
    if (!first) first = this;
    if (!last) last = this;
    if ( first==e ) {
      first = first->nextMultiItem();
      first->setMultiItem( 0, 0, first->nextMultiItem(), first->lastMultiItem() );
    }
    if ( last==e ) {
      last=last->prevMultiItem();
      last->setMultiItem( last->firstMultiItem(), last->prevMultiItem(), 0, 0 );
    }

    KOAgendaItem *tmp =  first;
    if ( first==last ) {
      delete mMultiItemInfo;
      tmp = 0;
      mMultiItemInfo = 0;
    }
    while ( tmp ) {
      next = tmp->nextMultiItem();
      prev = tmp->prevMultiItem();
      if ( e==next ) {
        next = next->nextMultiItem();
      }
      if ( e==prev ) {
        prev = prev->prevMultiItem();
      }
      tmp->setMultiItem((tmp==first)?0:first, (tmp==prev)?0:prev, (tmp==next)?0:next, (tmp==last)?0:last);
      tmp = tmp->nextMultiItem();
    }
  }

  return e;
}


void KOAgendaItem::startMove()
{
  KOAgendaItem* first = this;
  if ( isMultiItem() && mMultiItemInfo->mFirstMultiItem ) {
    first=mMultiItemInfo->mFirstMultiItem;
  }
  first->startMovePrivate();
}

void KOAgendaItem::startMovePrivate()
{
  mStartMoveInfo = new MultiItemInfo;
  mStartMoveInfo->mStartCellXLeft = mCellXLeft;
  mStartMoveInfo->mStartCellXRight = mCellXRight;
  mStartMoveInfo->mStartCellYTop = mCellYTop;
  mStartMoveInfo->mStartCellYBottom = mCellYBottom;
  if (mMultiItemInfo) {
    mStartMoveInfo->mFirstMultiItem = mMultiItemInfo->mFirstMultiItem;
    mStartMoveInfo->mLastMultiItem = mMultiItemInfo->mLastMultiItem;
    mStartMoveInfo->mPrevMultiItem = mMultiItemInfo->mPrevMultiItem;
    mStartMoveInfo->mNextMultiItem = mMultiItemInfo->mNextMultiItem;
  } else {
    mStartMoveInfo->mFirstMultiItem = 0;
    mStartMoveInfo->mLastMultiItem = 0;
    mStartMoveInfo->mPrevMultiItem = 0;
    mStartMoveInfo->mNextMultiItem = 0;
  }
  if ( isMultiItem() && mMultiItemInfo->mNextMultiItem )
  {
    mMultiItemInfo->mNextMultiItem->startMovePrivate();
  }
}

void KOAgendaItem::resetMove()
{
  if ( mStartMoveInfo ) {
    if ( mStartMoveInfo->mFirstMultiItem ) {
      mStartMoveInfo->mFirstMultiItem->resetMovePrivate();
    } else {
      resetMovePrivate();
    }
  }
}

void KOAgendaItem::resetMovePrivate()
{
  if (mStartMoveInfo) {
    mCellXLeft = mStartMoveInfo->mStartCellXLeft;
    mCellXRight = mStartMoveInfo->mStartCellXRight;
    mCellYTop = mStartMoveInfo->mStartCellYTop;
    mCellYBottom = mStartMoveInfo->mStartCellYBottom;

    // if we don't have mMultiItemInfo, the item didn't span two days before,
    // and wasn't moved over midnight, either, so we don't have to reset
    // anything. Otherwise, restore from mMoveItemInfo
    if ( mMultiItemInfo ) {
      // It was already a multi-day info
      mMultiItemInfo->mFirstMultiItem = mStartMoveInfo->mFirstMultiItem;
      mMultiItemInfo->mPrevMultiItem = mStartMoveInfo->mPrevMultiItem;
      mMultiItemInfo->mNextMultiItem = mStartMoveInfo->mNextMultiItem;
      mMultiItemInfo->mLastMultiItem = mStartMoveInfo->mLastMultiItem;

      if ( !mStartMoveInfo->mFirstMultiItem ) {
        // This was the first multi-item when the move started, delete all previous
        KOAgendaItem*toDel=mStartMoveInfo->mPrevMultiItem;
        KOAgendaItem*nowDel=0;
        while (toDel) {
          nowDel=toDel;
          if (nowDel->moveInfo()) {
            toDel=nowDel->moveInfo()->mPrevMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mFirstMultiItem = 0;
        mMultiItemInfo->mPrevMultiItem = 0;
      }
      if ( !mStartMoveInfo->mLastMultiItem ) {
        // This was the last multi-item when the move started, delete all next
        KOAgendaItem*toDel=mStartMoveInfo->mNextMultiItem;
        KOAgendaItem*nowDel=0;
        while (toDel) {
          nowDel=toDel;
          if (nowDel->moveInfo()) {
            toDel=nowDel->moveInfo()->mNextMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mLastMultiItem = 0;
        mMultiItemInfo->mNextMultiItem = 0;
      }

      if ( mStartMoveInfo->mFirstMultiItem==0 && mStartMoveInfo->mLastMultiItem==0 ) {
        // it was a single-day event before we started the move.
        delete mMultiItemInfo;
        mMultiItemInfo = 0;
      }
    }
    delete mStartMoveInfo;
    mStartMoveInfo = 0;
  }
  emit showAgendaItem( this );
  if ( nextMultiItem() ) {
    nextMultiItem()->resetMovePrivate();
  }
}

void KOAgendaItem::endMove()
{
  KOAgendaItem*first=firstMultiItem();
  if (!first) first=this;
  first->endMovePrivate();
}

void KOAgendaItem::endMovePrivate()
{
  if ( mStartMoveInfo ) {
    // if first, delete all previous
    if ( !firstMultiItem() || firstMultiItem()==this ) {
      KOAgendaItem*toDel=mStartMoveInfo->mPrevMultiItem;
      KOAgendaItem*nowDel = 0;
      while (toDel) {
        nowDel=toDel;
        if (nowDel->moveInfo()) {
          toDel=nowDel->moveInfo()->mPrevMultiItem;
        }
        emit removeAgendaItem( nowDel );
      }
    }
    // if last, delete all next
    if ( !lastMultiItem() || lastMultiItem()==this ) {
      KOAgendaItem*toDel=mStartMoveInfo->mNextMultiItem;
      KOAgendaItem*nowDel = 0;
      while (toDel) {
        nowDel=toDel;
        if (nowDel->moveInfo()) {
          toDel=nowDel->moveInfo()->mNextMultiItem;
        }
        emit removeAgendaItem( nowDel );
      }
    }
    // also delete the moving info
    delete mStartMoveInfo;
    mStartMoveInfo=0;
    if ( nextMultiItem() )
      nextMultiItem()->endMovePrivate();
  }
}

void KOAgendaItem::moveRelative(int dx, int dy)
{
  int newXLeft = cellXLeft() + dx;
  int newXRight = cellXRight() + dx;
  int newYTop = cellYTop() + dy;
  int newYBottom = cellYBottom() + dy;
  setCellXY(newXLeft,newYTop,newYBottom);
  setCellXRight(newXRight);
}

void KOAgendaItem::expandTop(int dy)
{
  int newYTop = cellYTop() + dy;
  int newYBottom = cellYBottom();
  if (newYTop > newYBottom) newYTop = newYBottom;
  setCellY(newYTop, newYBottom);
}

void KOAgendaItem::expandBottom(int dy)
{
  int newYTop = cellYTop();
  int newYBottom = cellYBottom() + dy;
  if (newYBottom < newYTop) newYBottom = newYTop;
  setCellY(newYTop, newYBottom);
}

void KOAgendaItem::expandLeft(int dx)
{
  int newXLeft = cellXLeft() + dx;
  int newXRight = cellXRight();
  if ( newXLeft > newXRight ) newXLeft = newXRight;
  setCellX( newXLeft, newXRight );
}

void KOAgendaItem::expandRight(int dx)
{
  int newXLeft = cellXLeft();
  int newXRight = cellXRight() + dx;
  if ( newXRight < newXLeft ) newXRight = newXLeft;
  setCellX( newXLeft, newXRight );
}

void KOAgendaItem::dragEnterEvent( QDragEnterEvent *e )
{
#ifndef KORG_NODND
  const QMimeData *md = e->mimeData();
  if ( ICalDrag::canDecode( md ) || VCalDrag::canDecode( md ) ) {
    // TODO: Allow dragging events/todos onto other events to create a relation
    e->ignore();
    return;
  }
  if ( KPIM::KVCardDrag::canDecode( md ) || md->hasText() )
    e->accept();
  else
    e->ignore();
#endif
}

void KOAgendaItem::addAttendee( const QString &newAttendee )
{
  kDebug(5850) <<" Email:" << newAttendee;
  QString name, email;
  KPIMUtils::extractEmailAddressAndName( newAttendee, email, name );
  if ( !( name.isEmpty() && email.isEmpty() ) ) {
      mIncidence->addAttendee(new Attendee(name,email));
    KMessageBox::information( this, i18n("Attendee \"%1\" added to the calendar item \"%2\"", KPIMUtils::normalizedAddress(name, email, QString()), text()), i18n("Attendee added"), "AttendeeDroppedAdded" );
  }

}

void KOAgendaItem::dropEvent( QDropEvent *e )
{
  // TODO: Organize this better: First check for attachment (not only file, also any other url!), then if it's a vcard, otherwise check for attendees, then if the data is binary, add a binary attachment.
#ifndef KORG_NODND
  const QMimeData *md = e->mimeData();

  bool decoded = md->hasText();
  QString text = md->text();
  if( decoded && text.startsWith( "file:" ) ) {
    mIncidence->addAttachment( new Attachment( text ) );
    return;
  }

#ifndef KORG_NOKABC
  KABC::Addressee::List list;

  if ( KPIM::KVCardDrag::fromMimeData( md, list ) ) {
    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      QString em( (*it).fullEmail() );
      if (em.isEmpty()) {
        em=(*it).realName();
      }
      addAttendee( em );
    }
  }
#else
  if( decoded ) {
    kDebug(5850) <<"Dropped :" << text;

    QStringList emails = text.split( ",", QString::SkipEmptyParts  );
    for( QStringList::ConstIterator it = emails.begin(); it != emails.end();
        ++it ) {
        addAttendee( *it );
    }
  }
#endif // KORG_NOKABC

#endif // KORG_NODND
}


QList<KOAgendaItem*> &KOAgendaItem::conflictItems()
{
  return mConflictItems;
}

void KOAgendaItem::setConflictItems( QList<KOAgendaItem*> ci )
{
  mConflictItems = ci;
  QList<KOAgendaItem*>::iterator it;
  for ( it = mConflictItems.begin(); it != mConflictItems.end(); ++it ) {
    (*it)->addConflictItem( this );
  }
}

void KOAgendaItem::addConflictItem( KOAgendaItem *ci )
{
  if ( mConflictItems.contains( ci ) ) mConflictItems.append( ci );
}

QString KOAgendaItem::label() const
{
  return mLabelText;
}

bool KOAgendaItem::overlaps( KOrg::CellItem *o ) const
{
  KOAgendaItem *other = static_cast<KOAgendaItem *>( o );

  if ( cellXLeft() <= other->cellXRight() &&
       cellXRight() >= other->cellXLeft() ) {
    if ( ( cellYTop() <= other->cellYBottom() ) &&
         ( cellYBottom() >= other->cellYTop() ) ) {
      return true;
    }
  }

  return false;
}


static void conditionalPaint( QPainter *p, bool condition, int &x, int ft,
                              const QPixmap &pxmp )
{
  if ( !condition ) return;

  p->drawPixmap( x, ft, pxmp );
  x += pxmp.width() + ft;
}

void KOAgendaItem::paintTodoIcon( QPainter *p, int &x, int ft )
{
  if ( !mIncidence ) return;
  static const QPixmap todoPxmp = KOGlobals::self()->smallIcon("view-calendar-tasks");
  static const QPixmap completedPxmp = KOGlobals::self()->smallIcon("checkedbox");

  if ( mIncidence->type() != "Todo" ) return;

  bool b = ( static_cast<Todo *>( mIncidence ) )->isCompleted();
  conditionalPaint( p, !b, x, ft, todoPxmp );
  conditionalPaint( p, b, x, ft, completedPxmp );
}

void KOAgendaItem::paintIcons( QPainter *p, int &x, int ft )
{
  paintTodoIcon( p, x, ft );
  conditionalPaint( p, mIconAlarm,          x, ft, *alarmPxmp );
  conditionalPaint( p, mIconRecur,          x, ft, *recurPxmp );
  conditionalPaint( p, mIconReadonly,       x, ft, *readonlyPxmp );
  conditionalPaint( p, mIconReply,          x, ft, *replyPxmp );
  conditionalPaint( p, mIconGroup,          x, ft, *groupPxmp );
  conditionalPaint( p, mIconGroupTentative, x, ft, *groupPxmpTentative );
  conditionalPaint( p, mIconOrganizer,      x, ft, *organizerPxmp );
}

void KOAgendaItem::paintEvent( QPaintEvent * )
{
  if ( !mIncidence ) return;

  QPainter p( this );
  p.setRenderHint( QPainter::Antialiasing );
  const int ft = 1; // frame thickness for layout, see drawRoundedRect(),
                    // keep multiple of 2
  const int margin = 5 + ft; // frame + space between frame and content


  // General idea is to always show the icons (even in the all-day events).
  // This creates a consistent feeling for the user when the view mode
  // changes and therefore the available width changes.
  // Also look at #17984

  if ( !alarmPxmp ) {
    alarmPxmp          = new QPixmap( KOGlobals::self()->smallIcon("bell") );
    recurPxmp          = new QPixmap( KOGlobals::self()->smallIcon("recur") );
    readonlyPxmp       = new QPixmap( KOGlobals::self()->smallIcon("readonlyevent") );
    replyPxmp          = new QPixmap( KOGlobals::self()->smallIcon("mail-reply-sender") );
    groupPxmp          = new QPixmap( KOGlobals::self()->smallIcon("groupevent") );
    groupPxmpTentative = new QPixmap( KOGlobals::self()->smallIcon("groupeventtentative") );
    organizerPxmp      = new QPixmap( KOGlobals::self()->smallIcon("organizer") );
  }

  QColor bgColor;

  QStringList categories = mIncidence->categories();
  if ( categories.isEmpty() ) {
    bgColor = KOPrefs::instance()->agendaCalendarItemsEventsBackgroundColor();
    if ( !bgColor.isValid() ) {
      bgColor = KOPrefs::instance()->agendaCalendarItemsBackgroundColor();
    }
  } else {
    bgColor = KOPrefs::instance()->categoryColor( categories.first() );
  }

  if ( mIncidence->type() == "Todo" ) {
    if ( static_cast<Todo*>(mIncidence)->isOverdue() )
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosOverdueBackgroundColor();
    else if ( static_cast<Todo*>(mIncidence)->dtDue().date() ==
              QDateTime::currentDateTime().date() )
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosDueTodayBackgroundColor();
  }

  QColor frameColor;
  if ( KOPrefs::instance()->agendaViewUsesResourceColor()
     && mResourceColor.isValid() ) {
    frameColor = mResourceColor;
  } else {
    frameColor = bgColor;
  }

  QColor textColor = getTextColor(bgColor);
  p.setPen( textColor );

  p.setFont( KOPrefs::instance()->agendaCalendarItemsFont() );
  if ( mIncidence ) {
    if ( mIncidence->type() == "Event" )
      p.setFont( KOPrefs::instance()->agendaCalendarItemsEventsFont() );
    if ( mIncidence->type() == "Todo" )
      p.setFont( KOPrefs::instance()->agendaCalendarItemsToDosFont() );
  }
  QFontMetrics fm = p.fontMetrics();

  int singleLineHeight = fm.boundingRect( mLabelText ).height();

  p.eraseRect( 0, 0, width(), height() );

  bool roundTop = !prevMultiItem();
  bool roundBottom = !nextMultiItem();

  drawRoundedRect( &p, QRect( ft, ft, width() - ft, height() - ft ),
                   mSelected, bgColor, true, ft, roundTop, roundBottom );

  // calculate the height of the full version (case 4) to test whether it is
  // possible

  QString shortH;
  QString longH;
  if ( !isMultiItem() ) {
    shortH = KGlobal::locale()->formatTime(
    mIncidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time());
    if (mIncidence->type() != "Todo")
      longH = i18n("%1 - %2", shortH,
                KGlobal::locale()->formatTime(
                mIncidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time()));
    else
      longH = shortH;
  } else if ( !mMultiItemInfo->mFirstMultiItem ) {
    shortH = KGlobal::locale()->formatTime(
             mIncidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time());
    longH = shortH;
  } else {
    shortH = KGlobal::locale()->formatTime(
             mIncidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time());
    longH = i18n("- %1", shortH);
  }

  KWordWrap *ww = KWordWrap::formatText( fm,
                                         QRect(0, 0, width() - (2 * margin), -1),
                                         0,
                                         mLabelText );
  int th = ww->boundingRect().height();
  delete ww;

  int hlHeight = qMax(fm.boundingRect(longH).height(),
     qMax(alarmPxmp->height(), qMax(recurPxmp->height(),
     qMax(readonlyPxmp->height(), qMax(replyPxmp->height(),
     qMax(groupPxmp->height(), organizerPxmp->height()))))));

  bool completelyRenderable = th < (height() - 2 * ft - 2 - hlHeight);

  // case 1: do not draw text when not even a single line fits
  // Don't do this any more, always try to print out the text. Even if
  // it's just a few pixel, one can still guess the whole text from just four pixels' height!
  if ( //( singleLineHeight > height()-4 ) || // ignore margin, be gentle.. Even ignore 2 pixel outside the item
       ( width() < 16 ) ) {
    int x = margin;
    paintTodoIcon( &p, x, ft );
    return;
  }

  // Used for multi-day events to make sure the summary is on screen
  QRect visRect=visibleRegion().boundingRect();

  // case 2: draw a single line when no more space
  if ( (2 * singleLineHeight) > (height() - 2 * margin) ) {
    int x = margin, txtWidth;

    if ( mIncidence->allDay() ) {
      x += visRect.left();
      paintIcons( &p, x, ft );
      txtWidth = visRect.right() - margin - x;
    }
    else {
      paintIcons( &p, x, ft );
      txtWidth = width() - margin - x;
    }

    int y = ((height() - 2 * ft - singleLineHeight) / 2) + fm.ascent();
    KWordWrap::drawFadeoutText( &p, x, y,
                                txtWidth, mLabelText );
    return;
  }

  // case 3: enough for 2-5 lines, but not for the header.
  //         Also used for the middle days in multi-events
  if ( ((!completelyRenderable) && ((height() - (2 * margin)) <= (5 * singleLineHeight)) ) ||
       (isMultiItem() && mMultiItemInfo->mNextMultiItem && mMultiItemInfo->mFirstMultiItem) ) {
    int x = margin, txtWidth;

    if ( mIncidence->allDay() ) {
      x += visRect.left();
      paintIcons( &p, x, ft );
      txtWidth = visRect.right() - margin - x;
    }
    else {
      paintIcons( &p, x, ft );
      txtWidth = width() - margin - x;
    }

    ww = KWordWrap::formatText( fm,
                                QRect( 0, 0, txtWidth,
                                (height() - (2 * margin)) ),
                                0,
                                mLabelText );

    //kDebug() <<"SIZES for" << mLabelText <<":" << width() <<" ::" << txtWidth;
    ww->drawText( &p, x, margin, Qt::AlignHCenter | KWordWrap::FadeOut );
    delete ww;
    return;
  }

  // case 4: paint everything, with header:
  // consists of (vertically) ft + headline&icons + ft + text + margin
  int y = 2 * ft + hlHeight;
  if ( completelyRenderable )
    y += (height() - (2 * ft) - margin - hlHeight - th) / 2;

  int x = margin, txtWidth, hTxtWidth, eventX;

  if ( mIncidence->allDay() ) {
    shortH = longH = "";

    if ( (mIncidence->type() != "Todo") &&
         (mIncidence->dtStart() != mIncidence->dtEnd()) ) { // multi days
      shortH = longH =
        i18n("%1 - %2",
              KGlobal::locale()->formatDate(mIncidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).date()),
              KGlobal::locale()->formatDate(mIncidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).date()));

      // paint headline
      drawRoundedRect( &p, QRect( ft, ft, width() - ft, - ft + margin + hlHeight), mSelected, bgColor, false, ft, roundTop, false );
    }

    x += visRect.left();
    eventX = x;
    txtWidth = visRect.right() - margin - x;
    paintIcons( &p, x, ft );
    hTxtWidth = visRect.right() - margin - x;
  }
  else {
    // paint headline
     drawRoundedRect( &p, QRect( ft, ft, width() - ft, - ft + margin + hlHeight), mSelected, bgColor, false, ft, roundTop, false );
    txtWidth = width() - margin - x;
    eventX = x;
    paintIcons( &p, x, ft );
    hTxtWidth = width() - margin - x;
  }

  QString headline;
  int hw = fm.boundingRect( longH ).width();
  if ( hw > hTxtWidth ) {
    headline = shortH;
    hw = fm.boundingRect( shortH ).width();
    if ( hw < txtWidth )
      x += (hTxtWidth - hw) / 2;
  } else {
    headline = longH;
    x += (hTxtWidth - hw) / 2;
  }
  p.setBackground( QBrush( frameColor ) );
  p.setPen( getTextColor( frameColor ) );
  KWordWrap::drawFadeoutText( &p, x, (margin + hlHeight + fm.ascent())/2 - 2, hTxtWidth, headline );

  // draw event text
  ww = KWordWrap::formatText( fm,
                              QRect( 0, 0, txtWidth, height() - margin - y ),
                              0,
                              mLabelText );

  p.setBackground( QBrush( bgColor ) );
  p.setPen( textColor );
  QString ws = ww->wrappedString();
  if ( ws.left( ws.length()-1 ).indexOf( '\n' ) >= 0 )
    ww->drawText( &p, eventX, y,
                  Qt::AlignLeft | KWordWrap::FadeOut );
  else
    ww->drawText( &p, eventX + (txtWidth-ww->boundingRect().width()-2*margin)/2,
                  y, Qt::AlignHCenter | KWordWrap::FadeOut );
  delete ww;

}

void KOAgendaItem::drawRoundedRect( QPainter *p, const QRect& rect,
				    bool selected, const QColor& bgColor,
                                    bool frame, int /*ft*/, bool roundTop, bool roundBottom )
{
	QRect r = rect;
	p->save();
	
	QPainterPath path;
	
	bool shrinkWidth = r.width() < 16;
	bool shrinkHeight = r.height() < 16;
	
	qreal rnd = 2.1;
	int sw = shrinkWidth? 7 : 11;
	int sh = shrinkHeight ? 7 : 11;
	QRectF tr( r.x()+r.width()-sw-rnd, r.y()+rnd, sw, sh );
	QRectF tl( r.x()+rnd, r.y()+rnd, sw, sh );
	QRectF bl( r.x()+rnd, r.y()+r.height()-sh-1-rnd, sw, sh );
	QRectF br( r.x()+r.width()-sw-rnd, r.y()+r.height()-sh-1-rnd, sw, sh );
	
	if( roundTop ) {
		path.moveTo( tr.topRight() );
		path.arcTo( tr, 0.0, 90.0 );
		path.lineTo( tl.topRight() );
		path.arcTo( tl, 90.0, 90.0 );
	} else {
		path.moveTo( tr.topRight() );
		path.lineTo( tl.topLeft() );
	}
	
	if( roundBottom ) {
		path.lineTo( bl.topLeft() );
		path.arcTo( bl, 180.0, 90.0 );
		path.lineTo( br.bottomLeft() );
		path.arcTo( br, 270.0, 90.0 );
	} else {
		path.lineTo( bl.bottomLeft() );
		path.lineTo( br.bottomRight() );
	}
	path.closeSubpath();
	
	// header
	if ( !frame ) {
		QLinearGradient gradient( QPointF(r.x(), r.y()), QPointF(r.x(), r.y()+r.height()) );
		
		if( selected ) {
			gradient.setColorAt(0, QColor(0,0,0,40));
			gradient.setColorAt(1, QColor(255,255,255,30));
		} else {
			gradient.setColorAt(0, QColor(255,255,255,70));
			gradient.setColorAt(1, QColor(0,0,0,20));
		}
		
		p->setBrush(gradient);
		p->setPen(Qt::NoPen);
		p->drawPath(path);
		
		p->setRenderHint(QPainter::Antialiasing, false);
		p->setPen(QColor(0,0,0,30));
		p->drawLine(r.x()+3, r.y()+r.height()-4, r.x()+r.width()-4, r.y()+r.height()-4);
		p->setPen(QColor(255,255,255,60));
		p->drawLine(r.x()+3, r.y()+r.height()-3, r.x()+r.width()-4, r.y()+r.height()-3);
		
		p->restore();
		return;
	}
	
	QLinearGradient gradient(QPointF(r.x(), r.y()), QPointF(r.x(), r.y()+r.height()));
	gradient.setColorAt(0, bgColor.light(115));
	if(r.height()-20 > 0) {
		qreal b = (r.height() - 20.0) / r.height();
		gradient.setColorAt(b, bgColor);
	}
	gradient.setColorAt(1, bgColor.dark(110));
	
	p->setBrush(gradient);
	p->setPen(Qt::NoPen);
	p->drawPath(path);
	
	p->setRenderHint(QPainter::Antialiasing, false);
	
	if ( r.width() - 16 > 0 ) {
		
		int x = r.x()+8;
		int x2 = r.x()+r.width()-9;
		int y = r.y();
		
		// drawLine don't draw points
		if ( x == x2 ) {
			x2 += 1;
			p->setClipRect(QRect(x, y, 1, r.height()));
		}
		
		// top lines
		p->setPen(QColor(0,0,0,4));
		p->drawLine(x, y, x2, y);
		p->setPen(QColor(0,0,0,32));
		p->drawLine(x, y+1, x2, y+1);
		p->setPen(QColor(252,252,252,85));
		p->drawLine(x, y+2, x2, y+2);
		p->setPen(QColor(244,244,244,25));
		p->drawLine(x, y+3, x2, y+3);
		p->setPen(QColor(191,191,191,4));
		p->drawLine(x, y+4, x2, y+4);
		
		// bottom lines
		y = r.y()+r.height()-6;
		p->setPen(QColor(255,255,255,3));
		p->drawLine(x, y, x2, y);
		p->setPen(QColor(255,255,255,24));
		p->drawLine(x, y+1, x2, y+1);
		p->setPen(QColor(255,255,255,84));
		p->drawLine(x, y+2, x2, y+2);
		p->setPen(QColor(0,0,0,102));
		p->drawLine(x, y+3, x2, y+3);
		p->setPen(QColor(0,0,0,51));
		p->drawLine(x, y+4, x2, y+4);
		p->setPen(QColor(0,0,0,15));
		p->drawLine(x, y+5, x2, y+5);
		
		p->setClipping(false);
	}
	if ( r.height() - 16 > 0 ) {
		
		int x = r.x();
		int y = r.y()+8;
		int y2 = r.y()+r.height()-9;
		
		if ( y == y2 ) {
			y2 += 1;
			p->setClipRect(QRect(x, y, r.width(), 1));
		}
		
		// left lines
		p->setPen(QColor(0,0,0,14));
		p->drawLine(x, y, x, y2);
		p->setPen(QColor(0,0,0,50));
		p->drawLine(x+1, y, x+1, y2);
		p->setPen(QColor(252,252,252,85));
		p->drawLine(x+2, y, x+2, y2);
		p->setPen(QColor(244,244,244,25));
		p->drawLine(x+2, y, x+2, y2);
		p->setPen(QColor(191,191,191,4));
		p->drawLine(x+3, y, x+3, y2);
		
		// right lines
		x = r.x()+r.width()-5;
		p->setPen(QColor(191,191,191,4));
		p->drawLine(x, y, x, y2);
		p->setPen(QColor(244,244,244,25));
		p->drawLine(x+1, y, x+1, y2);
		p->setPen(QColor(252,252,252,85));
		p->drawLine(x+2, y, x+2, y2);
		p->setPen(QColor(0,0,0,50));
		p->drawLine(x+3, y, x+3, y2);
		p->setPen(QColor(0,0,0,14));
		p->drawLine(x+4, y, x+4, y2);
		
		p->setClipping(false);
	}
	
	// don't overlap the edges
	int lw = shrinkWidth ? r.width()/2 : 8;
	int rw = shrinkWidth ? r.width() - lw : 8;
	int th = shrinkHeight ? r.height()/2 : 8;
	int bh = shrinkHeight ? r.height() - th : 8;
	
	// keep the bottom round for items which ending at 00:15
	if(shrinkHeight && !roundTop && roundBottom && r.height() > 3 ) {
		bh += th-3;
		th = 3;
	}
	
	if ( roundTop ) {
		QImage topLeft(8, 8, QImage::Format_ARGB32);
		topLeft.fill(Qt::transparent);
		QPainter painter(&topLeft);
		
		int y = 0;
		painter.setPen(QColor(0,0,0,2));
		painter.drawPoint(5, y);
		painter.setPen(QColor(0,0,0,2));
		painter.drawPoint(6, y);
		painter.setPen(QColor(0,0,0,3));
		painter.drawPoint(7, y);
		y = 1;
		painter.setPen(QColor(0,0,0,2));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,7));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,16));
		painter.drawPoint(5, y);
		painter.setPen(QColor(0,0,0,23));
		painter.drawPoint(6, y);
		painter.setPen(QColor(0,0,0,29));
		painter.drawPoint(7, y);
		y = 2;
		painter.setPen(QColor(0,0,0,4));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,16));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,37));
		painter.drawPoint(4, y);
		painter.setPen(QColor(179,179,179,91));
		painter.drawPoint(5, y);
		painter.setPen(QColor(224,224,224,100));
		painter.drawPoint(6, y);
		painter.setPen(QColor(252,252,252,94));
		painter.drawPoint(7, y);
		y = 3;
		painter.setPen(QColor(0,0,0,4));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,19));
		painter.drawPoint(2, y);
		painter.setPen(QColor(83,83,83,61));
		painter.drawPoint(3, y);
		painter.setPen(QColor(221,221,221,105));
		painter.drawPoint(4, y);
		painter.setPen(QColor(255,255,255,72));
		painter.drawPoint(5, y);
		painter.setPen(QColor(249,249,249,45));
		painter.drawPoint(6, y);
		painter.setPen(QColor(255,255,255,31));
		painter.drawPoint(7, y);
		y = 4;
		painter.setPen(QColor(0,0,0,3));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,16));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,46));
		painter.drawPoint(2, y);
		painter.setPen(QColor(218,218,218,106));
		painter.drawPoint(3, y);
		painter.setPen(QColor(250,250,250,60));
		painter.drawPoint(4, y);
		painter.setPen(QColor(254,254,254,27));
		painter.drawPoint(5, y);
		painter.setPen(QColor(233,233,233,12));
		painter.drawPoint(6, y);
		painter.setPen(QColor(212,212,212,6));
		painter.drawPoint(7, y);
		y = 5;
		painter.setPen(QColor(0,0,0,9));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,37));
		painter.drawPoint(1, y);
		painter.setPen(QColor(156,156,156,104));
		painter.drawPoint(2, y);
		painter.setPen(QColor(255,255,255,72));
		painter.drawPoint(3, y);
		painter.setPen(QColor(254,254,254,27));
		painter.drawPoint(4, y);
		painter.setPen(QColor(223,223,223,8));
		painter.drawPoint(5, y);
		painter.setPen(QColor(255,255,255,2));
		painter.drawPoint(6, y);
		y = 6;
		painter.setPen(QColor(0,0,0,11));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,46));
		painter.drawPoint(1, y);
		painter.setPen(QColor(205,205,205,109));
		painter.drawPoint(2, y);
		painter.setPen(QColor(249,249,249,45));
		painter.drawPoint(3, y);
		painter.setPen(QColor(255,255,255,12));
		painter.drawPoint(4, y);
		painter.setPen(QColor(255,255,255,2));
		painter.drawPoint(5, y);
		y = 7;
		painter.setPen(QColor(0,0,0,11));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,49));
		painter.drawPoint(1, y);
		painter.setPen(QColor(252,252,252,87));
		painter.drawPoint(2, y);
		painter.setPen(QColor(255,255,255,31));
		painter.drawPoint(3, y);
		painter.setPen(QColor(255,255,255,6));
		painter.drawPoint(4, y);
		
		painter.end();
		
		QImage topRight = topLeft.mirrored(true, false);
		p->drawImage(r.x(), r.y(), topLeft, 0, 0, lw, th);
		p->drawImage(r.x()+r.width()-rw, r.y(), topRight, 8-rw, 0, rw, th);
		
	} else {
		// rectangular
		QImage topLeft(8, 8, QImage::Format_ARGB32);
		topLeft.fill(Qt::transparent);
		QPainter painter(&topLeft);
		
		int y = 0;
		painter.setPen(QColor(0,0,0,2));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,3));
		painter.drawPoint(3, y);
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,4));
		painter.drawLine(5, y, 7, y);
		y = 1;
		painter.setPen(QColor(0,0,0,3));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,10));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,21));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,29));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,31));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,32));
		painter.drawLine(5, y, 7, y);
		y = 2;
		painter.setPen(QColor(0,0,0,11));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,33));
		painter.drawPoint(1, y);
		painter.setPen(QColor(253,253,253,141));
		painter.drawPoint(2, y);
		painter.setPen(QColor(252,252,252,101));
		painter.drawPoint(3, y);
		painter.setPen(QColor(252,252,252,87));
		painter.drawPoint(4, y);
		painter.setPen(QColor(252,252,252,85));
		painter.drawLine(5, y, 7, y);
		y = 3;
		painter.setPen(QColor(0,0,0,10));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,40));
		painter.drawPoint(1, y);
		painter.setPen(QColor(252,252,252,101));
		painter.drawPoint(2, y);
		painter.setPen(QColor(249,249,249,47));
		painter.drawPoint(3, y);
		painter.setPen(QColor(245,245,245,28));
		painter.drawPoint(4, y);
		painter.setPen(QColor(244,244,244,25));
		painter.drawLine(5, y, 7, y);
		y = 4;
		painter.setPen(QColor(0,0,0,13));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,46));
		painter.drawPoint(1, y);
		painter.setPen(QColor(252,252,252,87));
		painter.drawPoint(2, y);
		painter.setPen(QColor(245,245,245,28));
		painter.drawPoint(3, y);
		painter.setPen(QColor(218,218,218,7));
		painter.drawPoint(4, y);
		painter.setPen(QColor(191,191,191,4));
		painter.drawLine(5, y, 7, y);
		y = 5;
		painter.setPen(QColor(0,0,0,14));
		painter.drawLine(0, y, 0, 7);
		painter.setPen(QColor(0,0,0,50));
		painter.drawLine(1, y, 1, 7);
		painter.setPen(QColor(252,252,252,85));
		painter.drawLine(2, y, 2, 7);
		painter.setPen(QColor(244,244,244,25));
		painter.drawLine(3, y, 3, 7);
		painter.setPen(QColor(191,191,191,4));
		painter.drawLine(4, y, 4, 7);
		
		painter.end();
		
		QImage topRight = topLeft.mirrored(true, false);
		p->drawImage(r.x(), r.y(), topLeft, 0, 0, lw, th);
		p->drawImage(r.x()+r.width()-rw, r.y(), topRight, 8-rw, 0, rw, th);
	}
	
	if ( roundBottom ) {
		QImage bottomLeft(8, 8, QImage::Format_ARGB32);
		bottomLeft.fill(Qt::transparent);
		QPainter painter(&bottomLeft);
		
		int y = 0;
		painter.setPen(QColor(0,0,0,13));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,47));
		painter.drawPoint(1, y);
		painter.setPen(QColor(222,222,222,101));
		painter.drawPoint(2, y);
		painter.setPen(QColor(249,249,249,45));
		painter.drawPoint(3, y);
		painter.setPen(QColor(255,255,255,11));
		painter.drawPoint(4, y);
		y =1;
		painter.setPen(QColor(0,0,0,11));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,40));
		painter.drawPoint(1, y);
		painter.setPen(QColor(165,165,165,98));
		painter.drawPoint(2, y);
		painter.setPen(QColor(251,251,251,72));
		painter.drawPoint(3, y);
		painter.setPen(QColor(255,255,255,26));
		painter.drawPoint(4, y);
		painter.setPen(QColor(255,255,255,7));
		painter.drawPoint(5, y);
		y = 2;
		painter.setPen(QColor(0,0,0,7));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,28));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,67));
		painter.drawPoint(2, y);
		painter.setPen(QColor(216,216,216,106));
		painter.drawPoint(3, y);
		painter.setPen(QColor(250,250,250,60));
		painter.drawPoint(4, y);
		painter.setPen(QColor(245,245,245,27));
		painter.drawPoint(5, y);
		painter.setPen(QColor(255,255,255,11));
		painter.drawPoint(6, y);
		painter.setPen(QColor(255,255,255,5));
		painter.drawPoint(7, y);
		y = 3;
		painter.setPen(QColor(0,0,0,4));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,16));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,44));
		painter.drawPoint(2, y);
		painter.setPen(QColor(58,58,58,87));
		painter.drawPoint(3, y);
		painter.setPen(QColor(214,214,214,107));
		painter.drawPoint(4, y);
		painter.setPen(QColor(251,251,251,72));
		painter.drawPoint(5, y);
		painter.setPen(QColor(255,255,255,44));
		painter.drawPoint(6, y);
		painter.setPen(QColor(246,246,246,31));
		painter.drawPoint(7, y);
		y = 4;
		painter.setPen(QColor(0,0,0,7));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,23));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,51));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,83));
		painter.drawPoint(4, y);
		painter.setPen(QColor(149,149,149,108));
		painter.drawPoint(5, y);
		painter.setPen(QColor(211,211,211,105));
		painter.drawPoint(6, y);
		painter.setPen(QColor(255,255,255,93));
		painter.drawPoint(7, y);
		y = 5;
		painter.setPen(QColor(0,0,0,2));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,8));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,23));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,44));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,67));
		painter.drawPoint(5, y);
		painter.setPen(QColor(0,0,0,85));
		painter.drawPoint(6, y);
		painter.setPen(QColor(0,0,0,96));
		painter.drawPoint(7, y);
		y = 6;
		painter.setPen(QColor(0,0,0,2));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,7));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,16));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,28));
		painter.drawPoint(5, y);
		painter.setPen(QColor(0,0,0,40));
		painter.drawPoint(6, y);
		painter.setPen(QColor(0,0,0,47));
		painter.drawPoint(7, y);
		y = 7;
		painter.setPen(QColor(0,0,0,4));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,7));
		painter.drawPoint(5, y);
		painter.setPen(QColor(0,0,0,11));
		painter.drawPoint(6, y);
		painter.setPen(QColor(0,0,0,13));
		painter.drawPoint(7, y);
		
		painter.end();
		
		QImage bottomRight = bottomLeft.mirrored(true, false);
		p->drawImage(r.x(), r.y()+ r.height()-bh, bottomLeft, 0, 8-bh, lw, bh);
		p->drawImage(r.x()+r.width()-rw, r.y()+r.height()-bh, bottomRight, 8-rw, 8-bh, rw, 8);
		
	} else {
		// rectangular
		QImage bottomLeft(8, 8, QImage::Format_ARGB32);
		bottomLeft.fill(Qt::transparent);
		QPainter painter(&bottomLeft);
		
		int y = 0;
		painter.setPen(QColor(0,0,0,14));
		painter.drawLine(0, y, 0, 1);
		painter.setPen(QColor(0,0,0,50));
		painter.drawLine(1, y, 1, 1);
		painter.setPen(QColor(252,252,252,85));
		painter.drawLine(2, y, 2, 1);
		painter.setPen(QColor(244,244,244,25));
		painter.drawLine(3, y, 3, 1);
		painter.setPen(QColor(191,191,191,4));
		painter.drawLine(4, y, 4, 1);
		y = 2;
		painter.setPen(QColor(0,0,0,14));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,50));
		painter.drawPoint(1, y);
		painter.setPen(QColor(252,252,252,87));
		painter.drawPoint(2, y);
		painter.setPen(QColor(254,254,254,27));
		painter.drawPoint(3, y);
		painter.setPen(QColor(255,255,255,6));
		painter.drawPoint(4, y);
		painter.setPen(QColor(255,255,255,3));
		painter.drawLine(5, y, 7, y);
		y = 3;
		painter.setPen(QColor(0,0,0,14));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,50));
		painter.drawPoint(1, y);
		painter.setPen(QColor(252,252,252,101));
		painter.drawPoint(2, y);
		painter.setPen(QColor(255,255,255,46));
		painter.drawPoint(3, y);
		painter.setPen(QColor(254,254,254,27));
		painter.drawPoint(4, y);
		painter.setPen(QColor(255,255,255,24));
		painter.drawLine(5, y, 7, y);
		y = 4;
		painter.setPen(QColor(0,0,0,13));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,46));
		painter.drawPoint(1, y);
		painter.setPen(QColor(253,253,253,141));
		painter.drawPoint(2, y);
		painter.setPen(QColor(255,255,255,100));
		painter.drawPoint(3, y);
		painter.setPen(QColor(255,255,255,86));
		painter.drawPoint(4, y);
		painter.setPen(QColor(255,255,255,84));
		painter.drawLine(5, y, 7, y);
		y = 5;
		painter.setPen(QColor(0,0,0,10));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,34));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,68));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,92));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,100));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,102));
		painter.drawLine(5, y, 7, y);
		y = 6;
		painter.setPen(QColor(0,0,0,5));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,17));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,34));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,46));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,50));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,51));
		painter.drawLine(5, y, 7, y);
		y = 7;
		painter.setPen(QColor(0,0,0,1));
		painter.drawPoint(0, y);
		painter.setPen(QColor(0,0,0,5));
		painter.drawPoint(1, y);
		painter.setPen(QColor(0,0,0,10));
		painter.drawPoint(2, y);
		painter.setPen(QColor(0,0,0,13));
		painter.drawPoint(3, y);
		painter.setPen(QColor(0,0,0,14));
		painter.drawPoint(4, y);
		painter.setPen(QColor(0,0,0,15));
		painter.drawLine(5, y, 7, y);
		
		painter.end();
		
		QImage bottomRight = bottomLeft.mirrored(true, false);
		p->drawImage(r.x(), r.y()+ r.height()-bh, bottomLeft, 0, 8-bh, lw, bh);
		p->drawImage(r.x()+r.width()-rw, r.y()+r.height()-bh, bottomRight, 8-rw, 8-bh, rw, 8);
	}
	
	p->restore();
}

bool KOAgendaItem::event( QEvent *event )
{
  if ( event->type() == QEvent::ToolTip ) {
    if( !KOPrefs::instance()->mEnableToolTips) {
      return true;
    }
  }
  return QWidget::event( event );
}
