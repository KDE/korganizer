/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#include <qcombobox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include "exchangedialog.h"

ExchangeDialog::ExchangeDialog( const QDate &_start, const QDate &_end, QWidget *parent)
  : KDialogBase(Plain,i18n("Exchange Plugin"),Ok|Cancel,Ok,parent)
{
  QFrame *topFrame = plainPage();
  QGridLayout *topLayout = new QGridLayout( topFrame, 2, 2, 3 );

  QLabel *label = new QLabel(i18n("Start date:"),topFrame);
  topLayout->addWidget(label, 0, 0);

  m_start = new KDateWidget( _start, topFrame );
  topLayout->addWidget( m_start, 0, 1 );

  m_end = new KDateWidget( _end, topFrame );
  topLayout->addWidget( new QLabel( i18n( "End date:" ), topFrame ), 1, 0 );
  topLayout->addWidget( m_end, 1, 1 );
}

ExchangeDialog::~ExchangeDialog()
{
}

void ExchangeDialog::slotOk()
{
  accept();
}
#include "exchangedialog.moc"
