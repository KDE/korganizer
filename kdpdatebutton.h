/* 	$Id$	 */

#ifndef _KDPDATEBUTTON_H
#define _KDPDATEBUTTON_H

#include <qdatetm.h>
#include <qlabel.h>

class CalObject;

class KDateButton: public QLabel {
    Q_OBJECT
  public:
    KDateButton(QDate date, int index, CalObject *,
	        QWidget *parent=0, const char *name=0);
    ~KDateButton();

    QDate date();

    void setItalic(bool italic=true);
    void setEvent(bool event=true);
    void setSelected(bool select=true);
    void setToday(bool today=true);
    void setHoliday(bool holiday=true);

    bool isSelected() const { return mSelected; }

    void updateConfig();

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

    void setColors();
    void setBackColor(const QColor & color);
    void setTextColor(const QColor & color);

  private:
    bool mSelected;
    bool mEvent;
    bool mToday;
    bool mHoliday;
    bool mItalic;

    QColor mDefaultBackColor;
    QColor mDefaultTextColor;

    int my_index;
    QDate bt_Date;
    CalObject *calendar;
};

#endif // _KDPDATEBUTTON_H



