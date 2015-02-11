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

#include "korganizer/mainwindow.h"

#include <KParts/Part>
#include <kparts/readonlypart.h>

class CalendarView;

namespace Akonadi
{
class Item;
}

namespace KParts
{
class StatusBarExtension;
}

class KOrganizerPart: public KParts::ReadOnlyPart,
    public KOrg::MainWindow
{
    Q_OBJECT
public:
    KOrganizerPart(QWidget *parentWidget, QObject *parent, const QVariantList &);
    virtual ~KOrganizerPart();

    virtual KOrg::CalendarViewBase *view() const Q_DECL_OVERRIDE;

    /**
      Load calendar file from URL and merge it into the current calendar.

      @param url The URL to open.
      @param merge Whether the URL should be imported into the current calendar
      or added as a new calendar resource.

      @return true on success, false if an error occurred
    */
    virtual bool openURL(const QUrl &url, bool merge = false) Q_DECL_OVERRIDE;

    /** Save calendar file to URL of current calendar */
    virtual bool saveURL() Q_DECL_OVERRIDE;

    /** Save calendar file to URL */
    virtual bool saveAsURL(const QUrl &url) Q_DECL_OVERRIDE;

    /** Get current URL */
    virtual QUrl getCurrentURL() const Q_DECL_OVERRIDE;

    virtual KXMLGUIFactory *mainGuiFactory() Q_DECL_OVERRIDE
    {
        return factory();
    }
    virtual KXMLGUIClient *mainGuiClient() Q_DECL_OVERRIDE
    {
        return this;
    }
    virtual QWidget *topLevelWidget() Q_DECL_OVERRIDE;
    virtual ActionManager *actionManager() Q_DECL_OVERRIDE;
    virtual KActionCollection *getActionCollection() const Q_DECL_OVERRIDE
    {
        return actionCollection();
    }
    virtual void showStatusMessage(const QString &message) Q_DECL_OVERRIDE;

    void setTitle() Q_DECL_OVERRIDE;

public Q_SLOTS:
    void slotChangeInfo(const Akonadi::Item &, const QDate &date);

protected:
    virtual bool openFile() Q_DECL_OVERRIDE;

private:
    CalendarView *mView;
    ActionManager *mActionManager;
    KParts::StatusBarExtension *mStatusBarExtension;
    QWidget *mTopLevelWidget;

Q_SIGNALS:
    void textChanged(const QString &);
};

#endif
