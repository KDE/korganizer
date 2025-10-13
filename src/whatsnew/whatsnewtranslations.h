/*
   SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>
   SPDX-FileCopyrightText: 2025 Allen Winter <winter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once
#include "config-korganizer.h"
#include "korganizerprivate_export.h"
#include <PimCommon/WhatsNewTranslationsBase>

class KORGANIZERPRIVATE_EXPORT WhatsNewTranslations : public PimCommon::WhatsNewTranslationsBase
{
public:
    WhatsNewTranslations();
    ~WhatsNewTranslations() override;

#if HAVE_TEXTUTILS_HAS_WHATSNEW_SUPPORT
    [[nodiscard]] QList<TextAddonsWidgets::WhatsNewInfo> createWhatsNewInfo() const override;
#else
    [[nodiscard]] QList<PimCommon::WhatsNewInfo> createWhatsNewInfo() const override;
#endif
    [[nodiscard]] QList<KLazyLocalizedString> lastNewFeatures() const override;

private:
    [[nodiscard]] QList<KLazyLocalizedString> lastNewFeatures65() const;
};
