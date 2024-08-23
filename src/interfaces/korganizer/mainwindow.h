/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
      Return widget which represents this main window.
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
    [[nodiscard]] bool hasDocument() const;

private:
    bool mDocument = true;
};
}
