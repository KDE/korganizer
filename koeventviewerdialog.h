#ifndef KOEVENTVIEWERDIALOG_H
#define KOEVENTVIEWERDIALOG_H
// $Id$
//
// Viewer dialog for events.
//

#include <qtextview.h>

#include <kdialogbase.h>

class KOEvent;
class KOEventViewer;

class KOEventViewerDialog : public KDialogBase {
    Q_OBJECT
  public:
    KOEventViewerDialog(QWidget *parent=0,const char *name=0);
    virtual ~KOEventViewerDialog();

    void setEvent(KOEvent *event);
    void setTodo(Todo *event);
    
  private:
    KOEventViewer *mEventViewer;
};

#endif
