/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include <qcheckbox.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktextedit.h>
#include <ktimeedit.h>
#include <klineedit.h>

#include <libkcal/journal.h>
#include <libkcal/calendar.h>

#include "kodialogmanager.h"
#include "incidencechanger.h"
#include "koglobals.h"

#include "journalentry.h"
#include "journalentry.moc"

JournalEntry::JournalEntry( Calendar *calendar, QWidget *parent ) :
  QWidget( parent )
{
//kdDebug(5850)<<"JournalEntry::JournalEntry, parent="<<parent<<endl;
  mCalendar = calendar;
  mJournal = 0;
  mDirty = false;
  mChanger = 0;

  mLayout = new QGridLayout( this );
  
  mTitle = new QLabel(i18n("Title"),this);
  QFont f = mTitle->font();
  f.setBold( true );
  f.setItalic( true );
  mTitle->setFont( f );
  mTitle->setMargin(2);
  mTitle->setAlignment(AlignCenter);
  mTitle->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  mLayout->addMultiCellWidget( mTitle, 0, 0, 0, 3 );
  
  QString whatsThis = i18n("Sets the Title of this journal.");
  
  mTitleLabel = new QLabel( i18n("&Title: "), this );
  mLayout->addWidget( mTitleLabel, 1, 0 );
  mTitleEdit = new KLineEdit( this );
  mLayout->addWidget( mTitleEdit, 1, 1 );
  mTitleLabel->setBuddy( mTitleEdit );
  
  QWhatsThis::add( mTitleLabel, whatsThis );
  QWhatsThis::add( mTitleEdit, whatsThis );
  
  mTimeCheck = new QCheckBox( i18n("Ti&me: "), this );
  mLayout->addWidget( mTimeCheck, 1, 2 );
  mTimeEdit = new KTimeEdit( this );
  mLayout->addWidget( mTimeEdit, 1, 3 );
  connect( mTimeCheck, SIGNAL(toggled(bool)),
           mTimeEdit, SLOT(setEnabled(bool)) );
  QWhatsThis::add( mTimeCheck, i18n("Determines whether this journal has also "
                                    "a time associated") );
  QWhatsThis::add( mTimeEdit, i18n( "Sets the time associated with this journal" ) );
  
    
  mEditor = new KTextEdit(this);
  mLayout->addMultiCellWidget( mEditor, 2, 3, 0, 3 );
  
  connect( mTitleEdit, SIGNAL(textChanged( const QString& )), SLOT(setDirty()) );
  connect( mTimeCheck, SIGNAL(toggled(bool)), SLOT(setDirty()) );
  connect( mTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(setDirty()) );
  connect( mEditor, SIGNAL(textChanged()), SLOT(setDirty()) );
  
  mEditor->installEventFilter(this);
}

JournalEntry::~JournalEntry()
{
  writeJournal();
}

void JournalEntry::setDate(const QDate &date)
{
  writeJournal();
  mTitle->setText(KGlobal::locale()->formatDate(date));
  mDate = date;
}

void JournalEntry::clearFields()
{
  mTitleEdit->clear();
  mTimeCheck->setChecked( false );
  mTimeEdit->setEnabled( false );
  mEditor->clear();
}

void JournalEntry::setJournal(Journal *journal)
{
  writeJournal();

  mJournal = journal;
  if ( mJournal ) {
    readJournal( journal );
  } else {
    clearFields();
  }
    
  mDirty = false;
}

void JournalEntry::setDirty()
{
  mDirty = true;
  kdDebug(5850) << "JournalEntry::setDirty()" << endl;
}

void JournalEntry::clear()
{
  mJournal = 0;
  clearFields();
  writeJournal();
}

bool JournalEntry::eventFilter( QObject *o, QEvent *e )
{
//  kdDebug(5850) << "JournalEntry::event received " << e->type() << endl;

  if ( e->type() == QEvent::FocusOut || e->type() == QEvent::Hide || 
       e->type() == QEvent::Close ) {
    writeJournal();
  } 
  return QWidget::eventFilter( o, e );    // standard event processing
}


void JournalEntry::readJournal( Journal *j )
{
  mJournal = j;
  mTitleEdit->setText( mJournal->summary() );
  bool floats = mJournal->doesFloat();
  mTimeCheck->setChecked( floats );
  mTimeEdit->setEnabled( floats );
  if (!floats) 
    mTimeEdit->setTime( mJournal->dtStart().time() );
  mEditor->setText( mJournal->description() );
}

void JournalEntry::writeJournalPrivate( Journal *j ) 
{
  j->setSummary( mTitleEdit->text() );
  bool floating = !mTimeCheck->isChecked();
  QTime tm( mTimeEdit->getTime() );
  j->setDtStart( QDateTime( mDate, floating?QTime(0,0,0):tm ) );
  j->setFloats( floating );
  j->setDescription( mEditor->text() );
}

void JournalEntry::writeJournal()
{
//  kdDebug(5850) << "JournalEntry::writeJournal()" << endl;

  if ( !mDirty || !mChanger ) {
    kdDebug(5850)<<"Journal either unchanged or no changer object available"<<endl;
    return;
  }
  bool newJournal = false;
 
  if ( mEditor->text().isEmpty() && mTitleEdit->text().isEmpty() ) {
    if ( mJournal && mChanger ) { // delete the journal
      mChanger->deleteIncidence( mJournal );
    } 
    mJournal = 0;
    return;
  }

//  kdDebug(5850) << "JournalEntry::writeJournal()..." << endl;
  Journal *oldJournal = 0;
  
  if ( !mJournal ) {
    newJournal = true;
    mJournal = new Journal;
    writeJournalPrivate( mJournal );
    if ( !mChanger->addIncidence( mJournal ) ) {
      KODialogManager::errorSaveIncidence( this, mJournal );
      delete mJournal;
      mJournal = 0;
    }
  } else {
    oldJournal = mJournal->clone();
    if ( mChanger->beginChange( mJournal ) ) {
      writeJournalPrivate( mJournal );
      mChanger->changeIncidence( oldJournal, mJournal, KOGlobals::DESCRIPTION_MODIFIED );
      mChanger->endChange( mJournal );
    }
    delete oldJournal;
  } 
  mDirty = false;
}

void JournalEntry::flushEntry()
{
  if (!mDirty) return;
  
  writeJournal();
}
