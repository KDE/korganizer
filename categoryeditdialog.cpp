/*
    This file is part of KOrganizer.
    Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qstringlist.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qheader.h>

#include "koprefs.h"

#include "categoryeditdialog.h"

/* 
 *  Constructs a CategoryEditDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CategoryEditDialog::CategoryEditDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : CategoryEditDialog_base( parent, name, modal, fl )
{
  mCategories->header()->hide();

  QStringList::Iterator it;

  for (it = KOPrefs::instance()->mCustomCategories.begin();
       it != KOPrefs::instance()->mCustomCategories.end(); ++it ) {
    new QListViewItem(mCategories,*it);
  }

  connect(mCategories,SIGNAL(selectionChanged(QListViewItem *)),
          SLOT(editItem(QListViewItem *)));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CategoryEditDialog::~CategoryEditDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void CategoryEditDialog::add()
{
  if (!mEdit->text().isEmpty()) {
    new QListViewItem(mCategories,mEdit->text());
    mEdit->setText("");
  }
}

void CategoryEditDialog::remove()
{
  if (mCategories->currentItem()) {
    delete mCategories->currentItem();
  }
}

void CategoryEditDialog::modify()
{
  if (!mEdit->text().isEmpty()) {
    if (mCategories->currentItem()) {
      mCategories->currentItem()->setText(0,mEdit->text());
    }
  }
}

void CategoryEditDialog::slotOk()
{
  slotApply();
  accept();
}

void CategoryEditDialog::slotApply()
{
  KOPrefs::instance()->mCustomCategories.clear();

  QListViewItem *item = mCategories->firstChild();
  while(item) {
    KOPrefs::instance()->mCustomCategories.append(item->text(0));
    item = item->nextSibling();
  }
  
  emit categoryConfigChanged();
}

void CategoryEditDialog::editItem(QListViewItem *item)
{
  mEdit->setText(item->text(0));
}

#include "categoryeditdialog.moc"
