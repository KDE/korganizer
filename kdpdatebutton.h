/* $Id$ */
#ifndef _KDPDATEBUTTON_H
#define _KDPDATEBUTTON_H

#include <qdatetime.h>
#include <qlabel.h>

#include <libkcal/calendar.h>

using namespace KCal;

class KDateButton: public QLabel {
    Q_OBJECT
  public:
    KDateButton(QDate date, int index, Calendar *,
	        QWidget *parent=0, const char *name=0);
    ~KDateButton();

    QDate date();

    void setItalic(bool italic=true);
    void setEvent(bool event=true);
    void setSelected(bool select=true);
    void setToday(bool today=true);
    void setHoliday(bool holiday=true);
    void setShaded(bool shaded=true);

    bool isSelected() const { return mSelected; }

    void updateConfig();

    QSize sizeHint () const;
    QSize minimumSizeHint () const { return sizeHint(); }

  public slots:
    void setDate(QDate newdate);

  signals:
    void selected(QDate, int, bool);
    void updateMe(int);
    void eventDropped(Event *);

  protected:
    void mousePressEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dropEvent(QDropEvent *);

    void setColors();
    void setBackColor(const QColor & color);
    void setTextColor(const QColor & color);

    QColor getShadedColor(QColor color);

  private:
    bool mSelected;
    bool mEvent;
    bool mToday;
    bool mHoliday;
    bool mItalic;
    bool mShaded;

    QColor mDefaultBackColor;
    QColor mDefaultTextColor;
    QColor mDefaultTextColorShaded;
    QColor mHolidayColorShaded;

    int mTodayMarginWidth;

    int my_index;
    QDate mDate;
    Calendar *mCalendar;
};

#endif // _KDPDATEBUTTON_H



