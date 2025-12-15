/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2004, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/
#pragma once

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
    JournalPlugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &);
    ~JournalPlugin() override;

    [[nodiscard]] bool isRunningStandalone() const override;
    [[nodiscard]] int weight() const override
    {
        return 525;
    }

    [[nodiscard]] QStringList invisibleToolbarActions() const override;

    void select() override;

    OrgKdeKorganizerCalendarInterface *interface();

protected:
    KParts::Part *createPart() override;

private:
    void slotNewJournal();
    OrgKdeKorganizerCalendarInterface *mIface = nullptr;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher = nullptr;
};
