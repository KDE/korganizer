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
    
    bool install( QString &fileName );
    
    QString createUploadFile();
};

class MyWidget : public QWidget
{
    Q_OBJECT
  public:
    MyWidget();
    ~MyWidget();
    
  public slots:
    void upload();
    void download();

  private:
    KNewStuff *mNewStuff;
};

#endif
