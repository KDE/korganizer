#include <qapplication.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>

#include <calendarsystem/kcalendarsystem.h>

#include "koglobals.h"

KOGlobals *KOGlobals::mSelf = 0;

KOGlobals *KOGlobals::self()
{
  if (!mSelf) {
    mSelf = new KOGlobals;
  }
  
  return mSelf;
}

KOGlobals::KOGlobals()
{
  KGlobal::config()->setGroup("General");
  QString calSystem = KGlobal::config()->readEntry( "CalendarSystem",
                                                    "gregorian" );
  
  mCalendarSystem = KCalendarSystemFactory::create( calSystem );
}

KCalendarSystem *KOGlobals::calendarSystem()
{
  return mCalendarSystem;
}

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
    wid->resize( w, h );
    wid->move( 0, 15 );
    if ( force ) wid->setFixedSize( w, h );
  }
}

bool KOGlobals::reverseLayout()
{
#if QT_VERSION >= 0x030000
  return QApplication::reverseLayout();
#else
  return false;
#endif
}
