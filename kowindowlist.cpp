/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>

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

KOWindowList::KOWindowList( const char *name )
  : QObject( 0, name ), mDefaultWindow( 0 )
{
  kdDebug(5850) << "KOWindowList::KOWindowList()" << endl;
}

KOWindowList::~KOWindowList()
{
}

void KOWindowList::addWindow( KOrg::MainWindow *korg )
{
  if ( !korg->hasDocument() ) mDefaultWindow = korg;
  else mWindowList.append( korg );
}

void KOWindowList::removeWindow( KOrg::MainWindow *korg )
{
  if ( korg == mDefaultWindow ) mDefaultWindow = 0;
  else mWindowList.removeRef( korg );
}

bool KOWindowList::lastInstance()
{
  if ( mWindowList.count() == 1 && !mDefaultWindow ) return true;
  if ( mWindowList.count() == 0 && mDefaultWindow ) return true;
  else return false;
}

KOrg::MainWindow *KOWindowList::findInstance( const KURL &url )
{
  KOrg::MainWindow *inst;
  for( inst = mWindowList.first(); inst; inst = mWindowList.next() )
    if ( inst->getCurrentURL() == url )
      return inst;
  return 0;
}

KOrg::MainWindow *KOWindowList::defaultInstance()
{
  return mDefaultWindow;
}
