/*
   SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KORGANIZERUSERFEEDBACKPROVIDER_H
#define KORGANIZERUSERFEEDBACKPROVIDER_H

#include <KUserFeedback/Provider>
#include "korganizer_core_export.h"

class KORGANIZER_CORE_EXPORT KOrganizerUserFeedbackProvider : public KUserFeedback::Provider
{
    Q_OBJECT
public:
    explicit KOrganizerUserFeedbackProvider(QObject *parent = nullptr);
    ~KOrganizerUserFeedbackProvider();
};

#endif // KORGANIZERUSERFEEDBACKPROVIDER_H
