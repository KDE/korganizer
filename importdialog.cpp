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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "importdialog.h"

#include "koprefs.h"
#include "stdcalendar.h"

#include <libkcal/calendarresources.h>
#include <libkcal/resourcelocal.h>
#include <kresources/remote/resourceremote.h>

#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

using namespace KCal;

ImportDialog::ImportDialog( const KURL &url, QWidget *parent )
  : KDialogBase( Plain, i18n("Import Calendar"), Ok | Cancel, Ok, parent,
                 0, true, true ),
    mUrl( url )
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame, 0, spacingHint() );

  QString txt = i18n("Import calendar at '%1' into KOrganizer.")
                .arg( mUrl.prettyURL() );

  topLayout->addWidget( new QLabel( txt, topFrame ) );

  QButtonGroup *radioBox = new QButtonGroup( 1, Horizontal, topFrame );
  radioBox->setFlat( true );
  topLayout->addWidget( radioBox );

  mAddButton = new QRadioButton( i18n("Add as new calendar"), radioBox );
  
  mMergeButton = new QRadioButton( i18n("Merge into existing calendar"),
                                   radioBox );

  mOpenButton = new QRadioButton( i18n("Open in separate window"), radioBox );

  mAddButton->setChecked( true );
}

ImportDialog::~ImportDialog()
{
}

void ImportDialog::slotOk()
{
  kdDebug(5850) << "Adding resource for url '" << mUrl << "'" << endl;

  if ( mAddButton->isChecked() ) {
    emit addResource( mUrl );
  } else if ( mMergeButton->isChecked() ) {
    // emit a signal to action manager to merge mUrl into the current calendar
    emit openURL( mUrl, true );
  } else if ( mOpenButton->isChecked() ) {
    // emit a signal to the action manager to open mUrl in a separate window
    emit newWindow( mUrl );
  } else {
    kdError() << "ImportDialog: internal error." << endl;
  }
  
  emit dialogFinished( this );
  accept();
}


#include "importdialog.moc"
