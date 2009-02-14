/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "timescaleconfigdialog.h"

#include "koglobals.h"
#include "koprefs.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <ksystemtimezone.h>

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QListWidget>

TimeScaleConfigDialog::TimeScaleConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  ui.setupUi( this );

  QStringList list;
  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for ( KTimeZones::ZoneMap::ConstIterator it = timezones.begin();  it != timezones.end();  ++it ) {
    list.append( i18n( it.key().toUtf8() ) );
  }
  list.sort();
  ui.zoneCombo->addItems( list );
  ui.zoneCombo->setCurrentIndex( 0 );

  ui.addButton->setIcon( KIcon( "list-add" ) );
  ui.removeButton->setIcon( KIcon( "list-remove" ) );
  ui.upButton->setIcon( KIcon( "go-up" ) );
  ui.downButton->setIcon( KIcon( "go-down" ) );

  connect( ui.addButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( ui.removeButton, SIGNAL( clicked() ), SLOT( remove() ) );
  connect( ui.upButton, SIGNAL( clicked() ), SLOT( up() ) );
  connect( ui.downButton, SIGNAL( clicked() ), SLOT( down() ) );

  connect( ui.okButton, SIGNAL( clicked() ), SLOT( okClicked() ) );
  connect( ui.cancelButton, SIGNAL( clicked() ), SLOT( reject() ) );

  ui.listWidget->addItems( KOPrefs::instance()->timeScaleTimezones() );
}

void TimeScaleConfigDialog::okClicked()
{
  KOPrefs::instance()->setTimeScaleTimezones( zones() );
  accept();
}

void TimeScaleConfigDialog::add()
{
  // Do not add duplicates
  for ( int i=0; i < ui.listWidget->count(); i++ ) {
    if ( ui.listWidget->item( i )->text() == ui.zoneCombo->currentText() ) {
      return;
    }
  }

  ui.listWidget->addItem( ui.zoneCombo->currentText() );
}

void TimeScaleConfigDialog::remove()
{
  delete ui.listWidget->takeItem( ui.listWidget->currentRow() );
}

void TimeScaleConfigDialog::up()
{
  int row = ui.listWidget->currentRow();
  QListWidgetItem *item = ui.listWidget->takeItem( row );
  ui.listWidget->insertItem( qMax( row - 1, 0 ), item );
  ui.listWidget->setCurrentRow( qMax( row - 1, 0 ) );
}

void TimeScaleConfigDialog::down()
{
  int row = ui.listWidget->currentRow();
  QListWidgetItem *item = ui.listWidget->takeItem( row );
  ui.listWidget->insertItem( qMin( row + 1, ui.listWidget->count() ), item );
  ui.listWidget->setCurrentRow( qMin( row + 1, ui.listWidget->count() - 1 ) );
}

QStringList TimeScaleConfigDialog::zones()
{
  QStringList list;
  for ( int i=0; i < ui.listWidget->count(); i++ ) {
    list << ui.listWidget->item( i )->text();
  }
  return list;
}

#include "timescaleconfigdialog.moc"
