/*
  This file is part of KOrganizer.

  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Sergio Martins, <sergio.martins@kdab.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "previewdialog.h"
#include "koprefs.h"
#include "views/listview/kolistview.h"

#include <KCalCore/FileStorage>
#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>

#include <KFileDialog>
#include <KMessageBox>

#include <KIO/NetAccess>

#include <QFrame>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KGuiItem>

PreviewDialog::PreviewDialog( const KUrl &url, QWidget *parent )
  : QDialog( parent ), mOriginalUrl( url ), mFileStorage( 0 )
{
  setWindowTitle( i18n( "Import Calendar/Event" ) );
  // KGuiItem( i18n("&Merge into existing calendar"), "merge" )
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *user1Button = new QPushButton;
  buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
  QPushButton *user2Button = new QPushButton;
  buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &PreviewDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &PreviewDialog::reject);
  user1Button->setDefault(true);
  QFrame *topFrame = new QFrame( this );
  mainLayout->addWidget(topFrame);
  mainLayout->addWidget(buttonBox);

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  //QT5 topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  mCalendar =
    KCalCore::MemoryCalendar::Ptr(
      new KCalCore::MemoryCalendar( KOPrefs::instance()->mTimeZoneId ) );

  mListView = new KOListView( Akonadi::ETMCalendar::Ptr(), this, true );
  topLayout->addWidget( mListView );

  //QT5 topLayout->setSpacing( spacingHint() );
  //QT5 topLayout->setMargin( marginHint() );

  connect(user1Button, &QPushButton::clicked, this, &PreviewDialog::slotMerge);
  connect(user2Button, &QPushButton::clicked, this, &PreviewDialog::slotAdd);

  // when someone edits a kmail attachment he's editing a tmp file, check for that
  // and if it's a tmp file then open a save dialog
  if ( isTempFile() ) {
    KGuiItem::assign(user2Button, KGuiItem( i18n( "&Add as new calendar..." )));
  } else {
    KGuiItem::assign(user2Button, KGuiItem( i18n( "&Add as new calendar" )));
  }

  mLocalUrl = 0;
}

PreviewDialog::~PreviewDialog()
{
  if ( mLocalUrl && !mOriginalUrl.isLocalFile() ) {
    KIO::NetAccess::removeTempFile( mLocalUrl->path() );
    delete mLocalUrl;
  }

  delete mFileStorage;
}

bool PreviewDialog::loadCalendar()
{
  // If it's a remote file, download it so we can give it to CalendarLocal
  if ( !mOriginalUrl.isLocalFile() ) {
    if ( mLocalUrl ) {
      // loadCalendar already called.. remove old one.
      KIO::NetAccess::removeTempFile( mLocalUrl->path() );
      delete mLocalUrl;
    }

    QString tmpFile;
    if ( KIO::NetAccess::download( mOriginalUrl, tmpFile, 0 ) ) {
      mLocalUrl = new KUrl( tmpFile );
    } else {
      mLocalUrl = 0;
    }
  } else {
    mLocalUrl = &mOriginalUrl;
  }

  if ( mLocalUrl ) {
    mFileStorage = new KCalCore::FileStorage( mCalendar,
                                              mLocalUrl->path(),
                                              new KCalCore::ICalFormat() );

    const bool success = mFileStorage->load();

    if ( !success && !mOriginalUrl.isLocalFile() ) {
      KIO::NetAccess::removeTempFile( mLocalUrl->path() );
    } else {
      mListView->showAll();
    }
    return success;
  } else {
    return false;
  }
}

void PreviewDialog::slotMerge()
{
  if ( mLocalUrl ) {
    emit openURL( *mLocalUrl, true );
    emit dialogFinished( this );
    accept();
  }
}

void PreviewDialog::slotAdd()
{
  KUrl finalUrl = mOriginalUrl;
  if ( isTempFile() ) {
    const QString fileName =
      KFileDialog::getSaveFileName( QString(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/korganizer/")) ,
                                    i18n( "*.vcs *.ics|Calendar Files" ),
                                    this, i18n( "Select path for new calendar" ) );

    finalUrl = KUrl( fileName );

    if ( !KIO::NetAccess::file_copy( mOriginalUrl, finalUrl, this ) &&
         KIO::NetAccess::lastError() ) {
      KMessageBox::error( this, KIO::NetAccess::lastErrorString() );
      return;
    }
  }

  if ( finalUrl.isValid() ) {
    emit addResource( finalUrl );
    emit dialogFinished( this );
    accept();
  }
}

bool PreviewDialog::isTempFile() const
{
  const QString tmpPath = QDir::tempPath();
  return mOriginalUrl.path().startsWith( tmpPath );
}

