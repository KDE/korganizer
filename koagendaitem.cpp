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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qtooltip.h>
#include <qdragobject.h>

#include <kiconloader.h>
#include <kdebug.h>

#include "koprefs.h"

#include "koagendaitem.h"
#include "koagendaitem.moc"

QToolTipGroup *KOAgendaItem::mToolTipGroup = 0;

KOAgendaItem::KOAgendaItem(Event *event, QWidget *parent,
                           const char *name,WFlags) :
  QFrame( parent, name )
{
  mEvent = event;

  QStringList categories = mEvent->categories();
  QString cat = categories.first();
  if (cat.isEmpty()) {
    setPalette(QPalette(KOPrefs::instance()->mEventColor,
                        KOPrefs::instance()->mEventColor));
  } else {
    setPalette(QPalette(*(KOPrefs::instance()->categoryColor(cat)),
                        *(KOPrefs::instance()->categoryColor(cat))));
  }

  mItemLabel = new QLabel(mEvent->summary(),this,"KOAgendaItem::itemLabel");
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

  mIconAlarm->installEventFilter(this);
  mIconRecur->installEventFilter(this);
  mIconReadonly->installEventFilter(this);

  mIconAlarm->setMouseTracking(true);
  mIconRecur->setMouseTracking(true);
  mIconReadonly->setMouseTracking(true);

  static const QPixmap alarmPxmp = SmallIcon("bell");
  static const QPixmap recurPxmp = SmallIcon("recur");
  static const QPixmap readonlyPxmp = SmallIcon("readonlyevent");

  mIconAlarm->setPixmap(alarmPxmp);
  mIconRecur->setPixmap(recurPxmp);
  mIconReadonly->setPixmap(readonlyPxmp);

  QVBoxLayout *topLayout = new QVBoxLayout(this,margin()+3);
  topLayout->addWidget(mItemLabel,1);

  QBoxLayout *iconLayout = new QHBoxLayout;
  topLayout->addLayout(iconLayout);

  iconLayout->addWidget(mIconAlarm);
  iconLayout->addWidget(mIconRecur);
  iconLayout->addWidget(mIconReadonly);
  iconLayout->addStretch(1);

  updateIcons();

  // select() does nothing, if state hasn't change, so preset mSelected.
  mSelected = true;
  select(false);

//  QToolTip::add(this,mEvent->summary());
  QToolTip::add(this,mEvent->summary(),toolTipGroup(),"");

  setAcceptDrops(true);
}


void KOAgendaItem::updateIcons()
{
  if (mEvent->isReadOnly()) mIconReadonly->show();
  else mIconReadonly->hide();
  if (mEvent->recurrence()->doesRecur()) mIconRecur->show();
  else mIconRecur->hide();
  if (mEvent->isAlarmEnabled()) mIconAlarm->show();
  else mIconAlarm->hide();
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
  if (!QTextDrag::canDecode(e)) {
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
        mEvent->addAttendee(new Attendee(name,email));
      }
    }
  }
#endif
}
