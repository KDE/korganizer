#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H
// $Id$
//
// Alarm dialog.
//

#include <kdialogbase.h>

class KOEvent;
class KOEventViewer;
class QSpinBox;

class AlarmDialog : public KDialogBase {
    Q_OBJECT
  public:
    AlarmDialog(QWidget *parent=0,const char *name=0);
    virtual ~AlarmDialog();

    void appendEvent(KOEvent *event);

    void eventNotification();

  public slots:
    void slotOk();
    void slotUser1();

  signals:
    void suspendSignal(int duration);
    
  private:
    KOEventViewer *mEventViewer;
    
    QList<KOEvent> mEvents;

    QSpinBox *mSuspendSpin;
};

#endif
