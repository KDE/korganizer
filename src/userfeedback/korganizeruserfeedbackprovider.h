/*
   SPDX-FileCopyrightText: 2020-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KORGANIZERUSERFEEDBACKPROVIDER_H
#define KORGANIZERUSERFEEDBACKPROVIDER_H

#include "korganizerprivate_export.h"
#include <KUserFeedback/Provider>

class KORGANIZERPRIVATE_EXPORT KOrganizerUserFeedbackProvider : public KUserFeedback::Provider
{
    Q_OBJECT
public:
    explicit KOrganizerUserFeedbackProvider(QObject *parent = nullptr);
    ~KOrganizerUserFeedbackProvider() override;
};

#endif // KORGANIZERUSERFEEDBACKPROVIDER_H
