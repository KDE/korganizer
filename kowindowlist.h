/* $Id$ */
#ifndef _KOWINDOWLIST_H
#define _KOWINDOWLIST_H

#include <qobject.h>

#include "korganizer.h"

/**
 * This class manages a list of KOrganizer instances, each associated with a
 * window displaying a calendar. It acts as relay for signals between this
 * windows and manages information like the active calendar, which requires
 * interaction of all instances.
 *
 * @short manages a list of all KOrganizer instances
 * @author Cornelius Schumacher
 * @version $Revision$
 */
class KOWindowList : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructs a new list of KOrganizer windows. There should only be one
     * instance of this class. The KOrganizer class takes care of this.
     */
    KOWindowList(const char *name=0);
    virtual ~KOWindowList();

    /** Is there only one instance left? */
    bool lastInstance();

  signals:

  public slots:
    void addWindow(KOrganizer *);
    void removeWindow(KOrganizer *);
    
    /** Deactivating all calendars despite the one given in the argument*/
    void deactivateCalendars(KOrganizer *);
    
  private:
    QList<KOrganizer> mWindowList; // list of all existing KOrganizer instances
};

#endif // _KOWINDOWLIST_H
