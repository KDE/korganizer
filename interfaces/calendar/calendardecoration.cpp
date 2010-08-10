/*
  This file is part of the KOrganizer interfaces.

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

#include "calendardecoration.h"
#include "calendardecoration.moc"

using namespace KOrg::CalendarDecoration;

////////////////////////////////////////////////////////////////////////////////

Element::Element( const QString &id )
  : mId( id )
{
}

Element::~Element()
{
}

QString Element::id() const
{
  return mId;
}

QString Element::elementInfo() const
{
  return QString();
}

QString Element::shortText()
{
  return QString();
}

QString Element::longText()
{
  return QString();
}

QString Element::extensiveText()
{
  return QString();
}

QPixmap Element::newPixmap( const QSize & )
{
  return QPixmap();
}

KUrl Element::url()
{
  return KUrl();
}

////////////////////////////////////////////////////////////////////////////////

StoredElement::StoredElement( const QString &id ) : Element( id )
{
}

StoredElement::StoredElement( const QString &id, const QString &shortText )
  : Element( id ), mShortText( shortText )
{
}

StoredElement::StoredElement( const QString &id, const QString &shortText,
                              const QString &longText )
  : Element( id ), mShortText( shortText ), mLongText( longText )
{
}

StoredElement::StoredElement( const QString &id, const QString &shortText,
                              const QString &longText,
                              const QString &extensiveText )
  : Element( id ), mShortText( shortText ), mLongText( longText ),
    mExtensiveText( extensiveText )
{
}

StoredElement::StoredElement( const QString &id, const QPixmap &pixmap )
  : Element( id ), mPixmap( pixmap )
{
}

void StoredElement::setShortText( const QString &t )
{
  mShortText = t;
}

QString StoredElement::shortText()
{
  return mShortText;
}

void StoredElement::setLongText( const QString &t )
{
  mLongText = t;
}

QString StoredElement::longText()
{
  return mLongText;
}

void StoredElement::setExtensiveText( const QString &t )
{
  mExtensiveText = t;
}

QString StoredElement::extensiveText()
{
  return mExtensiveText;
}

void StoredElement::setPixmap( const QPixmap &p )
{
  mPixmap = p;
}

QPixmap StoredElement::pixmap()
{
  return mPixmap;
}

void StoredElement::setUrl( const KUrl &u )
{
  mUrl = u;
}

KUrl StoredElement::url()
{
  return mUrl;
}

////////////////////////////////////////////////////////////////////////////////

Decoration::Decoration()
{
}

Decoration::~Decoration()
{
  Q_FOREACH ( Element::List lst, mDayElements ) {
    qDeleteAll( lst );
    lst.clear();
  }
  Q_FOREACH ( Element::List lst, mWeekElements ) {
    qDeleteAll( lst );
    lst.clear();
  }
  Q_FOREACH ( Element::List lst, mMonthElements ) {
    qDeleteAll( lst );
    lst.clear();
  }
  Q_FOREACH ( Element::List lst, mYearElements ) {
    qDeleteAll( lst );
    lst.clear();
  }
  mDayElements.clear();
  mWeekElements.clear();
  mMonthElements.clear();
  mYearElements.clear();
}

Element::List Decoration::dayElements( const QDate &date )
{
  QMap<QDate,Element::List>::ConstIterator it;
  it = mDayElements.constFind( date );
  if ( it == mDayElements.constEnd() ) {
    return registerDayElements( createDayElements( date ), date );
  } else {
    return *it;
  }
}

Element::List Decoration::weekElements( const QDate &d )
{
  QDate date = weekDate( d );
  QMap<QDate,Element::List>::ConstIterator it;
  it = mWeekElements.constFind( date );
  if ( it == mWeekElements.constEnd() ) {
    return registerWeekElements( createWeekElements( date ), date );
  } else {
    return *it;
  }
}

Element::List Decoration::monthElements( const QDate &d )
{
  QDate date = monthDate( d );
  QMap<QDate,Element::List>::ConstIterator it;
  it = mMonthElements.constFind( date );
  if ( it == mMonthElements.constEnd() ) {
    return registerMonthElements( createMonthElements( date ), date );
  } else {
    return *it;
  }
}

Element::List Decoration::yearElements( const QDate &d )
{
  QDate date = yearDate( d );
  QMap<QDate,Element::List>::ConstIterator it;
  it = mYearElements.constFind( date );
  if ( it == mYearElements.constEnd() ) {
    return registerYearElements( createYearElements( date ), date );
  } else {
    return *it;
  }
}

Element::List Decoration::registerDayElements( Element::List e, const QDate &d )
{
  mDayElements.insert( d, e );
  return e;
}

Element::List Decoration::registerWeekElements( Element::List e, const QDate &d )
{
  mWeekElements.insert( weekDate( d ), e );
  return e;
}

Element::List Decoration::registerMonthElements( Element::List e, const QDate &d )
{
  mMonthElements.insert( monthDate( d ), e );
  return e;
}

Element::List Decoration::registerYearElements( Element::List e, const QDate &d )
{
  mYearElements.insert( yearDate( d ), e );
  return e;
}

Element::List Decoration::createDayElements( const QDate & )
{
  return Element::List();
}

Element::List Decoration::createWeekElements( const QDate & )
{
  return Element::List();
}

Element::List Decoration::createMonthElements( const QDate & )
{
  return Element::List();
}

Element::List Decoration::createYearElements( const QDate & )
{
  return Element::List();
}

QDate Decoration::weekDate( const QDate &date )
{
  QDate result = date;
  result.addDays( date.dayOfWeek() - 1 );
  return result;
}

QDate Decoration::monthDate( const QDate &date )
{
  return QDate( date.year(), date.month(), 1 );
}

QDate Decoration::yearDate( const QDate &date )
{
  return QDate( date.year(), 1, 1 );
}
