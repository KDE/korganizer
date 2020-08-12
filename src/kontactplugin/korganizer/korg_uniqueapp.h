/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KORG_UNIQUEAPP_H
#define KORG_UNIQUEAPP_H

#include <KontactInterface/UniqueAppHandler>

class KOrganizerUniqueAppHandler : public KontactInterface::UniqueAppHandler
{
    Q_OBJECT
public:
    explicit KOrganizerUniqueAppHandler(KontactInterface::Plugin *plugin)
        : KontactInterface::UniqueAppHandler(plugin)
    {
    }

    ~KOrganizerUniqueAppHandler() override
    {
    }

    void loadCommandLineOptions(QCommandLineParser *parser) override;
    int activate(const QStringList &args, const QString &workingDir) override;
};

#endif /* KORG_UNIQUEAPP_H */
