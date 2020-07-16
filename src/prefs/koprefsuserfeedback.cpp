/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koprefsuserfeedback.h"
#include <KAboutData>
#include <KLocalizedString>
#ifdef WITH_KUSERFEEDBACK
#include <KUserFeedback/FeedbackConfigWidget>
#include "userfeedback/userfeedbackmanager.h"
#include <QVBoxLayout>
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
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

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
