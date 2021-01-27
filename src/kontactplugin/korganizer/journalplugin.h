/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2004, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef KONTACT_JOURNALPLUGIN_H
#define KONTACT_JOURNALPLUGIN_H

#include <KontactInterface/Plugin>

class OrgKdeKorganizerCalendarInterface;

namespace KontactInterface
{
class UniqueAppWatcher;
}

class JournalPlugin : public KontactInterface::Plugin
{
    Q_OBJECT
public:
    JournalPlugin(KontactInterface::Core *core, const QVariantList &);
    ~JournalPlugin() override;

    Q_REQUIRED_RESULT bool isRunningStandalone() const override;
    int weight() const override
    {
        return 525;
    }

    Q_REQUIRED_RESULT QStringList invisibleToolbarActions() const override;

    void select() override;

    OrgKdeKorganizerCalendarInterface *interface();

protected:
    KParts::Part *createPart() override;

private Q_SLOTS:
    void slotNewJournal();

private:
    OrgKdeKorganizerCalendarInterface *mIface = nullptr;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher = nullptr;
};

#endif
