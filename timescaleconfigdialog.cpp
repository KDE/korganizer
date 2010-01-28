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
  : KDialog( parent )
{
  setCaption( i18n( "Timezone" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( false );

  QWidget *mainwidget = new QWidget( this );
  setupUi( mainwidget );
  setMainWidget( mainwidget );


  QStringList list;
  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for ( KTimeZones::ZoneMap::ConstIterator it = timezones.begin();  it != timezones.end();  ++it ) {
    list.append( i18n( it.key().toUtf8() ) );
  }
  list.sort();
  zoneCombo->addItems( list );
  zoneCombo->setCurrentIndex( 0 );

  addButton->setIcon( KIcon( "list-add" ) );
  removeButton->setIcon( KIcon( "list-remove" ) );
  upButton->setIcon( KIcon( "go-up" ) );
  downButton->setIcon( KIcon( "go-down" ) );

  connect( addButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( removeButton, SIGNAL( clicked() ), SLOT( remove() ) );
  connect( upButton, SIGNAL( clicked() ), SLOT( up() ) );
  connect( downButton, SIGNAL( clicked() ), SLOT( down() ) );

  connect( this, SIGNAL( okClicked() ), SLOT( okClicked() ) );
  connect( this, SIGNAL( cancelClicked() ), SLOT( reject() ) );

  listWidget->addItems( KOPrefs::instance()->timeScaleTimezones() );
}

void TimeScaleConfigDialog::okClicked()
{
  KOPrefs::instance()->setTimeScaleTimezones( zones() );
  accept();
}

void TimeScaleConfigDialog::add()
{
  // Do not add duplicates
  for ( int i=0; i < listWidget->count(); i++ ) {
    if ( listWidget->item( i )->text() == zoneCombo->currentText() ) {
      return;
    }
  }

  listWidget->addItem( zoneCombo->currentText() );
}

void TimeScaleConfigDialog::remove()
{
  delete listWidget->takeItem( listWidget->currentRow() );
}

void TimeScaleConfigDialog::up()
{
  int row = listWidget->currentRow();
  QListWidgetItem *item = listWidget->takeItem( row );
  listWidget->insertItem( qMax( row - 1, 0 ), item );
  listWidget->setCurrentRow( qMax( row - 1, 0 ) );
}

void TimeScaleConfigDialog::down()
{
  int row = listWidget->currentRow();
  QListWidgetItem *item = listWidget->takeItem( row );
  listWidget->insertItem( qMin( row + 1, listWidget->count() ), item );
  listWidget->setCurrentRow( qMin( row + 1, listWidget->count() - 1 ) );
}

QStringList TimeScaleConfigDialog::zones()
{
  QStringList list;
  for ( int i=0; i < listWidget->count(); i++ ) {
    list << listWidget->item( i )->text();
  }
  return list;
}

#include "timescaleconfigdialog.moc"
