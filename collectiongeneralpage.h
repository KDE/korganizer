/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of Korganizer.
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


#ifndef COLLECTIONGENERALPAGE_H
#define COLLECTIONGENERALPAGE_H

#include <akonadi/collectionpropertiespage.h>
class QCheckBox;
class KLineEdit;
class KIconButton;

class CollectionGeneralPage : public Akonadi::CollectionPropertiesPage
{
  Q_OBJECT
public:
  explicit CollectionGeneralPage( QWidget *parent = 0 );
  ~CollectionGeneralPage();

  void load( const Akonadi::Collection & col );
  void save( Akonadi::Collection & col );


protected:
  void init();

private:
  KLineEdit   *mNameEdit;
  KIconButton *mNormalIconButton;
  QCheckBox *mIconsCheckBox;
};


#endif /* COLLECTIONGENERALPAGE_H */

