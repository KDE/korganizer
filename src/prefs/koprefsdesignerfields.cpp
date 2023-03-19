/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdesignerfields.h"
#include <CalendarSupport/KCalPrefs>
#include <KPluginFactory>
#include <QStandardPaths>

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDesignerFields, "korganizer_configdesignerfields.json")

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KOPrefsDesignerFields::KOPrefsDesignerFields(QWidget *parent, const QVariantList &args)
    : KCMDesignerFields(parent, args)
#else
KOPrefsDesignerFields::KOPrefsDesignerFields(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCMDesignerFields(parent, data, args)
#endif
{
}

QString KOPrefsDesignerFields::localUiDir()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + uiPath();
    return dir;
}

QString KOPrefsDesignerFields::uiPath()
{
    return QStringLiteral("/korganizer/designer/event/");
}

void KOPrefsDesignerFields::writeActivePages(const QStringList &activePages)
{
    CalendarSupport::KCalPrefs::instance()->setActiveDesignerFields(activePages);
    CalendarSupport::KCalPrefs::instance()->save();
}

QStringList KOPrefsDesignerFields::readActivePages()
{
    return CalendarSupport::KCalPrefs::instance()->activeDesignerFields();
}

QString KOPrefsDesignerFields::applicationName()
{
    return QStringLiteral("KORGANIZER");
}

#include "koprefsdesignerfields.moc"
