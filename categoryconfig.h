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
#ifndef CATEGORYCONFIG_H
#define CATEGORYCONFIG_H

#include "korganizer_export.h"

#include <KConfigSkeleton>

#include <QStringList>

class QString;

class KConfig;

class KORGANIZER_CORE_EXPORT CategoryConfig : public QObject
{
  Q_OBJECT
public:
  explicit CategoryConfig( KCoreConfigSkeleton* cfg, QObject* parent=0 );
  ~CategoryConfig();
  QStringList customCategories() const;
  void setCustomCategories( const QStringList &categories );

  void writeConfig();

  static const QString categorySeparator;

private:
  Q_DISABLE_COPY(CategoryConfig)
  class Private;
  Private* const d;
  Q_PRIVATE_SLOT( d, void configChanged() )
};

#endif
