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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>
#include <kresources/resourceselectdialog.h>

#include "journalentry.h"
#include "journalentry.moc"

JournalEntry::JournalEntry(Calendar *calendar,QWidget *parent) :
  QFrame(parent)
{
  mCalendar = calendar;
  mJournal = 0;
  mDirty = false;

  mTitleLabel = new QLabel(i18n("Title"),this);
  mTitleLabel->setMargin(2);
  mTitleLabel->setAlignment(AlignCenter);
  
  mEditor = new KTextEdit(this);
  connect(mEditor,SIGNAL(textChanged()),SLOT(setDirty()));
  
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addWidget(mTitleLabel);
  topLayout->addWidget(mEditor);
  
  mEditor->installEventFilter(this);
}

JournalEntry::~JournalEntry()
{
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
  
  mEditor->setText(mJournal->description());

  mDirty = false;
}

Journal *JournalEntry::journal() const
{
  return mJournal;
}

void JournalEntry::setDirty()
{
  mDirty = true;
//  kdDebug() << "JournalEntry::setDirty()" << endl;
}

void JournalEntry::clear()
{
  mJournal = 0;
  mEditor->setText("");
}

bool JournalEntry::eventFilter( QObject *o, QEvent *e )
{
//  kdDebug() << "JournalEntry::event received " << e->type() << endl;

  if ( e->type() == QEvent::FocusOut ) {
    writeJournal();
  }
  return QFrame::eventFilter( o, e );    // standard event processing
}

void JournalEntry::writeJournal()
{
//  kdDebug() << "JournalEntry::writeJournal()" << endl;

  if (!mDirty) return;
 
  if (mEditor->text().isEmpty()) return;

//  kdDebug() << "JournalEntry::writeJournal()..." << endl;
  
  if (!mJournal) {
    mJournal = new Journal;
    mJournal->setDtStart(QDateTime(mDate,QTime(0,0,0)));
    mCalendar->addJournal(mJournal);
  }

  mJournal->setDescription(mEditor->text());

  mDirty = false;
}

void JournalEntry::flushEntry()
{
  if (!mDirty) return;
  
  writeJournal();
}
