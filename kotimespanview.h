#ifndef KOTIMESPANVIEW_H
#define KOTIMESPANVIEW_H

#include "koeventview.h"

class KConfig;
class TimeSpanView;

class KOTimeSpanView : public KOEventView
{
    Q_OBJECT
  public:
    KOTimeSpanView( Calendar *calendar, QWidget *parent = 0, 
	            const char *name = 0 );
    ~KOTimeSpanView();

    virtual int maxDatesHint();
    virtual int currentDateCount();
    virtual Incidence::List selectedIncidences();
    DateList selectedDates() { return DateList(); }

    void readSettings();
    void readSettings( KConfig * );
    void writeSettings( KConfig * );

  public slots:
    virtual void updateView();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showEvents( const Event::List & );

    void changeEventDisplay(Event *, int);

  private:
    void insertItems( const QDate &start, const QDate & end );

    TimeSpanView *mTimeSpanView;
};

#endif
