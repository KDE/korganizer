/*
    This file is part of KOrganizer.
    Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qpopupmenu.h>
#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkcal/calendar.h>

#include "koeventview.h"
using namespace KOrg;
#include "koeventview.moc"

KOEventView::KOEventView(Calendar *cal,QWidget *parent,const char *name)
  : KOrg::BaseView(cal,parent,name)
{
}

KOEventView::~KOEventView()
{
}

KOEventPopupMenu *KOEventView::eventPopup()
{
  KOEventPopupMenu *eventPopup = new KOEventPopupMenu;
  
  connect (eventPopup,SIGNAL(editEventSignal(Event *)),
           SIGNAL(editEventSignal(Event *)));
  connect (eventPopup,SIGNAL(showEventSignal(Event *)),
           SIGNAL(showEventSignal(Event *)));
  connect (eventPopup,SIGNAL(deleteEventSignal(Event *)),
           SIGNAL(deleteEventSignal(Event *)));

  return eventPopup;
}

void KOEventView::showEventPopup(QPopupMenu *popup,Event *event)
{
  mCurrentEvent = event;
  if (event) popup->popup(QCursor::pos());
  else kdDebug() << "KOEventView::showEventPopup(): No event selected" << endl;
}

void KOEventView::popupShow()
{
  emit showEventSignal(mCurrentEvent);
}

void KOEventView::popupEdit()
{
  emit editEventSignal(mCurrentEvent);
}

void KOEventView::popupDelete()
{
  emit deleteEventSignal(mCurrentEvent);
}

void KOEventView::defaultAction( Incidence *incidence )
{
  kdDebug() << "KOEventView::defaultAction()" << endl;

  if ( !incidence ) return;

  kdDebug() << "  type: " << incidence->type() << endl;

  if ( incidence->type() == "Event" ) {
    Event *event = static_cast<Event *>( incidence );
    if (event->isReadOnly()) emit showEventSignal(event);
    else emit editEventSignal(event);
  }
}

#include "baseview.moc"

