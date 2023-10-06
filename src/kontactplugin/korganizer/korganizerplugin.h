/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KontactInterface/Plugin>

class OrgKdeKorganizerCalendarInterface;

namespace KontactInterface
{
class UniqueAppWatcher;
}

class KOrganizerPlugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    KOrganizerPlugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &);
    ~KOrganizerPlugin() override;

    [[nodiscard]] bool isRunningStandalone() const override;
    int weight() const override
    {
        return 400;
    }

    [[nodiscard]] bool canDecodeMimeData(const QMimeData *) const override;
    void processDropEvent(QDropEvent *) override;

    KontactInterface::Summary *createSummaryWidget(QWidget *parent) override;

    [[nodiscard]] QStringList invisibleToolbarActions() const override;

    void select() override;

    OrgKdeKorganizerCalendarInterface *interface();

protected:
    KParts::Part *createPart() override;

private Q_SLOTS:
    void slotNewEvent();

private:
    OrgKdeKorganizerCalendarInterface *mIface = nullptr;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher = nullptr;
};
