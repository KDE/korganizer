/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QObject>

namespace KOrg
{
class MainWindow;
}

class QUrl;

/**
  This class manages a list of KOrganizer instances, each associated with a
  window displaying a calendar. It acts as relay for signals between this
  windows and manages information, which requires interaction of all instances.

  @short manages a list of all KOrganizer instances
  @author Cornelius Schumacher
*/
class KOWindowList : public QObject
{
    Q_OBJECT
public:
    /**
      Constructs a new list of KOrganizer windows. There should only be one
      instance of this class. The ActionManager class takes care of this.
    */
    KOWindowList();
    ~KOWindowList() override;

    /**
      Is there a instance with this URL?
    */
    KOrg::MainWindow *findInstance(const QUrl &url);

    /**
      Return default instance. This is the main window for the resource based
      calendar.
    */
    KOrg::MainWindow *defaultInstance();

public Q_SLOTS:
    /**
      Register a main window.
    */
    void addWindow(KOrg::MainWindow *);
    /**
      Unregister a main window.
    */
    void removeWindow(KOrg::MainWindow *);

private:
    QList<KOrg::MainWindow *> mWindowList; // list of all existing KOrganizer instances

    KOrg::MainWindow *mDefaultWindow = nullptr;
};
