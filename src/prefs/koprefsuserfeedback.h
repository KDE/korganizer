/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KCModule>

#ifdef WITH_KUSERFEEDBACK
namespace KUserFeedback
{
class FeedbackConfigWidget;
}
class KOPrefsUserFeedBack : public KCModule
{
public:
    explicit KOPrefsUserFeedBack(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

protected:
    void load() override;
    void save() override;

private:
    KUserFeedback::FeedbackConfigWidget *mUserFeedbackWidget = nullptr;
};
#endif

