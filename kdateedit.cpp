/*
    This file is part of KOrganizer.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$	

#include <qapplication.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qevent.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdatepicker.h>
#include <knotifyclient.h>

#include "kdateedit.h"
#include "kdateedit.moc"

KDateEdit::KDateEdit(QWidget *parent, const char *name)
  : QHBox(parent, name)
{
  mDateEdit = new QLineEdit(this);
  mDateEdit->setText(KGlobal::locale()->formatDate(QDate::currentDate(),true));
  setFocusProxy(mDateEdit);
  mDateEdit->installEventFilter(this);

  QPixmap pixmap = SmallIcon("smallcal");
  mDateButton = new QPushButton(this);
  mDateButton->setPixmap(pixmap);
  
  mDateFrame = new QVBox(0,0,WType_Popup);
  mDateFrame->setFrameStyle(QFrame::PopupPanel | QFrame::Raised);
  mDateFrame->setFixedSize(200,200);
  mDateFrame->setLineWidth(3);
  mDateFrame->hide();

  mDatePicker = new KDatePicker(mDateFrame,QDate::currentDate());

  connect(mDateEdit,SIGNAL(returnPressed()),SLOT(lineEnterPressed()));
  connect(mDateEdit,SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));

  connect(mDateButton,SIGNAL(clicked()),SLOT(toggleDatePicker()));

  connect(mDatePicker,SIGNAL(dateSelected(QDate)),SLOT(setDate(QDate)));
  connect(mDatePicker,SIGNAL(dateSelected(QDate)),SIGNAL(dateChanged(QDate)));
  connect(mDatePicker,SIGNAL(dateSelected(QDate)),mDateFrame,SLOT(hide()));
  
  // Create the keyword list. This will be used to match against when the user
  // enters information.
  mKeywordMap[i18n("tomorrow")] = 1;
  mKeywordMap[i18n("today")] = 0;
  mKeywordMap[i18n("yesterday")] = -1;
  
  /*
   * This loop uses some math tricks to figure out the offset in days
   * to the next date the given day of the week occurs. There
   * are two cases, that the new day is >= the current day, which means
   * the new day has not occured yet or that the new day < the current day,
   * which means the new day is already passed (so we need to find the
   * day in the next week).
   */
  QString dayName;
  int currentDay = QDate::currentDate().dayOfWeek();
  for (int i = 1; i <= 7; ++i)
  {
    dayName = KGlobal::locale()->weekDayName(i).lower();
    if (i >= currentDay)
      mKeywordMap[dayName] = i - currentDay;
    else
      mKeywordMap[dayName] = 7 - currentDay + i;
  }
  
  mTextChanged = false;
}

KDateEdit::~KDateEdit()
{
  delete mDateFrame;
}

void KDateEdit::setDate(QDate newDate)
{
  if (!newDate.isValid())
    return;

  mTextChanged = false;
  
  // We do not want to generate a signal here, since we explicity setting
  // the date
  bool b = mDateEdit->signalsBlocked();
  mDateEdit->blockSignals(true);
  mDateEdit->setText(KGlobal::locale()->formatDate( newDate, true ));
  mDateEdit->blockSignals(b);
}

void KDateEdit::setEnabled(bool on)
{
  mDateEdit->setEnabled(on);
  mDateButton->setEnabled(on);
}

QDate KDateEdit::date() const
{
  QDate date = readDate();

  if (date.isValid()) {
    return date;
  } else {
    KNotifyClient::beep();
    return QDate::currentDate();
  }
}

void KDateEdit::toggleDatePicker()
{
  if( mDateFrame->isVisible() ) {
    mDateFrame->hide();
  } else {
    QPoint tmpPoint = mapToGlobal(mDateButton->geometry().bottomRight());

    if ( tmpPoint.x() < 207 ) tmpPoint.setX( 207 );

    int h = QApplication::desktop()->height();

    if ( tmpPoint.y() + 200 > h ) tmpPoint.setY( h - 200 );
    	
    mDateFrame->setGeometry(tmpPoint.x()-207, tmpPoint.y(), 200, 200);

    QDate date = readDate();
    if(date.isValid()) {
      mDatePicker->setDate(date);
    } else {
      mDatePicker->setDate(QDate::currentDate());
    }
    mDateFrame->show();
  }
}


void KDateEdit::lineEnterPressed()
{
  QDate date = readDate();

  if(date.isValid()) 
  {
    // Update the edit. This is needed if the user has entered a 
    // word rather than the actual date.
    setDate(date);
    
    emit(dateChanged(date));
  }
  else
  {
    KNotifyClient::beep();
  }
}

bool KDateEdit::inputIsValid()
{
  return readDate().isValid();
}

QDate KDateEdit::readDate() const
{
  QString text = mDateEdit->text();
  QDate date;
  
  if (mKeywordMap.contains(text.lower()))
  {
    date = QDate::currentDate().addDays(mKeywordMap[text.lower()]);
  }
  else
  {
    date = KGlobal::locale()->readDate(text);
  }
  
  return date;
}

bool KDateEdit::eventFilter(QObject *, QEvent *e)
{
  // We only process the focus out event if the text has changed
  // since we got focus
  if ((e->type() == QEvent::FocusOut) && mTextChanged)
  {
    lineEnterPressed();
    mTextChanged = false;
  }
    
  return false;
}

void KDateEdit::textChanged(const QString &)
{
  mTextChanged = true;
}
