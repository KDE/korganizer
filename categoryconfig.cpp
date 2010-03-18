/*
  This file is part of libkdepim.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "categoryconfig.h"

#include <KConfigGroup>
#include <KConfigSkeleton>
#include <KLocalizedString>

#include <QString>
#include <QStringList>

static QStringList categoryDefaults()
{
  QStringList l;
  l << i18nc( "incidence category: appointment", "Appointment" )
    << i18nc( "incidence category", "Business" )
    << i18nc( "incidence category", "Meeting" )
    << i18nc( "incidence category: phone call","Phone Call" )
    << i18nc( "incidence category", "Education" )
    << i18nc( "incidence category: "
              "official or unofficial observance of "
              "religious/national/cultural/other significance, "
              "often accompanied by celebrations or festivities", "Holiday" )
    << i18nc( "incidence category: "
              "a lengthy time away from work or school, a trip abroad, "
              "or simply a pleasure trip away from home", "Vacation" )
    << i18nc( "incidence category: "
              "examples: anniversary of historical or personal event; "
              "big date; remembrance, etc", "Special Occasion" )
    << i18nc( "incidence category", "Personal" )
    << i18nc( "incidence category: "
              "typically associated with leaving home for business, "
              "and not pleasure", "Travel" )
    << i18nc( "incidence category", "Miscellaneous" )
    << i18nc( "incidence category", "Birthday" );
  return l;
}

class CategoryConfig::Private
{
public:
  explicit Private( KCoreConfigSkeleton* cfg ) : config( cfg )
  {
  }

  void configChanged()
  {

  }

  KCoreConfigSkeleton* config;
};

CategoryConfig::CategoryConfig( KCoreConfigSkeleton* cfg, QObject* parent ) : QObject( parent ), d( new Private( cfg ) )
{
}

CategoryConfig::~CategoryConfig()
{
  delete d;
}

void CategoryConfig::writeConfig()
{
  d->config->writeConfig();
}

QStringList CategoryConfig::customCategories() const
{
  KConfigGroup group( d->config->config(), "General" );
  QStringList cats = group.readEntry( "Custom Categories", QStringList() );

  if ( cats.isEmpty() )
    cats = categoryDefaults();
  cats.sort();
  return cats;
}

void CategoryConfig::setCustomCategories( const QStringList& categories )
{
  KConfigGroup group( d->config->config(), "General" );
  group.writeEntry( "Custom Categories", categories );
}

const QString CategoryConfig::categorySeparator( ":" );

#include "categoryconfig.moc"
