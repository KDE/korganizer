/*
    This file is part of the KOrganizer interfaces.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KORG_TEXTDECORATION_H
#define KORG_TEXTDECORATION_H
// $Id$

#include <qstring.h>
#include <qdatetime.h>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

class TextDecoration : public Plugin {
  public:
    typedef QPtrList<TextDecoration> List;

    TextDecoration() {};
    virtual ~TextDecoration() {};
    
    virtual QString dayShort(const QDate &) { return QString::null; }
    virtual QString dayLong(const QDate &) { return QString::null; }
};

class TextDecorationFactory : public PluginFactory {
  public:
    virtual TextDecoration *create() = 0;
};

}

#endif
