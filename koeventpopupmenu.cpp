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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <libkcal/event.h>

#include "koeventpopupmenu.h"
#include "koeventpopupmenu.moc"

KOEventPopupMenu::KOEventPopupMenu()
{
  mCurrentIncidence = 0;
  mHasAdditionalItems = false;

  insertItem (i18n("&Show"),this,SLOT(popupShow()));
  mEditOnlyItems.append(insertItem (i18n("&Edit..."),this,SLOT(popupEdit())));
  mEditOnlyItems.append(insertItem (SmallIcon("editdelete"),i18n("&Delete"),
                                   this,SLOT(popupDelete())));
}

void KOEventPopupMenu::showIncidencePopup(Incidence *incidence)
{
  mCurrentIncidence = incidence;
  
  if (mCurrentIncidence) {
    // Enable/Disabled menu items only valid for editable events.
    QValueList<int>::Iterator it;
    for( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
      setItemEnabled(*it,!mCurrentIncidence->isReadOnly());
    }
    popup(QCursor::pos());
  } else {
    kdDebug(5850) << "KOEventPopupMenu::showEventPopup(): No event selected" << endl;
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
  if (mCurrentIncidence) emit showIncidenceSignal(mCurrentIncidence);
}

void KOEventPopupMenu::popupEdit()
{
  if (mCurrentIncidence) emit editIncidenceSignal(mCurrentIncidence);
}

void KOEventPopupMenu::popupDelete()
{
  if (mCurrentIncidence) emit deleteIncidenceSignal(mCurrentIncidence);
}
