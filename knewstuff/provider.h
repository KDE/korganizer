/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KNEWSTUFF_PROVIDER_H
#define KNEWSTUFF_PROVIDER_H

#include <qcstring.h>
#include <qdom.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>

#include <kurl.h>

namespace KIO { class Job; }

namespace KNS {

class Provider
{
  public:
    typedef QPtrList<Provider> List;

    Provider();
    Provider( const QDomElement & );
    ~Provider();
    
    void setName( const QString & );
    QString name() const;

    void setDownloadUrl( const KURL & );
    KURL downloadUrl() const;

    void setUploadUrl( const KURL & );
    KURL uploadUrl() const;

    void setNoUploadUrl( const KURL & );
    KURL noUploadUrl() const;

    void setNoUpload( bool );
    bool noUpload() const;

  protected:
    void parseDomElement( const QDomElement & );

    QDomElement createDomElement( QDomDocument &, QDomElement &parent );

  private:
    QString mName;
    KURL mDownloadUrl;
    KURL mUploadUrl;
    KURL mNoUploadUrl;
    bool mNoUpload;
};

class ProviderLoader : public QObject
{
    Q_OBJECT
  public:
    ProviderLoader( QWidget *parentWidget );

    void load( const QString &type );

  signals:
    void providersLoaded( Provider::List * );

  protected slots:
    void slotJobData( KIO::Job *, const QByteArray & );
    void slotJobResult( KIO::Job * );

  private:
    QWidget *mParentWidget;
    
    QString mJobData;
    
    Provider::List mProviders;
};

}

#endif
