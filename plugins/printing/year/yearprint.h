/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_PLUGINS_PRINTING_YEARPRINT_H
#define KORG_PLUGINS_PRINTING_YEARPRINT_H

#include "calprintpluginbase.h"
#include "ui_calprintyearconfig_base.h"

using namespace KOrg;

class CalPrintYear : public CalPrintPluginBase
{
  public:
    CalPrintYear():CalPrintPluginBase() {}
    virtual ~CalPrintYear() {}
    virtual QString groupName() { return QString::fromLatin1( "Print year" ); }
    virtual QString description() { return i18n( "Print &year" ); }
    virtual QString info() const { return i18n( "Prints a calendar for an entire year" ); }
    virtual int sortID() { return CalPrinterBase::Year; }
    virtual bool enabled() { return true; }
    virtual QWidget *createConfigWidget( QWidget * );
    virtual QPrinter::Orientation defaultOrientation();

  public:
    virtual void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate &from, const QDate &to );

  protected:
    int mYear;
    int mPages;
    int mSubDaysEvents, mHolidaysEvents;
};

class CalPrintYearConfig : public QWidget, public Ui::CalPrintYearConfig_Base
{
  public:
    explicit CalPrintYearConfig( QWidget *parent ) : QWidget( parent )
    {
      setupUi( this );
    }
};

#endif
