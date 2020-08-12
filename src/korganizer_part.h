/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_KORGANIZER_PART_H
#define KORG_KORGANIZER_PART_H

#include "mainwindow.h"

#include <KParts/ReadOnlyPart>

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
    ~KOrganizerPart() override;

    KOrg::CalendarViewBase *view() const override;

    /**
      Load calendar file from URL and merge it into the current calendar.

      @param url The URL to open.
      @param merge Whether the URL should be imported into the current calendar
      or added as a new calendar resource.

      @return true on success, false if an error occurred
    */
    Q_REQUIRED_RESULT bool openURL(const QUrl &url, bool merge = false) override;

    /** Save calendar file to URL of current calendar */
    Q_REQUIRED_RESULT bool saveURL() override;

    /** Save calendar file to URL */
    Q_REQUIRED_RESULT bool saveAsURL(const QUrl &url) override;

    /** Get current URL */
    Q_REQUIRED_RESULT QUrl getCurrentURL() const override;

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
