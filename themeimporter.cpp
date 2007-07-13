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
      if ( name() == "view" )
        readView();
      else if ( name() == "year" || name() == "month" || name() == "day" )
        readDate();
      else
        readElement();
    }
  }
}

void ThemeImporter::readView()
{
  Q_ASSERT(isStartElement());

  QString viewType = attributes().value("type").toString();
  kDebug() << "view type:" << viewType << endl;
  
  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      readElement( viewType );
    }
  }
}

void ThemeImporter::readDate()
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readElement( const QString &viewType,
                                 const int year, const int month,
                                 const int day )
{
  if ( name() == "grid" )
    readGrid( viewType );
  else if ( name() == "time-labels" )
    readTimeLabels( viewType );
  else if ( name() == "calendar-items" )
    readCalendarItems( viewType );
  else if ( name() == "marcus-bains-line" )
    readMarcusBainsLine( viewType );
  else if ( name() == "holidays" )
    readHolidays( viewType );
  else
    readUnknownElement();
}


void ThemeImporter::readUnknownElement()
{
  Q_ASSERT(isStartElement());

  kWarning() << "ThemeImporter: Unknown element found at line " << lineNumber()
             << ", ending at column " << columnNumber() << ": "
             << name().toString() << endl;
  
  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() ) {
      kDebug() << "is end element of " << name().toString() << endl;
      break;
    }

    if ( isStartElement() )
      readUnknownElement();
  }
}

void ThemeImporter::readGrid( const QString &viewType,
                              const int year, const int month,
                              const int day )
{
  while ( !atEnd() ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      if ( name() == "background" ) {
        kDebug() << "grid bgimg: " << attributes().value("src").toString() << endl;
        setColor( viewType, year, month, day,
                  "Grid__BackgroundColor",
                  attributes().value("color").toString() );
        readNext();
      }
      else if ( name() == "work-hours" ) {
        while ( !atEnd() ) {
          readNext();

          if ( isEndElement() )
            break;

          if ( isStartElement() ) {
            if ( name() == "background" ) {
              kDebug() << "wh bgcolor: " << attributes().value("color").toString() << endl;
              kDebug() << "wh bgimg: " << attributes().value("src").toString() << endl;
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

void ThemeImporter::readTimeLabels( const QString &viewType,
                                    const int year, const int month,
                                    const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readCalendarItems( const QString &viewType,
                                       const int year, const int month,
                                       const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readEvents( const QString &viewType,
                                const int year, const int month,
                                const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readToDos( const QString &viewType,
                               const int year, const int month,
                               const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readCategories( const QString &viewType,
                                    const int year, const int month,
                                    const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readResources( const QString &viewType,
                                   const int year, const int month,
                                   const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readMarcusBainsLine( const QString &viewType,
                                         const int year, const int month,
                                         const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

void ThemeImporter::readHolidays( const QString &viewType,
                                  const int year, const int month,
                                  const int day )
{
  // TODO
  kDebug() << "element not implemented yet." << endl;
}

///////////////////////////////////////////////////////////////////////////////

void ThemeImporter::setColor( const QString &viewType,
                              const int year, const int month,
                              const int day,
                              const QString &key, const QString &value )
{
  QString htmlColor = value.toUpper();
  if ( htmlColor.count( QRegExp("^#[0-9A-F]{6}$") ) == 1 ) {
    // Let's convert from (hexadecimal) QChars to ints:
    QChar c;
    c = htmlColor[1];
    int r = ( (c >= '0' && c <= '9') ? (c.unicode() - '0') :
              ( ( c >= 'A' && c <= 'F' ) ? (c.unicode() - 'A' + 10) : 0 ) );
    r *= 16;
    c = htmlColor[2];
    r += ( (c >= '0' && c <= '9') ? (c.unicode() - '0') :
           ( ( c >= 'A' && c <= 'F' ) ? (c.unicode() - 'A' + 10) : 0 ) );
    c = htmlColor[3];
    int g = ( (c >= '0' && c <= '9') ? (c.unicode() - '0') :
              ( ( c >= 'A' && c <= 'F' ) ? (c.unicode() - 'A' + 10) : 0 ) );
    g *= 16;
    c = htmlColor[4];
    g += ( (c >= '0' && c <= '9') ? (c.unicode() - '0') :
           ( ( c >= 'A' && c <= 'F' ) ? (c.unicode() - 'A' + 10) : 0 ) );
    c = htmlColor[5];
    int b = ( (c >= '0' && c <= '9') ? (c.unicode() - '0') :
              ( ( c >= 'A' && c <= 'F' ) ? (c.unicode() - 'A' + 10) : 0 ) );
    b *= 16;
    c = htmlColor[6];
    b += ( (c >= '0' && c <= '9') ? (c.unicode() - '0') :
           ( ( c >= 'A' && c <= 'F' ) ? (c.unicode() - 'A' + 10) : 0 ) );

    QColor color(r, g, b);
    foreach ( KConfigGroup* g, perViewConfigGroups( viewType ) ) {
      kDebug() << viewType << ": " << key << ": " << value << endl;
      g->writeEntry( key, color );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

QList<KConfigGroup*> ThemeImporter::perViewConfigGroups(
     const QString &viewType )
{
  QMap<QString, KConfigGroup*>::ConstIterator it;
  QList<KConfigGroup*> l;
  if ( ! viewType.isEmpty() ) {
    it = mPerViewConfigGroups.find( viewType );
    if ( it == mPerViewConfigGroups.end() ) {
      l.append( registerPerViewConfigGroup(
                createPerViewConfigGroup( viewType ), viewType
                                          ) );
    } else {
      l.append( *it );
    }
  }
  else {
    foreach ( QString v, themableViews() ) {
      foreach ( KConfigGroup* g, perViewConfigGroups( v ) ) {
        l.append( g );
      }
    }
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
  QString formattedViewType = viewType;
  formattedViewType.replace( 0, 1, viewType.toUpper().left(1) );
  formattedViewType = "Theme/" + formattedViewType + " view";
  return new KConfigGroup( KSharedConfig::openConfig(), formattedViewType );
}

const QStringList ThemeImporter::themableViews() const
{
  QStringList l;
  l.append( "agenda" );
  l.append( "month" );
  return l;
}
