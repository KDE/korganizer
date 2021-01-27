/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefs.h"
#include "korganizer_debug.h"
#include <QFontDatabase>

class KOPrefsPrivate
{
public:
    KOPrefsPrivate()
        : prefs(new KOPrefs)
    {
    }

    ~KOPrefsPrivate()
    {
        delete prefs;
    }

    KOPrefs *const prefs;
};

Q_GLOBAL_STATIC(KOPrefsPrivate, sInstance)

KOPrefs::KOPrefs()
    : KOPrefsBase()
{
    mEventViewsPrefs = EventViews::PrefsPtr(new EventViews::Prefs(this));

    mDefaultMonthViewFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    // make it a bit smaller
    mDefaultMonthViewFont.setPointSize(qMax(mDefaultMonthViewFont.pointSize() - 2, 6));

    KConfigSkeleton::setCurrentGroup(QStringLiteral("General"));

    monthViewFontItem()->setDefaultValue(mDefaultMonthViewFont);
}

KOPrefs::~KOPrefs()
{
    mEventViewsPrefs->writeConfig();
}

KOPrefs *KOPrefs::instance()
{
    if (!sInstance.exists()) {
        sInstance->prefs->load();
        sInstance->prefs->mEventViewsPrefs->readConfig();
    }

    return sInstance->prefs;
}

void KOPrefs::usrSetDefaults()
{
    setMonthViewFont(mDefaultMonthViewFont);

    KConfigSkeleton::usrSetDefaults();
}

void KOPrefs::usrRead()
{
    KConfigGroup timeScaleConfig(config(), "Timescale");
    setTimeScaleTimezones(timeScaleConfig.readEntry("Timescale Timezones", QStringList()));

    KConfigSkeleton::usrRead();
}

bool KOPrefs::usrSave()
{
    KConfigGroup timeScaleConfig(config(), "Timescale");
    timeScaleConfig.writeEntry("Timescale Timezones", timeScaleTimezones());

    return KConfigSkeleton::usrSave();
}

void KOPrefs::setResourceColor(const QString &cal, const QColor &color)
{
    mEventViewsPrefs->setResourceColor(cal, color);
}

QColor KOPrefs::resourceColor(const QString &cal)
{
    return mEventViewsPrefs->resourceColor(cal);
}

QColor KOPrefs::resourceColorKnown(const QString &cal) const
{
    return mEventViewsPrefs->resourceColorKnown(cal);
}

QStringList KOPrefs::timeScaleTimezones() const
{
    return mTimeScaleTimeZones;
}

void KOPrefs::setTimeScaleTimezones(const QStringList &list)
{
    mTimeScaleTimeZones = list;
}

EventViews::PrefsPtr KOPrefs::eventViewsPreferences() const
{
    return mEventViewsPrefs;
}
