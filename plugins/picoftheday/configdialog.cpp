/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
  setCaption( i18n("Configure Picture of the Day") );
  setButtons( Ok|Cancel );
  setDefaultButton(  Ok );
  setModal( true );
  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  QGroupBox *aspectRatioBox 
    = new QGroupBox( i18n("Thumbnail Aspect Ratio Mode"), topFrame );
  topLayout->addWidget( aspectRatioBox );
  QVBoxLayout *groupLayout = new QVBoxLayout( aspectRatioBox );

  QRadioButton *btn;
  mAspectRatioGroup = new QButtonGroup( this );
  btn = new QRadioButton( i18n("Ignore aspect ratio"), aspectRatioBox );
  btn->setWhatsThis( i18n("The thumbnail will be scaled freely. "
                          "The aspect ratio will not be preserved.") );
  mAspectRatioGroup->addButton( btn, int(Qt::IgnoreAspectRatio) );
  groupLayout->addWidget( btn );
  btn = new QRadioButton( i18n("Keep aspect ratio"), aspectRatioBox );
  btn->setWhatsThis( i18n("The thumbnail will be scaled to a rectangle "
                          "as large as possible inside a given rectangle, "
                          "preserving the aspect ratio.") );
  mAspectRatioGroup->addButton( btn, int(Qt::KeepAspectRatio) );
  groupLayout->addWidget( btn );
  btn = new QRadioButton( i18n("Keep aspect ratio by expanding"), 
                          aspectRatioBox );
  btn->setWhatsThis( i18n("The thumbnail will be scaled to a rectangle "
                          "as small as possible outside a given rectangle, "
                          "preserving the aspect ratio.") );
  mAspectRatioGroup->addButton( btn, int(Qt::KeepAspectRatioByExpanding) );
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
  KConfigGroup config(&_config, "Calendar/Picoftheday Plugin");
  int datenum = config.readEntry( "AspectRatioMode", 0 );
  QAbstractButton *btn = mAspectRatioGroup->button( datenum );
  if (!btn) btn = mAspectRatioGroup->button( 0 );
  btn->setChecked( true );
}

void ConfigDialog::save()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals  );
  KConfigGroup config(&_config, "Calendar/Picoftheday Plugin");
  config.writeEntry("AspectRatioMode", mAspectRatioGroup->checkedId() );
  config.sync();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
