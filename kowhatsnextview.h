#ifndef _KOWHATSNEXTVIEW_H
#define _KOWHATSNEXTVIEW_H
// $Id$

#include <qlistview.h>

#include "kobaseview.h"

class QTextView;

/**
 * This class provides a view of the next events and todos
 *
 * @author Cornelius Schumacher <schumacher@kde.org>
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
  
  private:
    QTextView *mView;
    QString mText;
};

#endif
