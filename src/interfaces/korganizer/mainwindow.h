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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KORG_INTERFACES_MAINWINDOW_H
#define KORG_INTERFACES_MAINWINDOW_H

#include "korganizer_interfaces_export.h"
#include <QString>
class ActionManager;

class KActionCollection;
class QUrl;
class KXMLGUIClient;
class KXMLGUIFactory;

class QWidget;

namespace KOrg
{

class CalendarViewBase;

/**
  @short interface for korganizer main window
  @author Cornelius Schumacher
*/
class KORGANIZER_INTERFACES_EXPORT MainWindow
{
public:
    MainWindow();
    virtual ~MainWindow();

    virtual void init(bool hasDocument);

    virtual CalendarViewBase *view() const = 0;

    /** Load calendar file from URL. Merge into current calendar, if \a merge is true.
         @param url The URL of the calendar to open
         @param merge If true, the items from the url are inserted into the
                      current calendar (default resource). Otherwise the URL
                      is added as a new resource.
    */
    virtual bool openURL(const QUrl &url, bool merge = false) = 0;
    /** Save calendar file to URL of current calendar */
    virtual bool saveURL() = 0;
    /** Save calendar file to URL */
    virtual bool saveAsURL(const QUrl &kurl) = 0;

    /** Get current URL */
    virtual QUrl getCurrentURL() const = 0;

    /**
      Return XML GUI factory of this main window.
    */
    virtual KXMLGUIFactory *mainGuiFactory() = 0;
    /**
      Return XML GUI client of this main window.
    */
    virtual KXMLGUIClient *mainGuiClient() = 0;
    /**
      Return widget whcih represents this main window.
    */
    virtual QWidget *topLevelWidget() = 0;
    /**
      Return ActionManager of this main window.
    */
    virtual ActionManager *actionManager() = 0;
    /**
      Return actionCollection of this main window.
    */
    virtual KActionCollection *getActionCollection() const = 0;
    /**
      Show status message in status bar.
    */
    virtual void showStatusMessage(const QString &message) = 0;

    /**
      Set window title.
    */
    virtual void setTitle() = 0;

    void setHasDocument(bool d);
    bool hasDocument() const;

private:
    bool mDocument;
};

}

#endif
