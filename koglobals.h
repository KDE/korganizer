#ifndef KORG_GLOBALS_H
#define KORG_GLOBALS_H

class KCalendarSystem;

class KOGlobals
{
  public:
    static KOGlobals *self();
  
    enum { EVENTADDED, EVENTEDITED, EVENTDELETED };  
    enum { PRIORITY_MODIFIED, COMPLETION_MODIFIED, CATEGORY_MODIFIED, UNKNOWN_MODIFIED };

    static void fitDialogToScreen( QWidget *widget, bool force=false );

    static bool reverseLayout();

    KCalendarSystem *calendarSystem();

  protected:
    KOGlobals();
    
  private:
    static KOGlobals *mSelf;
    
    KCalendarSystem *mCalendarSystem;    
};

#endif
