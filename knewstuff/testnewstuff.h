#ifndef TESTNEWSTUFF_H
#define TESTNEWSTUFF_H

#include <stdlib.h>

#include <qpushbutton.h>

#include <kapplication.h>
#include <kdebug.h>

#include "knewstuff.h"

class TestNewStuff : public KNewStuff
{
  public:
    TestNewStuff() : KNewStuff( "korganizer/calendar" ) {}
    
    bool install( QString &fileName )
    {
      kdDebug() << "TestNewStuff::install(): " << fileName << endl;
      return true;
    }
    
    QString createUploadFile()
    {
      QString fileName = "/tmp/" + KApplication::randomString( 5 );
      QString cmd = "touch " + fileName;
      system( cmd.latin1() );
      kdDebug() << "TestNewStuff::createUploadFile(): " << fileName << endl;
      return fileName;
    }
};

class MyWidget : public QPushButton
{
    Q_OBJECT
  public:
    MyWidget() : QPushButton( "Hallo", 0 )
    {
      connect( this, SIGNAL( clicked() ), SLOT( mySlot() ) );
    }
    
  public slots:
    void mySlot();
};

#endif
