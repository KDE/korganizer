/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "freebusyurldialog.h"

#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qlabel.h>

FreeBusyUrlDialog::FreeBusyUrlDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n("Edit Free/Busy Location"), Ok|Cancel, Ok, parent,
                 name, true, false )
{
  QFrame *topFrame = plainPage();

  QBoxLayout *topLayout = new QVBoxLayout( topFrame, 0, spacingHint() );

  mWidget = new FreeBusyUrlWidget( topFrame );
  topLayout->addWidget( mWidget );

  mWidget->loadConfig();
}

void FreeBusyUrlDialog::slotOk()
{
  mWidget->saveConfig();
  accept();
}


FreeBusyUrlWidget::FreeBusyUrlWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  
  mUrlEdit = new KLineEdit( this );
  topLayout->addWidget( mUrlEdit );
}

void FreeBusyUrlWidget::loadConfig()
{
  kdDebug() << "FreeBusyUrlWidget::loadConfig()" << endl;
}

void FreeBusyUrlWidget::saveConfig()
{
  kdDebug() << "FreeBusyUrlWidget::saveConfig()" << endl;
}

#include "freebusyurldialog.moc"
