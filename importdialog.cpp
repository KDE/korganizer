/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "importdialog.h"

#include "koprefs.h"
#include "stdcalendar.h"

#include <klocale.h>

#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QGroupBox>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QFrame>

using namespace KCal;

ImportDialog::ImportDialog( const KUrl &url, QWidget *parent )
  : KDialog( parent),
    mUrl( url )
{
  setCaption( i18n("Import Calendar") );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );
  enableButtonSeparator( true );
  QFrame *topFrame = new QFrame(this );
  setMainWidget( topFrame );
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  QString txt = i18n("Import calendar at '%1' into KOrganizer.",
                  mUrl.prettyUrl() );

  topLayout->addWidget( new QLabel( txt, topFrame ) );

  QGroupBox *radioBox = new QGroupBox( topFrame );
  QBoxLayout *boxLayout = new QVBoxLayout( radioBox );
  radioBox->setFlat( true );
  topLayout->addWidget( radioBox );

  mAddButton = new QRadioButton( i18n("Add as new calendar"), radioBox );
  boxLayout->addWidget( mAddButton );

  mMergeButton = new QRadioButton( i18n("Merge into existing calendar"),
                                   radioBox );
  boxLayout->addWidget( mMergeButton );

  mOpenButton = new QRadioButton( i18n("Open in separate window"), radioBox );
  boxLayout->addWidget( mOpenButton );

  mAddButton->setChecked( true );
}

ImportDialog::~ImportDialog()
{
}

void ImportDialog::slotOk()
{
  kDebug(5850) << "Adding resource for url '" << mUrl << "'" << endl;

  if ( mAddButton->isChecked() ) {
    emit addResource( mUrl );
  } else if ( mMergeButton->isChecked() ) {
    // emit a signal to action manager to merge mUrl into the current calendar
    emit openURL( mUrl, true );
  } else if ( mOpenButton->isChecked() ) {
    // emit a signal to the action manager to open mUrl in a separate window
    emit newWindow( mUrl );
  } else {
    kError() << "ImportDialog: internal error." << endl;
  }

  emit dialogFinished( this );
  accept();
}


#include "importdialog.moc"
