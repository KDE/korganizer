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

#include <klocale.h>

#include <libkcal/event.h>

#include "koeventviewer.h"
#include "koprefs.h"

#include "koeventviewerdialog.h"
#include "koeventviewerdialog.moc"

KOEventViewerDialog::KOEventViewerDialog(QWidget *parent,const char *name)
  : KDialogBase(parent,name,false,i18n("Event Viewer"),Ok,Ok,false,
                i18n("Edit"))
{
  mEventViewer = new KOEventViewer(this);
  setMainWidget(mEventViewer);

  // TODO: Set a sensible size (based on the content?).
  if ( KOPrefs::instance()->mCompactDialogs ) {
    setFixedSize( 240,284 );
    move( 0, 15 );
  } else {
    setMinimumSize(300,200);
    resize(320,300);
  }
}

KOEventViewerDialog::~KOEventViewerDialog()
{
}

void KOEventViewerDialog::setEvent(Event *event)
{
  mEventViewer->setEvent(event);
}

void KOEventViewerDialog::addEvent(Event *event)
{
  mEventViewer->addEvent(event);
}

void KOEventViewerDialog::setTodo(Todo *event)
{
  mEventViewer->setTodo(event);
}

void KOEventViewerDialog::addText(QString text)
{
  mEventViewer->addText(text);
}
