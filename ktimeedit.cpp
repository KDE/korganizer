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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qkeycode.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qlineedit.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>

#include "ktimeedit.h"
#include <qvalidator.h>
#include "ktimeedit.moc"

// Validator for a time value with only hours and minutes (no seconds)
// Mostly locale aware. Author: David Faure <faure@kde.org>
class KOTimeValidator : public QValidator
{
public:
    KOTimeValidator(QWidget* parent, const char* name=0) : QValidator(parent, name) {}

    virtual State validate(QString& str, int& /*cursorPos*/) const
    {
        bool ok = false;
        // TODO use KLocale::WithoutSeconds in HEAD
        /*QTime time =*/ KGlobal::locale()->readTime(str, &ok);
        if ( ok )
            return Acceptable;
        // readTime doesn't help knowing when the string is "Intermediate".
        int length = str.length();
        if ( !str ) // empty string?
            return Invalid; // there should always be a ':' in it, right?
        // HACK. Not fully locale aware etc. (esp. the separator is '.' in sv_SE...)
        QChar sep = ':';
        // I want to allow "HH:", ":MM" and ":" to make editing easier
        if ( str[0] == sep )
        {
            if ( length == 1 ) // just ":"
                return Intermediate;
            QString minutes = str.mid(1);
            int m = minutes.toInt(&ok);
            if ( ok && m >= 0 && m < 60 )
                return Intermediate;
        } else if ( str[str.length()-1] == sep )
        {
            QString hours = str.left(length-1);
            int h = hours.toInt(&ok);
            if ( ok && h >= 0 && h < 24 )
                return Intermediate;
        }
        return Invalid;
    }
};

// KTimeWidget/QTimeEdit provide nicer editing, but don't provide a combobox.
// Difficult to get all in one...
// But Qt-3.2 will offer QLineEdit::setMask, so a "99:99" mask would help.
KOTimeEdit::KOTimeEdit(QWidget *parent, QTime qt, const char *name)
  : QComboBox(TRUE, parent, name)
{
  setInsertionPolicy(NoInsertion);
  setValidator( new KOTimeValidator( this ) );

  mTime = qt;

//  mNoTimeString = i18n("No Time");
//  insertItem( mNoTimeString );

  // Fill combo box with selection of times in localized format.
  QTime timeEntry(0,0,0);
  do {
    insertItem(KGlobal::locale()->formatTime(timeEntry));
    timeEntry = timeEntry.addSecs(60*15);
  } while (!timeEntry.isNull());
  // Add end of day.
  insertItem( KGlobal::locale()->formatTime( QTime( 23, 59, 59 ) ) );

  updateText();
  setFocusPolicy(QWidget::StrongFocus);

  connect(this, SIGNAL(activated(int)), this, SLOT(activ(int)));
  connect(this, SIGNAL(highlighted(int)), this, SLOT(hilit(int)));
  connect(this,SIGNAL(textChanged(const QString&)),this,SLOT(changedText()));
}

KOTimeEdit::~KOTimeEdit()
{
}

bool KOTimeEdit::hasTime() const
{
  // Can't happen
  if ( currentText().isEmpty() ) return false;
  //if ( currentText() == mNoTimeString ) return false;

  return true; // always
}

QTime KOTimeEdit::getTime() const
{
  //kdDebug(5850) << "KOTimeEdit::getTime(), currentText() = " << currentText() << endl;
  // TODO use KLocale::WithoutSeconds in HEAD
  QTime time = KGlobal::locale()->readTime(currentText());
  kdDebug(5850) << "KOTimeEdit::getTime(): " << time.toString() << endl;
  return time;
}

QSizePolicy  KOTimeEdit::sizePolicy() const
{
  // Set size policy to Fixed, because edit cannot contain more text than the
  // string representing the time. It doesn't make sense to provide more space.
  QSizePolicy sizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

  return sizePolicy;
}

void KOTimeEdit::setTime(QTime newTime)
{
  if ( mTime != newTime )
  {
    kdDebug(5850) << "KOTimeEdit::setTime(): " << newTime.toString() << endl;

    mTime = newTime;
    updateText();
  }
}

void KOTimeEdit::activ(int i)
{
    // The last entry, 23:59, is a special case
    if( i == count() - 1 )
        mTime = QTime( 23, 59, 0 );
    else
        mTime = QTime(0,0,0).addSecs(i*15*60);
    emit timeChanged(mTime);
}

void KOTimeEdit::hilit(int )
{
  // we don't currently need to do anything here.
}

void KOTimeEdit::addTime(QTime qt)
{
  // Calculate the new time.
  mTime = qt.addSecs(mTime.minute()*60+mTime.hour()*3600);
  updateText();
  emit timeChanged(mTime);
}

void KOTimeEdit::subTime(QTime qt)
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
  updateText();
  emit timeChanged(mTime);
}

void KOTimeEdit::keyPressEvent(QKeyEvent *qke)
{
  switch(qke->key()) {
  case Key_Down:
    addTime(QTime(0,1,0));
    break;
  case Key_Up:
    subTime(QTime(0,1,0));
    break;
  case Key_Prior:
    subTime(QTime(1,0,0));
    break;
  case Key_Next:
    addTime(QTime(1,0,0));
    break;
  default:
    QComboBox::keyPressEvent(qke);
    break;
  } // switch
}

void KOTimeEdit::updateText()
{
//  kdDebug(5850) << "KOTimeEdit::updateText() " << endl;
  QString s = KGlobal::locale()->formatTime(mTime);
  // Set the text but without emitting signals, nor losing the cursor position
  QLineEdit *line = lineEdit();
  line->blockSignals(true);
  int pos = line->cursorPosition();
  line->setText(s);
  line->setCursorPosition(pos);
  line->blockSignals(false);

//  kdDebug(5850) << "KOTimeEdit::updateText(): " << s << endl;

  if (!mTime.minute() % 15) {
    setCurrentItem((mTime.hour()*4)+(mTime.minute()/15));
  }
}

bool KOTimeEdit::inputIsValid() const
{
  int cursorPos = lineEdit()->cursorPosition();
  QString str = currentText();
  return validator()->validate( str, cursorPos ) == QValidator::Acceptable;
}

void KOTimeEdit::changedText()
{
  //kdDebug(5850) << "KOTimeEdit::changedText()" << endl;
  if ( inputIsValid() )
  {
    mTime = getTime();
    emit timeChanged(mTime);
  }
}
