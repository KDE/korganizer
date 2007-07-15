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

#include "themeimporter.h"
#include "theme.h"

#include <KLocale>

using namespace KOrg;

ThemeImporter::ThemeImporter() : QXmlStreamReader()
{
}

ThemeImporter::ThemeImporter( QIODevice* device ) : QXmlStreamReader( device )
{
  ThemeImporter();
  read( device );
}

ThemeImporter::~ThemeImporter()
{
  qDeleteAll( mPerViewConfigGroups );
  mPerViewConfigGroups.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool ThemeImporter::read( QIODevice* device )
{
  setDevice( device );

  while ( !atEnd() ) {
    readNext();

    if ( isStartElement() ) {
      if ( name( ) == "korganizer-theme"
           && attributes().value("version") == "1.0" )
        readThemeXml();
      else
        raiseError( i18n("This file is not a KOrganizer theme file.") );
    }
  }

  return !error();
}

void ThemeImporter::readThemeXml()
{
  Q_ASSERT( isStartElement() && name() == "korganizer-theme" );

  kDebug() << "Reading theme XML." << endl;
  
  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
        readElement();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void ThemeImporter::readElement( const QString &viewType,
                                 const int year, const int month,
                                 const int day )
{
  if ( name() == "view" )
    readView( viewType, year, month, day );
  else if ( name() == "year" || name() == "month" || name() == "day" )
    readDate( viewType, year, month, day );

  else if ( name() == "grid" )
    readGrid( viewType, year, month, day );
  else if ( name() == "time-labels" )
    readTimeLabels( viewType, year, month, day );
  else if ( name() == "calendar-items" )
    readCalendarItems( viewType, year, month, day );
  else if ( name() == "marcus-bains-line" )
    readMarcusBainsLine( viewType, year, month, day );
  else if ( name() == "holidays" )
    readHolidays( viewType, year, month, day );

  else
    readUnknownElement();
}

void ThemeImporter::readDate( const QString &viewType,
                              const int year, const int month,
                              const int day )
{
  Q_ASSERT( isStartElement() && ( name() == "year" || name() == "month"
      || name() == "day" ) );

  int y = year;
  int m = month;
  int d = day;

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "year" ) {
        y = attributes().value("value").toString().toInt();
        readElement( QString(), y, m, d );
      }
      else if ( name() == "month" ) {
        m = attributes().value("value").toString().toInt();
        readElement( QString(), y, m, d );
      }
      else if ( name() == "day" ) {
        d = attributes().value("value").toString().toInt();
        readElement( QString(), y, m, d );
      }
      kDebug() << "date: " << y << "-" << m << "-" << d;
    }
  }
}

void ThemeImporter::readView( const QString &viewType,
                              const int year, const int month,
                              const int day )
{
  Q_ASSERT( isStartElement() && name() == "view" );

  QString v = attributes().value("type").toString();
  kDebug() << "viewType: " << v << endl;

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      readElement( v, year, month, day );
    }
  }
}

void ThemeImporter::readUnknownElement()
{
  Q_ASSERT( isStartElement() );

  kWarning() << "ThemeImporter: Unknown element found at line " << lineNumber()
             << ", ending at column " << columnNumber() << ": "
             << name().toString() << endl;

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() ) {
      break;
    }

    if ( isStartElement() )
      readUnknownElement();
  }
}

///////////////////////////////////////////////////////////////////////////////

void ThemeImporter::readCalendarItems( const QString &viewType,
                                       const int year, const int month,
                                       const int day )
{
  Q_ASSERT( isStartElement() && name() == "calendar-items" );

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "background" ) {
        setColor( viewType, year, month, day,
                  "CalendarItems__BackgroundColor",
                  attributes().value("color").toString() );
        setPath( viewType, year, month, day,
                 "CalendarItems__BackgroundImage",
                 attributes().value("src").toString() );
        readNext();
      }
      else if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 "CalendarItems__Font",
                 attributes().value("family").toString(),
                 attributes().value("style-hint").toString(),
                 attributes().value("point-size").toString().toInt(),
                 attributes().value("weight").toString().toInt(),
                 attributes().value("style").toString(),
                 attributes().value("stretch-factor").toString().toInt() );
        readNext();
      }
      else if ( name() == "frame" ) {
        setColor( viewType, year, month, day,
                  "CalendarItems__FrameColor",
                  attributes().value("color").toString() );
        readNext();
      }
      else if ( name() == "icon" ) {
        setString( viewType, year, month, day,
                   "CalendarItems__Icon",
                   attributes().value("name").toString() );
        setPath( viewType, year, month, day,
                 "CalendarItems__IconFile",
                 attributes().value("src").toString() );
        readNext();
      }
      else if ( name() == "events" ) {
        readEvents( viewType, year, month, day );
      }
      else if ( name() == "to-dos" ) {
        readToDos( viewType, year, month, day );
      }
      else {
        readUnknownElement();
      }
    }
  }
}

