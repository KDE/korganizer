/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsuserfeedback.h"
#include <KAboutData>
#include <KLocalizedString>
#include <QVBoxLayout>
#ifdef WITH_KUSERFEEDBACK
#include <KUserFeedback/FeedbackConfigWidget>
#include "userfeedback/userfeedbackmanager.h"
#endif

#ifdef WITH_KUSERFEEDBACK

extern "C"
{
Q_DECL_EXPORT KCModule *create_korguserfeeback(QWidget *parent, const char *)
{
    return new KOPrefsUserFeedBack(parent);
}
}

KOPrefsUserFeedBack::KOPrefsUserFeedBack(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    KAboutData *about = new KAboutData(QStringLiteral("KCMUserFeedBack"),
                                       i18n("KCMUserFeedBack"),
                                       QString(),
                                       i18n("KOrganizer Configure User FeedBack"),
                                       KAboutLicense::LGPL,
                                       i18n("(c) 2020 Laurent Montel"));
    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));
    setAboutData(about);
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});

    mUserFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(this);
    connect(mUserFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &KOPrefsUserFeedBack::markAsChanged);

    layout->addWidget(mUserFeedbackWidget);
    mUserFeedbackWidget->setFeedbackProvider(UserFeedBackManager::self()->userFeedbackProvider());
}

void KOPrefsUserFeedBack::load()
{
    mUserFeedbackWidget->setFeedbackProvider(UserFeedBackManager::self()->userFeedbackProvider());
}

void KOPrefsUserFeedBack::save()
{
    UserFeedBackManager::self()->userFeedbackProvider()->setTelemetryMode(mUserFeedbackWidget->telemetryMode());
    UserFeedBackManager::self()->userFeedbackProvider()->setSurveyInterval(mUserFeedbackWidget->surveyInterval());
}

#endif
