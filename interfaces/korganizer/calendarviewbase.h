#ifndef KORG_CALENDARVIEWBASE_H
#define KORG_CALENDARVIEWBASE_H
/* $Id$ */

#include <qwidget.h>

#include <libkcal/calendar.h>

#include <korganizer/baseview.h>

namespace KOrg {

/**
  @short interface for main calendar view widget
  @author Cornelius Schumacher
*/
class CalendarViewBase : public QWidget
{
  public:
    CalendarViewBase(QWidget *parent, const char *name) :
      QWidget(parent,name) {};
    virtual ~CalendarViewBase() {};
  
    virtual KCal::Calendar *calendar() = 0;

    virtual void addView(KOrg::BaseView *) = 0;

    /** changes the view to be the currently selected view */
    virtual void showView(KOrg::BaseView *) = 0;
};

}

#endif
