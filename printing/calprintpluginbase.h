/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef CALPRINTPLUGINBASE_H
#define CALPRINTPLUGINBASE_H
// #define KORG_NOPRINTER

#ifndef KORG_NOPRINTER

#include <qdatetime.h>
#include <kprinter.h>
#include <libkcal/event.h>
#include "korganizer/printplugin.h"
#include "korganizer/corehelper.h"

class PrintCellItem;
class CalPrintHelper;

namespace KCal {
class Calendar;
class Todo;
}
class QWidget;

using namespace KCal;

/**
  Base class for KOrganizer printing classes. Each sub class represents one
  calendar print format.
*/
class CalPrintPluginBase : public KOrg::PrintPlugin
{
  public:
    /**
      Constructor
    */
    CalPrintPluginBase() : KOrg::PrintPlugin() {}
    virtual ~CalPrintPluginBase() {}

    /**
      Returns widget for configuring the print format.
    */
    virtual QWidget *createConfigWidget( QWidget * );

    /**
      Actually do the printing.

      \param p QPainter the print result is painted to
      \param width Width of printable area
      \param height Height of printable area
    */
    virtual void print( QPainter &p, int width, int height ) = 0;
    /**
      Start printing.
    */
    virtual void doPrint();

    /**
      Load print format configuration from config file.
    */
    virtual void loadConfig() = 0;
    /**
      Write print format configuration to config file.
    */
    virtual void saveConfig() = 0;

    /**
      Load complete config. This also calls loadConfig() of the derived class.
    */
    void doLoadConfig();
    /**
      Save complete config. This also calls saveConfig() of the derived class.
    */
    void doSaveConfig();

  protected:
    bool mUseColors;

  public:
    /**
      Internal class representing the start of a todo.
    */
    class TodoParentStart;
};

#endif

#endif
