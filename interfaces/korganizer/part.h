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
#ifndef KORG_PART_H
#define KORG_PART_H

#include <qstring.h>

#include <klibloader.h>
#include <kparts/part.h>

#include <korganizer/mainwindow.h>

namespace KOrg {

class Part : public KParts::Part
{
  public:
    static int interfaceVersion() { return 2; }
    static QString serviceType() { return "KOrganizer/Part"; }

    typedef QPtrList<Part> List;

    Part( MainWindow *parent, const char *name )
      : KParts::Part( parent?(parent->topLevelWidget()):0, name ), mMainWindow( parent ) {}

    virtual ~Part() {}

    virtual QString info() = 0;
    /** short name of the part, used as category in the keybindings dialog */
    virtual QString shortInfo() = 0;

    MainWindow *mainWindow() { return mMainWindow; }

  private:
    MainWindow *mMainWindow;
};

class PartFactory : public KLibFactory
{
  public:
    virtual Part *create( MainWindow *parent, const char *name = 0 ) = 0;

  protected:
    virtual QObject *createObject( QObject *, const char *,const char *,
                                   const QStringList & )
    {
      return 0;
    }
};

}

#endif
