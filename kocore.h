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

    KOrg::Part *loadPart(KService::Ptr,KOrganizer *parent);
    KOrg::Part *loadPart(const QString &,KOrganizer *parent);

    KOrg::TextDecoration::List textDecorations();
    KOrg::WidgetDecoration::List widgetDecorations();
    KOrg::Part::List parts(KOrganizer *parent);

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
    
    KOrg::Part::List mParts;
    bool mPartsLoaded;

    KOrg::TextDecoration *mHolidays;
    bool mHolidaysLoaded;    
};

#endif
