/*
  This file is part of KOrganizer.

  Copyright (c) 1997-1999 Preston Brown <pbrown@kde.org>
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_KORGANIZER_OPTIONS_H
#define KORG_KORGANIZER_OPTIONS_H

#include <KCmdLineOptions>

static KCmdLineOptions korganizer_options ()
{
  KCmdLineOptions options;
  options.add( "i" );
  options.add( "import",
               ki18n( "Import the specified files as separate calendars" ) );
  options.add( "m" );
  options.add( "merge",
               ki18n( "Merge the specified files into the standard calendar " ) );
  options.add( "o" );
  options.add( "open",
               ki18n( "Open the specified files as calendars in a new window" ) );
  options.add( "+[calendars]",
               ki18n( "Calendar files or urls. "
                      "Unless -i, -o or -m is explicitly specified, "
                      "the user will be asked whether to import, merge "
                      "or open in a separate window." ) );
  return options;
}

#endif

