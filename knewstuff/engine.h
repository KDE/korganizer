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
#ifndef KNEWSTUFF_ENGINE_H
#define KNEWSTUFF_ENGINE_H

#include <qmap.h>
#include <qobject.h>
#include <qstring.h>

#include "entry.h"
#include "provider.h"

namespace KIO { class Job; }

class KNewStuff;

namespace KNS {

class DownloadDialog;
class UploadDialog;
class ProviderDialog;

class Engine : public QObject
{
    Q_OBJECT
  public:
    Engine( KNewStuff *, const QString &, QWidget *parentWidget = 0 );
    virtual ~Engine();
    
    QString type() const { return mType; }

    QWidget *parentWidget() const { return mParentWidget; }
    
    void download();
    void upload();

    void download( Entry * );

    void requestMetaInformation( Provider * );
    
    void upload( Entry * );
    
  protected slots:
    void getMetaInformation( Provider::List *providers );
    void selectUploadProvider( Provider::List *providers );

    void slotNewStuffJobData( KIO::Job *job, const QByteArray &data );
    void slotNewStuffJobResult( KIO::Job *job );
    
    void slotDownloadJobResult( KIO::Job *job );

    void slotUploadPayloadJobResult( KIO::Job *job );
    void slotUploadMetaJobResult( KIO::Job *job );

  protected:
    bool createMetaFile( Entry * );

  private:
    QWidget *mParentWidget;
  
    ProviderLoader *mProviderLoader;
  
    QMap<KIO::Job *,QCString> mNewStuffJobData;
    
    QPtrList<Entry> mNewStuffList;
    
    DownloadDialog *mDownloadDialog;
    UploadDialog *mUploadDialog;
    ProviderDialog *mProviderDialog;
    
    QString mDownloadDestination;
    
    Provider *mUploadProvider;

    QString mUploadMetaFile;

    KNewStuff *mNewStuff;

    QString mType;
};

}

#endif
