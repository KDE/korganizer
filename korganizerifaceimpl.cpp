/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

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
#include "kogroupware.h"


KOrganizerIfaceImpl::KOrganizerIfaceImpl( ActionManager* actionManager,
                                          QObject* parent, const char* name )
  : DCOPObject( "KOrganizerIface" ), QObject( parent, name ),
    mActionManager( actionManager )
{
}

KOrganizerIfaceImpl::~KOrganizerIfaceImpl()
{
}

bool KOrganizerIfaceImpl::openURL( QString url )
{
  return mActionManager->openURL( url );
}

bool KOrganizerIfaceImpl::mergeURL( QString url )
{
  return mActionManager->mergeURL( url );
}

void KOrganizerIfaceImpl::closeURL()
{
  return mActionManager->closeURL();
}

bool KOrganizerIfaceImpl::saveURL()
{
  return mActionManager->saveURL();
}

bool KOrganizerIfaceImpl::saveAsURL( QString url )
{
  return mActionManager->saveAsURL( url );
}

QString KOrganizerIfaceImpl::getCurrentURLasString() const
{
  return mActionManager->getCurrentURLasString();
}

bool KOrganizerIfaceImpl::deleteIncidence( QString uid )
{
  return mActionManager->deleteIncidence( uid );
}

bool KOrganizerIfaceImpl::editIncidence( QString uid )
{
  return mActionManager->editIncidence( uid );
}

bool KOrganizerIfaceImpl::eventRequest( QString request, QString receiver,
                                        QString ical )
{
  return mActionManager->eventRequest( request, receiver, ical );
}

bool KOrganizerIfaceImpl::eventReply( QString ical )
{
  return mActionManager->eventReply( ical );
}

bool KOrganizerIfaceImpl::cancelEvent( QString ical )
{
  return mActionManager->cancelEvent( ical );
}

QString KOrganizerIfaceImpl::formatICal( QString iCal )
{
  if( !KOGroupware::instance() ) return QString();
  return KOGroupware::instance()->formatICal( iCal );
}

QString KOrganizerIfaceImpl::formatTNEF( QByteArray tnef )
{
  if( !KOGroupware::instance() ) return QString();
  return KOGroupware::instance()->formatTNEF( tnef );
}

QString KOrganizerIfaceImpl::msTNEFToVPart( QByteArray tnef )
{
  if( !KOGroupware::instance() ) return QString();
  return KOGroupware::instance()->msTNEFToVPart( tnef );
}
