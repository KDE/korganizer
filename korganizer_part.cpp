// $Id$
#include "korganizer_part.h"

#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kdebug.h>

#include "calendarview.h"

extern "C"
{
  /**
   * This function is the 'main' function of this part.  It takes
   * the form 'void *init_lib<library name>()  It always returns a
   * new factory object
   */
  void *init_libkorganizer()
  {
    return new KOrganizerFactory;
  }
};

/**
* We need one static instance of the factory for our C 'main'
* function
*/
KInstance *KOrganizerFactory::s_instance = 0L;
KAboutData *KOrganizerFactory::s_about = 0L;

KOrganizerFactory::KOrganizerFactory()
{
}

KOrganizerFactory::~KOrganizerFactory()
{
  delete s_instance;
  s_instance = 0;
  delete s_about;
}

QObject *KOrganizerFactory::create(QObject *parent, const char *name,
                                   const char*,const QStringList& )
{
  QObject *obj = new KOrganizerPart((QWidget*)parent, name);
  emit objectCreated(obj);
  return obj;
}

KInstance *KOrganizerFactory::instance()
{
  if ( !s_instance ) {
    s_about = new KAboutData("korganizer", I18N_NOOP("KOrganizer"),"1.99");
    s_instance = new KInstance(s_about);
  }
  
  kdDebug() << "KOrganizerFactory::instance(): Name: " <<
               s_instance->instanceName() << endl;
  
  return s_instance;
}

KOrganizerPart::KOrganizerPart(QWidget *parent, const char *name) :
  KParts::ReadOnlyPart(parent, name)
{
  setInstance(KOrganizerFactory::instance());

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget(parent);
  canvas->setFocusPolicy(QWidget::ClickFocus);
  setWidget(canvas);

  m_extension = new KOrganizerBrowserExtension(this);

  QVBoxLayout *topLayout = new QVBoxLayout(canvas);

  KGlobal::iconLoader()->addAppDir("korganizer");

  widget = new CalendarView(canvas);
  topLayout->addWidget(widget);

  widget->show();

  (void)new KAction(i18n("&List"), "list", 0,
                    widget, SLOT(view_list()),
                    actionCollection(), "view_list");
  (void)new KAction(i18n("&Day"), "1day", 0,
                    widget, SLOT(view_day()),
                    actionCollection(), "view_day");
  (void)new KAction(i18n("W&ork Week"), "5days", 0,
                    widget, SLOT(view_workweek()),
                    actionCollection(), "view_workweek");
  (void)new KAction(i18n("&Week"), "7days", 0,
                    widget, SLOT(view_week()),
                    actionCollection(), "view_week");
  (void)new KAction(i18n("&Month"), "month", 0,
                    widget, SLOT(view_month()),
                    actionCollection(), "view_month");
  (void)new KAction(i18n("&To-do list"), "todo", 0,
                    widget, SLOT(view_todolist()),
                    actionCollection(), "view_todo");

  setXMLFile( "korganizer_part.rc" );
}

KOrganizerPart::~KOrganizerPart()
{
  closeURL();
}

bool KOrganizerPart::openFile()
{
  widget->openCalendar(m_file);
  widget->show();
  return true;
}

KOrganizerBrowserExtension::KOrganizerBrowserExtension(KOrganizerPart *parent) :
  KParts::BrowserExtension(parent, "KOrganizerBrowserExtension")
{
}

KOrganizerBrowserExtension::~KOrganizerBrowserExtension()
{
}
