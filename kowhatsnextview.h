#ifndef KOWHATSNEXTVIEW_H
#define KOWHATSNEXTVIEW_H
// $Id$

#include <qtextbrowser.h>

#include "kobaseview.h"

class QListView;

class KOEventViewerDialog;

class WhatsNextTextBrowser : public QTextBrowser {
    Q_OBJECT
  public:
    WhatsNextTextBrowser(QWidget *parent) : QTextBrowser(parent) {}

    void setSource(const QString &);

  signals:
    void showIncidence(const QString &uid);
};


/**
 This class provides a view of the next events and todos
*/
class KOWhatsNextView : public KOBaseView
{
    Q_OBJECT
  public:
    KOWhatsNextView(Calendar *calendar, QWidget *parent = 0, 
	            const char *name = 0);
    ~KOWhatsNextView();

    virtual int maxDatesHint();
    virtual int currentDateCount();
    virtual QList<Incidence> getSelected();

    virtual void printPreview(CalPrinter *calPrinter,
                              const QDate &, const QDate &);
  
  public slots:
    virtual void updateView();
    virtual void selectDates(const QDateList dateList);
    virtual void selectEvents(QList<Event> eventList);

    void changeEventDisplay(Event *, int);
  
  protected:
    void appendEvent(Event *);
    void appendTodo(Todo *);
  
  private slots:
    void showIncidence(const QString &);
  
  private:
    void createEventViewer();
  
    QTextBrowser *mView;
    QString mText;
    
    KOEventViewerDialog *mEventViewer;
};

#endif
