/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

//
// Journal Entry

#include <qlabel.h>
#include <qlayout.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktextedit.h>

#include <libkcal/journal.h>
#include <libkcal/calendar.h>

#include "kodialogmanager.h"

#include "journalentry.h"
#include "journalentry.moc"

JournalEntry::JournalEntry(Calendar *calendar,QWidget *parent) :
  QVBox(parent)
{
//kdDebug(5850)<<"JournalEntry::JournalEntry, parent="<<parent<<endl;
  mCalendar = calendar;
  mJournal = 0;
  mDirty = false;

  mTitleLabel = new QLabel(i18n("Title"),this);
  mTitleLabel->setMargin(2);
  mTitleLabel->setAlignment(AlignCenter);
  
  mEditor = new KTextEdit(this);
  connect(mEditor,SIGNAL(textChanged()),SLOT(setDirty()));
  
  mEditor->installEventFilter(this);
}

JournalEntry::~JournalEntry()
{
  writeJournal();
}

void JournalEntry::setDate(const QDate &date)
{
  writeJournal();
  mTitleLabel->setText(KGlobal::locale()->formatDate(date));
  mDate = date;
}

void JournalEntry::setJournal(Journal *journal)
{
  writeJournal();

  mJournal = journal;
  if (mJournal) 
    mEditor->setText(mJournal->description());
  else mEditor->clear();
  mDirty = false;
}

void JournalEntry::setDirty()
{
  mDirty = true;
//  kdDebug(5850) << "JournalEntry::setDirty()" << endl;
}

void JournalEntry::clear()
{
  mJournal = 0;
  mEditor->setText("");
  writeJournal();
}

bool JournalEntry::eventFilter( QObject *o, QEvent *e )
{
//  kdDebug(5850) << "JournalEntry::event received " << e->type() << endl;

  if ( e->type() == QEvent::FocusOut || e->type() == QEvent::Hide || 
       e->type() == QEvent::Close ) {
    writeJournal();
  } 
  return QFrame::eventFilter( o, e );    // standard event processing
}

void JournalEntry::writeJournal()
{
//  kdDebug(5850) << "JournalEntry::writeJournal()" << endl;

  if (!mDirty) return;
  bool newJournal = false;
 
  if (mEditor->text().isEmpty() ) {
    if (mJournal ) { // delete the journal
      emit incidenceToBeDeleted( mJournal );
      mCalendar->deleteJournal( mJournal );
      emit incidenceDeleted( mJournal );
    } 
    mJournal = 0;
    return;
  }

//  kdDebug(5850) << "JournalEntry::writeJournal()..." << endl;
  
  if (!mJournal) {
    newJournal = true;
    mJournal = new Journal;
    mJournal->setDtStart(QDateTime(mDate,QTime(0,0,0)));
    if ( !mCalendar->addJournal( mJournal ) ) {
      KODialogManager::errorSaveIncidence( this, mJournal );
      delete mJournal;
      mJournal = 0;
      return;
    }
  }

  Journal* oldJournal = mJournal->clone();
  mJournal->setDescription(mEditor->text());
  if (newJournal) {
    emit incidenceAdded( mJournal );
  } else {
    emit incidenceChanged( oldJournal, mJournal );
    delete oldJournal;
  }

  mDirty = false;
}

void JournalEntry::flushEntry()
{
  if (!mDirty) return;
  
  writeJournal();
}
