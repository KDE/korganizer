#ifndef TIMESPANVIEW_H
#define TIMESPANVIEW_H

#include <qwidget.h>

#include <libkcal/event.h>

class QSplitter;
class QListView;
class LineView;

class TimeSpanView : public QWidget
{
    Q_OBJECT
  public:
    TimeSpanView( QWidget *parent=0, const char *name=0 );
    virtual ~TimeSpanView();
    
    void addItem( KCal::Event * );

    QValueList<int> splitterSizes();
    void setSplitterSizes( QValueList<int> );
    
  private:
    QSplitter *mSplitter;
    QListView *mList;
    LineView *mLineView;
};

#endif
