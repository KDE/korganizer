/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <qlayout.h>

#include <keditlistbox.h>
#include <klocale.h>

#include "koprefs.h"

#include "savetemplatedialog.h"
#include "savetemplatedialog.moc"

SaveTemplateDialog::SaveTemplateDialog( IncidenceType type, QWidget *parent )
  : KDialogBase( Plain, i18n("Save Template"), Ok | Cancel, Ok, parent, 0,
                 false, false ),
    mType( type )
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame, 0, spacingHint() );

  mEditListBox = new KEditListBox( i18n("Select Template Name"), topFrame,
                                   0, false, KEditListBox::Add |
                                   KEditListBox::Remove );
  topLayout->addWidget( mEditListBox );
  connect( mEditListBox, SIGNAL( changed() ), SLOT( slotChanged() ) );

  QStringList templates;

  if ( mType == EventType ) {
    templates = KOPrefs::instance()->mEventTemplates;
  } else if( mType == TodoType ) {
    templates = KOPrefs::instance()->mTodoTemplates;
  }

  mEditListBox->insertStringList( templates );
}

SaveTemplateDialog::~SaveTemplateDialog()
{
}

void SaveTemplateDialog::slotOk()
{
  emit templateSelected( mEditListBox->currentText() );
  accept();
}

void SaveTemplateDialog::slotChanged()
{
  if ( mType == EventType ) {
    KOPrefs::instance()->mEventTemplates = mEditListBox->items();
  } else if( mType == TodoType ) {
    KOPrefs::instance()->mTodoTemplates = mEditListBox->items();
  }
}
