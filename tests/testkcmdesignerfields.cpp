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

#include <KAboutData>


#include <qdebug.h>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

#include "kcmdesignerfields.h"

class MyDesignerFields : public KCMDesignerFields
{
  public:
    MyDesignerFields( ) : KCMDesignerFields( 0 ) {}
    QString localUiDir() { return QString::fromLatin1( KDESRCDIR ); }
    QString uiPath() { return QString::fromLatin1( KDESRCDIR ); }
    void writeActivePages( const QStringList & )  {}
    QStringList readActivePages() { return QStringList(); }
    QString applicationName() { return QLatin1String("textkcmdesignerfields"); }
};

int main(int argc,char **argv)
{
  KAboutData aboutData( QLatin1String("testkcmdesignerfields"), QString(), QLatin1String("0.1") );
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);


  MyDesignerFields *kcm = new MyDesignerFields();
  kcm->show();

  app.exec();
  delete kcm;
}
