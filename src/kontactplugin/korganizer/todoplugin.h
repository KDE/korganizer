/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/
#pragma once

#include <KontactInterface/Plugin>

class OrgKdeKorganizerCalendarInterface;

namespace KontactInterface
{
class UniqueAppWatcher;
}

class TodoPlugin : public KontactInterface::Plugin
{
    Q_OBJECT
public:
    TodoPlugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &);
    ~TodoPlugin() override;

    [[nodiscard]] bool isRunningStandalone() const override;
    int weight() const override
    {
        return 450;
    }

    [[nodiscard]] bool canDecodeMimeData(const QMimeData *) const override;
    void processDropEvent(QDropEvent *) override;

    [[nodiscard]] QStringList invisibleToolbarActions() const override;

    KontactInterface::Summary *createSummaryWidget(QWidget *parent) override;

    void select() override;

    OrgKdeKorganizerCalendarInterface *interface();

protected:
    KParts::Part *createPart() override;

private Q_SLOTS:
    void slotNewTodo();

private:
    OrgKdeKorganizerCalendarInterface *mIface = nullptr;
    KontactInterface::UniqueAppWatcher *mUniqueAppWatcher = nullptr;
};
