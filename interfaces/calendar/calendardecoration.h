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
#ifndef KORG_CALENDARDECORATION_H
#define KORG_CALENDARDECORATION_H

#include <QtCore/QString>
#include <QtCore/QFlags>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtGui/QPixmap>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

namespace CalendarDecoration {

/**
  @class Element

  @brief Class for calendar decoration elements

  It provides entities like texts and pictures for a given date.
  Implementations can implement all functions or only a subset.
 */
class Element
{
  public:
    typedef QList<Element *> List;
  
    Element() {}
    virtual ~Element() {}

    /**
      Return a name for easy identification.
      This will be used for example for internal configuration (position, etc.),
      so don't i18n it and make it unique for your decoration.
     */
    virtual QString id() const { return QString(); }
    /**
      Description of element.
     */
    virtual QString elementInfo() const { return QString(); }
    
    /**
      Return a short text for a given date,
      usually only a few words.
     */
    virtual QString shortText() const
      { return QString(); }
    /**
      Return a long text for a given date.
      This text can be of any length,
      but usually it will have one or a few lines.
      
      Can for example be used as a tool tip.
     */
    virtual QString longText() const
      { return QString(); }
    /**
      Return an extensive text for a given date.
      This text can be of any length,
      but usually it will have one or a few paragraphs.
     */
    virtual QString extensiveText() const
      { return QString(); }

    /**
      Return a pixmap for a give date and a given size.
     */
    virtual QPixmap pixmap( const QSize & ) const
      { return QPixmap(); }

    /**
      Return a URL pointing to more information about the content of the
      element.
     */
    virtual KUrl url() const
      { return KUrl(); }

/*    // TODO: think about this:
  signals:
    virtual void pixmapReady( const QPixmap & ) const;
    */
};

/**
  This class provides a stored element, which contains all data for the given
  date/month/year.
*/
class StoredElement : public Element
{
  public:
    StoredElement() {}
    StoredElement( const QString &shortText ) : mShortText( shortText ) {}
    StoredElement( const QString &shortText, const QString &longText )
      : mShortText( shortText ), mLongText( longText ) {}
    StoredElement( const QPixmap &pixmap ) : mPixmap( pixmap ) {}
    
    void setShortText( const QString &t ) { mShortText = t; }
    QString shortText() const { return mShortText; }

    void setLongText( const QString &t ) { mLongText = t; }
    QString LongText() const { return mLongText; }

    void setExtensiveText( const QString &t ) { mExtensiveText = t; }
    QString ExtensiveText() const { return mExtensiveText; }

    void setPixmap( const QPixmap &p ) { mPixmap = p; }
    QPixmap pixmap() const { return mPixmap; }

    void setUrl( const KUrl &u ) { mUrl = u; }
    KUrl url() const { return mUrl; }

  private:
    QString mShortText;
    QString mLongText;
    QString mExtensiveText;
    QPixmap mPixmap;
    KUrl mUrl;
};


/**
  @class Decoration

  @brief This class provides the interface for a date dependent decoration.

  The decoration is made of various decoration elements,
  which show a defined text/picture/widget for a given date.
 */
class Decoration : public Plugin
{
  public:
    static int interfaceVersion() { return 2; }
    static QString serviceType() { return QLatin1String("Calendar/Decoration"); }

    typedef QList<Decoration*> List;

    Decoration() {}
    virtual ~Decoration()
    {
      mDayElements.clear();
      mWeekElements.clear();
      mMonthElements.clear();
      mYearElements.clear();
    }

    /**
      Return all Elements for the given day.
    */
    virtual Element::List dayElements( const QDate &date )
    {
      QMap<QDate,Element::List>::ConstIterator it;
      it = mDayElements.find( date );
      if ( it == mDayElements.end() ) {
        return registerDayElements( createDayElements( date ), date );
      } else {
        return *it;
      }
    }
    
    /**
      Return all elements for the week the given date belongs to.
    */
    virtual Element::List weekElements( const QDate &d )
    {
      QDate date = weekDate( d );
      QMap<QDate,Element::List>::ConstIterator it;
      it = mWeekElements.find( date );
      if ( it == mWeekElements.end() ) {
        return registerWeekElements( createWeekElements( date ), date );
      } else {
        return *it;
      }
    }

    /**
      Return all elements for the month the given date belongs to.
    */
    virtual Element::List monthElement( const QDate &d )
    {
      QDate date = monthDate( d );
      QMap<QDate,Element::List>::ConstIterator it;
      it = mMonthElements.find( date );
      if ( it == mMonthElements.end() ) {
        return registerMonthElements( createMonthElements( date ), date );
      } else {
        return *it;
      }
    }

    /**
      Return all elements for the year the given date belongs to.
    */
    virtual Element::List yearElement( const QDate &d )
    {
      QDate date = yearDate( d );
      QMap<QDate,Element::List>::ConstIterator it;
      it = mYearElements.find( date );
      if ( it == mYearElements.end() ) {
        return registerYearElements( createYearElements( date ), date );
      } else {
        return *it;
      }
    }

  protected:
    /**
      Register the given elements for the given date. They will be deleted when
      this object is destroyed.
    */
    Element::List registerDayElements( Element::List e, const QDate &d )
    {
      mDayElements.insert( d, e );
      return e;
    }

    /**
      Register the given elements for the week the given date belongs to. They
      will be deleted when this object is destroyed.
    */
    Element::List registerWeekElements( Element::List e, const QDate &d )
    {
      mWeekElements.insert( weekDate( d ), e );
      return e;
    }

    /**
      Register the given elements for the month the given date belongs to. They
      will be deleted when this object is destroyed.
    */
    Element::List registerMonthElements( Element::List e, const QDate &d )
    {
      mMonthElements.insert( monthDate( d ), e );
      return e;
    }

    /**
      Register the given elements for the year the given date belongs to. They
      will be deleted when this object is destroyed.
    */
    Element::List registerYearElements( Element::List e, const QDate &d )
    {
      mYearElements.insert( yearDate( d ), e );
      return e;
    }

    /**
      Create day elements for given date.
    */
    virtual Element::List createDayElements( const QDate & )
    {
      return Element::List();
    }

    /**
      Create elements for the week the given date belongs to. 
    */
    virtual Element::List createWeekElements( const QDate & )
    {
      return Element::List();
    }

    /**
      Create elements for the month the given date belongs to. 
    */
    virtual Element::List createMonthElements( const QDate & )
    {
      return Element::List();
    }

    /**
      Create elements for the year the given date belongs to. 
    */
    virtual Element::List createYearElements( const QDate & )
    {
      return Element::List();
    }

  protected:
    /**
      Map all dates of the same week to a single date.
    */
    QDate weekDate( const QDate &date )
    {
      QDate result = date;
      result.addDays( date.dayOfWeek() - 1 );
      return result;
    }
    
    /**
      Map all dates of the same month to a single date.
    */
    QDate monthDate( const QDate &date )
    {
      return QDate( date.year(), date.month(), 1 );
    }
  
    /**
      Map all dates of the same year to a single date.
    */
    QDate yearDate( const QDate &date )
    {
      return QDate( date.year(), 1, 1 );
    }
  
  private:
    QMap<QDate,Element::List> mDayElements;
    QMap<QDate,Element::List> mWeekElements;
    QMap<QDate,Element::List> mMonthElements;
    QMap<QDate,Element::List> mYearElements;
};

class DecorationFactory : public PluginFactory
{
  public:
    virtual Decoration *create() = 0;
};

}

}

#endif
