/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "komessagebox.h"

#include <kmessagebox.h>
#include <kdialog.h>
#include <kpushbutton.h>

int KOMessageBox::fourBtnMsgBox( QWidget *parent, QMessageBox::Icon type,
                                 const QString &text,
                                 const QString &caption,
                                 const KGuiItem &button1,
                                 const KGuiItem &button2,
                                 const KGuiItem &button3,
                                 KMessageBox::Options options )
{
  KDialog *dialog= new KDialog( parent );
  dialog->setCaption( caption );
  dialog->setButtons( KDialog::Yes | KDialog::No | KDialog::Ok | KDialog::Cancel );
  dialog->setObjectName( "KOMessageBox" );
  dialog->setDefaultButton( KDialog::Yes );
  dialog->setButtonGuiItem( KDialog::Ok, button3 );
  dialog->setButtonGuiItem( KDialog::Yes, button1 );
  dialog->setButtonGuiItem( KDialog::No, button2 );
//  QObject::connect( dialog, SIGNAL( yesClicked() ), dialog, SLOT(slotYes()));
//  QObject::connect( dialog, SIGNAL( noClicked() ), dialog, SLOT(slotNo()));

  bool checkboxResult = false;
  int result = KMessageBox::createKMessageBox(
    dialog, type, text, QStringList(), QString(), &checkboxResult, options );

  switch (result) {
  case KDialog::Yes:
    result = KMessageBox::Yes;
    break;
  case KDialog::No:
    result = KMessageBox::No;
    break;
  case KDialog::Ok:
    result = KMessageBox::Continue;
    break;
  case KDialog::Cancel:
    result = KMessageBox::Cancel;
    break;
  default:
    break;
  }

  return result;
}
