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

  connect(mDateButton,SIGNAL(clicked()),SLOT(toggleDatePicker()));

  connect(mDatePicker,SIGNAL(dateSelected(QDate)),SLOT(setDate(QDate)));
  connect(mDatePicker,SIGNAL(dateSelected(QDate)),SIGNAL(dateChanged(QDate)));
  connect(mDatePicker,SIGNAL(dateSelected(QDate)),mDateFrame,SLOT(hide()));
}

KDateEdit::~KDateEdit()
{
  delete mDateFrame;
}

void KDateEdit::setDate(QDate newDate)
{
  if (!newDate.isValid())
    return;

  mDateEdit->setText(KGlobal::locale()->formatDate(newDate,true));
}

void KDateEdit::setEnabled(bool on)
{
  mDateEdit->setEnabled(on);
  mDateButton->setEnabled(on);
}

QDate KDateEdit::getDate() const
{
  QDate date = KGlobal::locale()->readDate(mDateEdit->text());

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

    QDate date = KGlobal::locale()->readDate(mDateEdit->text());
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
  QDate date = KGlobal::locale()->readDate(mDateEdit->text());

  if(date.isValid()) {
    emit(dateChanged(date));
  } else {
    KNotifyClient::beep();
  }
}

bool KDateEdit::inputIsValid()
{
  QDate date = KGlobal::locale()->readDate(mDateEdit->text());
  return date.isValid();
}
