/*
   SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>
   SPDX-FileCopyrightText: 2025 Allen Winter <winter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "whatsnewtranslations.h"

WhatsNewTranslations::WhatsNewTranslations() = default;

WhatsNewTranslations::~WhatsNewTranslations() = default;

// Use by newFeaturesMD5. changes in this list will trigger a "new stuff" notification
QList<KLazyLocalizedString> WhatsNewTranslations::lastNewFeatures() const
{
    const QList<KLazyLocalizedString> info{
        kli18n("Add What's New menu"),
        kli18n("Add a Date Picker dialog available on the Go menu"),
        kli18n("Agenda view: consolidated the \"Day\", \"Next X Days\", \"Work Schedule\", \"Week\" time ranges into the Agenda tool button"),
        kli18n("Agenda view: added timelabels context menu option to toggle a 24-hour (military style) clock"),
        kli18n("Agenda view: added context menu to create new Event or To-do"),
        kli18n("To-do view: added context menu options for toggling completion and reminders"),
        kli18n("To-do view: save/restore fullview setting"),
        kli18n("To-do view: save/restore flatview setting"),
        kli18n("Date navigator: added an option in the Configure->Views settings for hiding the week numbers"),
        kli18n("Date navigator: the \"Week numbers select a work week when in work week mode\" setting is true by default"),
        kli18n("Incidence editor: remember/restore the template manager geometry"),
        kli18n("Incidence editor: support drag-and-drop attachments"),
        kli18n("The export calendar dialog remembers its last export folder location"),
        kli18n("The import calendar dialog remembers its last import folder location"),
        kli18n("Get Hot New Stuff removed"),
        kli18n("Add calendar file: remote calendars are always read-only"),
        kli18n("Add calendar file: will not permit an empty file path"),
        kli18n("DAV configuration: dialog allows seeing the account password when editing"),
        kli18n("DAV configuration: wizard shows icons for the various providers"),
        kli18n("DAV configuration: wizard updated list of providers")};

    return info;
}

QList<PimCommon::WhatsNewInfo> WhatsNewTranslations::createWhatsNewInfo() const
{
    QList<PimCommon::WhatsNewInfo> listInfo;
    {
        PimCommon::WhatsNewInfo info65;
        info65.setVersion(QStringLiteral("6.5.0"));
        QStringList lst;
        const auto newFeatureStrings = lastNewFeatures();
        for (const KLazyLocalizedString &l : newFeatureStrings) {
            lst += l.toString();
        }
        info65.setNewFeatures(lst);
        info65.setChanges({i18n("Semantics are changed from a \"working schedule\" concept to a \"week schedule\" concept to accommodate non-working users"),
                           i18n("Initial \"New Event\" start times are consistent across all the views"),
                           i18n("Improved default event calendar handling")});
        info65.setBugFixings({
            i18n("Incidences with a long excluded dates list no longer create huge tooltips"),
            i18n("Fixed the organizer email address for incidences with attendees"),
            i18n("Agenda view: events without a duration are displayed as 30 long (rather than 15 minutes long)"),
        });
        listInfo.append(std::move(info65));
    }

    return listInfo;
}
