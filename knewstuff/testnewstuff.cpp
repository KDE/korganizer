#include <iostream>

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kprocess.h>
#include <kdialog.h>

#include "testnewstuff.h"
#include "testnewstuff.moc"

using namespace std;

bool TestNewStuff::install( const QString &fileName )
{
  kdDebug() << "TestNewStuff::install(): " << fileName << endl;
  QFile f( fileName );
  if ( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "Error opening file." << endl;
    return false;
  }
  QTextStream ts( &f );
  kdDebug() << "--BEGIN-NEW_STUFF--" << endl;
  cout << ts.read().utf8();
  kdDebug() << "---END-NEW_STUFF---" << endl;
  return true;
}

bool TestNewStuff::createUploadFile( const QString &fileName )
{
  KProcess p;
  p << "touch" << fileName;
  p.start(KProcess::Block);
  kdDebug() << "TestNewStuff::createUploadFile(): " << fileName << endl;
  return fileName;
}


MyWidget::MyWidget()
{
  mNewStuff = new TestNewStuff;

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );
  
  QPushButton *downloadButton = new QPushButton( "Download", this );
  topLayout->addWidget( downloadButton );
  connect( downloadButton, SIGNAL( clicked() ), SLOT( download() ) );

  QPushButton *uploadButton = new QPushButton( "Upload", this );
  topLayout->addWidget( uploadButton );
  connect( uploadButton, SIGNAL( clicked() ), SLOT( upload() ) );

  topLayout->addSpacing( 5 );

  QPushButton *closeButton = new QPushButton( "Close", this );
  topLayout->addWidget( closeButton );
  connect( closeButton, SIGNAL( clicked() ), kapp, SLOT( quit() ) );
}

MyWidget::~MyWidget()
{
  delete mNewStuff;
}

void MyWidget::download()
{
  kdDebug() << "MyWidget::download()" << endl;

  mNewStuff->download();
}

void MyWidget::upload()
{
  kdDebug() << "MyWidget::download()" << endl;

  mNewStuff->upload();
}


int main(int argc,char **argv)
{
  KAboutData aboutData("knewstufftest","KNewStuff Test","0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  MyWidget wid;
  app.setMainWidget( &wid );
  wid.show();

  app.exec();
}
