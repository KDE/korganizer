#ifndef TIMELINE_H
#define TIMELINE_H

#include <qscrollview.h>
#include <qdatetime.h>

class TimeLine : public QScrollView
{
    Q_OBJECT
  public:
    TimeLine( QWidget *parent = 0, const char *name = 0 );
    virtual ~TimeLine();

    void setDateRange( const QDateTime &start, const QDateTime &end );

  public slots:
    void setContentsPos( int pos );

  protected:
    void drawContents(QPainter* p, int cx, int cy, int cw, int ch);

  private:
    QDateTime mStartDate;
    QDateTime mEndDate;

    int mPixelWidth;
    int mDaySpacing;
    int mDayOffset;
    int mSecsPerPixel;
};

#endif

