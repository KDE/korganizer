/*
    This file is part of KOrganizer.
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

#ifndef POTDWIDGET_H
#define POTDWIDGET_H

#include <kurl.h>
#include <kurllabel.h>
#include <kio/job.h>

#include <QString>
#include <QDate>

class POTDWidget : public KUrlLabel {
  Q_OBJECT

  public:
    POTDWidget( QWidget *parent = 0 );
    virtual ~POTDWidget();
    
    void setAspectRatioMode( const Qt::AspectRatioMode mode );
    void setDate( const QDate &date );
    void setThumbnailSize( const int size );
    void downloadPOTD();
    void getImagePage();
    void generateThumbnailUrl();
    void getThumbnail();

  protected:
    QDate mDate;
    QString mFileName;
    KUrl mImagePageUrl;
    KUrl mThumbUrl;
    int mThumbSize;
    QString mDescription;
    Qt::AspectRatioMode mARMode;
    QPixmap mPixmap;

  private slots:
    void downloadStep1Result( KJob* job );
    void downloadStep2Result( KJob* job );
    void downloadStep3Result( KJob* job );

  public slots:
    void invokeBrowser( const QString &url );
    void resizeEvent( QResizeEvent *event );
};


#endif
