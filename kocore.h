#ifndef KOCORE_H
#define KOCORE_H
// $Id$

#include <ktrader.h>

#include <calendar/textdecoration.h>
#include <calendar/widgetdecoration.h>
#include <korganizer/part.h>

class KOCore {
  public:
    static KOCore *self();

    KTrader::OfferList availablePlugins(const QString &type);
  
    KOrg::Plugin *loadPlugin(KService::Ptr service);
    KOrg::Plugin *loadPlugin(const QString &);
    
    KOrg::TextDecoration *loadTextDecoration(KService::Ptr service);

    KOrg::WidgetDecoration *loadWidgetDecoration(KService::Ptr service);
    KOrg::WidgetDecoration *loadWidgetDecoration(const QString &);

    KOrg::Part *loadPart(KService::Ptr,CalendarView *,QWidget *parent);
    KOrg::Part *loadPart(const QString &,CalendarView *,QWidget *parent);

    KOrg::TextDecoration::List textDecorations();
    KOrg::WidgetDecoration::List widgetDecorations();

    void reloadPlugins();

    QString holiday( const QDate & );

  protected:
    KOCore();
    
  private:
    static KOCore *mSelf;
    
    KOrg::TextDecoration::List mTextDecorations;
    bool mTextDecorationsLoaded;
    
    KOrg::WidgetDecoration::List mWidgetDecorations;
    bool mWidgetDecorationsLoaded;
    
    KOrg::TextDecoration *mHolidays;
    bool mHolidaysLoaded;
};

#endif
