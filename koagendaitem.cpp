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
  setAttribute( Qt::WA_NoSystemBackground, true );

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
  if ( KVCardDrag::canDecode( md ) || md->hasText() )
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

  if ( KVCardDrag::fromMimeData( md, list ) ) {
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
  static const QPixmap todoPxmp = KOGlobals::self()->smallIcon("todo");
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
  const int ft = 2; // frame thickness for layout, see drawRoundedRect(),
                    // keep multiple of 2
  const int margin = 1 + ft; // frame + space between frame and content

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

  QColor bgColor = KOPrefs::instance()->agendaCalendarItemsEventsBackgroundColor();
  if ( mIncidence->type() == "Todo" ) {
    if ( static_cast<Todo*>(mIncidence)->isOverdue() )
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosOverdueBackgroundColor();
    else if ( static_cast<Todo*>(mIncidence)->dtDue().date() ==
              QDateTime::currentDateTime().date() )
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosDueTodayBackgroundColor();
  }

  if ( !bgColor.isValid() ) {
    kDebug() <<"invalid bgColor:" << bgColor.name();
    QStringList categories = mIncidence->categories();
    QString cat;
    if ( !categories.isEmpty() ) cat = categories.first();
    if (cat.isEmpty())
      bgColor = KOPrefs::instance()->agendaCalendarItemsBackgroundColor();
    else
      bgColor = KOPrefs::instance()->categoryColor(cat);
  }
  QColor frameColor;
  if ( KOPrefs::instance()->agendaViewUsesResourceColor()
    && mResourceColor.isValid() ) {
     frameColor = mSelected ? QColor( 85 + mResourceColor.red() * 2/3,
                                      85 + mResourceColor.green() * 2/3,
                                      85 + mResourceColor.blue() * 2/3 )
                                : mResourceColor;
  } else {
    frameColor = mSelected ? QColor( 85 + bgColor.red() * 2/3,
                                     85 + bgColor.green() * 2/3,
                                     85 + bgColor.blue() * 2/3 )
                                : bgColor.dark(115);
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
		   frameColor, bgColor, true, ft, roundTop, roundBottom );

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

    if ( mIncidence->floats() ) {
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

    if ( mIncidence->floats() ) {
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

  if ( mIncidence->floats() ) {
    shortH = longH = "";

    if ( (mIncidence->type() != "Todo") &&
         (mIncidence->dtStart() != mIncidence->dtEnd()) ) { // multi days
      shortH = longH =
        i18n("%1 - %2",
              KGlobal::locale()->formatDate(mIncidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).date()),
              KGlobal::locale()->formatDate(mIncidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).date()));

      // paint headline
      drawRoundedRect( &p, QRect( ft/2, ft/2, width(), - ft/2 + margin + hlHeight), frameColor, frameColor, false, ft, roundTop, false );
    }

    x += visRect.left();
    eventX = x;
    txtWidth = visRect.right() - margin - x;
    paintIcons( &p, x, ft );
    hTxtWidth = visRect.right() - margin - x;
  }
  else {
    // paint headline
    drawRoundedRect( &p, QRect( ft/2, ft/2, width(), - ft/2 + margin + hlHeight), frameColor, frameColor, false, ft, roundTop, false );
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
  KWordWrap::drawFadeoutText( &p, x, ft + fm.ascent(), hTxtWidth, headline );

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

  setMask( QRegion( roundedRect( QRect( ft/2, ft/2, width(), height() ), 
				 roundTop, roundBottom ).toFillPolygon().toPolygon() ) );
}

QPainterPath KOAgendaItem::roundedRect( const QRect& rect, bool roundTop, bool roundBottom )
{
  int xRnd = 10; 
  int yRnd = 10;
  int xDiam = 2 * xRnd;
  int yDiam = 2 * yRnd;

  int x1, y1, x2, y2;
  rect.getCoords( &x1, &y1, &x2, &y2 );

  QPainterPath path;
  if ( roundTop ) {
    path.moveTo( x2, y1 + yRnd );
    path.arcTo( QRect( x2 - xDiam, y1, xDiam, yDiam ), 0, +90.0 );
    path.lineTo( x1 + xRnd, y1 );
    path.arcTo( QRect( x1, y1, xDiam, yDiam ), 90.0, +90.0 );
  } else {
    path.moveTo( x2, y1 );
    path.lineTo( x1, y1 );
  }

  if ( roundBottom ) {
    path.lineTo( x1, y2 - yRnd );
    path.arcTo( QRect(x1, y2 - yDiam, xDiam, yDiam), 180.0, +90.0 );
    path.lineTo( x1 + xRnd, y2 );
    path.arcTo( QRect(x2 - xDiam, y2 - yDiam, xDiam, yDiam), 270.0, +90.0 );
  } else {
    path.lineTo( x1, y2 );
    path.lineTo( x2, y2 );
  }
  path.closeSubpath();

  return path;
}

void KOAgendaItem::drawRoundedRect( QPainter *p, const QRect& rect, 
				    const QColor& color, const QColor& bgcolor,
				    bool frame, int ft, bool roundTop, bool roundBottom )
{
  QPainterPath path = roundedRect( rect, roundTop, roundBottom );
  
  QColor oldPen(p->pen().color());
  QColor oldBrush(p->brush().color());
  if ( frame ) {
    QPen pen( color );
    pen.setWidth( ft );
    p->setPen( pen );
  }
  else
    p->setPen( Qt::NoPen );

  p->setBrush( bgcolor );
  p->drawPath( path );
  p->setPen( oldPen );
  p->setBrush( oldBrush );
}
