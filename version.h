/*
    This file is part of KOrganizer.

    Copyright (c) 1998-1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000-2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_VERSION_H
#define KORG_VERSION_H

/*
  Version scheme: "x.y.z build".

  x is the version number.
  y is the major release number.
  z is the minor release number.

  "x.y.z" follow the kdelibs version KOrganizer is released with.
  
  If "z" is 0, it the version is "x.y"
  
  "build" is empty for final versions. For development versions "build" is
  something like "pre", "alpha1", "alpha2", "beta1", "beta2", "rc1", "rc2".

  Examples in chronological order:
  
    3.0
    3.0.1
    3.1 alpha1
    3.1 beta1
    3.1 beta2
    3.1 rc1
    3.1
    3.1.1
    3.2 pre
    3.2 alpha1
*/

static const char korgVersion[] = "3.4";

#endif
