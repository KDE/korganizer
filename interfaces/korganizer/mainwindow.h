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
};

}

#endif
