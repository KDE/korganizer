/*
  This file is part of KOrganizer.
  Copyright (c) 2009 Sebastian Sauer <sebsauer@kdab.net>

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

#include "koeditorconfig.h"
#include "koeditorconfig.moc"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

class KOEditorConfig::Private
{
  public:
    static KOEditorConfig *config;
    static void cleanup_config() { delete config; config = 0; }

    QHash<QString, QStringList> mTemplates;
};

KOEditorConfig *KOEditorConfig::Private::config = 0;

KOEditorConfig::KOEditorConfig()
  : KPIM::KPimPrefs()
  , d(new Private)
{
}

KOEditorConfig::~KOEditorConfig()
{
  delete d;
}

KOEditorConfig* KOEditorConfig::instance()
{
  if( ! Private::config)
    setKOEditorConfig(new KOEditorConfig());
  return Private::config;
}

void KOEditorConfig::setKOEditorConfig(KOEditorConfig *config)
{
  delete Private::config;
  Private::config = config;
  qAddPostRoutine(Private::cleanup_config);
}

QString KOEditorConfig::fullName() const
{
  if(Private::config != this)
    return Private::config->fullName();
  return QString();
}

QString KOEditorConfig::email() const
{
  if(Private::config != this)
    return Private::config->email();
  return QString();
}

bool KOEditorConfig::thatIsMe( const QString &mail ) const
{
  if(Private::config != this)
    return Private::config->thatIsMe(mail);
  return false;
}

QStringList KOEditorConfig::allEmails() const
{
  if(Private::config != this)
    return Private::config->allEmails();

  QStringList mails;
  const QString m = email();
  if( ! m.isEmpty())
    mails << m;
  return mails;
}

QStringList KOEditorConfig::fullEmails() const
{
  if(Private::config != this)
    return Private::config->fullEmails();
  return QStringList();
}

bool KOEditorConfig::showTimeZoneSelectorInIncidenceEditor() const
{
  if(Private::config != this)
    return Private::config->showTimeZoneSelectorInIncidenceEditor();
  return true;
}

QStringList& KOEditorConfig::templates(const QString &type)
{
  return d->mTemplates[type];
}
