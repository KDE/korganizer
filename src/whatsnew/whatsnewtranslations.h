/*
   SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>
   SPDX-FileCopyrightText: 2025 Allen Winter <winter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once
#include "config-korganizer.h"
#include "korganizerprivate_export.h"
#include <TextAddonsWidgets/WhatsNewTranslationsBase>
class KORGANIZERPRIVATE_EXPORT WhatsNewTranslations : public TextAddonsWidgets::WhatsNewTranslationsBase
{
public:
    WhatsNewTranslations();
    ~WhatsNewTranslations() override;

    [[nodiscard]] QList<TextAddonsWidgets::WhatsNewInfo> createWhatsNewInfo() const override;
    [[nodiscard]] QList<KLazyLocalizedString> lastNewFeatures() const override;

private:
    [[nodiscard]] QList<KLazyLocalizedString> lastNewFeatures65() const;
    [[nodiscard]] QList<KLazyLocalizedString> lastNewFeatures66() const;
};
