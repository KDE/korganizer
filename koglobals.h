#ifndef KORG_GLOBALS_H
#define KORG_GLOBALS_H

class KOGlobals
{
  public:
    enum { EVENTADDED, EVENTEDITED, EVENTDELETED };  
    enum { PRIORITY_MODIFIED, COMPLETION_MODIFIED, CATEGORY_MODIFIED, UNKNOWN_MODIFIED };

    static void fitDialogToScreen( QWidget *widget, bool force=false );
};

#endif
