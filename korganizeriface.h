#ifndef KORGANIZERIFACE_H
#define KORGANIZERIFACE_H
 
#include <dcopobject.h>
#include <kurl.h>

class KOrganizerIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual bool openURL(QString url) = 0;
    virtual bool mergeURL(QString url) = 0;
    virtual void closeURL() = 0;
    virtual bool saveURL() = 0;
    virtual bool saveAsURL(QString url) = 0;
    virtual QString getCurrentURLasString() const = 0;
    virtual bool deleteEvent(QString VUID) = 0;
};

#endif
