#ifndef KORG_GLOBALS_H
#define KORG_GLOBALS_H

class KOGlobals
{
  public:
    enum { EVENTADDED, EVENTEDITED, EVENTDELETED };  

    static void fitDialogToScreen( QWidget *widget, bool force=false );
};

#endif
