#ifndef TIMESPANVIEW_H
#define TIMESPANVIEW_H

#include <qwidget.h>

#include <libkcal/event.h>

class QSplitter;
class QListView;
class LineView;
class TimeLine;

class TimeSpanView : public QWidget
{
    Q_OBJECT
  public:
    TimeSpanView( QWidget *parent=0, const char *name=0 );
    virtual ~TimeSpanView();
    
    void addItem( KCal::Event * );

    QValueList<int> splitterSizes();
    void setSplitterSizes( QValueList<int> );
    
    void setDateRange( const QDateTime &start, const QDateTime &end );
    
  private:
    QSplitter *mSplitter;
    QListView *mList;
    TimeLine *mTimeLine;
    LineView *mLineView;
    
    QDateTime mStartDate;
    QDateTime mEndDate;
};

#endif
