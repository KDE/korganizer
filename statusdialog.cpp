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
*/

#include <qlabel.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qlayout.h>

#include <kdebug.h>
#include <klocale.h>

#include "statusdialog.h"
#include "statusdialog.moc"

StatusDialog::StatusDialog(QWidget* parent, const char* name) :
  KDialog(parent,name,true)
{
  setCaption(i18n("Set Your Status"));

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );

  QBoxLayout *statusLayout = new QHBoxLayout( topLayout );

  QLabel *text = new QLabel(i18n("Set your status"),this);
  statusLayout->addWidget( text );

  mStatus = new QComboBox(false,this);
  mStatus->insertStringList(Attendee::statusList());
  statusLayout->addWidget( mStatus );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );

  QPushButton *ok = new QPushButton(i18n("&OK"), this);
  connect ( ok,SIGNAL(clicked()), this,SLOT(accept()) );
  buttonLayout->addWidget( ok );

  QPushButton *cancel = new QPushButton(i18n("&Cancel"), this);
  connect ( cancel,SIGNAL(clicked()), this,SLOT(reject()) );
  buttonLayout->addWidget( cancel );
}

StatusDialog::~StatusDialog()
{
}

Attendee::PartStat StatusDialog::status()
{
  return Attendee::PartStat( mStatus->currentItem() ) ;
}
