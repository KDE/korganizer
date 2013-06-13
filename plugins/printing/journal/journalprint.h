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

#ifndef KORG_PLUGINS_PRINTING_JOURNALPRINT_H
#define KORG_PLUGINS_PRINTING_JOURNALPRINT_H

#include "calprintpluginbase.h"
#include "ui_calprintjournalconfig_base.h"

using namespace KOrg;

class CalPrintJournal : public CalPrintPluginBase
{
  public:
    CalPrintJournal():CalPrintPluginBase() {}
    virtual ~CalPrintJournal() {}

    virtual QString groupName() { return QString::fromLatin1( "Print journal" ); }
    virtual QString description() { return i18n( "Print &journal" ); }
    virtual QString info() const { return i18n( "Prints all journals for a given date range" ); }
    virtual QWidget *createConfigWidget( QWidget * );
    virtual int sortID() { return CalPrinterBase::Journallist; }
    virtual bool enabled() { return true; }

  public:
    virtual void print( QPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const QDate &from, const QDate &to );

  protected:
    bool mUseDateRange;
};

class CalPrintJournalConfig : public QWidget, public Ui::CalPrintJournalConfig_Base
{
  public:
    explicit CalPrintJournalConfig( QWidget *parent ) : QWidget( parent ) {
      setupUi( this );
    }
};

#endif
