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

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qstring.h>
#include <ktextedit.h>

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurlrequester.h>
#include <kmessagebox.h>

#include "engine.h"
#include "entry.h"

#include "uploaddialog.h"
#include "uploaddialog.moc"

using namespace KNS;

UploadDialog::UploadDialog( Engine *engine, QWidget *parent ) :
  KDialogBase( Plain, i18n("Share Hot New Stuff"), Ok | Cancel, Cancel,
               parent, 0, false, true ),
  mEngine( engine )
{
  mEntryList.setAutoDelete( true );

  QFrame *topPage = plainPage();

  QGridLayout *topLayout = new QGridLayout( topPage );
  topLayout->setSpacing( spacingHint() );

  QLabel *nameLabel = new QLabel( i18n("Name:"), topPage );
  topLayout->addWidget( nameLabel, 0, 0 );  
  mNameEdit = new QLineEdit( topPage );
  topLayout->addWidget( mNameEdit, 0, 1 );

  QLabel *authorLabel = new QLabel( i18n("Author:"), topPage );
  topLayout->addWidget( authorLabel, 1, 0 );
  mAuthorEdit = new QLineEdit( topPage );
  topLayout->addWidget( mAuthorEdit, 1, 1 );

  QLabel *versionLabel = new QLabel( i18n("Version:"), topPage );
  topLayout->addWidget( versionLabel, 2, 0 );  
  mVersionEdit = new QLineEdit( topPage );
  topLayout->addWidget( mVersionEdit, 2, 1 );

  QLabel *releaseLabel = new QLabel( i18n("Release:"), topPage );
  topLayout->addWidget( releaseLabel, 3, 0 );  
  mReleaseSpin = new QSpinBox( topPage );
  mReleaseSpin->setMinValue( 1 );
  topLayout->addWidget( mReleaseSpin, 3, 1 );

  QLabel *licenceLabel = new QLabel( i18n("License:"), topPage );
  topLayout->addWidget( licenceLabel, 4, 0 );
  mLicenceCombo = new QComboBox( topPage );
  mLicenceCombo->setEditable( true );
  mLicenceCombo->insertItem( i18n("GPL") );
  mLicenceCombo->insertItem( i18n("LGPL") );
  mLicenceCombo->insertItem( i18n("BSD") );
  topLayout->addWidget( mLicenceCombo, 4, 1 );

  QLabel *languageLabel = new QLabel( i18n("Language:"), topPage );
  topLayout->addWidget( languageLabel, 5, 0 );
  mLanguageCombo = new QComboBox( topPage );
  topLayout->addWidget( mLanguageCombo, 5, 1 );
  mLanguageCombo->insertStringList( KGlobal::locale()->languagesTwoAlpha() );

  QLabel *previewLabel = new QLabel( i18n("Preview URL:"), topPage );
  topLayout->addWidget( previewLabel, 6, 0 );
  mPreviewUrl = new KURLRequester( topPage );
  topLayout->addWidget( mPreviewUrl, 6, 1 );

  QLabel *summaryLabel = new QLabel( i18n("Summary:"), topPage );
  topLayout->addMultiCellWidget( summaryLabel, 7, 7, 0, 1 );
  mSummaryEdit = new KTextEdit( topPage );
  topLayout->addMultiCellWidget( mSummaryEdit, 8, 8, 0, 1 );
}

UploadDialog::~UploadDialog()
{
  mEntryList.clear();
}

void UploadDialog::slotOk()
{
  if ( mNameEdit->text().isEmpty() ) {
    KMessageBox::error( this, i18n("Please put in a name.") );
    return;
  }

  Entry *entry = new Entry;

  mEntryList.append( entry );

  entry->setName( mNameEdit->text() );
  entry->setAuthor( mAuthorEdit->text() );
  entry->setVersion( mVersionEdit->text() );
  entry->setRelease( mReleaseSpin->value() );
  entry->setLicence( mLicenceCombo->currentText() );
  entry->setPreview( KURL( mPreviewUrl->url().section("/", -1) ), mLanguageCombo->currentText() );
  entry->setSummary( mSummaryEdit->text(), mLanguageCombo->currentText() );

  mEngine->upload( entry );

  accept();
}

void UploadDialog::setPreviewFile( const QString &previewFile )
{
  mPreviewUrl->setURL( previewFile );
}

