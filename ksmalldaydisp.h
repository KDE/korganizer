/* 	$Id$	 */

#ifndef _KSMALLDAYDISP_H 
#define _KSMALLDAYDISP_H

#include <qdatetime.h>
#include <qlist.h>
#include <qlistbox.h>
#include <qintdict.h>
#include <qkeycode.h>

#include "calobject.h"
#include "koevent.h"

class KNoScrollListBox: public QListBox {
  Q_OBJECT
 public:
  KNoScrollListBox(QWidget *parent=0, const char *name=0)
    :QListBox(parent, name)
    {
      clearTableFlags();
      setTableFlags(Tbl_clipCellPainting | Tbl_cutCellsV | Tbl_snapToVGrid |
		    Tbl_scrollLastHCell| Tbl_smoothHScrolling);
    }
  ~KNoScrollListBox() {}

 signals:
  void shiftDown();
  void shiftUp();

 protected slots:
  void keyPressEvent(QKeyEvent *);
  void keyReleaseEvent(QKeyEvent *);
  void mousePressEvent(QMouseEvent *);
};

class KDPSmallDayDisp: public QFrame {
   Q_OBJECT
 public:
   KDPSmallDayDisp(QWidget    *parent   = 0, 
		   CalObject  *calendar = 0, 
		   QDate       qd       = QDate::currentDate(),
		   int         index    = 0,
		   bool        showFullHeader = FALSE,
		   const char *name     = 0);

   ~KDPSmallDayDisp();
   void setShowFullHeader(bool on);
   void setSelected(bool on);
   QDate getDate();
   
 public slots:
   void updateDisp();
  /** change the date for this display and update it's contents */
   void setDate(QDate qd);

 signals:
   void editEventSignal(KOEvent *);
   void shiftDaySelectedSignal(QDate, int);
   void daySelectedSignal(QDate, int);

 protected slots:
   void eventSelected(int);
   void eventHilited(int);
   void daySelected(int);
   void resizeEvent(QResizeEvent *);
   void shiftDown();
   void shiftUp();

 protected:
   
 private:
   QDate               myDate;
   int                 myIndex;
   CalObject          *myCal;
   bool                showFullHeader;
   KNoScrollListBox   *header;
   KNoScrollListBox   *summaries; 
   QList<KOEvent>     events;
   QIntDict<KOEvent> *currIdxs; 
   bool                shiftPressed;
   bool                selected;
   int                 evtSelected;
};

#endif _KDPSMALLDAYDISP_H

