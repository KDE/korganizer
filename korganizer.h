/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998, 1999  Preston Brown <preston.brown@yale.edu>
  Fester Zigterman <F.J.F.ZigtermanRustenburg@student.utwente.nl>
  Ian Dawes <iadawes@globalserve.net>
  Laszlo Boloni <boloni@cs.purdue.edu>

  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KORG_KORGANIZER_H
#define KORG_KORGANIZER_H

#include "korganizer/mainwindow.h"
#include "korganizer/part.h"

#include <KParts/MainWindow>

class CalendarView;

// Workaround for moc workaround for visual c++ 6.0 sucking
typedef KOrg::MainWindow KOrgMainWindow;
typedef KParts::MainWindow KPartsMainWindow;

/**
  This is the main class for KOrganizer. It extends the KDE KMainWindow.
  it provides the main view that the user sees upon startup, as well as
  menus, buttons, etc. etc.

  @short constructs a new main window for korganizer
  @author Preston Brown
*/
class KOrganizer : public KPartsMainWindow, public KOrgMainWindow
{
    Q_OBJECT
public:
    KOrganizer();
    virtual ~KOrganizer();

    void init(bool hasDocument) Q_DECL_OVERRIDE;

    KOrg::CalendarViewBase *view() const Q_DECL_OVERRIDE;
    ActionManager *actionManager() Q_DECL_OVERRIDE {
        return mActionManager;
    }
    KActionCollection *getActionCollection() const Q_DECL_OVERRIDE
    {
        return actionCollection();
    }

    /**
      Open calendar file from URL. Merge into current calendar, if \a merge is
      true.
        @param url The URL to open
        @param merge true if the incidences in URL should be imported into the
                     current calendar (default resource or calendar file),
                     false if the URL should be added as a new resource.
        @return true on success, false if an error occurred
    */
    bool openURL(const QUrl &url, bool merge = false) Q_DECL_OVERRIDE;

    /** Save calendar file to URL of current calendar */
    bool saveURL() Q_DECL_OVERRIDE;

    /** Save calendar file to URL */
    bool saveAsURL(const QUrl &url) Q_DECL_OVERRIDE;

    /** Get current URL */
    QUrl getCurrentURL() const Q_DECL_OVERRIDE;

    KXMLGUIFactory *mainGuiFactory() Q_DECL_OVERRIDE {
        return factory();
    }
    KXMLGUIClient *mainGuiClient() Q_DECL_OVERRIDE {
        return this;
    }
    QWidget *topLevelWidget() Q_DECL_OVERRIDE {
        return this;
    }

public Q_SLOTS:
    /** show status message */
    void showStatusMessage(const QString &) Q_DECL_OVERRIDE;

protected Q_SLOTS:

    /** using the KConfig associated with the kapp variable, read in the
     * settings from the config file.
     */
    void readSettings();

    /** write current state to config file. */
    void writeSettings();

    /** Sets title of window according to filename and modification state */
    void setTitle() Q_DECL_OVERRIDE;

    void newMainWindow(const QUrl &);

    void slotEditKeys();

protected:
    void initActions();
//    void initViews();

    /** supplied so that close events close calendar properly.*/
    bool queryClose() Q_DECL_OVERRIDE;

    /* Session management */
    void saveProperties(KConfigGroup &) Q_DECL_OVERRIDE;
    void readProperties(const KConfigGroup &) Q_DECL_OVERRIDE;

private:
    CalendarView *mCalendarView;  // Main view widget
    KOrg::Part::List mParts; // List of parts loaded

    ActionManager *mActionManager;
};

#endif
