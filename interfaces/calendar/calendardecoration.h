/*
    This file is part of the KOrganizer interfaces.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_CALENDARDECORATION_H
#define KORG_CALENDARDECORATION_H

#include <qstring.h>
#include <qdatetime.h>
#include <qpixmap.h>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

/**
  This class provides the interface for a date dependent decoration.

  It provides entities like texts and pictures for a given date. Implementations
  can implement all functions or only a subset.
*/
class CalendarDecoration : public Plugin
{
  public:
    static int interfaceVersion() { return 2; }
    static QString serviceType() { return "Calendar/Decoration"; }

    typedef QPtrList<CalendarDecoration> List;

    CalendarDecoration() {}
    virtual ~CalendarDecoration() {}

    /**
      Return a short text for a given date, ususally only a few words.
    */
    virtual QString shortText( const QDate & ) { return QString::null; }
    /**
      Return along text for a given date. This text can be of any length, but
      usually it will have one or a few paragraphs.
    */
    virtual QString longText( const QDate & ) { return QString::null; }

    /**
      Return a small pixmap. The size should be something like 30x30 pixels.
    */
    virtual QPixmap smallPixmap( const QDate &) { return QPixmap(); }
    /**
      Return a large pixmap. The size should be something like 300x300 pixels.
    */
    virtual QPixmap largePixmap( const QDate &) { return QPixmap(); }

    /**
      Return a small widget. It should have the size of a pushbutton.
    */
    virtual QWidget *smallWidget( QWidget *, const QDate & ) { return 0; }
};

class CalendarDecorationFactory : public PluginFactory
{
  public:
    virtual CalendarDecoration *create() = 0;
};

}

#endif
