/*
   SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>
   SPDX-FileCopyrightText: 2025 Allen Winter <winter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "whatsnewtranslations.h"

WhatsNewTranslations::WhatsNewTranslations() = default;

WhatsNewTranslations::~WhatsNewTranslations() = default;

// Use by newFeaturesMD5
QList<KLazyLocalizedString> WhatsNewTranslations::lastNewFeatures() const
{
    const QList<KLazyLocalizedString> info{
        kli18n("Add What's New menu"),
        kli18n("Add a Date Picker dialog available from the Go menu"),
        kli18n("Agenda view move day, next X Days, work week into the Agenda tool button"),
        kli18n("Agenda view timelabels can switch to a 24-hour clock"),
        kli18n("Agenda view menu adds create new Event/To-do"),
        kli18n("To-do view menu has options for toggling completion and reminders"),
        kli18n("To-do view no longer will show the to-do list in the sidebar"),
        kli18n("Get Hot New Stuff removed"),
    };

    return info;
}

QList<PimCommon::WhatsNewInfo> WhatsNewTranslations::createWhatsNewInfo() const
{
    QList<PimCommon::WhatsNewInfo> listInfo;
    {
        PimCommon::WhatsNewInfo info;
        QStringList lst;
        const auto newFeatureStrings = lastNewFeatures();
        for (const KLazyLocalizedString &l : newFeatureStrings) {
            lst += l.toString();
        }
        info.setNewFeatures(lst);
        // info.setBugFixings({i18n("Fixed Bug XYZ."), i18n("Fixed Bug ABC.")});
        info.setVersion(QStringLiteral("6.5.0"));
        listInfo.append(std::move(info));
    }
    return listInfo;
}
