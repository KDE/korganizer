/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KODECORATIONLABEL_H
#define KODECORATIONLABEL_H

#include "calendar/calendardecoration.h"

#include <KUrl>

#include <QtGui/QLabel>

class KODecorationLabel : public QLabel
{
  Q_OBJECT

  public:
    explicit KODecorationLabel( KOrg::CalendarDecoration::Element *e,
                       QWidget *parent = 0 );
    explicit KODecorationLabel( const QString &shortText,
                       const QString &longText = QString(),
                       const QString &extensiveText = QString(),
                       const QPixmap &pixmap = QPixmap(),
                       const KUrl &url = KUrl(),
                       QWidget *parent = 0 );
    ~KODecorationLabel();

  public slots:
    void setExtensiveText( const QString & );
    void setLongText( const QString & );
    void setPixmap( const QPixmap & );
    void setShortText( const QString & );
    void setText( const QString & );
    void setUrl( const KUrl & );
    void useShortText( bool allowAutomaticSqueeze = false );
    void useLongText( bool allowAutomaticSqueeze = false );
    void useExtensiveText( bool allowAutomaticSqueeze = false );
    void usePixmap( bool allowAutomaticSqueeze = false );
    void useDefaultText();

  protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void squeezeContentsToLabel();
    bool mAutomaticSqueeze;
    KOrg::CalendarDecoration::Element *mDecorationElement;
    QString mShortText, mLongText, mExtensiveText;
    QPixmap mPixmap;
    KUrl mUrl;
};

#endif
