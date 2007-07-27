/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KORG_PICOFTHEDAY_H
#define KORG_PICOFTHEDAY_H

#include <KIO/Job>

#include <calendar/calendardecoration.h>

using namespace KOrg::CalendarDecoration;

class Picoftheday : public Decoration
{
  public:
    Picoftheday();
    ~Picoftheday();

    Element::List createDayElements( const QDate & );

/*    void configure( QWidget *parent );*/

    QString info();

  private:
    QSize mThumbSize;
};


class POTDElement : public StoredElement
{
    Q_OBJECT

  public:
    POTDElement( const QDate &date, const QSize &initialThumbSize );
    ~POTDElement() {}

    void setDate( const QDate &date );
    void setThumbnailSize( const QSize &size );
    QPixmap pixmap( const QSize &size );
    KUrl thumbnailUrl( const KUrl &fullSizeUrl, const int width = 0 ) const;

  public slots:
    void download();
    void getImagePage();
    void getThumbnail();

  signals:
    void gotNewPixmap( const QPixmap & ) const;
    void gotNewShortText( const QString & ) const;
    void gotNewLongText( const QString & ) const;
    void gotNewExtensiveText( const QString & ) const;
    void gotNewUrl( const KUrl & ) const;

  private:
    QDate mDate;
    QString mDescription;
    QString mFileName;
    KUrl mFullSizeImageUrl;
    float mHWRatio;
    QSize mThumbSize;
    KUrl mThumbUrl;

  private slots:
    void downloadStep1Result( KJob* job );
    void downloadStep2Result( KJob* job );
    void downloadStep3Result( KJob* job );
};

#endif
