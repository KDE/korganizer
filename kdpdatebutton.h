/* 	$Id$	 */

#ifndef _KDPDATEBUTTON_H
#define _KDPDATEBUTTON_H

#include <qdatetm.h>
#include <qlabel.h>
#include <qdropsite.h>

class CalObject;

class KDateButton: public QLabel, QDropSite {
   Q_OBJECT
 public:
   
   enum { NoHilite, SelectHilite, EventHilite, TodayHilite,
	  TodaySelectHilite, HolidayHilite, HolidaySelectHilite };
   
   enum { EVENTADDED, EVENTEDITED, EVENTDELETED };

   KDateButton(QDate date, int index, CalObject *,
	       QWidget *parent=0, const char *name=0);
   ~KDateButton();
   QDate date();
   void setItalics(bool ital = FALSE);
   void setHiliteStyle(int HiliteStyle);
   int  hiliteStyle();
   bool isSelected() const { return selFlag; }

   // static, for all date buttons
   static void updateConfig();

 public slots:
   void setDate(QDate newdate);

 signals:
   void selected(QDate, int, bool);
   void updateMe(int);

 protected:
   void mousePressEvent(QMouseEvent *);
   void dragEnterEvent(QDragEnterEvent *);
   void dragLeaveEvent(QDragLeaveEvent *);
   void dropEvent(QDropEvent *);

 private:
   bool selFlag;
   int my_index;
   QDate bt_Date;
   CalObject *calendar;
   int currHiliteStyle;
   QFont myFont;
   QPalette oldPalette;

   static QPalette my_OrigPalette;
   static QPalette my_NormalPalette;
   static QPalette my_HilitePalette;
   static QPalette my_TodayPalette;
   static QPalette my_TodaySelectPalette;
   static QPalette my_EventPalette;
   static QPalette my_HolidayPalette;
   static QPalette my_HolidaySelectPalette;
   static QColorGroup my_NormalGroup;
   static QColorGroup my_DisabledGroup;
   static QColorGroup my_HiliteGroup;
   static QColorGroup my_HiDisabledGroup;
   static QColorGroup my_TodayGroup;
   static QColorGroup my_TodaySelectGroup;
   static QColorGroup my_EventGroup;
   static QColorGroup my_HolidayGroup;
   static QColorGroup my_HolidaySelectGroup;
};

#endif // _KDPDATEBUTTON_H



