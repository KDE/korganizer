#ifndef KOEVENTPOPUPMENU_H
#define KOEVENTPOPUPMENU_H
// $Id$
//
// Context menu for event views with standard event actions
//

#include <qpopupmenu.h>

#include <event.h>

using namespace KCal;

class KOEventPopupMenu : public QPopupMenu {
    Q_OBJECT
  public:
    KOEventPopupMenu();
  
    void addAdditionalItem(const QIconSet &icon,const QString &text,
                           const QObject *receiver, const char *member,
                           bool editOnly=false);


  public slots:
    void showEventPopup(Event *);

  protected slots:
    void popupShow();
    void popupEdit();
    void popupDelete();

  signals:
    void editEventSignal(Event *);
    void showEventSignal(Event *);
    void deleteEventSignal(Event *);
    
  private:
    Event *mCurrentEvent;
    
    bool mHasAdditionalItems;
    QValueList<int> mEditOnlyItems;
};

#endif
