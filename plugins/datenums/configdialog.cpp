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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>

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
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame,0,spacingHint());

//  QLabel *label = new QLabel(i18n("Show date numbers:"),topFrame);
//  topLayout->addWidget(label);
  mDayNumGroup = new QVButtonGroup( i18n("Show Date Number"), topFrame );
	topLayout->addWidget( mDayNumGroup );

	new QRadioButton( i18n("Show day number"), mDayNumGroup );
	new QRadioButton( i18n("Show days to end of year"), mDayNumGroup );
	new QRadioButton( i18n("Show both"), mDayNumGroup );

  load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
  KConfig config( locateLocal( "config", "korganizerrc" ));
  config.setGroup("Calendar/DateNum Plugin");
	int datenum = config.readNumEntry( "ShowDayNumbers", 0 );
  mDayNumGroup->setButton( datenum );
}

void ConfigDialog::save()
{
  KConfig config( locateLocal( "config", "korganizerrc" ));

  config.setGroup("Calendar/DateNum Plugin");
  config.writeEntry("ShowDayNumbers", mDayNumGroup->selectedId() );
  config.sync();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
