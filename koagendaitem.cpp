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
#include <kcal/event.h>
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
#include <QPixmapCache>
#include <QToolTip>

#include "koagendaitem.moc"

//-----------------------------------------------------------------------------

QPixmap *KOAgendaItem::alarmPxmp = 0;
QPixmap *KOAgendaItem::recurPxmp = 0;
QPixmap *KOAgendaItem::readonlyPxmp = 0;
QPixmap *KOAgendaItem::replyPxmp = 0;
QPixmap *KOAgendaItem::groupPxmp = 0;
QPixmap *KOAgendaItem::groupPxmpTent = 0;
QPixmap *KOAgendaItem::organizerPxmp = 0;
QPixmap *KOAgendaItem::eventPxmp = 0;
QPixmap *KOAgendaItem::todoPxmp = 0;
QPixmap *KOAgendaItem::journalPxmp = 0;
QPixmap *KOAgendaItem::completedPxmp = 0;

//-----------------------------------------------------------------------------

KOAgendaItem::KOAgendaItem( Incidence *incidence, const QDate &qd, QWidget *parent )
  : QWidget( parent ), mIncidence( incidence ), mDate( qd ), mValid( true )
{
  if ( !mIncidence ) {
    mValid = false;
    return;
  }
  mLabelText = mIncidence->summary();
  mIconAlarm = false;
  mIconRecur = false;
  mIconReadonly = false;
  mIconReply = false;
  mIconGroup = false;
  mIconGroupTent = false;
  mIconOrganizer = false;
  mMultiItemInfo = 0;
  mStartMoveInfo = 0;

  QPalette pal = palette();
  pal.setColor( QPalette::Window, Qt::transparent );
  setPalette( pal );

  setCellXY( 0, 0, 1 );
  setCellXRight( 0 );
  setMouseTracking( true );
  mResourceColor = QColor();
  updateIcons();

  // select() does nothing if the state hasn't changed, so preset mSelected.
  mSelected = true;
  select( false );

  setAcceptDrops( true );
}

void KOAgendaItem::updateIcons()
{
  if ( !mValid ) {
    return;
  }
  mIconReadonly = mIncidence->isReadOnly();
  mIconRecur = mIncidence->recurs();
  mIconAlarm = mIncidence->isAlarmEnabled();
  if ( mIncidence->attendeeCount() > 1 ) {
    if ( KOPrefs::instance()->thatIsMe( mIncidence->organizer().email() ) ) {
      mIconReply = false;
      mIconGroup = false;
      mIconGroupTent = false;
      mIconOrganizer = true;
    } else {
      Attendee *me = mIncidence->attendeeByMails( KOPrefs::instance()->allEmails() );
      if ( me ) {
        if ( me->status() == Attendee::NeedsAction && me->RSVP() ) {
          mIconReply = true;
          mIconGroup = false;
          mIconGroupTent = false;
          mIconOrganizer = false;
        } else if ( me->status() == Attendee::Tentative ) {
          mIconReply = false;
          mIconGroup = false;
          mIconGroupTent = true;
          mIconOrganizer = false;
        } else {
          mIconReply = false;
          mIconGroup = true;
          mIconGroupTent = false;
          mIconOrganizer = false;
        }
      } else {
        mIconReply = false;
        mIconGroup = true;
        mIconGroupTent = false;
        mIconOrganizer = false;
      }
    }
  }
  update();
}

void KOAgendaItem::select( bool selected )
{
  if ( mSelected == selected ) {
    return;
  }
  mSelected = selected;

  update();
}

bool KOAgendaItem::dissociateFromMultiItem()
{
  if ( !isMultiItem() ) {
    return false;
  }

  KOAgendaItem *firstItem = firstMultiItem();
  if ( firstItem == this ) {
    firstItem = nextMultiItem();
  }

  KOAgendaItem *lastItem = lastMultiItem();
  if ( lastItem == this ) {
    lastItem = prevMultiItem();
  }

  KOAgendaItem *prevItem = prevMultiItem();
  KOAgendaItem *nextItem = nextMultiItem();

  if ( prevItem ) {
    prevItem->setMultiItem( firstItem, prevItem->prevMultiItem(), nextItem, lastItem );
  }
  if ( nextItem ) {
    nextItem->setMultiItem( firstItem, prevItem, nextItem->prevMultiItem(), lastItem );
  }
  delete mMultiItemInfo;
  mMultiItemInfo = 0;
  return true;
}

