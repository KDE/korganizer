/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <libkcal/incidence.h>
#include <libkcal/incidenceformatter.h>

#include "koincidencetooltip.h"

/**
@author Reinhold Kainhofer
some improvements by Mikolaj Machowski
*/

void KOIncidenceToolTip::add ( QWidget * widget, Incidence *incidence,
        QToolTipGroup * group, const QString & longText )
{
  if ( !widget || !incidence ) return;
  QToolTip::add(widget, IncidenceFormatter::toolTipString( incidence ), group, longText);
}

