/* $Id$ */
#ifndef KORG_MAINWINDOW_H
#define KORG_MAINWINDOW_H

#include <kparts/mainwindow.h>

namespace KOrg {

class CalendarViewBase;

/**
 * @short interface for korganizer main window
 * @author Cornelius Schumacher
 */
class MainWindow : public KParts::MainWindow
{
  public:
    MainWindow(const char *name) : KParts::MainWindow(0,name) {}
    virtual ~MainWindow() {};

    virtual CalendarViewBase *view() const = 0;

    /** Load calendar file from URL. Merge into current calendar, if \a merge is true. */
    virtual bool openURL(const KURL &url,bool merge=false) = 0;
    /** Save calendar file to URL of current calendar */
    virtual bool saveURL() = 0;
    /** Save calendar file to URL */
    virtual bool saveAsURL(const KURL & kurl) = 0;

    /** Get current URL */
    virtual KURL getCurrentURL() const = 0;
};

}

#endif
