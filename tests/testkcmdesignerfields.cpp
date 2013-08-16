/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

#include "kcmdesignerfields.h"

class MyDesignerFields : public KCMDesignerFields
{
  public:
    MyDesignerFields( const KComponentData &kcd ) : KCMDesignerFields( kcd, 0 ) {}
    QString localUiDir() { return QString( KDESRCDIR ); }
    QString uiPath() { return QString( KDESRCDIR ); }
    void writeActivePages( const QStringList & )  {}
    QStringList readActivePages() { return QStringList(); }
    QString applicationName() { return "textkcmdesignerfields"; }
};

int main(int argc,char **argv)
{
  KAboutData aboutData( "testkcmdesignerfields", 0, KLocalizedString(), "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  MyDesignerFields *kcm = new MyDesignerFields( KComponentData( aboutData ) );
  kcm->show();

  app.exec();
  delete kcm;
}