void ThemeImporter::readEvents( const QString &viewType,
                                const int year, const int month,
                                const int day )
{
  Q_ASSERT( isStartElement() && name() == "events" );

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "background" ) {
        setColor( viewType, year, month, day,
                  "Events__BackgroundColor",
                  attributes().value("color").toString() );
        setPath( viewType, year, month, day,
                 "Events__BackgroundImage",
                 attributes().value("src").toString() );
        readNext();
      }
      else if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 "Events__Font",
                 attributes().value("family").toString(),
                 attributes().value("style-hint").toString(),
                 attributes().value("point-size").toString().toInt(),
                 attributes().value("weight").toString().toInt(),
                 attributes().value("style").toString(),
                 attributes().value("stretch-factor").toString().toInt() );
        readNext();
      }
      else if ( name() == "frame" ) {
        setColor( viewType, year, month, day,
                  "Events__FrameColor",
                  attributes().value("color").toString() );
        readNext();
      }
      else if ( name() == "icon" ) {
        setString( viewType, year, month, day,
                   "Events__Icon",
                   attributes().value("name").toString() );
        setPath( viewType, year, month, day,
                 "Events__IconFile",
                 attributes().value("src").toString() );
        readNext();
      }
      else {
        readUnknownElement();
      }
    }
  }
}

void ThemeImporter::readToDos( const QString &viewType,
                               const int year, const int month,
                               const int day )
{
  Q_ASSERT( isStartElement() && name() == "to-dos" );

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "background" ) {
        setColor( viewType, year, month, day,
                  "ToDos__BackgroundColor",
                  attributes().value("color").toString() );
        setPath( viewType, year, month, day,
                 "ToDos__BackgroundImage",
                 attributes().value("src").toString() );
        readNext();
      }
      else if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 "ToDos__Font",
                 attributes().value("family").toString(),
                 attributes().value("style-hint").toString(),
                 attributes().value("point-size").toString().toInt(),
                 attributes().value("weight").toString().toInt(),
                 attributes().value("style").toString(),
                 attributes().value("stretch-factor").toString().toInt() );
        readNext();
      }
      else if ( name() == "frame" ) {
        setColor( viewType, year, month, day,
                  "ToDos__FrameColor",
                  attributes().value("color").toString() );
        readNext();
      }
      else if ( name() == "icon" ) {
        setString( viewType, year, month, day,
                   "ToDos__Icon",
                   attributes().value("name").toString() );
        setPath( viewType, year, month, day,
                 "ToDos__IconFile",
                 attributes().value("src").toString() );
        readNext();
      }
      else {
        readUnknownElement();
      }
    }
  }
}

void ThemeImporter::readCategories( const QString &viewType,
                                    const int year, const int month,
                                    const int day )
{
  Q_ASSERT( isStartElement() && name() == "categories" );

  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readResources( const QString &viewType,
                                   const int year, const int month,
                                   const int day )
{
  Q_ASSERT( isStartElement() && name() == "resources" );

  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readGrid( const QString &viewType,
                              const int year, const int month,
                              const int day )
{
  Q_ASSERT( isStartElement() && name() == "grid" );

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "background" ) {
        setColor( viewType, year, month, day,
                  "Grid__BackgroundColor",
                  attributes().value("color").toString() );
        setPath( viewType, year, month, day,
                 "Grid__BackgroundImage",
                 attributes().value("src").toString() );
        readNext();
      }
      else if ( name() == "work-hours" ) {
        while ( !atEnd() ) {
          readNext();

          if ( isEndElement() )
            break;

          if ( isStartElement() ) {
            if ( name() == "background" ) {
              setColor( viewType, year, month, day,
                        "Grid_WorkHours__BackgroundColor",
                        attributes().value("color").toString() );
              setPath( viewType, year, month, day,
                       "Grid_WorkHours__BackgroundImage",
                       attributes().value("src").toString() );
              readNext();
            }
            else {
              readUnknownElement();
            }
          }
        }
      }
      else {
        readUnknownElement();
      }
    }
  }
}

void ThemeImporter::readHolidays( const QString &viewType,
                                  const int year, const int month,
                                  const int day )
{
  Q_ASSERT( isStartElement() && name() == "holidays" );

  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readMarcusBainsLine( const QString &viewType,
                                         const int year, const int month,
                                         const int day )
{
  Q_ASSERT( isStartElement() && name() == "marcus-bains-line" );

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 "MarcusBainsLine__Font",
                 attributes().value("family").toString(),
                 attributes().value("style-hint").toString(),
                 attributes().value("point-size").toString().toInt(),
                 attributes().value("weight").toString().toInt(),
                 attributes().value("style").toString(),
                 attributes().value("stretch-factor").toString().toInt() );
        readNext();
      }
      else if ( name() == "line" ) {
        setColor( viewType, year, month, day,
                  "MarcusBainsLine__LineColor",
                  attributes().value("color").toString() );
        readNext();
      }
      else {
        readUnknownElement();
      }
    }
  }
}

