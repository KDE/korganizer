/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <kmessagebox.h>
#include <kdialogbase.h>
#include <qpushbutton.h>

#include "komessagebox.h"

int KOMessageBox::fourBtnMsgBox( QWidget *parent, QMessageBox::Icon type, 
            const QString &text, const QString &caption, 
            const KGuiItem &button1, const KGuiItem &button2, 
            const KGuiItem &button3, int options)
{
  KDialogBase *dialog= new KDialogBase( parent, "KOMessageBox", true,
                     caption.isEmpty() ? "" : caption,
                     KDialogBase::Yes | KDialogBase::No | KDialogBase::Ok | KDialogBase::Cancel,
                     KDialogBase::Yes, 
                     true/*, button1, button2, button3*/);
  dialog->setButtonOK( button3 );
  dialog->setButtonText( KDialogBase::Yes, button1.text() );
  dialog->setButtonText( KDialogBase::No, button2.text() );
  QObject::connect( dialog->actionButton( KDialogBase::Yes ), SIGNAL( clicked() ), dialog, SLOT(slotYes()));
  QObject::connect( dialog->actionButton( KDialogBase::No ), SIGNAL( clicked() ), dialog, SLOT(slotNo()));
//  QObject::connect( dialog, SIGNAL( noClicked() ), dialog, SLOT(slotNo()));
  

  bool checkboxResult = false;
  int result = KMessageBox::createKMessageBox(dialog, type, text, QStringList(),
                     QString::null, &checkboxResult, options);
  switch (result) {
    case KDialogBase::Yes: result = KMessageBox::Yes; break;
    case KDialogBase::No: result = KMessageBox::No; break;
    case KDialogBase::Ok: result = KMessageBox::Continue; break;
    case KDialogBase::Cancel: result = KMessageBox::Cancel; break;
    default: break;
  }

  return result;
}


