/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include <iostream>

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlabel.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kprocess.h>
#include <kdialog.h>

#include "knewstuffgeneric.h"

#include "ghns.h"

using namespace std;

GhnsWidget::GhnsWidget()
{
  mWallpapers = new KNewStuffGeneric( "kdesktop/wallpaper", this );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  topLayout->addWidget( new QLabel( i18n("Get Hot New Stuff:"), this ) );
  
  QPushButton *button = new QPushButton( "Wallpapers", this );
  topLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( downloadWallpapers() ) );

  topLayout->addSpacing( 5 );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );
  
  buttonLayout->addStretch();

  QPushButton *closeButton = new QPushButton( "Close", this );
  buttonLayout->addWidget( closeButton );
  connect( closeButton, SIGNAL( clicked() ), kapp, SLOT( quit() ) );
}

GhnsWidget::~GhnsWidget()
{
  delete mWallpapers;
}

void GhnsWidget::downloadWallpapers()
{
  kdDebug(5850) << "downloadWallpapers()" << endl;

  mWallpapers->download();
}

int main(int argc,char **argv)
{
  KAboutData aboutData("ghns","Get Hot New Stuff","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  GhnsWidget wid;
  app.setMainWidget( &wid );
  wid.show();

  app.exec();
}

#include "ghns.moc"
