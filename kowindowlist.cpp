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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kdebug.h>

#include "actionmanager.h"
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

void KOWindowList::addWindow(KOrg::MainWindow *korg)
{
  mWindowList.append(korg);
  connect(korg->actionManager(),SIGNAL(calendarActivated(KOrg::MainWindow *)),
	  SLOT(deactivateCalendars(KOrg::MainWindow *)));
}

void KOWindowList::removeWindow(KOrg::MainWindow *korg)
{
  mWindowList.removeRef(korg);
}

bool KOWindowList::lastInstance()
{
  if (mWindowList.count() == 1) return true;
  else return false;
}

KOrg::MainWindow* KOWindowList::findInstance(const KURL &url)
{
  KOrg::MainWindow *inst;
  for(inst=mWindowList.first();inst;inst=mWindowList.next())
    if (inst->getCurrentURL()==url)
      return inst;
  return 0;
}

void KOWindowList::deactivateCalendars(KOrg::MainWindow *korg)
{
  KOrg::MainWindow *k;
  for(k=mWindowList.first();k;k=mWindowList.next()) {
    if (k != korg) k->setActive(false);
  }
}
