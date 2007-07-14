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

void ThemeImporter::readTimeLabels( const QString &viewType,
                                    const int year, const int month,
                                    const int day )
{
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
    int r = htmlColor.mid(1,2).toInt(0, 16);
    int g = htmlColor.mid(3,2).toInt(0, 16);
    int b = htmlColor.mid(5,2).toInt(0, 16);
    QColor color(r, g, b);

    foreach ( KConfigGroup* g, perViewConfigGroups( viewType ) ) {
      // FIXME: the date is ignored
      kDebug() << viewType << ": " << key << ": " << value << endl;
      g->writeEntry( key, color );
    }
  }
}

void ThemeImporter::setPath( const QString &viewType,
                             const int year, const int month,
                             const int day,
                             const QString &key, const QString &value )
{
  if ( ! value.isEmpty() ) {
    foreach ( KConfigGroup* g, perViewConfigGroups( viewType ) ) {
        // FIXME: the date is ignored
      kDebug() << viewType << ": " << key << ": " << value << endl;
      g->writePathEntry( key, value );
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
  f.setStretch( stretchFactor );

  foreach ( KConfigGroup* g, perViewConfigGroups( viewType ) ) {
    // FIXME: the date is ignored
    kDebug() << viewType << ": " << key << ": " << family << "\t"
             << styleHint << "\t" << pointSize << "\t" << weight << "\t"
             << style << "\t" << stretchFactor << endl;
    g->writeEntry( key, f );
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
