/*
    This file is part of KOrganizer.
    Copyright (c) 1999 Preston Brown, Ian Dawes

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

#include <qkeycode.h>
#include <qcombobox.h>
#include <qdatetime.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>

#include "ktimeedit.h"
#include "ktimeedit.moc"

KTimeEdit::KTimeEdit(QWidget *parent, QTime qt, const char *name)
  : QComboBox(TRUE, parent, name)
{
  setInsertionPolicy(NoInsertion);

  mTime = qt;

  mNoTimeString = i18n("No Time");
//  insertItem( mNoTimeString );

  // Fill combo box with selection of times in localized format.
  QTime timeEntry(0,0,0);
  do {
    insertItem(KGlobal::locale()->formatTime(timeEntry));
    timeEntry = timeEntry.addSecs(60*15);
  } while (!timeEntry.isNull());

  updateSelection();
  setFocusPolicy(QWidget::StrongFocus);

  connect(this, SIGNAL(activated(int)), this, SLOT(activ(int)));
  connect(this, SIGNAL(highlighted(int)), this, SLOT(hilit(int)));
  connect(this,SIGNAL(textChanged(const QString&)),this,SLOT(changedText()));
}

KTimeEdit::~KTimeEdit()
{
}

bool KTimeEdit::hasTime()
{
  if ( currentText().isEmpty() ) return false;
  if ( currentText() == mNoTimeString ) return false;

  return true;
}

QTime KTimeEdit::getTime()
{
//  kdDebug() << "KTimeEdit::getTime()" << endl;
  QTime time = KGlobal::locale()->readTime(currentText());
//  kdDebug() << "KTimeEdit::getTime(): " << time.toString() << endl;
  return time;
}

QSizePolicy  KTimeEdit::sizePolicy() const
{
  // Set size policy to Fixed, because edit cannot contain more text than the
  // string representing the time. It doesn't make sense to provide more space.
  QSizePolicy sizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

  return sizePolicy;
}

void KTimeEdit::setTime(QTime newTime)
{
//  kdDebug() << "KTimeEdit::setTime(): " << newTime.toString() << endl;

  mTime = newTime;
  updateSelection();
}

void KTimeEdit::activ(int i)
{
  mTime = QTime(0,0,0).addSecs(i*15*60);
  //emit timeChanged(mTime);
}

void KTimeEdit::hilit(int )
{
  // we don't currently need to do anything here.
}

void KTimeEdit::addTime(QTime qt)
{
  // Calculate the new time.
  mTime = qt.addSecs(mTime.minute()*60+mTime.hour()*3600);
  //emit timeChanged(mTime);
  updateSelection();
}

void KTimeEdit::subTime(QTime qt)
{
  int h, m;

  // Note that we cannot use the same method for determining the new
  // time as we did in addTime, because QTime does not handle adding
  // negative seconds well at all.
  h = mTime.hour()-qt.hour();
  m = mTime.minute()-qt.minute();

  if(m < 0) {
    m += 60;
    h -= 1;
  }

  if(h < 0) {
    h += 24;
  }

  // store the newly calculated time.
  mTime.setHMS(h, m, 0);
  //emit timeChanged(mTime);
  updateSelection();
}

void KTimeEdit::keyPressEvent(QKeyEvent *qke)
{
  switch(qke->key()) {
  case Key_Enter:
  case Key_Return:
    mTime = getTime();
    emit timeChanged(mTime);
//    validateEntry();
    break;
  case Key_Down:
    addTime(QTime(0,15,0));
    break;
  case Key_Up:
    subTime(QTime(0,15,0));
    break;
  default:
    QComboBox::keyPressEvent(qke);
    break;
  } // switch
}

void KTimeEdit::validateEntry()
{
// Disabled because it does not make anything useful. Should probably try to fix
// up invalid time input.
/*
  QTime t = KGlobal::locale()->readTime(currentText());

  if (!t.isValid()) {
//    KMessageBox::sorry(this,"You must specify a valid time");
    current_display_valid = false;
  } else {
    mTime = t;
    current_display_valid = true;
  }
*/
}

void KTimeEdit::updateSelection()
{
  QString s = KGlobal::locale()->formatTime(mTime);
  setEditText(s);

  if (!mTime.minute() % 15) {
    setCurrentItem((mTime.hour()*4)+(mTime.minute()/15));
  }
}

bool KTimeEdit::inputIsValid()
{
//  if ( !hasTime() ) return true;

  QTime t = KGlobal::locale()->readTime(currentText());

  return t.isValid();
}

void KTimeEdit::changedText()
{
//  kdDebug() << "KTimeEdit::changedText()" << endl;
  mTime = getTime();
  emit timeChanged(mTime);
}