void ThemeImporter::readTimeLabels( const QString &viewType,
                                    const int year, const int month,
                                    const int day )
{
  Q_ASSERT( isStartElement() && name() == "time-labels" );

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 "TimeLabels__Font",
                 attributes().value("family").toString(),
                 attributes().value("style-hint").toString(),
                 attributes().value("point-size").toString().toInt(),
                 attributes().value("weight").toString().toInt(),
                 attributes().value("style").toString(),
                 attributes().value("stretch-factor").toString().toInt() );
        readNext();
      }
      else {
        readUnknownElement();
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void ThemeImporter::setColor( const QString &viewType,
                              const int year, const int month,
                              const int day,
                              const QString &key, const QString &value )
{
  QString htmlColor = value.toUpper();
  if ( htmlColor.count( QRegExp("^#[0-9A-F]{6}$") ) == 1 ) {
    int r = htmlColor.mid(1,2).toInt(0, 16);
    int g = htmlColor.mid(3,2).toInt(0, 16);
    int b = htmlColor.mid(5,2).toInt(0, 16);
    QColor color(r, g, b);

    foreach ( QString v, Theme::themableViews( viewType ) ) {
      // FIXME: the date is ignored
      kDebug() << "setting: " << v << ": " << key << ": " << value << endl;
      configGroup( v )->writeEntry( v + "__" + key, color );
    }
  }
}

void ThemeImporter::setFont( const QString &viewType,
                             const int year, const int month,
                             const int day,
                             const QString &key,
                             const QString &family, const QString &styleHint,
                             const int pointSize, const int weight,
                             const QString &style, const int stretchFactor )
{
  QFont f( family, pointSize, weight );

  QFont::StyleHint sh = QFont::AnyStyle;
  if ( styleHint == "AnyStyle" )         sh = QFont::AnyStyle;
  else if ( styleHint == "SansSerif" )   sh = QFont::SansSerif;
  else if ( styleHint == "Helvetica" )   sh = QFont::Helvetica;
  else if ( styleHint == "Serif" )       sh = QFont::Serif;
  else if ( styleHint == "Times" )       sh = QFont::Times;
  else if ( styleHint == "TypeWriter" )  sh = QFont::TypeWriter;
  else if ( styleHint == "Courier" )     sh = QFont::Courier;
  else if ( styleHint == "OldEnglish" )  sh = QFont::OldEnglish;
  else if ( styleHint == "Decorative" )  sh = QFont::Decorative;
  else if ( styleHint == "System" )      sh = QFont::System;
  f.setStyleHint( sh );
  QFont::Style s = QFont::StyleNormal;
  if ( style == "Normal" )        s = QFont::StyleNormal;
  else if ( style == "Italic" )   s = QFont::StyleItalic;
  else if ( style == "Oblique" )  s = QFont::StyleOblique;
  f.setStyle( s );
  int sf = ( stretchFactor < 1 ? 100 : stretchFactor );
  f.setStretch( sf );

  foreach ( QString v, Theme::themableViews( viewType ) ) {
    // FIXME: the date is ignored
    kDebug() << "setting: " << v << ": " << key << ": " << family << "\t"
             << styleHint << "\t" << pointSize << "\t" << weight << "\t"
             << style << "\t" << sf << endl;
    configGroup( v )->writeEntry( v + "__" + key, f );
  }
}

void ThemeImporter::setPath( const QString &viewType,
                             const int year, const int month,
                             const int day,
                             const QString &key, const QString &value )
{
  if ( ! value.isEmpty() ) {
    foreach ( QString v, Theme::themableViews( viewType ) ) {
      // FIXME: the date is ignored
      kDebug() << "setting: " << v << ": " << key << ": " << value << endl;
      configGroup( v )->writePathEntry( v + "__" + key, value );
    }
  }
}

void ThemeImporter::setString( const QString &viewType,
                               const int year, const int month,
                               const int day,
                               const QString &key, const QString &value )
{
  if ( ! value.isEmpty() ) {
    foreach ( QString v, Theme::themableViews( viewType ) ) {
      // FIXME: the date is ignored
      kDebug() << "setting: " << v << ": " << key << ": " << value << endl;
      configGroup( v )->writeEntry( v + "__" + key, value );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

KConfigGroup* ThemeImporter::configGroup( const QString &viewType )
{
  QMap<QString, KConfigGroup*>::ConstIterator it;
  KConfigGroup* g;
  if ( ! viewType.isEmpty() ) {
    it = mPerViewConfigGroups.find( viewType );
    if ( it == mPerViewConfigGroups.end() ) {
      g = registerPerViewConfigGroup( createPerViewConfigGroup( viewType ),
                                      viewType );
    } else {
      g = *it;
    }
  }
  return g;
}

QList<KConfigGroup*> ThemeImporter::perViewConfigGroups()
{
  QList<KConfigGroup*> l;
  foreach ( QString v, Theme::themableViews() ) {
      l.append( configGroup( v ) );
  }
  return l;
}

KConfigGroup* ThemeImporter::registerPerViewConfigGroup( KConfigGroup* g,
     const QString &viewType )
{
  mPerViewConfigGroups.insert( viewType, g );
  return g;
}

KConfigGroup* ThemeImporter::createPerViewConfigGroup(
     const QString &viewType ) const
{
  return new KConfigGroup( KSharedConfig::openConfig(), "Theme/" + viewType + " view" );
}
