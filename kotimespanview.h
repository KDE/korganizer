#ifndef KOTIMESPANVIEW_H
#define KOTIMESPANVIEW_H

#include "koeventview.h"

class KConfig;
class TimeSpanView;

class KOTimeSpanView : public KOEventView
{
    Q_OBJECT
  public:
    KOTimeSpanView(Calendar *calendar, QWidget *parent = 0, 
	           const char *name = 0);
    ~KOTimeSpanView();

    virtual int maxDatesHint();
    virtual int currentDateCount();
    virtual QPtrList<Incidence> getSelected();

    void readSettings();
    void readSettings( KConfig * );
    void writeSettings( KConfig * );

  public slots:
    virtual void updateView();
    virtual void selectDates(const QDateList dateList);
    virtual void selectEvents(QPtrList<Event> eventList);

    void changeEventDisplay(Event *, int);

  private:
    TimeSpanView *mTimeSpanView;
};

#endif
