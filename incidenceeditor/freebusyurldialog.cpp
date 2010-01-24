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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "freebusyurldialog.h"

#include <kcal/attendee.h>
#include <kcal/freebusyurlstore.h>

#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>
#include <QBoxLayout>

FreeBusyUrlDialog::FreeBusyUrlDialog( KCal::Attendee *attendee, QWidget *parent )
  : KDialog( parent )
{
  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );
  setModal( true );
  setCaption( i18n( "Edit Free/Busy Location" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  mWidget = new FreeBusyUrlWidget( attendee, topFrame );
  topLayout->addWidget( mWidget );

  mWidget->loadConfig();
  connect( this, SIGNAL(okClicked()), this, SLOT(slotOk()) );
}

void FreeBusyUrlDialog::slotOk()
{
  mWidget->saveConfig();
  accept();
}

FreeBusyUrlWidget::FreeBusyUrlWidget( KCal::Attendee *attendee, QWidget *parent )
  : QWidget( parent ), mAttendee( attendee )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label =
    new QLabel( i18n( "Location of Free/Busy information for %1 <placeholder>%2</placeholder>:",
                     mAttendee->name(), mAttendee->email() ), this );
  topLayout->addWidget( label );

  mUrlEdit = new KLineEdit( this );
  topLayout->addWidget( mUrlEdit );
}

FreeBusyUrlWidget::~FreeBusyUrlWidget()
{
}

void FreeBusyUrlWidget::loadConfig()
{
  kDebug();

  const QString url = KCal::FreeBusyUrlStore::self()->readUrl( mAttendee->email() );
  mUrlEdit->setText( url );
}

void FreeBusyUrlWidget::saveConfig()
{
  kDebug();

  const QString url = mUrlEdit->text();
  KCal::FreeBusyUrlStore::self()->writeUrl( mAttendee->email(), url );
  KCal::FreeBusyUrlStore::self()->sync();
}

#include "freebusyurldialog.moc"
