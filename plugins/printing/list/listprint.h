/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef LISTPRINT_H
#define LISTPRINT_H

#include <klocale.h>
#include "calprintpluginbase.h"

#ifndef KORG_NOPRINTER
namespace KCal {
class Calendar;
}

using namespace KCal;

class CalPrintList : public CalPrintPluginBase
{
  public:
    CalPrintList():CalPrintPluginBase() {}
    virtual ~CalPrintList() {}
    virtual QString description() { return i18n("Print list"); }
    virtual QString info() { return i18n("Prints a list of events and to-dos"); }
    virtual QWidget *createConfigWidget( QWidget* );

  public:
    virtual void print(QPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate& from, const QDate& to );

  protected:
    bool mUseDateRange;
};


#endif
#endif
