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

#include "plugin.h"

#include <kurl.h>

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtGui/QPixmap>

namespace KOrg {

namespace CalendarDecoration {

/**
  @class Element

  @brief Class for calendar decoration elements

  It provides entities like texts and pictures for a given date.
  Implementations can implement all functions or only a subset.
 */
class Element : public QObject
{
  Q_OBJECT

  public:
    typedef QList<Element *> List;

    Element( const QString &id );
    virtual ~Element();

    /**
      Return a name for easy identification.
      This will be used for example for internal configuration (position, etc.),
      so don't i18n it and make it unique for your decoration.
    */
    virtual QString id() const;

    /**
      Description of element.
    */
    virtual QString elementInfo() const;

    /**
      Return a short text for a given date,
      usually only a few words.
    */
    virtual QString shortText();

    /**
      Return a long text for a given date.
      This text can be of any length,
      but usually it will have one or a few lines.

      Can for example be used as a tool tip.
    */
    virtual QString longText();

    /**
      Return an extensive text for a given date.
      This text can be of any length,
      but usually it will have one or a few paragraphs.
    */
    virtual QString extensiveText();

    /**
      Return a pixmap for a given date and a given size.
    */
    virtual QPixmap newPixmap( const QSize & );

    /**
      Return a URL pointing to more information about the content of the
      element.
     */
    virtual KUrl url();

  Q_SIGNALS:
    void gotNewPixmap( const QPixmap & ) const;
    void gotNewShortText( const QString & ) const;
    void gotNewLongText( const QString & ) const;
    void gotNewExtensiveText( const QString & ) const;
    void gotNewUrl( const KUrl & ) const;

  protected:
    QString mId;
};

/**
  This class provides a stored element, which contains all data for the given
  date/month/year.
*/
class StoredElement : public Element
{
  public:
    StoredElement( const QString &id );
    StoredElement( const QString &id, const QString &shortText );
    StoredElement( const QString &id, const QString &shortText,
                   const QString &longText );
    StoredElement( const QString &id, const QString &shortText,
                   const QString &longText, const QString &extensiveText );
    StoredElement( const QString &id, const QPixmap &pixmap );

    virtual void setShortText( const QString &t );
    virtual QString shortText();

    virtual void setLongText( const QString &t );
    virtual QString longText();

    virtual void setExtensiveText( const QString &t );
    virtual QString extensiveText();

    virtual void setPixmap( const QPixmap &p );
    virtual QPixmap pixmap();

    virtual void setUrl( const KUrl &u );
    virtual KUrl url();

  protected:
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
    static QString serviceType() { return QLatin1String( "Calendar/Decoration" ); }

    typedef QList<Decoration*> List;

    Decoration();
    virtual ~Decoration();

    /**
      Return all Elements for the given day.
    */
    virtual Element::List dayElements( const QDate &date );

    /**
      Return all elements for the week the given date belongs to.
    */
    virtual Element::List weekElements( const QDate &d );

    /**
      Return all elements for the month the given date belongs to.
    */
    virtual Element::List monthElements( const QDate &d );

    /**
      Return all elements for the year the given date belongs to.
    */
    virtual Element::List yearElements( const QDate &d );

  protected:
    /**
      Register the given elements for the given date. They will be deleted when
      this object is destroyed.
    */
    Element::List registerDayElements( Element::List e, const QDate &d );

    /**
      Register the given elements for the week the given date belongs to. They
      will be deleted when this object is destroyed.
    */
    Element::List registerWeekElements( Element::List e, const QDate &d );

    /**
      Register the given elements for the month the given date belongs to. They
      will be deleted when this object is destroyed.
    */
    Element::List registerMonthElements( Element::List e, const QDate &d );

    /**
      Register the given elements for the year the given date belongs to. They
      will be deleted when this object is destroyed.
    */
    Element::List registerYearElements( Element::List e, const QDate &d );

    /**
      Create day elements for given date.
    */
    virtual Element::List createDayElements( const QDate & );

    /**
      Create elements for the week the given date belongs to.
    */
    virtual Element::List createWeekElements( const QDate & );

    /**
      Create elements for the month the given date belongs to.
    */
    virtual Element::List createMonthElements( const QDate & );

    /**
      Create elements for the year the given date belongs to.
    */
    virtual Element::List createYearElements( const QDate & );

    /**
      Map all dates of the same week to a single date.
    */
    QDate weekDate( const QDate &date );

    /**
      Map all dates of the same month to a single date.
    */
    QDate monthDate( const QDate &date );

    /**
      Map all dates of the same year to a single date.
    */
    QDate yearDate( const QDate &date );

  private:
    QMap<QDate,Element::List> mDayElements;
    QMap<QDate,Element::List> mWeekElements;
    QMap<QDate,Element::List> mMonthElements;
    QMap<QDate,Element::List> mYearElements;
};

class DecorationFactory : public PluginFactory
{
  public:
    virtual Decoration *createPluginFactory() = 0;
};

}

}

#endif
