/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#include <qdragobject.h>
#include <qpainter.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kwordwrap.h>

#include <libkcal/icaldrag.h>
#include <libkcal/vcaldrag.h>
#include <libkdepim/kvcarddrag.h>
#ifndef KORG_NOKABC
#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#endif

#include "koprefs.h"
#include "koglobals.h"

#include "koincidencetooltip.h"
#include "koagendaitem.h"
#include "koagendaitem.moc"

//--------------------------------------------------------------------------

QToolTipGroup *KOAgendaItem::mToolTipGroup = 0;

//--------------------------------------------------------------------------

KOAgendaItem::KOAgendaItem( Incidence *incidence, QDate qd, QWidget *parent,
                            const char *name, WFlags f ) :
  QWidget( parent, name, f ), mIncidence( incidence ), mDate( qd ),
  mLabelText( mIncidence->summary() ), mIconAlarm( false ),
  mIconRecur( false ), mIconReadonly( false ), mIconReply( false ),
  mIconGroup( false ), mIconGroupTentative( false ), mIconOrganizer( false ),
  mMultiItemInfo( 0 ), mStartMoveInfo( 0 )
{
  setBackgroundMode( Qt::NoBackground );

  setCellXY( 0, 0, 1 );
  setCellXRight( 0 );
  setMouseTracking( true );

  updateIcons();

  // select() does nothing, if state hasn't change, so preset mSelected.
  mSelected = true;
  select( false );

  KOIncidenceToolTip::add( this, incidence, toolTipGroup() );
  setAcceptDrops( true );
}

