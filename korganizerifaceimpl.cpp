/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "korganizerifaceimpl.h"
#include "actionmanager.h"

#include "korganizeradaptor.h"
#include <QtDBus/QtDBus>

KOrganizerIfaceImpl::KOrganizerIfaceImpl( ActionManager* actionManager,
                                          QObject* parent, const char* name )
  : QObject( parent ),
    mActionManager( actionManager )
{
  setObjectName( name );
  new KorganizerAdaptor( this );
  QDBusConnection::sessionBus().registerObject( "/Korganizer", this, QDBusConnection::ExportAdaptors );
}

KOrganizerIfaceImpl::~KOrganizerIfaceImpl()
{
}

bool KOrganizerIfaceImpl::openURL( const QString &url )
{
  return mActionManager->openURL( url );
}

bool KOrganizerIfaceImpl::mergeURL( const QString &url )
{
  return mActionManager->mergeURL( url );
}

void KOrganizerIfaceImpl::closeUrl()
{
  return mActionManager->closeUrl();
}

bool KOrganizerIfaceImpl::saveURL()
{
  return mActionManager->saveURL();
}

bool KOrganizerIfaceImpl::saveAsURL( const QString &url )
{
  return mActionManager->saveAsURL( url );
}

QString KOrganizerIfaceImpl::getCurrentURLasString() const
{
  return mActionManager->getCurrentURLasString();
}

bool KOrganizerIfaceImpl::deleteIncidence( const QString &uid, bool force )
{
  return mActionManager->deleteIncidence( uid, force );
}

bool KOrganizerIfaceImpl::editIncidence( const QString &uid )
{
  return mActionManager->editIncidence( uid );
}

bool KOrganizerIfaceImpl::addIncidence( const QString &ical )
{
  return mActionManager->addIncidence( ical );
}

bool KOrganizerIfaceImpl::showIncidence( const QString &uid )
{
  return mActionManager->showIncidence( uid );
}

bool KOrganizerIfaceImpl::showIncidenceContext( const QString &uid )
{
  return mActionManager->showIncidenceContext( uid );
}

#include "korganizerifaceimpl.moc"
