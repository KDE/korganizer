#ifndef LINEVIEW_H
#define LINEVIEW_H

#include <qscrollview.h>
#include <qptrlist.h>

class LineView : public QScrollView
{
    Q_OBJECT
  public:
    LineView( QWidget *parent = 0, const char *name = 0 );
    virtual ~LineView();

    int pixelWidth();
    
    void addLine( int start, int end );

  protected:
    void drawContents(QPainter* p, int cx, int cy, int cw, int ch);

  private:
    struct Line {
      Line( int c, int s, int e ) : column( c ), start( s ), end( e ) {}
      int column;
      int start;
      int end;
    };

    QPtrList<Line> mLines;
    int mPixelWidth;
};

#endif

