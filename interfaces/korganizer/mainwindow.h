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
#ifndef KORG_MAINWINDOW_H
#define KORG_MAINWINDOW_H

#include <kxmlguiclient.h>

#include <qwidget.h>

class KActionCollection;
class KAction;

class ActionManager;

namespace KOrg {

class CalendarViewBase;

/**
  @short interface for korganizer main window
  @author Cornelius Schumacher
*/
class MainWindow
{
  public:
    MainWindow( bool document = true ) : mDocument( document ) {};
    virtual ~MainWindow() {};

    virtual CalendarViewBase *view() const = 0;

    /** Load calendar file from URL. Merge into current calendar, if \a merge is true. */
    virtual bool openURL(const KURL &url,bool merge=false) = 0;
    /** Save calendar file to URL of current calendar */
    virtual bool saveURL() = 0;
    /** Save calendar file to URL */
    virtual bool saveAsURL(const KURL & kurl) = 0;

    /** Get current URL */
    virtual KURL getCurrentURL() const = 0;

    /**
      Return XML GUI factory of this main window.
    */
    virtual KXMLGUIFactory *mainGuiFactory() = 0;
    /**
      Return widget whcih represents this main window.
    */
    virtual QWidget *topLevelWidget() = 0;
    /**
      Return ActionManager of this main window.
    */
    virtual ActionManager *actionManager() = 0;
    /**
      ?
    */
    virtual void setActive(bool active) = 0;
    /**
      Show status mesage in status bar.
    */
    virtual void showStatusMessage(const QString& message) = 0;
    /**
      Add action of plugin to main window.
    */
    virtual void addPluginAction( KAction* ) = 0;

    /**
      Set window title.
    */
    virtual void setTitle() = 0;

    bool hasDocument() const { return mDocument; }

  private:
    bool mDocument;
};

}

#endif
