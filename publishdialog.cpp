/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qlineedit.h>
#include <kdebug.h>

#include <kglobal.h>
#include <klocale.h>
#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#endif

#include "koprefs.h"
#include "publishdialog.h"

PublishDialog::PublishDialog(QWidget* parent, const char* name,
                               bool modal, WFlags fl)
    : PublishDialog_base(parent,name,modal,fl)
{
  setCaption(i18n("Select Addresses"));
  mNameLineEdit->setEnabled(false);
  mEmailLineEdit->setEnabled(false);
  connect(mAddressListView,SIGNAL(selectionChanged(QListViewItem *)),
          SLOT(updateInput()));
}

PublishDialog::~PublishDialog()
{
}

void PublishDialog::addAttendee(Attendee *attendee)
{
  mNameLineEdit->setEnabled(true);
  mEmailLineEdit->setEnabled(true);
  QListViewItem *item = new QListViewItem(mAddressListView);
  item->setText(0,attendee->name());
  item->setText(1,attendee->email());
  mAddressListView->insertItem(item);
}

QString PublishDialog::addresses()
{
  QString to = "";
  QListViewItem *item;
  int i, count;
  count = mAddressListView->childCount();
  for (i=0;i<count;i++) {
    item = mAddressListView->firstChild();
    mAddressListView->takeItem(item);
    to += item->text(1);
    if (i<count-1) {
      to += ", ";
    }
  }
  return to;
}

void PublishDialog::addItem()
{
  mNameLineEdit->setEnabled(true);
  mEmailLineEdit->setEnabled(true);
  QListViewItem *item = new QListViewItem(mAddressListView);
  mAddressListView->insertItem(item);
  mAddressListView->setSelected(item,true);
  mNameLineEdit->setText(i18n("(EmptyName)"));
  mEmailLineEdit->setText(i18n("(EmptyEmail)"));
}

void PublishDialog::removeItem()
{
  QListViewItem *item;
  item = mAddressListView->selectedItem();
  if (!item) return;
  mAddressListView->takeItem(item);
  item = mAddressListView->selectedItem();
  if (!item) {
    mNameLineEdit->setText("");
    mEmailLineEdit->setText("");
    mNameLineEdit->setEnabled(false);
    mEmailLineEdit->setEnabled(false);
  }
  if (mAddressListView->childCount() == 0) {
    mNameLineEdit->setEnabled(false);
    mEmailLineEdit->setEnabled(false);
  }
}

void PublishDialog::openAddressbook()
{
#ifndef KORG_NOKABC
  KABC::Addressee::List addressList;
  addressList = KABC::AddresseeDialog::getAddressees(this);
  //KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
  KABC::Addressee a = addressList.first();
  if (!a.isEmpty()) {
    uint i;
    for (i=0;i<addressList.size();i++) {
      a = addressList[i];
      mNameLineEdit->setEnabled(true);
      mEmailLineEdit->setEnabled(true);
      QListViewItem *item = new QListViewItem(mAddressListView);
      mAddressListView->setSelected(item,true);
      mNameLineEdit->setText(a.realName());
      mEmailLineEdit->setText(a.preferredEmail());
      mAddressListView->insertItem(item);
    }
  }
#endif
}

void PublishDialog::updateItem()
{
  QListViewItem *item;
  item = mAddressListView->selectedItem();
  if (!item) return;
  item->setText(0,mNameLineEdit->text());
  item->setText(1,mEmailLineEdit->text());
}

void PublishDialog::updateInput()
{
  QListViewItem *item;
  item = mAddressListView->selectedItem();
  if (!item) return;
  mNameLineEdit->setEnabled(true);
  mEmailLineEdit->setEnabled(true);
  QString mail = item->text(1);
  mNameLineEdit->setText(item->text(0));
  mEmailLineEdit->setText(mail);
}

#include "publishdialog.moc"
