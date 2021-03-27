/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "koprefs_base.h"
#include "korganizer_core_export.h"

#include <EventViews/Prefs>

class KORGANIZER_CORE_EXPORT KOPrefs : public KOPrefsBase
{
    Q_OBJECT
public:
    ~KOPrefs() override;

    /** Get instance of KOPrefs. It is made sure that there is only one
    instance. */
    static KOPrefs *instance();

    EventViews::PrefsPtr eventViewsPreferences() const;

    /** Set preferences to default values */
    void usrSetDefaults() override;

    /** Read preferences from config file */
    void usrRead() override;

    /** Write preferences to config file */
    bool usrSave() override;

private:
    /** Constructor disabled for public. Use instance() to create a KOPrefs
    object. */
    KOPrefs();
    friend class KOPrefsPrivate;

public:
    void setResourceColor(const QString &, const QColor &);
    QColor resourceColor(const QString &);
    QColor resourceColorKnown(const QString &cal) const;

    QStringList timeScaleTimezones() const;
    void setTimeScaleTimezones(const QStringList &list);

private:
    QFont mDefaultMonthViewFont;

    QStringList mTimeScaleTimeZones;

    EventViews::PrefsPtr mEventViewsPrefs;

public: // Do not use - except in KOPrefsDialogMain
    QString mName;
    QString mEmail;
};

