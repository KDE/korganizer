/*
    This file is part of KOrganizer.
    Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qpopupmenu.h>
#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#include <libkcal/calendar.h>


#include "kocore.h"
#include "koeventview.h"
#include "koeventpopupmenu.h"

using namespace KOrg;
#include "koeventview.moc"

//---------------------------------------------------------------------------

KOEventView::KOEventView(Calendar *cal,QWidget *parent,const char *name)
  : KOrg::BaseView(cal,parent,name)
{
}

//---------------------------------------------------------------------------

KOEventView::~KOEventView()
{
}

//---------------------------------------------------------------------------

KOEventPopupMenu *KOEventView::eventPopup()
{
  KOEventPopupMenu *eventPopup = new KOEventPopupMenu;
  
  connect(eventPopup,SIGNAL(editIncidenceSignal(Incidence *)),
                     SIGNAL(editIncidenceSignal(Incidence *)));
  connect(eventPopup,SIGNAL(showIncidenceSignal(Incidence *)),
                     SIGNAL(showIncidenceSignal(Incidence *)));
  connect(eventPopup,SIGNAL(deleteIncidenceSignal(Incidence *)),
                     SIGNAL(deleteIncidenceSignal(Incidence *)));
  connect(eventPopup,SIGNAL(toggleAlarmSignal(Incidence *)),
                     SIGNAL(toggleAlarmSignal(Incidence*)));
  connect(eventPopup,SIGNAL(dissociateOccurrenceSignal( Incidence *, const QDate & )),
                     SIGNAL(dissociateOccurrenceSignal( Incidence *, const QDate & )));
  connect(eventPopup,SIGNAL(dissociateFutureOccurrenceSignal( Incidence *, const QDate & )),
                     SIGNAL(dissociateFutureOccurrenceSignal( Incidence *, const QDate & )));

  return eventPopup;
}

QPopupMenu *KOEventView::newEventPopup()
{
  KXMLGUIClient *client = KOCore::self()->xmlguiClient( this );
  if ( !client ) {
    kdError() << "KOEventView::newEventPopup(): no xmlGuiClient." << endl;
    return 0;
  }
  if ( !client->factory() ) {
    kdError() << "KOEventView::newEventPopup(): no factory" << endl;
    return 0; // can happen if called too early
  }

  return static_cast<QPopupMenu*>
      ( client->factory()->container( "rmb_selection_popup", client ) );
}
//---------------------------------------------------------------------------

void KOEventView::popupShow()
{
  emit showIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupEdit()
{
  emit editIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupDelete()
{
  emit deleteIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::showNewEventPopup()
{
  QPopupMenu *popup = newEventPopup();
  if ( !popup ) {
    kdError() << "KOEventView::showNewEventPopup(): popup creation failed"
              << endl;
    return;
  }

  popup->popup( QCursor::pos() );
}

//---------------------------------------------------------------------------

void KOEventView::defaultAction( Incidence *incidence )
{
  kdDebug(5850) << "KOEventView::defaultAction()" << endl;

  if ( !incidence ) return;

  kdDebug(5850) << "  type: " << incidence->type() << endl;

  if ( incidence->isReadOnly() )
    emit showIncidenceSignal(incidence);
  else
    emit editIncidenceSignal(incidence);
}

//---------------------------------------------------------------------------

#include "baseview.moc"

