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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <libkcal/event.h>

#include "koeventpopupmenu.h"
#include "koeventpopupmenu.moc"

KOEventPopupMenu::KOEventPopupMenu()
{
  mCurrentEvent = 0;
  mHasAdditionalItems = false;

  insertItem (i18n("&Show"),this,SLOT(popupShow()));
  mEditOnlyItems.append(insertItem (i18n("edit event","&Edit..."),this,SLOT(popupEdit())));
  mEditOnlyItems.append(insertItem (SmallIcon("editdelete"),i18n("&Delete..."),
                                   this,SLOT(popupDelete())));
}

void KOEventPopupMenu::showEventPopup(Event *event)
{
  mCurrentEvent = event;
  
  if (mCurrentEvent) {
    // Enable/Disabled menu items only valid for editable events.
    QValueList<int>::Iterator it;
    for( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
      setItemEnabled(*it,!mCurrentEvent->isReadOnly());
    }
    popup(QCursor::pos());
  } else {
    kdDebug() << "KOEventPopupMenu::showEventPopup(): No event selected" << endl;
  }
}

void KOEventPopupMenu::addAdditionalItem(const QIconSet &icon,const QString &text,
                                    const QObject *receiver, const char *member,
                                    bool editOnly)
{
  if (!mHasAdditionalItems) {
    mHasAdditionalItems = true;
    insertSeparator();
  }
  int id = insertItem(icon,text,receiver,member);
  if (editOnly) mEditOnlyItems.append(id);
}

void KOEventPopupMenu::popupShow()
{
  if (mCurrentEvent) emit showEventSignal(mCurrentEvent);
}

void KOEventPopupMenu::popupEdit()
{
  if (mCurrentEvent) emit editEventSignal(mCurrentEvent);
}

void KOEventPopupMenu::popupDelete()
{
  if (mCurrentEvent) emit deleteEventSignal(mCurrentEvent);
}
