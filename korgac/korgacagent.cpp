/*
  This file is part of the KDE reminder agent.

  Copyright (c) 2010 Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "korgacagent.h"
#include "koalarmclient.h"

#include <Akonadi/AgentFactory>

#include <KLocale>

using namespace Akonadi;

KorgacAgent::KorgacAgent( const QString &id )
  : AgentBase( id ),
    m_alarmClient( new KOAlarmClient )
{
  KGlobal::locale()->insertCatalog( "korganizer" );
}

KorgacAgent::~KorgacAgent()
{
  delete m_alarmClient;
}

AKONADI_AGENT_FACTORY( KorgacAgent, korgac )

#include "korgacagent.moc"
