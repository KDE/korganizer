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
    KOWhatsNextView(CalObject *calendar, QWidget *parent = 0, 
	            const char *name = 0);
    ~KOWhatsNextView();

    virtual int maxDatesHint();
    virtual int currentDateCount();
    virtual QList<KOEvent> getSelected();

    virtual void printPreview(CalPrinter *calPrinter,
                              const QDate &, const QDate &);
  
  public slots:
    virtual void updateView();
    virtual void selectDates(const QDateList dateList);
    virtual void selectEvents(QList<KOEvent> eventList);

    void changeEventDisplay(KOEvent *, int);
  
  protected:
    void appendEvent(KOEvent *);
    void appendTodo(KOEvent *);
  
  private:
    QTextView *mView;
    QString mText;
};

#endif
