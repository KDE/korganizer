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
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qtooltip.h>
#include <qdragobject.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>

#include <libkcal/icaldrag.h>
#include <libkcal/vcaldrag.h>

#include "koprefs.h"

#include "koagendaitem.h"
#include "koagendaitem.moc"

//--------------------------------------------------------------------------

QToolTipGroup *KOAgendaItem::mToolTipGroup = 0;

//--------------------------------------------------------------------------

KOAgendaItem::KOAgendaItem(Incidence *incidence, QDate qd, QWidget *parent,
                           const char *name,WFlags) :
  QFrame(parent, name), mIncidence(incidence), mDate(qd)
{
  mFirstMultiItem = 0;
  mNextMultiItem = 0;
  mLastMultiItem = 0;

  QHBox *box = 0;

  if ( incidence->type() == "Todo" )
  {
    static const QPixmap todoPxmp = SmallIcon("todo");
    static const QPixmap completedPxmp = SmallIcon("checkedbox");

    box = new QHBox(this);
    QLabel *todoIcon = new QLabel(box);

    if ( (static_cast<Todo*>(incidence))->isCompleted() )
      todoIcon->setPixmap(completedPxmp);
    else
      todoIcon->setPixmap(todoPxmp);

    todoIcon->setAlignment(AlignLeft|AlignTop);

    mItemLabel = new QLabel(mIncidence->summary(),box,"KOAgendaItem::itemLabel");
  }
  else
    mItemLabel = new QLabel(mIncidence->summary(),this,"KOAgendaItem::itemLabel");

  // if a Todo item is overdue and not completed, always show it in overdue color
  if ( (incidence->type() == "Todo") &&
       ( !((static_cast<Todo*>(incidence))->isCompleted()) &&
         ((static_cast<Todo*>(incidence))->dtDue() < QDate::currentDate()) ) )
    setPalette(QPalette(KOPrefs::instance()->mTodoOverdueColor,
                        KOPrefs::instance()->mTodoOverdueColor));
  else {
    QStringList categories = mIncidence->categories();
    QString cat = categories.first();
    if (cat.isEmpty()) {
      setPalette(QPalette(KOPrefs::instance()->mEventColor,
                          KOPrefs::instance()->mEventColor));
    } else {
      setPalette(QPalette(*(KOPrefs::instance()->categoryColor(cat)),
                          *(KOPrefs::instance()->categoryColor(cat))));
    }
  }

  mItemLabel->setAlignment(AlignCenter|WordBreak);
  mItemLabel->setMouseTracking(true);

  mItemLabel->installEventFilter(this);

  mItemLabel->setFrameStyle(Panel|Sunken);

  mItemLabel->setFont(KOPrefs::instance()->mAgendaViewFont);

  setCellXY(0,0,1);
  setCellXWidth(0);
  setSubCell(0);
  setSubCells(1);
  setMouseTracking(true);
  setMultiItem(0,0,0);

  startMove();

  mIconAlarm = new QLabel(this,"KOAgendaItem::IconAlarmLabel");
  mIconRecur = new QLabel(this,"KOAgendaItem::IconRecurLabel");
  mIconReadonly = new QLabel(this,"KOAgendaItem::IconReadonlyLabel");
  mIconReply = new QLabel(this,"KOAgendaItem::IconReplyLabel");
  mIconGroup = new QLabel(this,"KOAgendaItem::IconGroupLabel");
  mIconOrganizer = new QLabel(this,"KOAgendaItem::IconOrganizerLabel");

  mIconAlarm->installEventFilter(this);
  mIconRecur->installEventFilter(this);
  mIconReadonly->installEventFilter(this);
  mIconReply->installEventFilter(this);
  mIconGroup->installEventFilter(this);
  mIconOrganizer->installEventFilter(this);

  mIconAlarm->setMouseTracking(true);
  mIconRecur->setMouseTracking(true);
  mIconReadonly->setMouseTracking(true);
  mIconReply->setMouseTracking(true);
  mIconGroup->setMouseTracking(true);
  mIconOrganizer->setMouseTracking(true);

  static const QPixmap alarmPxmp = SmallIcon("bell");
  static const QPixmap recurPxmp = SmallIcon("recur");
  static const QPixmap readonlyPxmp = SmallIcon("readonlyevent");
  static const QPixmap replyPxmp = SmallIcon("mail_reply");
  static const QPixmap groupPxmp = SmallIcon("groupevent");
  static const QPixmap organizerPxmp = SmallIcon("organizer");

  mIconAlarm->setPixmap(alarmPxmp);
  mIconRecur->setPixmap(recurPxmp);
  mIconReadonly->setPixmap(readonlyPxmp);
  mIconReply->setPixmap(replyPxmp);
  mIconGroup->setPixmap(groupPxmp);
  mIconOrganizer->setPixmap(organizerPxmp);

  QVBoxLayout *topLayout = new QVBoxLayout(this,margin()+3);
  if ( incidence->type() == "Todo" )
    topLayout->addWidget(box, 1);
  else
    topLayout->addWidget(mItemLabel,1);

  QBoxLayout *iconLayout = new QHBoxLayout;
  topLayout->addLayout(iconLayout);

  iconLayout->addWidget(mIconAlarm);
  iconLayout->addWidget(mIconRecur);
  iconLayout->addWidget(mIconReadonly);
  iconLayout->addWidget(mIconReply);
  iconLayout->addWidget(mIconGroup);
  iconLayout->addWidget(mIconOrganizer);
  iconLayout->addStretch(1);

  updateIcons();

  // select() does nothing, if state hasn't change, so preset mSelected.
  mSelected = true;
  select(false);

//  QToolTip::add(this,mEvent->summary());
  QString tipText = mIncidence->summary();

  if ( !mIncidence->doesFloat() )
    if ( mIncidence->type() == "Event" ) {
      if ( (static_cast<Event*>(mIncidence))->isMultiDay() ) {
        tipText += "\n"+i18n("From: ")+mIncidence->dtStartStr();
        tipText += "\n"+i18n("To: ")+(static_cast<Event*>(mIncidence))->dtEndStr();
      }
      else {
        tipText += "\n"+i18n("Time: ")+mIncidence->dtStartTimeStr();
        tipText += " - "+(static_cast<Event*>(mIncidence))->dtEndTimeStr();
      }
    }
    else if ( mIncidence->type() == "Todo" ) {
      tipText += "\n"+i18n("Due: ")+ (static_cast<Todo*>(mIncidence))->dtDueTimeStr();
    }

  if (!mIncidence->location().isEmpty()) {
    tipText += "\n"+i18n("Location: ")+mIncidence->location();
  }
  //QToolTip::add(this,mEvent->summary(),toolTipGroup(),"");
  QToolTip::add(this,tipText,toolTipGroup(),"");

  setAcceptDrops(true);
}


