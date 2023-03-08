/*
   SPDX-FileCopyrightText: 2020-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "korganizerprivate_export.h"
#ifdef USE_KUSERFEEDBACK_QT6
#include <KUserFeedbackQt6/Provider>
#else
#include <KUserFeedback/Provider>
#endif

class KORGANIZERPRIVATE_EXPORT KOrganizerUserFeedbackProvider : public KUserFeedback::Provider
{
    Q_OBJECT
public:
    explicit KOrganizerUserFeedbackProvider(QObject *parent = nullptr);
    ~KOrganizerUserFeedbackProvider() override;
};
