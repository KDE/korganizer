#ifndef KOEVENTVIEWER_H
#define KOEVENTVIEWER_H
// $Id$
//
// Viewer widget for events.
//

#include <qtextview.h>


class Event;


class KOEventViewer : public QTextView {
    Q_OBJECT
  public:
    KOEventViewer(QWidget *parent=0,const char *name=0);
    virtual ~KOEventViewer();

    void setEvent(Event *event);
    void setTodo(Todo *event);
    
    void appendEvent(Event *event);
    void appendTodo(Todo *event);
    
    void clearEvents(bool now=false);
    
  protected:
    void addTag(const QString & tag,const QString & text);

    void formatCategories(Incidence *event);
    void formatAttendees(Incidence *event);
    void formatReadOnly(Incidence *event);

  private:
    QTextView *mEventTextView;

    QString mText;    
};

#endif
