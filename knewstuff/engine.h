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

/**
* Central class combining all possible KNewStuff operations.
* In most cases, Engine objects are built and used internally.
* Using this class explicitely does however give fine-grained control about the
* upload and download operations.
*/
class Engine : public QObject
{
    Q_OBJECT
  public:
    /**
      Constructor.

      @param newStuff KNewStuff object.
      @param type Hotstuff data type such as "korganizer/calendar".
      @param parentWidget The parent window.
    */
    Engine( KNewStuff *, const QString &, QWidget *parentWidget = 0 );

    /**
      Destructor.
    */
    virtual ~Engine();

    /**
      Returns the previously set data type.

      @return Hotstuff data type.
    */
    QString type() const { return mType; }

    /**
      Returns the previously set parent widget.

      @return Parent widget.
    */
    QWidget *parentWidget() const { return mParentWidget; }

    /**
      Initiates the download process, retrieving provider lists and invoking
      the download dialog.
    */
    void download();

    /**
      Initiates the upload process, invoking the provider selection dialog
      and the file upload dialog.

      @param fileName Name of the payload data file.
      @param previewName Name of the preview image file.
    */
    void upload( const QString &fileName = QString::null, const QString &previewName = QString::null );

    /**
      Downloads the specified data file.

      @param entry Hotstuff data object.
    */
    void download( Entry * );

    /**
      Asynchronous lookup of provider information such as upload and
      download locations, icon etc.

      @param provider Hotstuff provider.
    */
    void requestMetaInformation( Provider * );

    /**
      Uploads the specified data file to the provider-dependent location.

      @param entry Hotstuff data object.
    */
    void upload( Entry * );

  protected slots:
    void getMetaInformation( Provider::List *providers );
    void selectUploadProvider( Provider::List *providers );

    void slotNewStuffJobData( KIO::Job *job, const QByteArray &data );
    void slotNewStuffJobResult( KIO::Job *job );

    void slotDownloadJobResult( KIO::Job *job );

    void slotUploadPayloadJobResult( KIO::Job *job );
    void slotUploadPreviewJobResult (KIO::Job *job );
    void slotUploadMetaJobResult( KIO::Job *job );

  protected:
    bool createMetaFile( Entry * );

  private:
    QWidget *mParentWidget;

    ProviderLoader *mProviderLoader;

    QMap<KIO::Job *,QString> mNewStuffJobData;
    QMap<KIO::Job *,Provider *> mProviderJobs;

    QPtrList<Entry> mNewStuffList;

    DownloadDialog *mDownloadDialog;
    UploadDialog *mUploadDialog;
    ProviderDialog *mProviderDialog;

    QString mDownloadDestination;

    Provider *mUploadProvider;

    QString mUploadMetaFile;
    QString mUploadFile;
    QString mPreviewFile;

    KNewStuff *mNewStuff;

    QString mType;
};

}

#endif
