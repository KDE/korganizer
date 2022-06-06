/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KontactInterface/Plugin>
class KAboutData;

class SpecialdatesPlugin : public KontactInterface::Plugin
{
    Q_OBJECT
public:
    SpecialdatesPlugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &);
    ~SpecialdatesPlugin() override;

    int weight() const override
    {
        return 325;
    }

    const KAboutData aboutData() override;

    KontactInterface::Summary *createSummaryWidget(QWidget *parentWidget) override;

protected:
    KParts::Part *createPart() override
    {
        return nullptr;
    }
};
