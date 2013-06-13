/*
  This file is part of the KOrganizer interfaces.

  Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KORG_INTERFACES_PRINTPLUGIN_H
#define KORG_INTERFACES_PRINTPLUGIN_H

#include <calendarsupport/plugin.h>
#include <Akonadi/Calendar/ETMCalendar>

#include <KCalCore/Incidence>

#include <QPrinter>

namespace Akonadi {
  class ETMCalendar;
}
namespace KOrg {

class CoreHelper;

/**
  Base class of KOrganizer printer class.
*/
class CalPrinterBase
{
  public:
    enum PrintType {
      Incidence = 100,
      Day=200,
      Week=300,
      Month=400,
      Year=900,
      Todolist=1000,
      Journallist=2000,
      WhatsNext=2100,
      ItemList=2200
    };
};

/**
  Base class for KOrganizer printing classes. Each sub class represents one
  calendar print format.
*/
class PrintPlugin : public CalendarSupport::Plugin
{
  public:
    PrintPlugin() : CalendarSupport::Plugin(), mConfigWidget(0), mCoreHelper(0), mPrinter(0),
         mCalendar(0), mConfig(0) {}
    virtual ~PrintPlugin() {}

    typedef QList<PrintPlugin*> List;
    static int interfaceVersion() { return 2; }

    static QString serviceType()
    { return QLatin1String( "KOrganizer/PrintPlugin" ); }

    virtual void setKOrgCoreHelper( KOrg::CoreHelper *helper )
    { mCoreHelper = helper; }

    virtual void setConfig( KConfig *cfg ) { mConfig = cfg; }

    virtual void setCalendar( const Akonadi::ETMCalendar::Ptr &cal ) { mCalendar = cal; }

    virtual void setSelectedIncidences( KCalCore::Incidence::List inc )
    { mSelectedIncidences = inc; }

    virtual KCalCore::Incidence::List selectedIncidences() const
    { return mSelectedIncidences; }

    /**
     Returns KConfig group name where store settings
    */
    virtual QString groupName() = 0;
    /**
      Returns short description of print format.
    */
    virtual QString description() = 0;
    /**
      Returns long description of print format.
    */
    virtual QString info() const = 0;

    /**
      Returns the sort ID of the plugin. This value will be used to identify
      the config widget in the widget stack, and to sort the plugin name in the
      print style selection list.
      If another plugin uses the same ID or a value of -1 is returned, a unique
      (negative) ID will be automatically generated and thus the position of
      the plugin in the selection list is undefined.
    */
    virtual int sortID() { return -1; }

    /**
      Returns true if the plugin should be enabled; false otherwise.
    */
    virtual bool enabled() { return false; }

    QWidget *configWidget( QWidget *w )
    {
      if ( !mConfigWidget ) {
        mConfigWidget = createConfigWidget( w );
        setSettingsWidget();
      }
      return mConfigWidget;
    }
    /* Create the config widget. setSettingsWidget will be automatically
       called on it */
    virtual QWidget *createConfigWidget( QWidget * ) = 0;

    /**
      Actually do the printing.
    */
    virtual void doPrint( QPrinter *printer ) = 0;

    /**
      Orientation of printout. Default is Portrait. If your plugin wants
      to use some other orientation as default (e.g. depending on some
      config settings), implement this function in your subclass and
      return the desired orientation.
    */
    virtual QPrinter::Orientation defaultOrientation()
    { return QPrinter::Portrait; }

    /**
      Load complete config.
    */
    virtual void doLoadConfig() {}
    /**
      Save complete config.
    */
    virtual void doSaveConfig() {}

  public:
    /**
      Read settings from configuration widget and apply them to current object.
    */
    virtual void readSettingsWidget() {}

    /**
      Set configuration widget to reflect settings of current object.
    */
    virtual void setSettingsWidget() {}

    /**
      Set date range which should be printed.
    */
    virtual void setDateRange( const QDate &from, const QDate &to )
    {
      mFromDate = from;
      mToDate = to;
    }

  protected:
    QDate mFromDate;
    QDate mToDate;

  protected:
    QPointer<QWidget> mConfigWidget;
    KOrg::CoreHelper *mCoreHelper;
    /** The printer object. This will only be available in the doPrint method
        of the selected plugin */
    QPrinter *mPrinter;
    Akonadi::ETMCalendar::Ptr mCalendar;
    KCalCore::Incidence::List mSelectedIncidences;
    KConfig *mConfig;
};

class PrintPluginFactory : public CalendarSupport::PluginFactory
{
  public:
    virtual PrintPlugin *createPluginFactory() = 0;
};

}

#endif