void KOAgendaItem::setIncidence( Incidence *incidence )
{
  mValid = false;
  if ( incidence ) {
    mValid = true;
    mIncidence = incidence;
    updateIcons();
  }
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

void KOAgendaItem::setMultiItem( KOAgendaItem *first, KOAgendaItem *prev,
                                 KOAgendaItem *next, KOAgendaItem *last )
{
  if ( !mMultiItemInfo ) {
    mMultiItemInfo = new MultiItemInfo;
  }
  mMultiItemInfo->mFirstMultiItem = first;
  mMultiItemInfo->mPrevMultiItem = prev;
  mMultiItemInfo->mNextMultiItem = next;
  mMultiItemInfo->mLastMultiItem = last;
}

bool KOAgendaItem::isMultiItem()
{
  return mMultiItemInfo;
}

KOAgendaItem *KOAgendaItem::prependMoveItem( KOAgendaItem *e )
{
  if ( !e ) {
    return e;
  }

  KOAgendaItem *first = 0, *last = 0;
  if ( isMultiItem() ) {
    first = mMultiItemInfo->mFirstMultiItem;
    last = mMultiItemInfo->mLastMultiItem;
  }
  if ( !first ) {
    first = this;
  }
  if ( !last ) {
    last = this;
  }

  e->setMultiItem( 0, 0, first, last );
  first->setMultiItem( e, e, first->nextMultiItem(), first->lastMultiItem() );

  KOAgendaItem *tmp = first->nextMultiItem();
  while ( tmp ) {
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

  if ( first && first->moveInfo() ) {
    first->moveInfo()->mPrevMultiItem = e;
  }
  return e;
}

KOAgendaItem *KOAgendaItem::appendMoveItem( KOAgendaItem *e )
{
  if ( !e ) {
    return e;
  }

  KOAgendaItem *first = 0, *last = 0;
  if ( isMultiItem() ) {
    first = mMultiItemInfo->mFirstMultiItem;
    last = mMultiItemInfo->mLastMultiItem;
  }
  if ( !first ) {
    first = this;
  }
  if ( !last ) {
    last = this;
  }

  e->setMultiItem( first, last, 0, 0 );
  KOAgendaItem *tmp = first;

  while ( tmp ) {
    tmp->setMultiItem( tmp->firstMultiItem(), tmp->prevMultiItem(), tmp->nextMultiItem(), e );
    tmp = tmp->nextMultiItem();
  }
  last->setMultiItem( last->firstMultiItem(), last->prevMultiItem(), e, e );

  if ( mStartMoveInfo && !e->moveInfo() ) {
    e->mStartMoveInfo=new MultiItemInfo( *mStartMoveInfo );
//    e->moveInfo()->mFirstMultiItem = moveInfo()->mFirstMultiItem;
//    e->moveInfo()->mLastMultiItem = moveInfo()->mLastMultiItem;
    e->moveInfo()->mPrevMultiItem = last;
    e->moveInfo()->mNextMultiItem = 0;
  }
  if ( last && last->moveInfo() ) {
    last->moveInfo()->mNextMultiItem = e;
  }
  return e;
}

KOAgendaItem *KOAgendaItem::removeMoveItem( KOAgendaItem *e )
{
  if ( isMultiItem() ) {
    KOAgendaItem *first = mMultiItemInfo->mFirstMultiItem;
    KOAgendaItem *next, *prev;
    KOAgendaItem *last = mMultiItemInfo->mLastMultiItem;
    if ( !first ) {
      first = this;
    }
    if ( !last ) {
      last = this;
    }
    if ( first == e ) {
      first = first->nextMultiItem();
      first->setMultiItem( 0, 0, first->nextMultiItem(), first->lastMultiItem() );
    }
    if ( last == e ) {
      last = last->prevMultiItem();
      last->setMultiItem( last->firstMultiItem(), last->prevMultiItem(), 0, 0 );
    }

    KOAgendaItem *tmp =  first;
    if ( first == last ) {
      delete mMultiItemInfo;
      tmp = 0;
      mMultiItemInfo = 0;
    }
    while ( tmp ) {
      next = tmp->nextMultiItem();
      prev = tmp->prevMultiItem();
      if ( e == next ) {
        next = next->nextMultiItem();
      }
      if ( e == prev ) {
        prev = prev->prevMultiItem();
      }
      tmp->setMultiItem( ( tmp == first ) ? 0 : first,
                         ( tmp == prev ) ? 0 : prev,
                         ( tmp == next ) ? 0 : next,
                         ( tmp == last ) ? 0 : last );
      tmp = tmp->nextMultiItem();
    }
  }

  return e;
}

void KOAgendaItem::startMove()
{
  KOAgendaItem *first = this;
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
  if ( mMultiItemInfo ) {
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
  if ( isMultiItem() && mMultiItemInfo->mNextMultiItem ) {
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
  if ( mStartMoveInfo ) {
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
        KOAgendaItem *toDel = mStartMoveInfo->mPrevMultiItem;
        KOAgendaItem *nowDel = 0;
        while ( toDel ) {
          nowDel = toDel;
          if ( nowDel->moveInfo() ) {
            toDel = nowDel->moveInfo()->mPrevMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mFirstMultiItem = 0;
        mMultiItemInfo->mPrevMultiItem = 0;
      }
      if ( !mStartMoveInfo->mLastMultiItem ) {
        // This was the last multi-item when the move started, delete all next
        KOAgendaItem *toDel = mStartMoveInfo->mNextMultiItem;
        KOAgendaItem *nowDel = 0;
        while ( toDel ) {
          nowDel = toDel;
          if ( nowDel->moveInfo() ) {
            toDel=nowDel->moveInfo()->mNextMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mLastMultiItem = 0;
        mMultiItemInfo->mNextMultiItem = 0;
      }

      if ( mStartMoveInfo->mFirstMultiItem == 0 && mStartMoveInfo->mLastMultiItem == 0 ) {
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
  KOAgendaItem *first = firstMultiItem();
  if ( !first ) {
    first = this;
  }
  first->endMovePrivate();
}

void KOAgendaItem::endMovePrivate()
{
  if ( mStartMoveInfo ) {
    // if first, delete all previous
    if ( !firstMultiItem() || firstMultiItem() == this ) {
      KOAgendaItem *toDel = mStartMoveInfo->mPrevMultiItem;
      KOAgendaItem *nowDel = 0;
      while ( toDel ) {
        nowDel = toDel;
        if ( nowDel->moveInfo() ) {
          toDel=nowDel->moveInfo()->mPrevMultiItem;
        }
        emit removeAgendaItem( nowDel );
      }
    }
    // if last, delete all next
    if ( !lastMultiItem() || lastMultiItem() == this ) {
      KOAgendaItem *toDel=mStartMoveInfo->mNextMultiItem;
      KOAgendaItem *nowDel = 0;
      while ( toDel ) {
        nowDel = toDel;
        if ( nowDel->moveInfo() ) {
          toDel=nowDel->moveInfo()->mNextMultiItem;
        }
        emit removeAgendaItem( nowDel );
      }
    }
    // also delete the moving info
    delete mStartMoveInfo;
    mStartMoveInfo = 0;
    if ( nextMultiItem() ) {
      nextMultiItem()->endMovePrivate();
    }
  }
}

void KOAgendaItem::moveRelative( int dx, int dy )
{
  int newXLeft = cellXLeft() + dx;
  int newXRight = cellXRight() + dx;
  int newYTop = cellYTop() + dy;
  int newYBottom = cellYBottom() + dy;
  setCellXY( newXLeft, newYTop, newYBottom );
  setCellXRight( newXRight );
}

void KOAgendaItem::expandTop( int dy, const bool allowOverLimit )
{
  int newYTop = cellYTop() + dy;
  int newYBottom = cellYBottom();
  if ( newYTop > newYBottom && !allowOverLimit ) {
    newYTop = newYBottom;
  }
  setCellY( newYTop, newYBottom );
}

void KOAgendaItem::expandBottom( int dy )
{
  int newYTop = cellYTop();
  int newYBottom = cellYBottom() + dy;
  if ( newYBottom < newYTop ) {
    newYBottom = newYTop;
  }
  setCellY( newYTop, newYBottom );
}

void KOAgendaItem::expandLeft( int dx )
{
  int newXLeft = cellXLeft() + dx;
  int newXRight = cellXRight();
  if ( newXLeft > newXRight ) {
    newXLeft = newXRight;
  }
  setCellX( newXLeft, newXRight );
}

void KOAgendaItem::expandRight( int dx )
{
  int newXLeft = cellXLeft();
  int newXRight = cellXRight() + dx;
  if ( newXRight < newXLeft ) {
    newXRight = newXLeft;
  }
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
  if ( KPIM::KVCardDrag::canDecode( md ) || md->hasText() ) {
    e->accept();
  } else {
    e->ignore();
  }
#endif
}

void KOAgendaItem::addAttendee( const QString &newAttendee )
{
  if ( !mValid ) {
    return;
  }

  QString name, email;
  KPIMUtils::extractEmailAddressAndName( newAttendee, email, name );
  if ( !( name.isEmpty() && email.isEmpty() ) ) {
    mIncidence->addAttendee( new Attendee( name, email ) );
    KMessageBox::information(
      this,
      i18n( "Attendee \"%1\" added to the calendar item \"%2\"",
            KPIMUtils::normalizedAddress( name, email, QString() ), text() ),
      i18n( "Attendee added" ), "AttendeeDroppedAdded" );
  }
}

void KOAgendaItem::dropEvent( QDropEvent *e )
{
  // TODO: Organize this better: First check for attachment
  // (not only file, also any other url!), then if it's a vcard,
  // otherwise check for attendees, then if the data is binary,
  // add a binary attachment.
#ifndef KORG_NODND
  if ( !mValid ) {
    return;
  }

  const QMimeData *md = e->mimeData();

  bool decoded = md->hasText();
  QString text = md->text();
  if ( decoded && text.startsWith( QLatin1String( "file:" ) ) ) {
    mIncidence->addAttachment( new Attachment( text ) );
    return;
  }

#ifndef KORG_NOKABC
  KABC::Addressee::List list;

  if ( KPIM::KVCardDrag::fromMimeData( md, list ) ) {
    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      QString em( (*it).fullEmail() );
      if ( em.isEmpty() ) {
        em = (*it).realName();
      }
      addAttendee( em );
    }
  }
#else
  if( decoded ) {
    QStringList emails = text.split( ",", QString::SkipEmptyParts );
    for ( QStringList::ConstIterator it = emails.begin(); it != emails.end(); ++it ) {
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
  if ( !mConflictItems.contains( ci ) ) {
    mConflictItems.append( ci );
  }
}

QString KOAgendaItem::label() const
{
  return mLabelText;
}

bool KOAgendaItem::overlaps( KOrg::CellItem *o ) const
{
  KOAgendaItem *other = static_cast<KOAgendaItem *>( o );

  if ( cellXLeft() <= other->cellXRight() && cellXRight() >= other->cellXLeft() ) {
    if ( ( cellYTop() <= other->cellYBottom() ) && ( cellYBottom() >= other->cellYTop() ) ) {
      return true;
    }
  }

  return false;
}

static void conditionalPaint( QPainter *p, bool condition, int &x, int y,
                              int ft, const QPixmap &pxmp )
{
  if ( !condition ) {
    return;
  }

  p->drawPixmap( x, y, pxmp );
  x += pxmp.width() + ft;
}

void KOAgendaItem::paintEventIcon( QPainter *p, int &x, int y, int ft )
{
  if ( !mValid ) {
    return;
  }
  conditionalPaint( p, mIncidence->type() == "Event", x, y, ft, *eventPxmp );
}

void KOAgendaItem::paintTodoIcon( QPainter *p, int &x, int y, int ft )
{
  if ( !mValid || mIncidence->type() != "Todo" ) {
    return;
  }

  bool b = ( static_cast<Todo *>( mIncidence ) )->isCompleted();
  conditionalPaint( p, !b, x, y, ft, *todoPxmp );
  conditionalPaint( p, b, x, y, ft, *completedPxmp );
}

void KOAgendaItem::paintJournalIcon( QPainter *p, int &x, int y, int ft )
{
  if ( !mValid ) {
    return;
  }
  conditionalPaint( p, mIncidence->type() == "Journal", x, y, ft, *journalPxmp );
}

void KOAgendaItem::paintIcons( QPainter *p, int &x, int y, int ft )
{
  paintEventIcon( p, x, y, ft );
  paintTodoIcon( p, x, y, ft );
  paintJournalIcon( p, x, y, ft );
  conditionalPaint( p, mIconAlarm, x, y, ft, *alarmPxmp );
  conditionalPaint( p, mIconRecur, x, y, ft, *recurPxmp );
  conditionalPaint( p, mIconReadonly, x, y, ft, *readonlyPxmp );
  conditionalPaint( p, mIconReply, x, y, ft, *replyPxmp );
  conditionalPaint( p, mIconGroup, x, y, ft, *groupPxmp );
  conditionalPaint( p, mIconGroupTent, x, y, ft, *groupPxmpTent );
  conditionalPaint( p, mIconOrganizer, x, y, ft, *organizerPxmp );
}

void KOAgendaItem::paintEvent( QPaintEvent *ev )
{
  if ( !mValid ) {
    return;
  }

  QRect visRect = visibleRegion().boundingRect();
  // when scrolling horizontally in the side-by-side view, the repainted area is clipped
  // to the newly visible area, which is a problem since the content changes when visRect
  // changes, so repaint the full item in that case
  if ( ev->rect() != visRect && visRect.isValid() && ev->rect().isValid() ) {
    update( visRect );
    return;
  }

  QPainter p( this );
  p.setRenderHint( QPainter::Antialiasing );
  const int fmargin = 0; // frame margin
  const int ft = 1; // frame thickness for layout, see drawRoundedRect(),
                    // keep multiple of 2
  const int margin = 5 + ft + fmargin ; // frame + space between frame and content

  // General idea is to always show the icons (even in the all-day events).
  // This creates a consistent feeling for the user when the view mode
  // changes and therefore the available width changes.
  // Also look at #17984

  if ( !alarmPxmp ) {
    alarmPxmp     = new QPixmap( KOGlobals::self()->smallIcon( "task-reminder" ) );
    recurPxmp     = new QPixmap( KOGlobals::self()->smallIcon( "appointment-recurring" ) );
    readonlyPxmp  = new QPixmap( KOGlobals::self()->smallIcon( "object-locked" ) );
    replyPxmp     = new QPixmap( KOGlobals::self()->smallIcon( "mail-reply-sender" ) );
    groupPxmp     = new QPixmap( KOGlobals::self()->smallIcon( "meeting-attending" ) );
    groupPxmpTent = new QPixmap( KOGlobals::self()->smallIcon( "meeting-attending-tentative" ) );
    organizerPxmp = new QPixmap( KOGlobals::self()->smallIcon( "meeting-organizer" ) );
    eventPxmp     = new QPixmap( KOGlobals::self()->smallIcon( "view-calendar-day" ) );
    todoPxmp      = new QPixmap( KOGlobals::self()->smallIcon( "view-calendar-tasks" ) );
    completedPxmp = new QPixmap( KOGlobals::self()->smallIcon( "task-complete" ) );
    journalPxmp   = new QPixmap( KOGlobals::self()->smallIcon( "view-pim-journal" ) );
  }

  QColor bgColor;

  if ( mIncidence->type() == "Todo" && !KOPrefs::instance()->todosUseCategoryColors() ) {
    if ( static_cast<Todo*>( mIncidence )->isOverdue() ) {
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosOverdueBackgroundColor();
    } else if ( static_cast<Todo*>( mIncidence )->dtDue().date() ==
                QDateTime::currentDateTime().date() ) {
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosDueTodayBackgroundColor();
    }
  }

  QColor categoryColor;
  QStringList categories = mIncidence->categories();
  QString cat;
  if ( !categories.isEmpty() ) {
    cat = categories.first();
  }
  if ( cat.isEmpty() ) {
    categoryColor = KOPrefs::instance()->defaultCategoryColor();
  } else {
    categoryColor = KOPrefs::instance()->categoryColor( cat );
  }

  QColor resourceColor = mResourceColor;
  if ( !resourceColor.isValid() ) {
    resourceColor = categoryColor;
  }

  QColor frameColor;
  if ( KOPrefs::instance()->agendaViewColors() == KOPrefs::ResourceOnly ||
       KOPrefs::instance()->agendaViewColors() == KOPrefs::CategoryInsideResourceOutside ) {
    frameColor = bgColor.isValid() ? bgColor : resourceColor;
  } else {
    frameColor = bgColor.isValid() ? bgColor : categoryColor;
  }

  if ( !bgColor.isValid() ) {
    if ( KOPrefs::instance()->agendaViewColors() == KOPrefs::ResourceOnly ||
         KOPrefs::instance()->agendaViewColors() == KOPrefs::ResourceInsideCategoryOutside ) {
      bgColor = resourceColor;
    } else {
      bgColor = categoryColor;
    }
  }

  if ( mSelected ) {
    frameColor = QColor( 85 + frameColor.red() * 2 / 3,
                         85 + frameColor.green() * 2 / 3,
                         85 + frameColor.blue() * 2 / 3 );
  } else {
    frameColor = frameColor.dark( 115 );
  }

  if ( !KOPrefs::instance()->hasCategoryColor( cat ) ) {
    categoryColor = resourceColor;
  }

  if ( !bgColor.isValid() ) {
    bgColor = categoryColor;
  }

  if ( mSelected ) {
    bgColor = bgColor.light( 110 ); // keep this in sync with month view
  }

  QColor textColor = getTextColor( bgColor );
  p.setPen( textColor );

  p.setFont( KOPrefs::instance()->agendaViewFont() );
  QFontMetrics fm = p.fontMetrics();

  int singleLineHeight = fm.boundingRect( mLabelText ).height();

  bool roundTop = !prevMultiItem();
  bool roundBottom = !nextMultiItem();

  drawRoundedRect( &p, QRect( fmargin, fmargin, width() - fmargin * 2, height() - fmargin * 2 ),
                   mSelected, bgColor, true, ft, roundTop, roundBottom );

  // calculate the height of the full version (case 4) to test whether it is
  // possible

  QString shortH;
  QString longH;
  if ( !isMultiItem() ) {
    shortH = KGlobal::locale()->formatTime(
      mIncidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    if ( mIncidence->type() != "Todo" ) {
      longH = i18n( "%1 - %2",
                    shortH,
                    KGlobal::locale()->formatTime(
                      mIncidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() ) );
    } else {
      longH = shortH;
    }
  } else if ( !mMultiItemInfo->mFirstMultiItem ) {
    shortH = KGlobal::locale()->formatTime(
      mIncidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    longH = shortH;
  } else {
    shortH = KGlobal::locale()->formatTime(
      mIncidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    longH = i18n( "- %1", shortH );
  }

  KWordWrap *ww = KWordWrap::formatText(
    fm, QRect( 0, 0, width() - ( 2 * margin ), -1 ), 0, mLabelText );
  int th = ww->boundingRect().height();
  delete ww;

  int hlHeight = qMax( fm.boundingRect( longH ).height(),
                       qMax( alarmPxmp->height(),
                            qMax( recurPxmp->height(),
                                 qMax( readonlyPxmp->height(),
                                      qMax( replyPxmp->height(),
                                           qMax( groupPxmp->height(),
                                                 organizerPxmp->height() ) ) ) ) ) );

  bool completelyRenderable = th < ( height() - 2 * ft - 2 - hlHeight );

  // case 1: do not draw text when not even a single line fits
  // Don't do this any more, always try to print out the text.
  // Even if it's just a few pixel, one can still guess the whole
  // text from just four pixels' height!
  if ( //( singleLineHeight > height() - 4 ) ||
       ( width() < 16 ) ) {
    int x = qRound( ( width() - 16 ) / 2.0 );
    paintTodoIcon( &p, x, margin, ft );
    return;
  }

  // case 2: draw a single line when no more space
  if ( ( 2 * singleLineHeight ) > ( height() - 2 * margin ) ) {
    int x = margin, txtWidth;

    if ( mIncidence->allDay() ) {
      x += visRect.left();
      int y =  qRound( ( height() - 16 ) / 2.0 );
      paintIcons( &p, x, y, ft );
      txtWidth = visRect.right() - margin - x;
    } else {
      int y =  qRound( ( height() - 16 ) / 2.0 );
      paintIcons( &p, x, y, ft );
      txtWidth = width() - margin - x;
    }

    int y = ( ( height() - singleLineHeight ) / 2 ) + fm.ascent();
    KWordWrap::drawFadeoutText( &p, x, y, txtWidth, mLabelText );
    return;
  }

  // case 3: enough for 2-5 lines, but not for the header.
  //         Also used for the middle days in multi-events
  if ( ( ( !completelyRenderable ) &&
         ( ( height() - ( 2 * margin ) ) <= ( 5 * singleLineHeight ) ) ) ||
       ( isMultiItem() && mMultiItemInfo->mNextMultiItem && mMultiItemInfo->mFirstMultiItem ) ) {
    int x = margin, txtWidth;

    if ( mIncidence->allDay() ) {
      x += visRect.left();
      paintIcons( &p, x, margin, ft );
      txtWidth = visRect.right() - margin - x;
    } else {
      paintIcons( &p, x, margin, ft );
      txtWidth = width() - margin - x;
    }

    ww = KWordWrap::formatText(
      fm, QRect( 0, 0, txtWidth, ( height() - ( 2 * margin ) ) ), 0, mLabelText );

    ww->drawText( &p, x, margin, Qt::AlignHCenter | KWordWrap::FadeOut );
    delete ww;
    return;
  }

  // case 4: paint everything, with header:
  // consists of (vertically) ft + headline&icons + ft + text + margin
  int y = 2 * ft + hlHeight;
  if ( completelyRenderable ) {
    y += ( height() - ( 2 * ft ) - margin - hlHeight - th ) / 2;
  }

  int x = margin, txtWidth, hTxtWidth, eventX;

  if ( mIncidence->allDay() ) {
    shortH = longH = "";
    if ( mIncidence->type() == "Event" ) {
      if ( static_cast<Event*>( mIncidence )->isMultiDay( KOPrefs::instance()->timeSpec() ) ) {
        // multi-day, all-day event
        shortH =
          i18n( "%1 - %2",
                KGlobal::locale()->formatDate(
                  mIncidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).date() ),
                KGlobal::locale()->formatDate(
                  mIncidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).date() ) );
        longH = shortH;

        // paint headline
        drawRoundedRect(
          &p,
          QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
          mSelected, frameColor, false, ft, roundTop, false );
      } else {
        // single-day, all-day event

        // paint headline
        drawRoundedRect(
          &p,
          QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
          mSelected, frameColor, false, ft, roundTop, false );
      }
    } else {
      // to-do

      // paint headline
      drawRoundedRect(
        &p,
        QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
        mSelected, frameColor, false, ft, roundTop, false );
    }

    x += visRect.left();
    eventX = x;
    txtWidth = visRect.right() - margin - x;
    paintIcons( &p, x, margin, ft );
    hTxtWidth = visRect.right() - margin - x;
  } else {
    // paint headline
     drawRoundedRect(
       &p,
       QRect( fmargin, fmargin, width() - fmargin * 2, - fmargin * 2 + margin + hlHeight ),
       mSelected, frameColor, false, ft, roundTop, false );

    txtWidth = width() - margin - x;
    eventX = x;
    paintIcons( &p, x, margin / 2, ft );
    hTxtWidth = width() - margin - x;
  }

  QString headline;
  int hw = fm.boundingRect( longH ).width();
  if ( hw > hTxtWidth ) {
    headline = shortH;
    hw = fm.boundingRect( shortH ).width();
    if ( hw < txtWidth ) {
      x += ( hTxtWidth - hw ) / 2;
    }
  } else {
    headline = longH;
    x += ( hTxtWidth - hw ) / 2;
  }
  p.setBackground( QBrush( frameColor ) );
  p.setPen( getTextColor( frameColor ) );
  KWordWrap::drawFadeoutText( &p, x, ( margin + hlHeight + fm.ascent() ) / 2 - 2,
                              hTxtWidth, headline );

  // draw event text
  ww = KWordWrap::formatText(
    fm, QRect( 0, 0, txtWidth, height() - margin - y ), 0, mLabelText );

  p.setBackground( QBrush( bgColor ) );
  p.setPen( textColor );
  QString ws = ww->wrappedString();
  if ( ws.left( ws.length()-1 ).indexOf( '\n' ) >= 0 ) {
    ww->drawText( &p, eventX, y, Qt::AlignLeft | KWordWrap::FadeOut );
  } else {
    ww->drawText( &p, eventX + ( txtWidth - ww->boundingRect().width() - 2 * margin ) / 2, y,
                  Qt::AlignHCenter | KWordWrap::FadeOut );
  }
  delete ww;

}

void KOAgendaItem::drawRoundedRect( QPainter *p, const QRect &rect,
                                    bool selected, const QColor &bgColor,
                                    bool frame, int ft, bool roundTop,
                                    bool roundBottom )
{
  Q_UNUSED( ft );
  if ( !mValid ) {
    return;
  }

  QRect r = rect;
  r.adjust( 0, 0, 1, 1 );

  p->save();

  QPainterPath path;

  bool shrinkWidth = r.width() < 16;
  bool shrinkHeight = r.height() < 16;

  qreal rnd = 2.1;
  int sw = shrinkWidth ? 10 : 11;
  int sh = shrinkHeight ? 10 : 11;
  QRectF tr( r.x() + r.width() - sw - rnd, r.y() + rnd, sw, sh );
  QRectF tl( r.x() + rnd, r.y() + rnd, sw, sh );
  QRectF bl( r.x() + rnd, r.y() + r.height() - sh - 1 - rnd, sw, sh );
  QRectF br( r.x() + r.width() - sw - rnd, r.y() + r.height() - sh - 1 - rnd, sw, sh );

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
    QLinearGradient gradient( QPointF( r.x(), r.y() ), QPointF( r.x(), r.height() ) );

    if ( selected ) {
      QColor top = bgColor.dark( 250 );
      top.setAlpha( 40 );
      gradient.setColorAt( 0, top );
      gradient.setColorAt( 1, QColor( 255, 255, 255, 30 ) );
    } else {
      gradient.setColorAt( 0, QColor( 255, 255, 255, 90 ) );
      gradient.setColorAt( 1, QColor( 0, 0, 0, 10 ) );
    }

    p->setBrush( bgColor );
    p->setPen( Qt::NoPen );
    p->drawPath( path );

    p->setBrush( gradient );
    p->setPen( Qt::NoPen );
    p->drawPath( path );

    QPixmap separator;
    QString key( "ko_hsep" );
    if ( !QPixmapCache::find( key, separator ) ) {
      separator = QPixmap( ":/headerSeparator.png" );
      QPixmapCache::insert( key, separator );
    }
    p->fillRect( QRect( r.x() + 3, r.y() + r.height() - 2, r.x() + r.width() - 4, 2 ),
                 QBrush( separator ) );

    p->restore();
    return;
  }

  QLinearGradient gradient( QPointF( r.x(), r.y() ), QPointF( r.x(), r.height() ) );

  if ( r.height() > 50 ) {
    if ( mIncidence->allDay() &&
         mIncidence->dtStart() == mIncidence->dtEnd() &&
         mIncidence->type() != "Todo" ) {
      gradient.setColorAt( 0, bgColor.light( 130 ) );
      qreal t = 1.0 - ( r.height() - 18.0 ) / r.height();
      gradient.setColorAt( t, bgColor.light( 115 ) );
      qreal b = ( r.height() - 20.0 ) / r.height();
      gradient.setColorAt( b, bgColor );
    } else {
      gradient.setColorAt( 0, bgColor.light( 115 ) );
      qreal b = ( r.height() - 20.0 ) / r.height();
      gradient.setColorAt( b, bgColor );
    }
    gradient.setColorAt( 1, bgColor.dark( 110 ) );
  } else {
    if ( mIncidence->allDay() &&
         mIncidence->dtStart() == mIncidence->dtEnd() &&
         mIncidence->type() != "Todo" ) {
      gradient.setColorAt( 0, bgColor.light( 130 ) );
      gradient.setColorAt( 0.35, bgColor.light( 115 ) );
      gradient.setColorAt( 0.65, bgColor );
    } else {
      gradient.setColorAt( 0, bgColor.light( 115 ) );
      gradient.setColorAt( 0.65, bgColor );
    }
    gradient.setColorAt( 1, bgColor.dark( 110 ) );
  }

  p->setBrush( gradient );
  p->setPen( Qt::NoPen );
  p->drawPath( path );

  p->setRenderHint( QPainter::Antialiasing, false );

  if ( r.width() - 16 > 0 ) {
    QPixmap topLines;
    QString key( "ko_t" );
    if ( !QPixmapCache::find( key, topLines ) ) {
      topLines = QPixmap( ":/topLines.png" );
      QPixmapCache::insert( key, topLines );
    }
    p->setBrushOrigin( r.x() + 8, r.y() );
    p->fillRect( QRect( r.x() + 8, r.y(), r.width() - 16, 5 ),
                 QBrush( topLines ) );

    QPixmap bottomLines;
    key = QString( "ko_b" );
    if ( !QPixmapCache::find( key, bottomLines ) ) {
      bottomLines = QPixmap( ":/bottomLines.png" );
      QPixmapCache::insert( key, bottomLines );
    }
    p->setBrushOrigin( r.x() + 8, r.y() + r.height() - 6 );
    p->fillRect( QRect( r.x() + 8, r.y() + r.height() - 6, r.width() - 16, 6 ),
                 QBrush( bottomLines ) );

  }

  if ( r.height() - 16 > 0 ) {

    QPixmap leftLines;
    QString key( "ko_l" );
    if ( !QPixmapCache::find( key, leftLines ) ) {
      leftLines = QPixmap( ":/leftLines.png" );
      QPixmapCache::insert( key, leftLines );
    }
    p->setBrushOrigin( r.x(), r.y() + 8 );
    p->fillRect( QRect( r.x(), r.y() + 8, 5, r.height() - 16 ),
                 QBrush( leftLines ) );

    QPixmap rightLines;
    key = QString( "ko_r" );
    if ( !QPixmapCache::find( key, rightLines ) ) {
      rightLines = QPixmap( ":/rightLines.png" );
      QPixmapCache::insert( key, rightLines );
    }
    p->setBrushOrigin( r.x() + r.width() - 5, r.y() + 8 );
    p->fillRect( QRect( r.x() + r.width() - 5, r.y() + 8, 5, r.height() - 16 ),
                 QBrush( rightLines ) );
  }

  // don't overlap the edges
  int lw = shrinkWidth ? r.width() / 2 : 8;
  int rw = shrinkWidth ? r.width() - lw : 8;
  int th = shrinkHeight ? r.height() / 2 : 8;
  int bh = shrinkHeight ? r.height() - th : 8;

  // keep the bottom round for items which ending at 00:15
  if( shrinkHeight && !roundTop && roundBottom && r.height() > 3 ) {
    bh += th - 3;
    th = 3;
  }

  QPixmap topLeft;
  QString key = roundTop ? QString( "ko_tl" ) : QString( "ko_rtl" );
  if ( !QPixmapCache::find( key, topLeft ) ) {
    topLeft = roundTop ? QPixmap( ":/roundTopLeft.png" ) : QPixmap( ":/rectangularTopLeft.png" );
    QPixmapCache::insert( key, topLeft );
  }
  p->drawPixmap( r.x(), r.y(), topLeft, 0, 0, lw, th );

  QPixmap topRight;
  key = roundTop ? QString( "ko_tr" ) : QString( "ko_rtr" );
  if ( !QPixmapCache::find( key, topRight ) ) {
    topRight = roundTop ? QPixmap( ":/roundTopRight.png" ) : QPixmap( ":/rectangularTopRight.png" );
    QPixmapCache::insert( key, topRight );
  }
  p->drawPixmap( r.x() + r.width() - rw, r.y(), topRight, 8 - rw, 0, rw, th );

  QPixmap bottomLeft;
  key = roundBottom ? QString( "ko_bl" ) : QString( "ko_rbl" );
  if ( !QPixmapCache::find( key, bottomLeft ) ) {
    bottomLeft = roundBottom ? QPixmap( ":/roundBottomLeft.png" ) :
                 QPixmap( ":/rectangularBottomLeft.png" );
    QPixmapCache::insert( key, bottomLeft );
  }
  p->drawPixmap( r.x(), r.y() + r.height() - bh, bottomLeft, 0, 8 - bh, lw, bh );

  QPixmap bottomRight;
  key = roundBottom ? QString( "ko_br" ) : QString( "ko_rbr" );
  if ( !QPixmapCache::find( key, bottomRight ) ) {
    bottomRight = roundBottom ? QPixmap( ":/roundBottomRight.png" ) :
                  QPixmap( ":/rectangularBottomRight.png" );
    QPixmapCache::insert( key, bottomRight );
  }
  p->drawPixmap( r.x() + r.width() - rw, r.y() + r.height() - bh, bottomRight,
                 8 - rw, 8 - bh, rw, 8 );

  p->restore();
}

bool KOAgendaItem::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::Paint ) {
    return mValid;
  } else {
    // standard event processing
    return QObject::eventFilter( obj, event );
  }
}

bool KOAgendaItem::event( QEvent *event )
{
  if ( event->type() == QEvent::ToolTip ) {
    if( !KOPrefs::instance()->mEnableToolTips ) {
      return true;
    } else {
      QHelpEvent *helpEvent = static_cast<QHelpEvent*>( event );
      QToolTip::showText( helpEvent->globalPos(),
                          IncidenceFormatter::toolTipString( mIncidence ),
                          this );
    }
  }
  return QWidget::event( event );
}
