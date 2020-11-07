/*
   SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USERFEEDBACKMANAGER_H
#define USERFEEDBACKMANAGER_H

#include <QObject>
#include "korganizerprivate_export.h"
namespace KUserFeedback {
class Provider;
}
class KORGANIZERPRIVATE_EXPORT UserFeedBackManager : public QObject
{
    Q_OBJECT
public:
    explicit UserFeedBackManager(QObject *parent = nullptr);
    static UserFeedBackManager *self();

    KUserFeedback::Provider *userFeedbackProvider() const;

private:
    KUserFeedback::Provider *const mUserFeedbackProvider;
};

#endif // USERFEEDBACKMANAGER_H
