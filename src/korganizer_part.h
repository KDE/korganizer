/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>
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

#ifndef KORG_KORGANIZER_PART_H
#define KORG_KORGANIZER_PART_H

#include "mainwindow.h"

#include <KParts/Part>
#include <kparts/readonlypart.h>

class CalendarView;

namespace Akonadi {
class Item;
}

namespace KParts {
class StatusBarExtension;
}

class KOrganizerPart : public KParts::ReadOnlyPart, public KOrg::MainWindow
{
    Q_OBJECT
public:
    KOrganizerPart(QWidget *parentWidget, QObject *parent, const QVariantList &);
    virtual ~KOrganizerPart();

    KOrg::CalendarViewBase *view() const override;

    /**
      Load calendar file from URL and merge it into the current calendar.

      @param url The URL to open.
      @param merge Whether the URL should be imported into the current calendar
      or added as a new calendar resource.

      @return true on success, false if an error occurred
    */
    bool openURL(const QUrl &url, bool merge = false) override;

    /** Save calendar file to URL of current calendar */
    bool saveURL() override;

    /** Save calendar file to URL */
    bool saveAsURL(const QUrl &url) override;

    /** Get current URL */
    QUrl getCurrentURL() const override;

    KXMLGUIFactory *mainGuiFactory() override
    {
        return factory();
    }

    KXMLGUIClient *mainGuiClient() override
    {
        return this;
    }

    QWidget *topLevelWidget() override;
    ActionManager *actionManager() override;
    KActionCollection *getActionCollection() const override
    {
        return actionCollection();
    }

    void showStatusMessage(const QString &message) override;

    void setTitle() override;

public Q_SLOTS:
    void slotChangeInfo(const Akonadi::Item &, const QDate &date);

protected:
    bool openFile() override;

private:
    CalendarView *mView = nullptr;
    ActionManager *mActionManager = nullptr;
    KParts::StatusBarExtension *mStatusBarExtension = nullptr;
    QWidget *mTopLevelWidget = nullptr;

Q_SIGNALS:
    void textChanged(const QString &);
};

#endif
