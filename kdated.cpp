// 	$Id$	

#include <qpixmap.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>

#include "koprefs.h"

#include "kdated.h"
#include "kdated.moc"

KDateEdit::KDateEdit(QWidget *parent, const char *name)
  : QFrame(parent, name)
{
  QPixmap pixmap;

  kfEdit = new DateEdit(this);
  kfEdit->setDate(QDate::currentDate(), kfEdit->currentFormat());
  updateConfig(); // adjusts the format of the display

  kfEdit->setFixedHeight(kfEdit->sizeHint().height());
  kfEdit->setFixedWidth(90);
  kfEdit->adjustSize();

  pixmap = UserIcon("smallcal");
  dateButton = new QPushButton(this);
  dateButton->setPixmap(pixmap);
  dateButton->move(kfEdit->geometry().topRight().x()+1,
		   kfEdit->geometry().topRight().y());
  dateButton->setFixedHeight(20);
  dateButton->setFixedWidth(30);
  dateButton->adjustSize();

  datePicker = new KODatePicker(0, QDate::currentDate());
  datePicker->setFixedSize(200,200);
  datePicker->hide();

  adjustSize();
  
  connect(dateButton,SIGNAL(clicked()),SLOT(toggleDatePicker()));
  connect(datePicker,SIGNAL(dateSelected(QDate)),SLOT(setDate(QDate)));
  connect(datePicker,SIGNAL(dateSelected(QDate)),SIGNAL(dateChanged(QDate)));
  connect(datePicker,SIGNAL(dateSelected(QDate)),datePicker, SLOT(hide()));
}

KDateEdit::~KDateEdit()
{
}

QSize KDateEdit::sizeHint() const
{
  return QSize(kfEdit->width()+dateButton->width(),
	       kfEdit->height());
}

QSize KDateEdit::minimumSizeHint() const
{
  return sizeHint();
}

void KDateEdit::setDate(QDate newDate)
{
  if (!newDate.isValid())
    return;

  // if it is the same, no need to change.
  if (kfEdit->date()) // it may be null, we need to check
    if (newDate == *kfEdit->date())
      return;

  kfEdit->setDate(newDate, fmt);
//  emit dateChanged(newDate);
}

void KDateEdit::setEnabled(bool on)
{
  kfEdit->setEnabled(on);
  dateButton->setEnabled(on);
}

QDate KDateEdit::getDate() const
{
  QString tmpStr;
  int month, day, year;

  tmpStr = kfEdit->text();
  month = numFromMonthName(tmpStr.left(3));
  day = tmpStr.mid(4,2).toInt();
  year = tmpStr.right(4).toInt();

  if (kfEdit->date())
    return QDate(*kfEdit->date());
  else
    return QDate::currentDate();
}

void KDateEdit::updateConfig()
{
  int dateFormat = KOPrefs::instance()->mDateFormat;

  switch(dateFormat) {
  case 0: 
    kfEdit->setFormat(fmt = DateValidator::German);
    break;
  case 1:
    kfEdit->setFormat(fmt = DateValidator::English_Slash);
    break;
  case 2:
    kfEdit->setFormat(fmt = DateValidator::English_Minus);
    break;
  default:
    kfEdit->setFormat(fmt = DateValidator::English_Slash);
    break;
  }
}

void KDateEdit::toggleDatePicker()
{
  static bool visible = FALSE;
  QPoint tmpPoint;

  tmpPoint = mapToGlobal(dateButton->geometry().bottomRight());

  datePicker->setGeometry(tmpPoint.x()-207, tmpPoint.y(), 200, 200);
  if (kfEdit->date())
    datePicker->setDate(*kfEdit->date());
  if (!visible) {
//    qDebug("------show");
    datePicker->show();
    datePicker->raise();
  } else {
//    qDebug("------hide");
    datePicker->hide();
  }
}

int KDateEdit::numFromMonthName(QString name) const
{
  name = name.upper();

  if (name == "JAN")
    return 1;
  else if (name == "FEB")
    return 2;
  else if (name == "MAR")
    return 3;
  else if (name == "APR")
    return 4;
  else if (name == "MAY")
    return 5;
  else if (name == "JUN")
    return 6;
  else if (name == "JUL")
    return 7;
  else if (name == "AUG")
    return 8;
  else if (name == "SEP")
    return 9;
  else if (name == "OCT")
    return 10;
  else if (name == "NOV")
    return 11;
  else if (name == "DEC")
    return 12;
  else
    // should never get here!
    return 0;  
}

