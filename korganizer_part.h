// $Id$
#ifndef __korganizer_part_h__
#define __korganizer_part_h__

#include <kparts/browserextension.h>
#include <klibloader.h>

class KInstance;
class KAboutData;
class KOrganizerBrowserExtension;

class CalendarView;

class KOrganizerFactory : public KLibFactory
{
    Q_OBJECT
  public:
    KOrganizerFactory();
    virtual ~KOrganizerFactory();

    virtual QObject* create(QObject* parent = 0, const char* name = 0,
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
    KOrganizerPart(QWidget *parent, const char *name);
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