void KOAgendaItem::updateIcons()
{
  if (mIncidence->isReadOnly()) mIconReadonly->show();
  else mIconReadonly->hide();

  if (mIncidence->recurrence()->doesRecur()) mIconRecur->show();
  else mIconRecur->hide();

  if (mIncidence->isAlarmEnabled()) mIconAlarm->show();
  else mIconAlarm->hide();

  if (mIncidence->attendeeCount()>0) {
    if (mIncidence->organizer() == KOPrefs::instance()->email()) {
      mIconReply->hide();
      mIconGroup->hide();
      mIconOrganizer->show();
    }
    else {
      Attendee *me = mIncidence->attendeeByMails(KOPrefs::instance()->mAdditionalMails,KOPrefs::instance()->email());
      if (me!=0) {
        if (me->status()==Attendee::NeedsAction && me->RSVP()) {
          mIconReply->show();
          mIconGroup->hide();
          mIconOrganizer->hide();
        }
        else {
          mIconReply->hide();
          mIconGroup->show();
          mIconOrganizer->hide();
        }
      }
      else {
        mIconReply->hide();
        mIconGroup->show();
        mIconOrganizer->hide();
      }
    }
  }
  else {
    mIconReply->hide();
    mIconGroup->hide();
    mIconOrganizer->hide();
  }
}


void KOAgendaItem::select(bool selected)
{
  if (mSelected == selected) return;
  mSelected = selected;
  if (mSelected) {
    mItemLabel->setFrameStyle(Panel|Sunken);
    mItemLabel->setLineWidth(1);
  } else {
    mItemLabel->setFrameStyle(Panel|Plain);
    mItemLabel->setLineWidth(0);
  }
}


/*
  The eventFilter has to filter the mouse events of the agenda item childs. The
  events are fed into the event handling method of KOAgendaItem. This allows the
  KOAgenda to handle the KOAgendaItems by using an eventFilter.
*/
bool KOAgendaItem::eventFilter ( QObject *object, QEvent *e )
{
//  kdDebug() << "KOAgendaItem::eventFilter" << endl;
  if (e->type() == QEvent::MouseButtonPress ||
      e->type() == QEvent::MouseButtonDblClick ||
      e->type() == QEvent::MouseButtonRelease ||
      e->type() == QEvent::MouseMove) {
    QMouseEvent *me = (QMouseEvent *)e;
    QPoint itemPos = this->mapFromGlobal(((QWidget *)object)->
                                         mapToGlobal(me->pos()));
    QMouseEvent returnEvent (e->type(),itemPos,me->button(),me->state());
    return event(&returnEvent);
  } else {
    return false;
  }
}


