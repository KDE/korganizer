#ifndef _KOJOURNALVIEW_H
#define _KOJOURNALVIEW_H
// $Id$

#include "kobaseview.h"

class JournalEntry;

/**
 * This class provides a journal view.
 
 * @short View for Journal components.
 * @author Cornelius Schumacher <schumacher@kde.org>
 * @see KOBaseView
 */
class KOJournalView : public KOBaseView
{
    Q_OBJECT
  public:
    KOJournalView(CalObject *calendar, QWidget *parent = 0, 
	       const char *name = 0);
    ~KOJournalView();

    virtual int currentDateCount();
    virtual QList<Incidence> getSelected();

  public slots:
    void updateView();
    void flushView();
    
    void selectDates(const QDateList dateList);
    void selectEvents(QList<Event> eventList);

    void changeEventDisplay(Event *, int);
  
  private:
    JournalEntry *mEntry;
};

#endif
