/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
		large parts of code taken from KOEditorGeneralJournal.cpp:
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

#include "koeditorgeneraljournal.h"
#include "koeditorgeneral.h"

#include <libkcal/journal.h>

#include <ktextedit.h>
#include <kdateedit.h>
#include <ktimeedit.h>
//#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qgroupbox.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>


KOEditorGeneralJournal::KOEditorGeneralJournal( QObject *parent,
                                                const char *name )
  : QObject( parent, name )
{
}

KOEditorGeneralJournal::~KOEditorGeneralJournal()
{
}

void KOEditorGeneralJournal::initTitle( QWidget *parent, QBoxLayout *topLayout )
{
  QHBoxLayout *hbox = new QHBoxLayout( topLayout );
  
  QString whatsThis = i18n("Sets the Title of this journal.");
  QLabel *summaryLabel = new QLabel( i18n("T&itle:"), parent );
  QWhatsThis::add( summaryLabel, whatsThis );
  QFont f = summaryLabel->font();
  f.setBold( true );
  summaryLabel->setFont( f );
  hbox->addWidget( summaryLabel );

  mSummaryEdit = new FocusLineEdit( parent );
  QWhatsThis::add( mSummaryEdit, whatsThis );
  connect( mSummaryEdit, SIGNAL( focusReceivedSignal() ),
           SIGNAL( focusReceivedSignal() ) );
  summaryLabel->setBuddy( mSummaryEdit );
  hbox->addWidget( mSummaryEdit );
}


void KOEditorGeneralJournal::initDate( QWidget *parent, QBoxLayout *topLayout )
{
//  QBoxLayout *dateLayout = new QVBoxLayout(topLayout);
  QBoxLayout *dateLayout = new QHBoxLayout( topLayout );
  
  mDateLabel = new QLabel( i18n("&Date:"), parent);
  dateLayout->addWidget( mDateLabel );

  mDateEdit = new KDateEdit( parent );
  dateLayout->addWidget( mDateEdit );
  mDateLabel->setBuddy( mDateEdit );
  
  dateLayout->addStretch();
  
  mTimeCheckBox = new QCheckBox( "&Time: ", parent );
  dateLayout->addWidget( mTimeCheckBox );
  
  mTimeEdit = new KTimeEdit( parent );
  dateLayout->addWidget( mTimeEdit );
  connect( mTimeCheckBox, SIGNAL(toggled(bool)),
           mTimeEdit, SLOT(setEnabled(bool)) );

  dateLayout->addStretch();
}

void KOEditorGeneralJournal::setDate( const QDate &date )
{
//  kdDebug(5850) << "KOEditorGeneralJournal::setDate(): Date: " << date.toString() << endl;

  mDateEdit->setDate( date );
}

void KOEditorGeneralJournal::setTime( const QTime &time )
{
kdDebug()<<"KOEditorGeneralJournal::setTime, time="<<time.toString()<<endl;
  mTimeCheckBox->setChecked( time.isValid() );
  if ( time.isValid() ) {
kdDebug()<<"KOEditorGeneralJournal::setTime, time is valid"<<endl;
    mTimeEdit->setTime( time );
  }
}

void KOEditorGeneralJournal::initDescription( QWidget *parent, QBoxLayout *topLayout )
{
  mDescriptionEdit = new KTextEdit( parent );
  mDescriptionEdit->append("");
  mDescriptionEdit->setReadOnly( false );
  mDescriptionEdit->setOverwriteMode( false );
  mDescriptionEdit->setWordWrap( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );
  topLayout->addWidget( mDescriptionEdit );
}

void KOEditorGeneralJournal::setDefaults( const QDate &date )
{
  setDate( date );
}

void KOEditorGeneralJournal::readJournal( Journal *journal, bool tmpl )
{
  setSummary( journal->summary() );
  if ( !tmpl ) {
    setDate( journal->dtStart().date() );
    if ( !journal->doesFloat() ) {
kdDebug()<<"KOEditorGeneralJournal::readJournal, does not float, time="<<(journal->dtStart().time().toString())<<endl;
      setTime( journal->dtStart().time() );
    } else { 
kdDebug()<<"KOEditorGeneralJournal::readJournal, does float"<<endl;
      setTime( QTime( -1, -1, -1 ) );
    } 
  }
  setDescription( journal->description() );
}

void KOEditorGeneralJournal::writeJournal( Journal *journal )
{
//  kdDebug(5850) << "KOEditorGeneralJournal::writeIncidence()" << endl;
  journal->setSummary( mSummaryEdit->text() );
  journal->setDescription( mDescriptionEdit->text() );
  
  QDateTime tmpDT( mDateEdit->date(), QTime(0,0,0) );
  bool floats= mTimeCheckBox->isChecked();
  journal->setFloats( floats );
  if ( !floats ) {
    tmpDT.setTime( mTimeEdit->getTime() );
  }
  journal->setDtStart(tmpDT);

//  kdDebug(5850) << "KOEditorGeneralJournal::writeJournal() done" << endl;
}


void KOEditorGeneralJournal::setDescription( const QString &text )
{
  mDescriptionEdit->setText( text );
}

void KOEditorGeneralJournal::setSummary( const QString &text )
{
  mSummaryEdit->setText( text );
}

void KOEditorGeneralJournal::finishSetup()
{
  QWidget::setTabOrder( mSummaryEdit, mDateEdit );
  QWidget::setTabOrder( mDateEdit, mTimeCheckBox );
  QWidget::setTabOrder( mTimeCheckBox, mTimeEdit );
  QWidget::setTabOrder( mTimeEdit, mDescriptionEdit );
  mSummaryEdit->setFocus();
}

bool KOEditorGeneralJournal::validateInput()
{
//  kdDebug(5850) << "KOEditorGeneralJournal::validateInput()" << endl;

  if (!mDateEdit->inputIsValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid date, for example '%1'.")
        .arg( KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  return true;
}

#include "koeditorgeneraljournal.moc"
