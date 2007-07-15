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

  // As the available settings are the same for the various calendar items
  // types, we use a "stack" to keep in mind where we are in the hierarchy
  // while having the possibility of using the same methods to read the
  // settings' tags.
  QList< QPair<QString, QString> > stack;
  stack.append( qMakePair( QString(), QString("CalendarItems") ) );

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() ) {
      if ( stack.count() > 1 ) {
        stack.removeLast();  // We are going down one level
      }
      else {
        break;
      }
    }

    if ( isStartElement() ) {
      /* Item type tags: first level */
      if ( stack.count() == 1 && name() == "events" ) {
        stack.append( qMakePair( QString("events"),
                                 QString("CalendarItemsEvents") ) );
      }
      else if ( stack.count() == 1 && name() == "to-dos" ) {
        stack.append( qMakePair( QString("to-dos"),
                                 QString("CalendarItemsToDos") ) );
      }
      /* Sub-elements of to-dos (second level) */
      else if ( stack.count() == 2 && stack.last().first == "to-dos"
                && name() == "overdue" ) {
        stack.append( qMakePair( QString("to-dos/overdue"),
                      QString("CalendarItemsToDosOverdue")
                               ) );
      }
      else if ( stack.count() == 2 && stack.last().first == "to-dos"
                && name() == "due-today" ) {
        stack.append( qMakePair( QString("to-dos/due-today"),
                      QString("CalendarItemsToDosDueToday")
                               ) );
      }
      /* The sub-elements of these tags allow free text */
      else if ( stack.count() == 1 && name() == "categories" ) {
        stack.append( qMakePair( QString("categories"),
                                 // When a setting applies to all categories,
                                 // it applies to all items.
                                 QString("CalendarItems") ) );
      }
      else if ( stack.count() == 1 && name() == "resources" ) {
        stack.append( qMakePair( QString("resources"),
                                 // When a setting applies to all resources,
                                 // it applies to all items.
                                 QString("CalendarItems") ) );
      }
      /* The said sub-elements */
      else if ( stack.count() == 2 && stack.last().first == "categories"
                && name() == "category" ) {
        QString n = attributes().value("name").toString();
        stack.append( qMakePair( QString("categories/" + n),
                                 QString("CalendarItemsCategories" + n)
                               ) );
      }
      else if ( stack.count() == 2 && stack.last().first == "resources"
                && name() == "resource" ) {
        QString n = attributes().value("name").toString();
        stack.append( qMakePair( QString("resources/" + n),
                                 QString("CalendarItemsResources" + n)
                               ) );
      }
      /* Settings' tags */
      else if ( name() == "background" ) {
        setColor( viewType, year, month, day,
                  stack.last().second + "BackgroundColor",
                  attributes().value("color").toString() );
        setPath( viewType, year, month, day,
                 stack.last().second + "BackgroundImage",
                 attributes().value("src").toString() );
        readNext();
      }
      else if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 stack.last().second + "Font",
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
                  stack.last().second + "FrameColor",
                  attributes().value("color").toString() );
        readNext();
      }
      else if ( name() == "icon" ) {
        setString( viewType, year, month, day,
                   stack.last().second + "Icon",
                   attributes().value("name").toString() );
        setPath( viewType, year, month, day,
                 stack.last().second + "IconFile",
                 attributes().value("src").toString() );
        readNext();
      }
      /* Unknown elements */
      else {
        readUnknownElement();
      }
    }
  }
}

void ThemeImporter::readGrid( const QString &viewType,
                              const int year, const int month,
                              const int day )
{
  Q_ASSERT( isStartElement() && name() == "grid" );

  QString cfg = "Grid";

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "background" ) {
        setColor( viewType, year, month, day,
                  cfg + "BackgroundColor",
                  attributes().value("color").toString() );
        setPath( viewType, year, month, day,
                 cfg + "BackgroundImage",
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
                        cfg + "WorkHoursBackgroundColor",
                        attributes().value("color").toString() );
              setPath( viewType, year, month, day,
                       cfg + "WorkHoursBackgroundImage",
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

  QString cfg = "Holidays";

  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readMarcusBainsLine( const QString &viewType,
                                         const int year, const int month,
                                         const int day )
{
  Q_ASSERT( isStartElement() && name() == "marcus-bains-line" );

  QString cfg = "MarcusBainsLine";

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 cfg + "Font",
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
                  cfg + "LineColor",
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

  QString cfg = "TimeLabels";

  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "font" ) {
        setFont( viewType, year, month, day,
                 cfg + "Font",
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
      configGroup( v )->writeEntry( v + key, color );
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
    configGroup( v )->writeEntry( v + key, f );
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
      configGroup( v )->writePathEntry( v + key, value );
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
      configGroup( v )->writeEntry( v + key, value );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

KConfigGroup* ThemeImporter::configGroup( const QString &viewType )
{
  QMap<QString, KConfigGroup*>::ConstIterator it;
  KConfigGroup* g;
  it = mPerViewConfigGroups.find( viewType );
  if ( it == mPerViewConfigGroups.end() ) {
    g = registerPerViewConfigGroup( createPerViewConfigGroup( viewType ),
                                    viewType );
  } else {
    g = *it;
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
