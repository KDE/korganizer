#include <qapplication.h>

#include <kdebug.h>

#include "koglobals.h"

void KOGlobals::fitDialogToScreen( QWidget *wid, bool force )
{
  bool resized = false;

  int w = wid->frameSize().width();
  int h = wid->frameSize().height();  

  if ( w > QApplication::desktop()->size().width() ) {
    w = QApplication::desktop()->size().width();
    resized = true;
  }
  if ( h > QApplication::desktop()->size().height() - 30 ) {
    h = QApplication::desktop()->size().height() - 30;
    resized = true;
  }
  
  if ( resized || force ) {
    kdDebug() << "KOGlobals::fitDialogToScreen(): resize" << endl;
    wid->resize( w, h );
    wid->move( 0, 15 );
    if ( force ) wid->setFixedSize( w, h );
  } else {
    kdDebug() << "-- No Resize" << endl;
  }
}
