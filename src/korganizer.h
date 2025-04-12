/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1997, 1998, 1999 Preston Brown <preston.brown@yale.edu>
  SPDX-FileCopyrightText: Fester Zigterman <F.J.F.ZigtermanRustenburg@student.utwente.nl>
  SPDX-FileCopyrightText: Ian Dawes <iadawes@globalserve.net>
  SPDX-FileCopyrightText: Laszlo Boloni <boloni@cs.purdue.edu>

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "mainwindow.h"

#include <KParts/MainWindow>

class CalendarView;
// Workaround for moc workaround for visual c++ 6.0 sucking
using KOrgMainWindow = KOrg::MainWindow;
using KPartsMainWindow = KParts::MainWindow;

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
    ~KOrganizer() override;

    void init(bool hasDocument) override;

    KOrg::CalendarViewBase *view() const override;
    ActionManager *actionManager() override;
    [[nodiscard]] KActionCollection *getActionCollection() const override;

    /**
      Open calendar file from URL. Merge into current calendar, if \a merge is
      true.
        @param url The URL to open
        @param merge true if the incidences in URL should be imported into the
                     current calendar (default resource or calendar file),
                     false if the URL should be added as a new resource.
        @return true on success, false if an error occurred
    */
    [[nodiscard]] bool openURL(const QUrl &url, bool merge = false) override;

    /** Save calendar file to URL of current calendar */
    [[nodiscard]] bool saveURL() override;

    /** Save calendar file to URL */
    [[nodiscard]] bool saveAsURL(const QUrl &url) override;

    /** Get current URL */
    [[nodiscard]] QUrl getCurrentURL() const override;

    KXMLGUIFactory *mainGuiFactory() override;
    KXMLGUIClient *mainGuiClient() override;
    QWidget *topLevelWidget() override;

public Q_SLOTS:
    /** show status message */
    void showStatusMessage(const QString &) override;

protected Q_SLOTS:

    /** using the KConfig associated with the kapp variable, read in the
     * settings from the config file.
     */
    void readSettings();

    /** write current state to config file. */
    void writeSettings();

    /** Sets title of window according to filename and modification state */
    void setTitle() override;

    void slotEditKeys();

protected:
    void initActions();
    //    void initViews();

    /** supplied so that close events close calendar properly.*/
    bool queryClose() override;

    /* Session management */
    void saveProperties(KConfigGroup &) override;
    void readProperties(const KConfigGroup &) override;

private:
    CalendarView *const mCalendarView; // Main view widget

    ActionManager *mActionManager = nullptr;
};