void KOAgendaItem::updateIcons()
{
  mIconReadonly = mIncidence->isReadOnly();
  mIconRecur = mIncidence->doesRecur();
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

void KOAgendaItem::setItemDate( QDate qd )
{
  mDate = qd;
}

void KOAgendaItem::setCellXY( int X, int YTop, int YBottom )
{
  mCellXLeft = X;
  mCellYTop = YTop;
  mCellYBottom = YBottom;
}

void KOAgendaItem::setCellXRight( int xright )
{
  mCellXRight = xright;
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
        KOAgendaItem*nowDel=0L;
        while (toDel) {
          nowDel=toDel;
          if (nowDel->moveInfo()) {
            toDel=nowDel->moveInfo()->mPrevMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mFirstMultiItem = 0L;
        mMultiItemInfo->mPrevMultiItem = 0L;
      }
      if ( !mStartMoveInfo->mLastMultiItem ) {
        // This was the last multi-item when the move started, delete all next
        KOAgendaItem*toDel=mStartMoveInfo->mNextMultiItem;
        KOAgendaItem*nowDel=0L;
        while (toDel) {
          nowDel=toDel;
          if (nowDel->moveInfo()) {
            toDel=nowDel->moveInfo()->mNextMultiItem;
          }
          emit removeAgendaItem( nowDel );
        }
        mMultiItemInfo->mLastMultiItem = 0L;
        mMultiItemInfo->mNextMultiItem = 0L;
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

QToolTipGroup *KOAgendaItem::toolTipGroup()
{
  if (!mToolTipGroup) mToolTipGroup = new QToolTipGroup(0);
  return mToolTipGroup;
}

void KOAgendaItem::dragEnterEvent( QDragEnterEvent *e )
{
#ifndef KORG_NODND
  if ( ICalDrag::canDecode( e ) || VCalDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }
  if ( KVCardDrag::canDecode( e ) || QTextDrag::canDecode( e ) )
    e->accept();
  else
    e->ignore();
#endif
}

void KOAgendaItem::addAttendee(QString newAttendee)
{
  kdDebug(5850) << " Email: " << newAttendee << endl;
  int pos = newAttendee.find("<");
  QString name = newAttendee.left(pos);
  QString email = newAttendee.mid(pos);
  if (!email.isEmpty()) {
    mIncidence->addAttendee(new Attendee(name,email));
  } else if (name.contains("@")) {
    mIncidence->addAttendee(new Attendee(name,name));
  } else {
    mIncidence->addAttendee(new Attendee(name,QString::null));
  }
}

void KOAgendaItem::dropEvent( QDropEvent *e )
{
#ifndef KORG_NODND
  QString text;
  QString vcards;

#ifndef KORG_NOKABC
  if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::VCardConverter converter;

    KABC::Addressee::List list = converter.parseVCards( vcards );
    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      QString em( (*it).fullEmail() );
      if (em.isEmpty()) {
        em=(*it).realName();
      }
      addAttendee( em );
    }
  } else
#endif
  if( QTextDrag::decode( e, text ) ) {
    kdDebug(5850) << "Dropped : " << text << endl;
    QStringList emails = QStringList::split( ",", text );
    for( QStringList::ConstIterator it = emails.begin(); it != emails.end();
         ++it ) {
      addAttendee( *it );
    }
  }
#endif
}


QPtrList<KOAgendaItem> KOAgendaItem::conflictItems()
{
  return mConflictItems;
}

void KOAgendaItem::setConflictItems( QPtrList<KOAgendaItem> ci )
{
  mConflictItems = ci;
  KOAgendaItem *item;
  for ( item = mConflictItems.first(); item != 0;
        item = mConflictItems.next() ) {
    item->addConflictItem( this );
  }
}

void KOAgendaItem::addConflictItem( KOAgendaItem *ci )
{
  if ( mConflictItems.find( ci ) < 0 ) mConflictItems.append( ci );
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

void KOAgendaItem::paintFrame( QPainter *p, const QColor &color )
{
  QColor oldpen(p->pen().color());
  p->setPen( color );
  p->drawRect( 0, 0, width(), height() );
  p->drawRect( 1, 1, width() - 2, height() - 2 );
  p->setPen( oldpen );
}

static void conditionalPaint( QPainter *p, bool cond, int &x, int ft,
                              const QPixmap &pxmp )
{
  if ( !cond ) return;

  p->drawPixmap( x, ft, pxmp );
  x += pxmp.width() + ft;
}

void KOAgendaItem::paintTodoIcon( QPainter *p, int &x, int ft )
{
  static const QPixmap todoPxmp = KOGlobals::self()->smallIcon("todo");
  static const QPixmap completedPxmp = KOGlobals::self()->smallIcon("checkedbox");
  if ( mIncidence->type() != "Todo" )
    return;
  bool b = ( static_cast<Todo *>( mIncidence ) )->isCompleted() ||
           ( mDate < ( static_cast<Todo *>( mIncidence )->dtDue().date() ) );
  conditionalPaint( p, !b, x, ft, todoPxmp );
  conditionalPaint( p, b, x, ft, completedPxmp );
}

void KOAgendaItem::paintEvent( QPaintEvent * )
{
  QPainter p( this );
  const int ft = 2; // frame thickness for layout, see paintFrame()
  const int margin = 1 + ft; // frame + space between frame and content
  bool isTodoOverdue = false;

  static const QPixmap alarmPxmp = KOGlobals::self()->smallIcon("bell");
  static const QPixmap recurPxmp = KOGlobals::self()->smallIcon("recur");
  static const QPixmap readonlyPxmp = KOGlobals::self()->smallIcon("readonlyevent");
  static const QPixmap replyPxmp = KOGlobals::self()->smallIcon("mail_reply");
  static const QPixmap groupPxmp = KOGlobals::self()->smallIcon("groupevent");
  static const QPixmap groupPxmpTentative = KOGlobals::self()->smallIcon("groupeventtentative");
  static const QPixmap organizerPxmp = KOGlobals::self()->smallIcon("organizer");

  QColor bgColor;
  if ( (mIncidence->type() == "Todo") &&
       ( !((static_cast<Todo*>(mIncidence))->isCompleted()) &&
         ((static_cast<Todo*>(mIncidence))->dtDue() < QDate::currentDate()) ) ) {
    bgColor = KOPrefs::instance()->mTodoOverdueColor;
    isTodoOverdue = true;
  } else {
    QStringList categories = mIncidence->categories();
    QString cat = categories.first();
    if (cat.isEmpty())
      bgColor = KOPrefs::instance()->mEventColor;
    else
      bgColor = *(KOPrefs::instance()->categoryColor(cat));
  }

  QColor frameColor = mSelected ? QColor( 85 + bgColor.red()*2/3,
                                          85 + bgColor.green()*2/3,
                                          85 + bgColor.blue()*2/3 )
                                : bgColor.dark(115);
  QColor textColor = getTextColor(bgColor);
  p.setPen( textColor );
  p.setBackgroundColor( bgColor );
  p.setFont(KOPrefs::instance()->mAgendaViewFont);
  QFontMetrics fm = p.fontMetrics();

  int singleLineHeight = fm.boundingRect( mLabelText ).height();

  // case 1: do not draw text when not even a single line fits
  // Don't do this any more, always try to print out the text. Even if
  // it's just a few pixel, one can still guess the whole text from just four pixels' height!
  if ( //( singleLineHeight > height()-4 ) || // ignore margin, be gentle.. Even ignore 2 pixel outside the item
       ( width() < 16 ) ) {
    p.eraseRect( 0, 0, width(), height() );
    int x = margin;
    paintTodoIcon( &p, x, ft );
    paintFrame( &p, frameColor );
    return;
  }

  // Used for multi-day events to make sure the summary is on screen
  QRect visRect=visibleRect();

  // case 2: draw a single line when no more space
  if ( (2 * singleLineHeight) > (height() - 2 * margin) ) {
    p.eraseRect( 0, 0, width(), height() );
    int x = margin;
    int txtWidth = width() - margin - x;
    if (mIncidence->doesFloat() ) {
      x += visRect.left();
      txtWidth = visRect.right() - margin - x;
    }

    paintTodoIcon( &p, x, ft );
    paintFrame( &p, frameColor );
    int y = ((height() - 2 * ft - singleLineHeight) / 2) + fm.ascent();
    KWordWrap::drawFadeoutText( &p, x, y,
                                txtWidth, mLabelText );
    return;
  }

  KWordWrap *ww = KWordWrap::formatText( fm,
                                         QRect(0, 0,
                                         width() - (2 * margin), -1),
                                         0,
                                         mLabelText );
  int th = ww->boundingRect().height();
  delete ww;

  // calculate the height of the full version (case 4) to test whether it is
  // possible
  QString shortH;
  QString longH;
  if ( !isMultiItem() ) {
    shortH = KGlobal::locale()->formatTime(mIncidence->dtStart().time());
    if (mIncidence->type() != "Todo")
      longH = i18n("%1 - %2").arg(shortH)
               .arg(KGlobal::locale()->formatTime(mIncidence->dtEnd().time()));
    else
      longH = shortH;
  } else if ( !mMultiItemInfo->mFirstMultiItem ) {
    shortH = KGlobal::locale()->formatTime(mIncidence->dtStart().time());
    longH = shortH;
  } else {
    shortH = KGlobal::locale()->formatTime(mIncidence->dtEnd().time());
    longH = i18n("- %1").arg(shortH);
  }

  int hlHeight = QMAX(fm.boundingRect(longH).height(),
     QMAX(alarmPxmp.height(), QMAX(recurPxmp.height(),
     QMAX(readonlyPxmp.height(), QMAX(replyPxmp.height(),
     QMAX(groupPxmp.height(), organizerPxmp.height()))))));
  bool completelyRenderable =
    th < (height() - 2 * ft - 2 - hlHeight);
  // case 3: enough for 2-5 lines, but not for the header.
  //         Also used for the middle days in multi-events
  //         or all-day events, or overdue todo items
  if ( ((!completelyRenderable) && ((height() - (2 * margin)) <= (5 * singleLineHeight)) ) ||
       (isMultiItem() && mMultiItemInfo->mNextMultiItem && mMultiItemInfo->mFirstMultiItem) ||
       mIncidence->doesFloat() ||
       isTodoOverdue ) {
    int x = margin;
    int txtWidth = width() - margin - x;
    if (mIncidence->doesFloat() ) {
      x += visRect.left();
      txtWidth = visRect.right() - margin - x;
    }
    ww = KWordWrap::formatText( fm,
                                QRect(x, 0, txtWidth,
                                height() - (2 * margin)),
                                0,
                                mLabelText );
    p.eraseRect( 0, 0, width(), height() );
    paintTodoIcon( &p, x, ft );
    paintFrame( &p, frameColor );
    ww->drawText( &p, x, margin, Qt::AlignAuto | KWordWrap::FadeOut );
    delete ww;
    return;
  }

  // case 4: paint everything, with header:
  // consists of (vertically) ft + headline&icons + ft + text + margin
  int y = 2 * ft + hlHeight;
  if ( completelyRenderable )
    y += (height() - (2 * ft) - margin - hlHeight - th) / 2;
  ww = KWordWrap::formatText( fm,
                              QRect(0, 0, width() - (2 * margin),
                              height() - margin - y),
                              0,
                              mLabelText );

  p.eraseRect( 0, 0, width(), height() );

  // paint headline
  p.fillRect( 0, 0, width(), (ft/2) + margin + hlHeight,
              QBrush( frameColor ) );

  int x = margin;
  paintTodoIcon( &p, x, ft );
  conditionalPaint( &p, mIconAlarm, x, ft, alarmPxmp );
  conditionalPaint( &p, mIconRecur, x, ft, recurPxmp );
  conditionalPaint( &p, mIconReadonly, x, ft, readonlyPxmp );
  conditionalPaint( &p, mIconReply, x, ft, replyPxmp );
  conditionalPaint( &p, mIconGroup, x, ft, groupPxmp );
  conditionalPaint( &p, mIconGroupTentative, x, ft, groupPxmpTentative );
  conditionalPaint( &p, mIconOrganizer, x, ft, organizerPxmp );

  QString headline;
  int hw = fm.boundingRect( longH ).width();
  if ( hw > (width() - x - margin) ) {
    headline = shortH;
    hw = fm.boundingRect( shortH ).width();
    if ( hw < (width() - x - margin) )
      x += (width() - x - margin - hw) / 2;
  } else {
    headline = longH;
    x += (width() - x - margin - hw) / 2;
  }
  p.setBackgroundColor( frameColor );
  p.setPen( getTextColor( frameColor ) );
  KWordWrap::drawFadeoutText( &p, x, ft + fm.ascent(),
                              width() - margin - x, headline );

  // draw event text
  p.setBackgroundColor( bgColor );
  p.setPen( textColor );
  paintFrame( &p, frameColor );
  QString ws = ww->wrappedString();
  if ( ws.left( ws.length()-1 ).find( '\n' ) >= 0 )
    ww->drawText( &p, margin, y,
                  Qt::AlignAuto | KWordWrap::FadeOut );
  else
    ww->drawText( &p, margin + (width()-ww->boundingRect().width()-2*margin)/2,
                  y, Qt::AlignHCenter | KWordWrap::FadeOut );
  delete ww;
}

