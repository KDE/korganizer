/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
#ifndef THEME_H
#define THEME_H

#include <kurl.h>

#include <QtCore/QDir>
#include <QtCore/QObject>

namespace KOrg {

/**
  @class Theme
  @brief Class for theme management (essentially import/export).
*/
class Theme : public QObject
{
  Q_OBJECT

  public:
    /**
      Import settings from a theme at the specified URL into KOrganizer's
      config.
    */
    static void useThemeFrom( const KUrl &url );

    /**
      Save the current theme settings to a theme file
    */
    static void saveThemeTo( const KUrl &url );

    /**
      Clear the current theme settings
    */
    static void clearCurrentTheme();

    /**
      Return the directory where the current theme is stored.
    */
    static const QDir storageDir();

    /**
      Return all themable views corresponding to the @p viewType.
    */
    static const QStringList themableViews( const QString &viewType = QString() );
};

}

#endif
