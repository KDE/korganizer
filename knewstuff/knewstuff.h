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
#ifndef KNEWSTUFF_H
#define KNEWSTUFF_H

#include <qstring.h>

class QWidget;

namespace KNS {
class Engine;
class Entry;
}

/**
  This class provides the functionality to download and upload "new stuff".
  
  Applications have to subclass KNewStuff, implement the pure virtual functions
  and link to against libknewstuff.
  
  By calling download() the download process is started which means that a list
  of "providers" is fetched from a "master server", information about new stuff
  is collected from the providers and presented to the user. Selected entries
  get downloaded and installed to the application. The required functions to
  install new stuff are provided by implementing install(). The location where
  the downloaded files are stored can be customized by reimplementing
  downloadDestination().
  
  By calling upload() the upload process is started which means the user has to
  select a provider from the list fetched from the master server and to put in
  infomration about the entry to be uploaded. Then the file to be uploaded is
  fetched from the application by calling createUploadFile() and transfered to
  the upload destination specified in the provider list.
*/  
class KNewStuff
{
  public:
    /**
      Constructor.
      
      @param type Type of data to be handled, should be something like
                  korganizer/calendar, kword/template, kdesktop/wallpaper.
      @param parentWidget Parent widget of dialogs opened by the KNewStuff
                          engine.
    */
    KNewStuff( const QString &type, QWidget *parentWidget = 0 );
    virtual ~KNewStuff();
    
    /**
      Return type of data.
    */
    QString type() const;
    
    /**
      Return parent widget.
    */
    QWidget *parentWidget() const;
    
    /**
      Start download process.
    */
    void download();
    
    /**
      Start upload process.
    */
    void upload();

    /**
      Upload with pre-defined files.
    */
    void upload( const QString &fileName, const QString previewName );

    /**
      Install file to application. The given fileName points to the file
      downloaded by the KNewStuff engine. This is a temporary file by default.
      The application can do whatever is needed to handle the information
      contained in the file.
      
      The function returns true, when the installation
      was successful and false if were errors.
    
      @param fileName Name of downloaded file.
    */
    virtual bool install( const QString &fileName ) = 0;
    /**
      Create a file to be uploaded to a "new stuff provider" and return the
      filename. The format of the file is application specific. The only
      constraint is that the corresponding install() implementation is able to
      use the file.
      
      @param fileName Name of the file to be written.
      @return true on success, false on error.
    */
    virtual bool createUploadFile( const QString &fileName ) = 0;

    /**
      Return a filename which should be used as destination for downloading the
      specified new stuff entry. Reimplement this function, if you don't want
      the new stuff to be downloaded to a temporary file.
    */
    virtual QString downloadDestination( KNS::Entry *entry );

  private:
    KNS::Engine *mEngine;
};

#endif
