// quickdialog.h
// (C)1998 by Fester Zigterman

#ifndef QUICKDIALOG_H
#define QUICKDIALOG_H

#include <qlined.h>
#include <qmlined.h>
#include <qframe.h>
#include <qlayout.h>
#include <qpushbt.h>

#include "calobject.h"
#include "eventwidget.h"

class QuickDialog : public QFrame
{
	Q_OBJECT
public:
	QuickDialog( CalObject *, QWidget *parent = 0, char *name = 0 );
	~QuickDialog();
public slots:
	void updateDialog();
	void setSelected( KOEvent *);
signals:
	void eventChanged();
protected:
	void fillDialog();
protected slots:
	void slot_new();
	void slot_openDlg();
private:
	CalObject *qCalendar;
	QVBoxLayout *qLayout;
	QHBoxLayout *qButtonLayout;
	QLineEdit *summaryBox;
	QMultiLineEdit *descriptionBox;
	QPushButton *button;
	KOEvent *qEvent;
	KOEvent *selectedEvent;
	bool Modified;
};

#endif
