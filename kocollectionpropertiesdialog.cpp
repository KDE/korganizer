/*
  This file is part of KOrganizer.
  Copyright (c) 2010 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "kocollectionpropertiesdialog.h"

#include "collectiongeneralpage.h"


#include <ktabwidget.h>

#include <QtGui/QBoxLayout>
#include <akonadi/collectionmodifyjob.h>

KOCollectionPropertiesDialog::KOCollectionPropertiesDialog( const Akonadi::Collection &collection, QWidget *parent ) :
  KDialog( parent )
{
  mCollection = collection;
  QBoxLayout *layout = new QHBoxLayout( mainWidget() );
  layout->setMargin( 0 );
  mTabWidget = new KTabWidget( mainWidget() );
  layout->addWidget( mTabWidget );

  Akonadi::CollectionPropertiesPage *page = new CollectionGeneralPage( mTabWidget );
  insertPage( page );

  connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
  connect( this, SIGNAL( cancelClicked() ), SLOT( deleteLater() ) );

}

void KOCollectionPropertiesDialog::insertPage( Akonadi::CollectionPropertiesPage * page )
{
  if ( page->canHandle( mCollection ) ) {
    mTabWidget->addTab( page, page->pageTitle() );
    page->load( mCollection );
  } else {
    delete page;
  }
}

void KOCollectionPropertiesDialog::save()
{
  for ( int i = 0; i < mTabWidget->count(); ++i ) {
    Akonadi::CollectionPropertiesPage *page = static_cast<Akonadi::CollectionPropertiesPage*>( mTabWidget->widget( i ) );
    page->save( mCollection );
  }

  Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob( mCollection, this );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( saveResult( KJob* ) ) );
}

void KOCollectionPropertiesDialog::saveResult( KJob *job )
{
  if ( job->error() ) {
    // TODO
    kWarning() << job->errorString();
  }
  deleteLater();
}

#include "kocollectionpropertiesdialog.moc"


