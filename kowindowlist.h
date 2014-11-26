/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KORG_KOWINDOWLIST_H
#define KORG_KOWINDOWLIST_H

#include <QObject>

namespace KOrg
{
class MainWindow;
}

class KUrl;

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
    virtual ~KOWindowList();

    /**
      Is there only one instance left?
    */
    bool lastInstance();

    /**
      Is there a instance with this URL?
    */
    KOrg::MainWindow *findInstance(const QUrl &url);

    /**
      Return default instance. This is the main window for the resource based
      calendar.
    */
    KOrg::MainWindow *defaultInstance();

public slots:
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

    KOrg::MainWindow *mDefaultWindow;
};

#endif
