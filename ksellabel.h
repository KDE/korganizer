/* 	$Id$	 */

#ifndef _KSELLABEL_H 
#define _KSELLABEL_H

#include <qframe.h>

class KSelLabel: public QFrame {
   Q_OBJECT
 public:
   KSelLabel(QWidget *parent = 0, const char *text = 0,
	     int idx =  0, const char *name = 0);
   ~KSelLabel();
   const char * text();
   QSize sizeHint() const;

 public slots:
   void setText(const char *);
   void setActivated(bool activated);
   void setAlignment(int align);

 signals:
   void labelActivated(int);
   void newEventSignal(int);

 protected slots:
   void paintEvent(QPaintEvent *); 
   void focusInEvent(QFocusEvent *);
   void mouseDoubleClickEvent(QMouseEvent *);
   void updateLabel();
 
 private:
   // this label's text.
   QString  labeltext;
   // this labels index. It will be passed back in all activation
   // signals, so it can be used to identify the label being selected.
   int      index;
   int      alignment;
   bool     act;
};

#endif
