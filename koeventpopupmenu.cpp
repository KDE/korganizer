/*
    This file is part of KOrganizer.
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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <libkcal/event.h>

#include "koglobals.h"

#include "koeventpopupmenu.h"
#include "koeventpopupmenu.moc"

KOEventPopupMenu::KOEventPopupMenu()
{
  mCurrentIncidence = 0;
  mCurrentDate = QDate();
  mHasAdditionalItems = false;

  addAction(i18n("&Show"),this,SLOT(popupShow()));
  mEditOnlyItems.append(addAction (i18n("&Edit..."),this,SLOT(popupEdit())));
  mEditOnlyItems.append(addSeparator());
  mEditOnlyItems.append(addAction (KOGlobals::self()->smallIcon("editcut"),i18n("&Cut"),
                                   this,SLOT(popupCut())));
  mEditOnlyItems.append(addAction (KOGlobals::self()->smallIcon("editcopy"),i18n("&Copy"),
                                   this,SLOT(popupCopy())));
  mEditOnlyItems.append(addAction (KOGlobals::self()->smallIcon("editdelete"),i18n("&Delete"),
                                   this,SLOT(popupDelete())));
  mEditOnlyItems.append( addSeparator() );
  mEditOnlyItems.append( addAction( QIcon( KOGlobals::self()->smallIcon("bell") ),
                                     i18n("&Toggle Reminder"), this,
                                     SLOT( popupAlarm() ) ) );
  mRecurrenceItems.append( addSeparator() );
  mRecurrenceItems.append( addAction( i18n("&Dissociate This Occurrence"),
                                       this, SLOT( dissociateOccurrence() ) ) );
  mRecurrenceItems.append( addAction( i18n("&Dissociate Future Occurrences"),
                                       this, SLOT( dissociateFutureOccurrence() ) ) );
}

void KOEventPopupMenu::showIncidencePopup( Incidence *incidence, const QDate &qd )
{
  mCurrentIncidence = incidence;
  mCurrentDate = qd;

  if (mCurrentIncidence) {
    // Enable/Disabled menu items only valid for editable events.
    QList<QAction *>::Iterator it;
    for( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
      (*it)->setEnabled(!mCurrentIncidence->isReadOnly());
    }
    for ( it = mRecurrenceItems.begin(); it != mRecurrenceItems.end(); ++it ) {
      (*it)->setVisible( mCurrentIncidence->doesRecur() );
    }
    popup(QCursor::pos());
  } else {
    kdDebug(5850) << "KOEventPopupMenu::showEventPopup(): No event selected" << endl;
  }
}

void KOEventPopupMenu::addAdditionalItem(const QIcon &icon,const QString &text,
                                    const QObject *receiver, const char *member,
                                    bool editOnly)
{
  if (!mHasAdditionalItems) {
    mHasAdditionalItems = true;
    insertSeparator();
  }
  QAction *id = addAction(icon,text,receiver,member);
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

void KOEventPopupMenu::popupCut()
{
  if (mCurrentIncidence) emit cutIncidenceSignal(mCurrentIncidence);
}

void KOEventPopupMenu::popupCopy()
{
  if (mCurrentIncidence) emit copyIncidenceSignal(mCurrentIncidence);
}


void KOEventPopupMenu::popupAlarm()
{
  if (mCurrentIncidence) emit toggleAlarmSignal( mCurrentIncidence );
}

void KOEventPopupMenu::dissociateOccurrence()
{
  if ( mCurrentIncidence )
    emit dissociateOccurrenceSignal( mCurrentIncidence, mCurrentDate );
}

void KOEventPopupMenu::dissociateFutureOccurrence()
{
  if ( mCurrentIncidence )
    emit dissociateFutureOccurrenceSignal( mCurrentIncidence, mCurrentDate );
}

