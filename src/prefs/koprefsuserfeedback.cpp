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

#ifdef USE_KUSERFEEDBACK_QT6
#include <KUserFeedbackQt6/FeedbackConfigWidget>
#else
#include <KUserFeedback/FeedbackConfigWidget>
#endif

K_PLUGIN_CLASS_WITH_JSON(KOPrefsUserFeedBack, "korganizer_userfeedback.json")

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KOPrefsUserFeedBack::KOPrefsUserFeedBack(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , mUserFeedbackWidget(new KUserFeedback::FeedbackConfigWidget(this))
#else
KOPrefsUserFeedBack::KOPrefsUserFeedBack(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , mUserFeedbackWidget(new KUserFeedback::FeedbackConfigWidget(widget()))
#endif
{
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto layout = new QVBoxLayout(this);
#else
    auto layout = new QVBoxLayout(widget());
#endif
    layout->setContentsMargins({});

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

#include "koprefsuserfeedback.moc"

#endif
