// $Id$
#ifndef __korganizer_part_h__
#define __korganizer_part_h__

#include <kparts/browserextension.h>
#include <kparts/factory.h>

class KInstance;
class KAboutData;
class KOrganizerBrowserExtension;

class CalendarView;

class KOrganizerFactory : public KParts::Factory
{
    Q_OBJECT
  public:
    KOrganizerFactory();
    virtual ~KOrganizerFactory();

    virtual KParts::Part* createPartObject(QWidget *parentWidget, const char *name,
    QObject* parent = 0, const char* name = 0,
    const char* classname = "QObject",
    const QStringList &args = QStringList());

    static KInstance *instance();

  private:
    static KInstance *s_instance;
    static KAboutData *s_about;
};

class KOrganizerPart: public KParts::ReadOnlyPart
{
    Q_OBJECT
  public:
    KOrganizerPart(QWidget *parentWidget, const char *widgetName,
                   QObject *parent, const char *name);
    virtual ~KOrganizerPart();

  protected:
    virtual bool openFile();

  private:
    CalendarView *widget;
    KOrganizerBrowserExtension *m_extension;
};

class KOrganizerBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
    friend class KOrganizerPart;
  public:
    KOrganizerBrowserExtension(KOrganizerPart *parent);
    virtual ~KOrganizerBrowserExtension();
};

#endif
