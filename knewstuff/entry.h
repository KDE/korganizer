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
#ifndef KNEWSTUFF_ENTRY_H
#define KNEWSTUFF_ENTRY_H

#include <qdatetime.h>
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kurl.h>

namespace KNS {

class Entry
{
  public:
    Entry();
    Entry( const QDomElement & );
    ~Entry();
    
    void setName( const QString & );
    QString name() const;

    void setType( const QString & );
    QString type() const;

    void setAuthor( const QString & );
    QString author() const;
    
    void setLicence( const QString & );
    QString license() const;

    void setSummary( const QString &, const QString &lang = QString::null );
    QString summary( const QString &lang = QString::null ) const;

    void setVersion( const QString & );
    QString version() const;
    
    void setRelease( int );
    int release() const;

    void setReleaseDate( const QDate & );
    QDate releaseDate() const;
    
    void setPayload( const KURL &, const QString &lan = QString::null );
    KURL payload( const QString &lang = QString::null ) const;
    
    void setPreview( const KURL &, const QString &lan = QString::null );
    KURL preview( const QString &lang = QString::null ) const;
    
    void setRating( int );
    int rating();
    
    void setDownloads( int );
    int downloads();

    /**
      Return the full name for the meta information. It is constructed as
      <name>-<version>-<release>.
    */
    QString fullName();

    QStringList langs();

    void parseDomElement( const QDomElement & );

    QDomElement createDomElement( QDomDocument &, QDomElement &parent );

  protected:
    QDomElement addElement( QDomDocument &doc, QDomElement &parent,
                            const QString &tag, const QString &value );

  private:
    QString mName;
    QString mType;
    QString mAuthor;
    QString mLicence;
    QMap<QString,QString> mSummaryMap;
    QString mVersion;
    int mRelease;
    QDate mReleaseDate;
    QMap<QString,KURL> mPayloadMap;
    QMap<QString,KURL> mPreviewMap;
    int mRating;
    int mDownloads;

    QStringList mLangs;
};

}

#endif
