/*
    This file is part of KOrganizer.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdebug.h>

#include "kowindowlist.h"
#include "kowindowlist.moc"

KOWindowList::KOWindowList(const char *name) 
  : QObject(0,name)
{
  kdDebug() << "KOWindowList::KOWindowList()" << endl;
}

KOWindowList::~KOWindowList()
{
}

void KOWindowList::addWindow(KOrganizer *korg)
{
  mWindowList.append(korg);
  connect(korg,SIGNAL(calendarActivated(KOrganizer *)),
          SLOT(deactivateCalendars(KOrganizer *)));
}

void KOWindowList::removeWindow(KOrganizer *korg)
{
  mWindowList.removeRef(korg);
}

bool KOWindowList::lastInstance()
{
  if (mWindowList.count() == 1) return true;
  else return false;
}

KOrganizer* KOWindowList::findInstance(const KURL &url)
{
  KOrganizer *inst;
  for(inst=mWindowList.first();inst;inst=mWindowList.next())
    if (inst->getCurrentURL()==url)
      return inst;
  return 0;
}

void KOWindowList::deactivateCalendars(KOrganizer *korg)
{
  KOrganizer *k;
  for(k=mWindowList.first();k;k=mWindowList.next()) {
    if (k != korg) k->setActive(false);
  }
}
