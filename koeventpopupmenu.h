#ifndef KOEVENTPOPUPMENU_H
#define KOEVENTPOPUPMENU_H
// $Id$
//
// Context menu for event views with standard event actions
//

#include <qpopupmenu.h>

class KOEvent;

class KOEventPopupMenu : public QPopupMenu {
    Q_OBJECT
  public:
    KOEventPopupMenu();
  
    void addAdditionalItem(const QIconSet &icon,const QString &text,
                           const QObject *receiver, const char *member,
                           bool editOnly=false);


  public slots:
    void showEventPopup(KOEvent *);

  protected slots:
    void popupShow();
    void popupEdit();
    void popupDelete();

  signals:
    void editEventSignal(KOEvent *);
    void showEventSignal(KOEvent *);
    void deleteEventSignal(KOEvent *);
    
  private:
    KOEvent *mCurrentEvent;
    
    bool mHasAdditionalItems;
    QValueList<int> mEditOnlyItems;
};

#endif
