#ifndef KOEVENTVIEWER_H
#define KOEVENTVIEWER_H
// $Id$
//
// Viewer widget for events.
//

#include <qtextview.h>


class KOEvent;


class KOEventViewer : public QTextView {
    Q_OBJECT
  public:
    KOEventViewer(QWidget *parent=0,const char *name=0);
    virtual ~KOEventViewer();

    void setEvent(KOEvent *event);
    void setTodo(KOEvent *event);
    
    void appendEvent(KOEvent *event);
    void appendTodo(KOEvent *event);
    
    void clearEvents(bool now=false);
    
  protected:
    void addTag(const QString & tag,const QString & text);

    void formatCategories(KOEvent *event);
    void formatAttendees(KOEvent *event);
    void formatReadOnly(KOEvent *event);

  private:
    QTextView *mEventTextView;

    QString mText;    
};

#endif
