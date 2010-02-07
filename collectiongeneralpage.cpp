/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KOrganizer
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


#include "collectiongeneralpage.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>

#include <KLocale>
#include <KDialog>
#include <KLineEdit>
#include <KIconButton>

#include <akonadi/attributefactory.h>
#include <akonadi/agentmanager.h>
#include <akonadi/collection.h>
#include <akonadi/entitydisplayattribute.h>


using namespace Akonadi;

CollectionGeneralPage::CollectionGeneralPage(QWidget * parent) :
    CollectionPropertiesPage( parent )
{
  setPageTitle(  i18nc("@title:tab General settings for a folder.", "General"));

}

CollectionGeneralPage::~CollectionGeneralPage()
{
}

void CollectionGeneralPage::init(const Akonadi::Collection &col)
{
  QLabel *label;

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QHBoxLayout *hl = new QHBoxLayout();
  topLayout->addItem( hl );
  hl->setSpacing( KDialog::spacingHint() );

  label = new QLabel( i18nc("@label:textbox Name of the folder.","&Name:"), this );
  hl->addWidget( label );

  mNameEdit = new KLineEdit( this );
  label->setBuddy( mNameEdit );
  hl->addWidget( mNameEdit );


  mIconsCheckBox = new QCheckBox( i18n( "Use custom &icons:"), this );
  mIconsCheckBox->setChecked( false );

  mNormalIconButton = new KIconButton( this );
  mNormalIconButton->setIconType( KIconLoader::NoGroup, KIconLoader::Place, false );
  mNormalIconButton->setIconSize( 16 );
  mNormalIconButton->setStrictIconSize( true );
  mNormalIconButton->setFixedSize( 28, 28 );
  // Can't use iconset here.
  mNormalIconButton->setIcon( "folder" );
  mNormalIconButton->setEnabled( false );

  QHBoxLayout * iconHLayout = new QHBoxLayout();
  iconHLayout->addWidget( mIconsCheckBox );
  iconHLayout->addStretch( 2 );
  iconHLayout->addWidget( mNormalIconButton );
  iconHLayout->addStretch( 1 );
  topLayout->addLayout( iconHLayout );

  topLayout->addStretch( 100 ); // eat all superfluous space
  connect( mIconsCheckBox, SIGNAL( toggled(bool) ), mNormalIconButton, SLOT( setEnabled(bool) ) );


}



void CollectionGeneralPage::load(const Akonadi::Collection & col)
{
  init( col );
  QString displayName;
  QString iconName;
  if ( col.hasAttribute<Akonadi::EntityDisplayAttribute>() ) {
    displayName = col.attribute<Akonadi::EntityDisplayAttribute>()->displayName();
    iconName = col.attribute<EntityDisplayAttribute>()->iconName();
  }
  if ( displayName.isEmpty() )
    mNameEdit->setText( col.name() );
  else
    mNameEdit->setText( displayName );

  if ( iconName.isEmpty() )
    mNormalIconButton->setIcon( "folder" );
  else
    mNormalIconButton->setIcon( iconName );
  mIconsCheckBox->setChecked( !iconName.isEmpty() );
}

void CollectionGeneralPage::save(Collection & col)
{

  if ( col.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
       !col.attribute<Akonadi::EntityDisplayAttribute>()->displayName().isEmpty() )
    col.attribute<Akonadi::EntityDisplayAttribute>()->setDisplayName( mNameEdit->text() );
  else if( !mNameEdit->text().isEmpty() )
    col.setName( mNameEdit->text() );


  if ( mIconsCheckBox->isChecked() )
    col.attribute<EntityDisplayAttribute>( Collection::AddIfMissing )->setIconName( mNormalIconButton->icon() );
  else if ( col.hasAttribute<EntityDisplayAttribute>() )
    col.attribute<EntityDisplayAttribute>()->setIconName( QString() );

}


#include "collectiongeneralpage.moc"
