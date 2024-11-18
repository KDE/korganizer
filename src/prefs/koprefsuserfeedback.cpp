/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsuserfeedback.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QVBoxLayout>
#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/userfeedbackmanager.h"

#include <KUserFeedback/FeedbackConfigWidget>

K_PLUGIN_CLASS_WITH_JSON(KOPrefsUserFeedBack, "korganizer_userfeedback.json")

KOPrefsUserFeedBack::KOPrefsUserFeedBack(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , mUserFeedbackWidget(new KUserFeedback::FeedbackConfigWidget(widget()))
{
    auto layout = new QVBoxLayout(widget());
    layout->setContentsMargins({});

    connect(mUserFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &KOPrefsUserFeedBack::markAsChanged);

    layout->addWidget(mUserFeedbackWidget);
}

void KOPrefsUserFeedBack::load()
{
    mUserFeedbackWidget->setFeedbackProvider(UserFeedBackManager::self()->userFeedbackProvider());
    setNeedsSave(false);
}

void KOPrefsUserFeedBack::save()
{
    UserFeedBackManager::self()->userFeedbackProvider()->setTelemetryMode(mUserFeedbackWidget->telemetryMode());
    UserFeedBackManager::self()->userFeedbackProvider()->setSurveyInterval(mUserFeedbackWidget->surveyInterval());
    setNeedsSave(false);
}

#include "koprefsuserfeedback.moc"

#endif
