#ifndef KOEVENTVIEWER_H
#define KOEVENTVIEWER_H
// $Id$
//
// Viewer widget and dialog for events.
//

#include <qtextview.h>

#include <kdialogbase.h>

class KOEvent;


class KOEventViewer : public QTextView {
    Q_OBJECT
  public:
    KOEventViewer(QWidget *parent=0,const char *name=0);
    virtual ~KOEventViewer();

    void setEvent(KOEvent *event);
    void setTodo(KOEvent *event);
    
  protected:
    void addTag(const QString & tag,const QString & text);

  private:
    QTextView *mEventTextView;

    QString mText;    
};

class KOEventViewerDialog : public KDialogBase {
    Q_OBJECT
  public:
    KOEventViewerDialog(QWidget *parent=0,const char *name=0);
    virtual ~KOEventViewerDialog();

    void setEvent(KOEvent *event);
    void setTodo(KOEvent *event);
    
  private:
    KOEventViewer *mEventViewer;
};

#endif
