/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "koeventviewerdialog.h"
#include "korganizerinterface.h"

#include <calendarsupport/utils.h>
#include <calendarsupport/next/incidenceviewer.h>
#include <Akonadi/Calendar/ETMCalendar>

#include <AkonadiCore/Item>

#include <KLocalizedString>
#include <KToolInvocation>
#include <QIcon>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KGuiItem>
#include <QVBoxLayout>

KOEventViewerDialog::KOEventViewerDialog( Akonadi::ETMCalendar *calendar, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( i18n( "Event Viewer" ) );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mUser1Button = new QPushButton;
  buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
  QPushButton *user2Button = new QPushButton;
  buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &KOEventViewerDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &KOEventViewerDialog::reject);
  setModal( false );
  KGuiItem::assign(mUser1Button, KGuiItem( i18n( "Edit..." ), QIcon::fromTheme( QLatin1String("document-edit") ) ));
  KGuiItem::assign(user2Button, KGuiItem( i18n( "Show in Context" ) ));
  mEventViewer = new CalendarSupport::IncidenceViewer( calendar, this );
  mainLayout->addWidget(mEventViewer);
  mainLayout->addWidget(buttonBox);


  resize( QSize( 500, 520 ).expandedTo( minimumSizeHint() ) );

  connect(this, &KOEventViewerDialog::finished, this, &KOEventViewerDialog::delayedDestruct);
  connect(mUser1Button, &QPushButton::clicked, this, &KOEventViewerDialog::editIncidence);
  connect(user2Button, &QPushButton::clicked, this, &KOEventViewerDialog::showIncidenceContext);
}

KOEventViewerDialog::~KOEventViewerDialog()
{
  delete mEventViewer;
}

void KOEventViewerDialog::delayedDestruct()
{
    if (isVisible()) {
        hide();
    }

    deleteLater();
}

QPushButton *KOEventViewerDialog::editButton() const
{
  return mUser1Button;
}

void KOEventViewerDialog::setIncidence( const Akonadi::Item &incidence, const QDate &date )
{
  mEventViewer->setIncidence( incidence, date );
}

void KOEventViewerDialog::addText( const QString &text )
{
  mEventViewer->setHeaderText( text );
}

void KOEventViewerDialog::editIncidence()
{
  const Akonadi::Item item = mEventViewer->item();

  if ( CalendarSupport::hasIncidence( item ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( QLatin1String("korganizer" ));

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      QLatin1String("org.kde.korganizer"), QLatin1String("/Korganizer"), QDBusConnection::sessionBus() );
    korganizerIface.editIncidence( QString::number( item.id() ) );
  }
}

void KOEventViewerDialog::showIncidenceContext()
{
  const Akonadi::Item item = mEventViewer->item();

  if ( CalendarSupport::hasIncidence( item ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( QLatin1String("korganizer") );

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      QLatin1String("org.kde.korganizer"), QLatin1String("/Korganizer"), QDBusConnection::sessionBus() );
    korganizerIface.showIncidenceContext( QString::number( item.id() ) );
  }
}

