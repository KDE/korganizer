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

#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kurl.h>

#include <libkcal/event.h>

#include "koglobals.h"

#include <korganizer/baseview.h>
#include "koeventpopupmenu.h"
#include "koeventpopupmenu.moc"
#include "kocorehelper.h"
#include "actionmanager.h"
#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif

KOEventPopupMenu::KOEventPopupMenu()
{
  mCurrentIncidence = 0;
  mCurrentDate = QDate();
  mHasAdditionalItems = false;

  insertItem( i18n("&Show"), this, SLOT( popupShow() ) );
  mEditOnlyItems.append(
    insertItem(i18n("&Edit..."), this, SLOT( popupEdit() ) ) );
#ifndef KORG_NOPRINTER
  insertItem( KOGlobals::self()->smallIcon("printer1"), i18n("&Print..."),
              this, SLOT( print() ) );
#endif
  //------------------------------------------------------------------------
  mEditOnlyItems.append( insertSeparator() );
  mEditOnlyItems.append(
    insertItem( KOGlobals::self()->smallIcon("editcut"), i18n("&Cut"),
                this, SLOT( popupCut() ) ) );
  mEditOnlyItems.append(
    insertItem( KOGlobals::self()->smallIcon("editcopy"), i18n("&Copy"),
                this, SLOT( popupCopy() ) ) );
  // paste is always possible
  insertItem( KOGlobals::self()->smallIcon("editpaste"), i18n("&Paste"),
                this, SLOT( popupPaste() ) );
  mEditOnlyItems.append(
    insertItem( KOGlobals::self()->smallIcon("editdelete"), i18n("&Delete"),
                this, SLOT( popupDelete() ) ) );
  //------------------------------------------------------------------------
  mEditOnlyItems.append( insertSeparator() );
  mEditOnlyItems.append(
    insertItem( KOGlobals::self()->smallIcon("bell"), i18n("&Toggle Reminder"),
                this, SLOT( popupAlarm() ) ) );
  //------------------------------------------------------------------------
  mRecurrenceItems.append( insertSeparator() );
  mRecurrenceItems.append(
    insertItem( i18n("&Dissociate This Occurrence"),
                this, SLOT( dissociateOccurrence() ) ) );
  mRecurrenceItems.append(
    insertItem( i18n("&Dissociate Future Occurrences"),
                this, SLOT( dissociateFutureOccurrence() ) ) );

  insertSeparator();
  insertItem( KOGlobals::self()->smallIcon("mail_forward"), i18n( "Send as iCalendar..."),
              this, SLOT(forward()) );
}

void KOEventPopupMenu::showIncidencePopup( Incidence *incidence, const QDate &qd )
{
  mCurrentIncidence = incidence;
  mCurrentDate = qd;

  if (mCurrentIncidence) {
    // Enable/Disabled menu items only valid for editable events.
    QValueList<int>::Iterator it;
    for( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
      setItemEnabled(*it,!mCurrentIncidence->isReadOnly());
    }
    for ( it = mRecurrenceItems.begin(); it != mRecurrenceItems.end(); ++it ) {
      setItemVisible( *it, mCurrentIncidence->doesRecur() );
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

void KOEventPopupMenu::print()
{
#ifndef KORG_NOPRINTER
  KOCoreHelper helper;
  CalPrinter printer( this, 0, &helper );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( mCurrentIncidence );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 mCurrentDate, mCurrentDate, selectedIncidences );
#endif
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

void KOEventPopupMenu::popupPaste()
{
  emit pasteIncidenceSignal();
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

void KOEventPopupMenu::forward()
{
  KOrg::MainWindow *w = ActionManager::findInstance( KURL() );
  if ( !w || !mCurrentIncidence )
    return;
  KActionCollection *ac = w->getActionCollection();
  KAction *action = ac->action( "schedule_forward" );
  action->activate();
}
