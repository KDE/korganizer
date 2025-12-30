/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1997-1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once
#include "config-korganizer.h"
#include <KLocalizedString>
#include <QCommandLineParser>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wunused-function")
QT_WARNING_DISABLE_CLANG("-Wunused-function")
static void korganizer_options(QCommandLineParser *parser)
{
    parser->addOption(QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("import"),
                                         i18nc("@info:shell", "Import the specified files as separate calendars")));
    parser->addOption(QCommandLineOption(QStringList() << QStringLiteral("m") << QStringLiteral("merge"),
                                         i18nc("@info:shell", "Merge the specified files into an existing calendar")));
    parser->addOption(QCommandLineOption({QStringLiteral("view")}, i18nc("@info:shell", "Display the specified incidence (by URL)"), i18n("Url")));

    parser->addPositionalArgument(QStringLiteral("calendars"),
                                  i18nc("@info:shell",
                                        "Calendar files or urls. Unless -i or -m is explicitly specified, "
                                        "the user will be asked whether to import or merge"),
                                  QStringLiteral("[calendar…]"));
#if KORGANIZER_WITH_KUSERFEEDBACK
    parser->addOption(QCommandLineOption(QStringLiteral("feedback"), i18nc("@info:shell", "Lists the available options for user feedback")));
#endif
}
QT_WARNING_POP
