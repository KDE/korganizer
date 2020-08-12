/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1997-1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_KORGANIZER_OPTIONS_H
#define KORG_KORGANIZER_OPTIONS_H

#include <QCommandLineParser>
#include <KLocalizedString>

static void korganizer_options(QCommandLineParser *parser)
{
    parser->addOption(QCommandLineOption(
                          QStringList() << QStringLiteral("i") << QStringLiteral("import"),
                              i18n("Import the specified files as separate calendars")));
    parser->addOption(QCommandLineOption(
                          QStringList() << QStringLiteral("m") << QStringLiteral("merge"),
                              i18n("Merge the specified files into an existing calendar")));
    parser->addOption(QCommandLineOption(
                          {QStringLiteral("view")},
                          i18n("Display the specified incidence (by URL)"), QStringLiteral("url")));

    parser->addPositionalArgument(
        QStringLiteral("calendars"),
        i18n("Calendar files or urls. Unless -i or -m is explicitly specified, "
             "the user will be asked whether to import or merge"),
        QStringLiteral("[calendar...]"));
#ifdef WITH_KUSERFEEDBACK
    parser->addOption(QCommandLineOption(QStringLiteral("feedback"), i18n("Lists the available options for user feedback")));
#endif
}

#endif