/*
  Return height of item in units of agenda cells
*/
int KOAgendaItem::cellHeight()
{
  return mCellYBottom - mCellYTop + 1;
}

/*
  Return height of item in units of agenda cells
*/
int KOAgendaItem::cellWidth()
{
  return mCellXWidth - mCellX + 1;
}

void KOAgendaItem::setItemDate(QDate qd)
{
  mDate = qd;
}

void KOAgendaItem::setCellXY(int X, int YTop, int YBottom)
{
  mCellX = X;
  mCellYTop = YTop;
  mCellYBottom = YBottom;
}

void KOAgendaItem::setCellXWidth(int xwidth)
{
  mCellXWidth = xwidth;
}

void KOAgendaItem::setCellX(int XLeft, int XRight)
{
  mCellX = XLeft;
  mCellXWidth = XRight;
}

void KOAgendaItem::setCellY(int YTop, int YBottom)
{
  mCellYTop = YTop;
  mCellYBottom = YBottom;
}

void KOAgendaItem::setSubCell(int subCell)
{
  mSubCell = subCell;
}

void KOAgendaItem::setSubCells(int subCells)
{
  mSubCells = subCells;
}

void KOAgendaItem::setMultiItem(KOAgendaItem *first,KOAgendaItem *next,
                                KOAgendaItem *last)
{
  mFirstMultiItem = first;
  mNextMultiItem = next;
  mLastMultiItem = last;
}

void KOAgendaItem::startMove()
{
  mStartCellX = mCellX;
  mStartCellXWidth = mCellXWidth;
  mStartCellYTop = mCellYTop;
  mStartCellYBottom = mCellYBottom;
}

void KOAgendaItem::resetMove()
{
  mCellX = mStartCellX;
  mCellXWidth = mStartCellXWidth;
  mCellYTop = mStartCellYTop;
  mCellYBottom = mStartCellYBottom;
}

void KOAgendaItem::moveRelative(int dx, int dy)
{
  int newX = cellX() + dx;
  int newXWidth = cellXWidth() + dx;
  int newYTop = cellYTop() + dy;
  int newYBottom = cellYBottom() + dy;
  setCellXY(newX,newYTop,newYBottom);
  setCellXWidth(newXWidth);
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
  int newX = cellX() + dx;
  int newXWidth = cellXWidth();
  if (newX > newXWidth) newX = newXWidth;
  setCellX(newX,newXWidth);
}

void KOAgendaItem::expandRight(int dx)
{
  int newX = cellX();
  int newXWidth = cellXWidth() + dx;
  if (newXWidth < newX) newXWidth = newX;
  setCellX(newX,newXWidth);
}

QToolTipGroup *KOAgendaItem::toolTipGroup()
{
  if (!mToolTipGroup) mToolTipGroup = new QToolTipGroup(0);
  return mToolTipGroup;
}

void KOAgendaItem::dragEnterEvent( QDragEnterEvent *e )
{
#ifndef KORG_NODND
  if ( ICalDrag::canDecode( e ) || VCalDrag::canDecode( e ) ||
       !QTextDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }
  e->accept();
#endif
}

void KOAgendaItem::dropEvent( QDropEvent *e )
{
#ifndef KORG_NODND
  QString text;
  if(QTextDrag::decode(e,text))
  {
    kdDebug() << "Dropped : " << text << endl;
    QStringList emails = QStringList::split(",",text);
    for(QStringList::ConstIterator it = emails.begin();it!=emails.end();++it) {
      kdDebug() << " Email: " << (*it) << endl;
      int pos = (*it).find("<");
      QString name = (*it).left(pos);
      QString email = (*it).mid(pos);
      if (!email.isEmpty()) {
        mIncidence->addAttendee(new Attendee(name,email));
      }
    }
  }
#endif
}


QPtrList<KOAgendaItem> KOAgendaItem::conflictItems()
{
  return mConflictItems;
}

void KOAgendaItem::setConflictItems(QPtrList<KOAgendaItem> ci)
{
  mConflictItems = ci;
  KOAgendaItem *item;
  for ( item=mConflictItems.first(); item != 0;
        item=mConflictItems.next() ) {
    item->addConflictItem(this);
  }
}

void KOAgendaItem::addConflictItem(KOAgendaItem *ci)
{
  if (mConflictItems.find(ci)<0)
    mConflictItems.append(ci);
}
