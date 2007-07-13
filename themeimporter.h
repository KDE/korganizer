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
#ifndef THEMEIMPORTER_H
#define THEMEIMPORTER_H

#include <QtXml/QXmlStreamReader>

#include <KConfig>
#include <KUrl>

namespace KOrg {

  class ThemeImporter : public QXmlStreamReader
  {
    public:
      ThemeImporter();
      ThemeImporter( QIODevice* device );
      ~ThemeImporter();

      bool read( QIODevice* device );

    private:
      void readThemeXml();

      void readView();
      void readDate();

      void readElement( const QString &viewType = QString(),
                        const int year = 0, const int month = 0,
                        const int day = 0 );


      void readUnknownElement();

      void readGrid( const QString &viewType = QString(),
                     const int year = 0, const int month = 0,
                     const int day = 0 );
      void readTimeLabels( const QString &viewType = QString(),
                           const int year = 0, const int month = 0,
                           const int day = 0 );
      void readCalendarItems( const QString &viewType = QString(),
                              const int year = 0, const int month = 0,
                              const int day = 0 );
        void readEvents( const QString &viewType = QString(),
                         const int year = 0, const int month = 0,
                         const int day = 0 );
        void readToDos( const QString &viewType = QString(),
                        const int year = 0, const int month = 0,
                        const int day = 0 );
        void readCategories( const QString &viewType = QString(),
                             const int year = 0, const int month = 0,
                             const int day = 0 );
        void readResources( const QString &viewType = QString(),
                            const int year = 0, const int month = 0,
                            const int day = 0 );
      void readMarcusBainsLine( const QString &viewType = QString(),
                                const int year = 0, const int month = 0,
                                const int day = 0 );
      void readHolidays( const QString &viewType = QString(),
                         const int year = 0, const int month = 0,
                         const int day = 0 );

      void setColor( const QString &viewType,
                     const int year, const int month,
                     const int day,
                     const QString &key, const QString &value );

      QList<KConfigGroup*> perViewConfigGroups( const QString &viewType = QString() );
      KConfigGroup* registerPerViewConfigGroup( KConfigGroup* g, const QString &viewType );
      KConfigGroup* createPerViewConfigGroup( const QString &viewType ) const;
      QMap<QString, KConfigGroup*> mPerViewConfigGroups;

      const QStringList themableViews() const;
  };

}

#endif
