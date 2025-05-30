/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004-2005, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "specialdates_plugin.h"
#include "sdsummarywidget.h"

#include <KontactInterface/Core>

#include <KAboutData>
#include <KLocalizedString>

EXPORT_KONTACT_PLUGIN_WITH_JSON(SpecialdatesPlugin, "specialdatesplugin.json")

SpecialdatesPlugin::SpecialdatesPlugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &)
    : KontactInterface::Plugin(core, core, data, nullptr)
{
    setComponentName(QStringLiteral("korganizer"), i18nc("@info/plain", "KOrganizer"));
}

SpecialdatesPlugin::~SpecialdatesPlugin() = default;

KontactInterface::Summary *SpecialdatesPlugin::createSummaryWidget(QWidget *parentWidget)
{
    return new SDSummaryWidget(this, parentWidget);
}

const KAboutData SpecialdatesPlugin::aboutData()
{
    KAboutData aboutData = KAboutData(QStringLiteral("specialdates"),
                                      i18nc("@info/plain", "Special Dates Summary"),
                                      QStringLiteral("1.0"),
                                      i18nc("@info/plain", "Kontact Special Dates Summary"),
                                      KAboutLicense::LGPL,
                                      i18nc("@info/plain",
                                            "Copyright © 2003 Tobias Koenig\n"
                                            "Copyright © 2004–2010 Allen Winter"));
    aboutData.addAuthor(i18nc("@info:credit", "Allen Winter"), i18nc("@info:credit", "Current Maintainer"), QStringLiteral("winter@kde.org"));
    aboutData.addAuthor(i18nc("@info:credit", "Tobias Koenig"), QString(), QStringLiteral("tokoe@kde.org"));
    aboutData.setProductName("kontact/specialdates");
    return aboutData;
}

#include "specialdates_plugin.moc"

#include "moc_specialdates_plugin.cpp"
