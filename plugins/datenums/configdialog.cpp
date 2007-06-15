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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "configdialog.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <QLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QGroupBox>
#include <QButtonGroup>

#include "configdialog.moc"

ConfigDialog::ConfigDialog(QWidget *parent)
  : KDialog(parent)
{
  setCaption( i18n("Configure Day Numbers") );
  setButtons( Ok|Cancel );
  setDefaultButton(  Ok );
  setModal( true );
  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  QGroupBox *dayNumBox = new QGroupBox( i18n("Show Date Number"), topFrame );
  topLayout->addWidget( dayNumBox );
  QVBoxLayout *groupLayout = new QVBoxLayout( dayNumBox );

  QRadioButton *btn;
  mDayNumGroup = new QButtonGroup( this );
  btn = new QRadioButton( i18n("Show day number"), dayNumBox );
  mDayNumGroup->addButton( btn, 0 );
  groupLayout->addWidget( btn );
  btn = new QRadioButton( i18n("Show days to end of year"), dayNumBox );
  mDayNumGroup->addButton( btn, 1 );
  groupLayout->addWidget( btn );
  btn = new QRadioButton( i18n("Show both"), dayNumBox );
  mDayNumGroup->addButton( btn, 2 );
  groupLayout->addWidget( btn );

  connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));

  load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals  );
  KConfigGroup config(&_config, "Calendar/DateNum Plugin");
  int datenum = config.readEntry( "ShowDayNumbers", 0 );
  QAbstractButton *btn = mDayNumGroup->button( datenum );
  if (!btn) btn = mDayNumGroup->button( 0 );
  btn->setChecked( true );
}

void ConfigDialog::save()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals  );
  KConfigGroup config(&_config, "Calendar/DateNum Plugin");
  config.writeEntry("ShowDayNumbers", mDayNumGroup->checkedId() );
  config.sync();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
