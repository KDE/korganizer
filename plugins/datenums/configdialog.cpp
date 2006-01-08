/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
		Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <QVBoxLayout>
#include <QFrame>
#include <QGroupBox>
#include <QButtonGroup>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include "configdialog.h"
#include "configdialog.moc"

ConfigDialog::ConfigDialog(QWidget *parent)
  : KDialogBase(Plain,i18n("Configure Day Numbers"),Ok|Cancel,Ok,parent)
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame, 0, spacingHint() );

  QGroupBox *dayNumBox = new QGroupBox( i18n("Show Date Number"), topFrame );
  topLayout->addWidget( dayNumBox );
  QVBoxLayout *groupLayout = new QVBoxLayout( dayNumBox );

  QRadioButton *btn;
  mDayNumGroup = new QButtonGroup( this );
  btn = new QRadioButton( i18n("Show day number"), dayNumBox );
  mDayNumGroup->addButton( btn );
  groupLayout->addWidget( btn );
  btn = new QRadioButton( i18n("Show days to end of year"), dayNumBox );
  mDayNumGroup->addButton( btn );
  groupLayout->addWidget( btn );
  btn = new QRadioButton( i18n("Show both"), dayNumBox );
  mDayNumGroup->addButton( btn );
  groupLayout->addWidget( btn );

  load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
  KConfig config( "korganizerrc", true, false); // Open read-only, no kdeglobals
  config.setGroup("Calendar/DateNum Plugin");
  int datenum = config.readNumEntry( "ShowDayNumbers", 0 );
  QAbstractButton *btn = mDayNumGroup->button( datenum );
  if (btn) btn->setChecked( true );
}

void ConfigDialog::save()
{
  KConfig config( "korganizerrc", false, false); // Open read-write, no kdeglobals
  config.setGroup("Calendar/DateNum Plugin");
  config.writeEntry("ShowDayNumbers", mDayNumGroup->checkedId() );
  config.sync();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
