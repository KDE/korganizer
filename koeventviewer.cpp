/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "koeventviewer.h"

#include "urihandler.h"

#include <libkcal/incidence.h>
#include <libkcal/incidenceformatter.h>
#include <kdebug.h>

KOEventViewer::KOEventViewer( QWidget *parent, const char *name )
  : QTextBrowser( parent, name ), mDefaultText("")
{
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::readSettings( KConfig * config )
{
  if ( config ) {
// With each restart of KOrganizer the font site gets halfed. What should this
// be good for?
#if 0
    config->setGroup( QString("EventViewer-%1").arg( name() )  );
    int zoomFactor = config->readNumEntry("ZoomFactor", pointSize() );
    zoomTo( zoomFactor/2 );
    kdDebug(5850) << " KOEventViewer: restoring the pointSize:  "<< pointSize() 
      << ", zoomFactor: " << zoomFactor << endl;
#endif
  }
}

void KOEventViewer::writeSettings( KConfig * config )
{
  if ( config ) {
    kdDebug(5850) << " KOEventViewer: saving the zoomFactor: "<< pointSize() << endl;
    config->setGroup( QString("EventViewer-%1").arg( name() ) );
    config->writeEntry("ZoomFactor", pointSize() );
  }
}

void KOEventViewer::setSource( const QString &n )
{
  UriHandler::process( n );
}

bool KOEventViewer::appendIncidence( Incidence *incidence )
{
  addText( IncidenceFormatter::extensiveDisplayString( incidence ) );
  return true;
}

void KOEventViewer::setIncidence( Incidence *incidence )
{
  clearEvents();
  if( incidence ) appendIncidence( incidence );
  else clearEvents(true);
}

void KOEventViewer::clearEvents( bool now )
{
  mText = "";
  if ( now ) setText( mDefaultText );
}

void KOEventViewer::addText( const QString &text )
{
  mText.append( text );
  setText( mText );
}

void KOEventViewer::setDefaultText( const QString &text )
{
  mDefaultText = text;
}

#include "koeventviewer.moc"
